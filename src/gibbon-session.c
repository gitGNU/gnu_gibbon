/*
 * This file is part of Gibbon, a graphical frontend to the First Internet 
 * Backgammon Server FIBS.
 * Copyright (C) 2009-2011 Guido Flohr, http://guido-flohr.net/.
 *
 * Gibbon is free software: you can redistribute it and/or modify 
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Gibbon is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Gibbon.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <glib/gi18n.h>
#include <gtk/gtk.h>

#include <unistd.h>
#include <sys/types.h>
#include <errno.h>
#include <stdlib.h>

#include "gibbon.h"
#include "gibbon-connection.h"
#include "gibbon-session.h"
#include "gui.h"
#include "game.h"
#include "gibbon-prefs.h"

#define CLIP_WELCOME 1
#define CLIP_WHO_INFO 5
#define CLIP_WHO_INFO_END 6

enum gibbon_connection_signals {
        HTML_SERVER_OUTPUT,
        LAST_SIGNAL
};
static guint signals[LAST_SIGNAL] = { 0 };

static void gibbon_session_send_server_message (GibbonSession *self, 
                                                const gchar *output);
static gboolean gibbon_session_clip_welcome (GibbonSession *self,
                                             const gchar *message,
                                             const gchar *ptr);                                                
static gboolean gibbon_session_clip_who_info (GibbonSession *self,
                                              const gchar *message,
                                              const gchar *ptr);                                                
static void gibbon_session_dispatch_clip_message (GibbonSession *self,
                                                  const gchar *message);
static gboolean gibbon_session_handle_board (GibbonSession *self,
                                             const gchar *board);

static gboolean free_vector (gchar **);
static gboolean parse_integer (const gchar *str, gint* result,
                               const gchar *what);
static gboolean parse_float (const gchar *str, gdouble* result,
                             const gchar *what);

struct _GibbonSessionPrivate {
        GibbonConnection *connection;

        gchar *watching;
        gchar *opponent;
};

#define GIBBON_SESSION_PRIVATE(o) (G_TYPE_INSTANCE_GET_PRIVATE ((o), \
                                      GIBBON_TYPE_SESSION,           \
                                      GibbonSessionPrivate))
G_DEFINE_TYPE (GibbonSession, gibbon_session, G_TYPE_OBJECT);

#define GIBBON_SESSION_WHITESPACE " \t\r\v\f"

static void
gibbon_session_init (GibbonSession *self)
{
        self->priv = G_TYPE_INSTANCE_GET_PRIVATE (self, 
                                                  GIBBON_TYPE_SESSION, 
                                                  GibbonSessionPrivate);

        self->priv->connection = NULL;
        self->priv->watching = NULL;
        self->priv->opponent = NULL;
}

static void
gibbon_session_finalize (GObject *object)
{
        GibbonSession *self = GIBBON_SESSION (object);

        G_OBJECT_CLASS (gibbon_session_parent_class)->finalize (object);

        if (self->priv->connection)
                g_object_unref (self->priv->connection);
        self->priv->connection = NULL;

        if (self->priv->watching)
                g_free (self->priv->watching);
        self->priv->watching = NULL;

        if (self->priv->opponent)
                g_free (self->priv->opponent);
        self->priv->opponent = NULL;
}

static void
gibbon_session_class_init (GibbonSessionClass *klass)
{
        GObjectClass* object_class = G_OBJECT_CLASS (klass);

        g_type_class_add_private (klass, sizeof (GibbonSessionPrivate));

        object_class->finalize = gibbon_session_finalize;

        signals[HTML_SERVER_OUTPUT] =
                g_signal_new ("html-server-output",
                              G_TYPE_FROM_CLASS (klass),
                              G_SIGNAL_RUN_FIRST,
                              0,
                              NULL, NULL,
                              g_cclosure_marshal_VOID__STRING,
                              G_TYPE_NONE,
                              1,
                              G_TYPE_STRING);
}

GibbonSession *
gibbon_session_new (GibbonConnection *connection)
{
        GibbonSession *self = g_object_new (GIBBON_TYPE_SESSION, NULL);

        self->priv->connection = connection;
        g_object_ref (connection);

        return self;
}

static void
gibbon_session_dispatch_clip_message (GibbonSession *self,
                                      const gchar *message)
{
        unsigned long int code;
        gchar *endptr;
        gboolean r = FALSE;
                
        g_return_if_fail (GIBBON_IS_SESSION (self));
        
        code = strtoul (message, &endptr, 10);
        
        /* Skip whitespace.  */
        while (*endptr == ' ' || *endptr == '\t' || *endptr == '\r'
               || *endptr == '\f' || *endptr == '\v')
                endptr++;
        
        switch (code) {
                case CLIP_WELCOME:
                        r = gibbon_session_clip_welcome (self, message, endptr);
                        break;
                case CLIP_WHO_INFO:
                        r = gibbon_session_clip_who_info (self, message, endptr);
                        break;
                case CLIP_WHO_INFO_END: /* Ignored.  */
                        r = TRUE;
                        break;
                default: 
                        break;
        }
        if (!r)
                gibbon_session_send_server_message (self, message);
}

static void
gibbon_session_send_server_message (GibbonSession *self,
                                    const gchar *output)
{
        /* FIXME! HTML escape the message first and embed it
         * in <em>emphasized</em>.
         */
        g_signal_emit (self, signals[HTML_SERVER_OUTPUT], 0, output);
}

G_MODULE_EXPORT void
gibbon_session_server_output_cb (GibbonSession *self, 
                                 const gchar *output,
                                 GObject *emitter)
{
        g_return_if_fail (GIBBON_IS_SESSION (self));

        if (output[0] >= '0' && output[0] <= '9') {
                gibbon_session_dispatch_clip_message (self, output);
                return;
        } else if (0 == strncmp ("board:", output, 6)) {
                if (gibbon_session_handle_board (self, output + 6))
                        return;
        }
        
        gibbon_session_send_server_message (self, output);
}

static gboolean
gibbon_session_clip_welcome (GibbonSession *self, 
                             const gchar *message, const gchar *ptr)
{
        const gchar* login;
        gchar **tokens;
        GTimeVal last_login;
        gchar *last_login_str;
        gchar *reply;
        gchar *mail;
        
        g_return_val_if_fail (GIBBON_IS_SESSION (self), FALSE);

        login = gibbon_connection_get_login (self->priv->connection);

        tokens = g_strsplit_set (ptr, GIBBON_SESSION_WHITESPACE, 4);
        
        last_login.tv_usec = 0;
        if (tokens[1])
                last_login.tv_sec = strtol (tokens[1], NULL, 10);
        
        if (!tokens[0] || strcmp (tokens[0], login)) {
                g_print ("Parser expected `%s' as login: %s\n",
                         login, message);
                gibbon_session_send_server_message (self, message);
        } else if (last_login.tv_usec < 0) {
                g_print ("Parser expected timestamp after login `%s': %s\n",
                         login, message);
                gibbon_session_send_server_message (self, message);
        } else if (!tokens[2]) {
                g_print ("Parser expected address after timestamp: %s\n",
                         message);
                gibbon_session_send_server_message (self, message);
        } else if (tokens[3]) {
                g_print ("Trailing garbage after address: %s\n", message);
                gibbon_session_send_server_message (self, message);
        } else {
                /* FIXME! Isn't there a better way to format a date and time
                 * in glib?
                 */
                last_login_str = ctime (&last_login.tv_sec);
                last_login_str[strlen (last_login_str) - 1] = 0;
                
                reply = g_strdup_printf (_("Last login on %s from %s."),
                                         last_login_str, tokens[2]);
                gibbon_session_send_server_message (self, reply);
                g_free (reply);
                
                gibbon_connection_queue_command (self->priv->connection,
                                                 "set boardstyle 3");

                mail = gibbon_prefs_get_string (prefs,
                                              GIBBON_PREFS_MAIL_ADDRESS);
                if (mail) {
                        gibbon_connection_queue_command (self->priv->connection,
                                                         "address %s",
                                                         mail);
                        g_free (mail);
                }
        }
        
        g_strfreev (tokens);
        
        return TRUE;
}

static gboolean
gibbon_session_clip_who_info (GibbonSession *self, 
                              const gchar *message, const gchar *ptr)
{
        gchar **tokens;
        gchar *who;
        gchar *opponent;
        gchar *watching;
        gboolean ready;
        gboolean available;
        gboolean away;
        gdouble rating;
        gint experience;
        gchar *idle;
        gchar *login;
        gchar *client;
        gchar *email;
        gchar *hostname;
        gint i;

        g_return_val_if_fail (GIBBON_IS_SESSION (self), FALSE);

        tokens = g_strsplit_set (ptr, GIBBON_SESSION_WHITESPACE, 13);
        g_return_val_if_fail (tokens, FALSE);
        for (i = 0; i <= 11; ++i)
                g_return_val_if_fail (tokens[i], free_vector (tokens));
        
        who = tokens[0];
        
        opponent = tokens[1];
        if (opponent[0] == '-' && opponent[1] == 0)
                opponent = "";
                
        watching = tokens[2];
        if (watching[0] == '-' && watching[1] == 0)
                watching = "";
                
        g_return_val_if_fail (tokens[3][0] == '0' || tokens[3][0] == '1',
                              free_vector (tokens));
        g_return_val_if_fail (tokens[3][1] == 0, free_vector (tokens));
        if (tokens[3][0] == '1') {
                ready = TRUE;
        } else {
                ready = FALSE;
        }
        
        g_return_val_if_fail (tokens[4][0] == '0' || tokens[4][0] == '1',
                              free_vector (tokens));
        g_return_val_if_fail (tokens[4][1] == 0, free_vector (tokens));
        if (tokens[4][0] == '1') {
                away = TRUE;
        } else {
                away = FALSE;
        }

        g_return_val_if_fail (parse_float (tokens[5], &rating,
                                           "rating"),
                              free_vector (tokens));
        g_return_val_if_fail (parse_integer (tokens[6], &experience,
                                             "experience"),
                              free_vector (tokens));
        
        idle = tokens[7];
        login = tokens[8];
        hostname = tokens[9];
        client = tokens[10];
        email = tokens[11];

        available = ready && !away && !opponent[0];
        
        gibbon_player_list_set (players, who, available, rating, experience,
                                opponent, watching);

        if (!g_strcmp0 (who,
                        gibbon_connection_get_login (self->priv->connection))) {
                if (!opponent[0])
                        opponent = NULL;
                if (!watching[0])
                        watching = NULL;
                if (self->priv->opponent) {
                        if (opponent) {
                                if (strcmp (opponent, self->priv->opponent)) {
                                        g_free (self->priv->opponent);
                                        self->priv->opponent = g_strdup (opponent);
                                }
                        } else {
                                self->priv->opponent = g_strdup (opponent);
                        }
                } else if (opponent) {
                        self->priv->opponent = g_strdup (opponent);
                }
                if (self->priv->watching) {
                        if (watching) {
                                if (strcmp (watching, self->priv->watching)) {
                                        g_free (self->priv->watching);
                                        self->priv->watching = g_strdup (watching);
                                }
                        } else {
                                self->priv->watching = g_strdup (watching);
                        }
                } else if (watching) {
                        self->priv->watching = g_strdup (watching);
                }
        }

        g_strfreev (tokens);

        return TRUE;
}

static gboolean
parse_integer (const gchar *str, gint *result, const gchar *what)
{
        char *endptr;
        long int r;
                
        if (!str) {
                g_print ("Error parsing %s: NULL pointer passed.\n",
                         what);
                return FALSE;
        }
        
        errno = 0;

        r = strtol (str, &endptr, 10);
        
        if (errno) {
                g_print ("Error parsing %s: `%s': %s.\n",
                         what, str, strerror (errno));
                return FALSE;
        }
        
        if (*endptr != 0) {
                g_print ("Error parsing %s: `%s': %s.\n",
                         what, str, "Trailing garbage in integer");
                return FALSE;
        }
        
        *result = (gint) r;
        
        return TRUE;       
}

static gboolean
parse_float (const gchar *str, gdouble *result, const gchar *what)
{
        char *endptr;
        long int r;
        gdouble fract = 0.1;
        
        if (!str) {
                g_print ("Error parsing %s: NULL pointer passed.\n",
                         what);
                return FALSE;
        }
        
        errno = 0;

        r = strtol (str, &endptr, 10);
        
        if (errno) {
                g_print ("Error parsing %s: `%s': %s.\n",
                         what, str, strerror (errno));
                return FALSE;
        }
        
        if (*endptr == 0) {
                *result = (gdouble) r;
                return TRUE;
        }
        
        if (*endptr != '.') {
                g_print ("Error parsing %s: `%s': %s.\n",
                         what, str, "Expected decimal point");
                return FALSE;
        }

        *result = (gdouble) r;

        while (TRUE) {
                ++endptr;
                if (!*endptr)
                        break;
                if (*endptr < '0' || *endptr > '9') {
                        g_print ("Error parsing %s: `%s': %s.\n",
                                 what, str, "Trailing garbage");
                        return FALSE;
                }
                *result += (*endptr - '0') / fract;
                fract /= 10;
                
                if (fract < 0.000001)
                        break;
        }
        
        return TRUE;       
}

/* Convenience function: Free a vector (returned from the tokenizer)
 * and return FALSE so that it can be used in g_return_val_if_fail().
 */
static gboolean
free_vector (gchar ** v)
{
        g_free (v);
        return FALSE;
}

static gboolean
gibbon_session_handle_board (GibbonSession *self, const gchar *string)
{
        gchar **tokens;
        struct GibbonPosition pos;
        gint i;
                        
        g_return_val_if_fail (GIBBON_IS_SESSION (self), FALSE);
        g_return_val_if_fail (string, FALSE);
        
        /* FIXME! Fill structure completely instead! */
        memset (&pos, 0, sizeof pos);
        
        tokens = g_strsplit (string, ":", 99);

        g_return_val_if_fail (tokens, FALSE);
        
        for (i = 0; i <= 38; ++i)
                g_return_val_if_fail (tokens[i], free_vector (tokens));
        
        pos.player[0] = tokens[0];
        g_return_val_if_fail (pos.player[0][0], free_vector (tokens));
        
        pos.player[1] = tokens[1];
        g_return_val_if_fail (pos.player[0][0], free_vector (tokens));
        
        g_return_val_if_fail (parse_integer (tokens[2], &pos.match_length, 
                              "match length"),
                              free_vector (tokens));
        g_return_val_if_fail (pos.match_length > 0, free_vector (tokens));

        g_return_val_if_fail (parse_integer (tokens[3], &pos.score[0], "score0"),
                              free_vector (tokens));
        g_return_val_if_fail (pos.score[0] >= 0, free_vector (tokens));
        g_return_val_if_fail (parse_integer (tokens[4], &pos.score[1], "score1"),
                              free_vector (tokens));
        g_return_val_if_fail (pos.score[1] >= 0, free_vector (tokens));
        
        g_return_val_if_fail (parse_integer (tokens[5], &pos.bar[0], "bar0"),
                              free_vector (tokens));
        g_return_val_if_fail (pos.bar[0] >= -15 && pos.bar[0] <= 15,
                              free_vector (tokens));
        
        for (i = 6; i < 30; ++i) {
                g_return_val_if_fail (parse_integer (tokens[i], 
                                                     &pos.checkers[i - 6],
                                                     "checker"),
                                      free_vector (tokens));
                g_return_val_if_fail (pos.checkers[i - 6] >= -15
                                      && pos.checkers[i - 6] <= 15,
                                      free_vector (tokens));
        }
        
        g_return_val_if_fail (parse_integer (tokens[30], &pos.bar[1], "bar1"),
                              free_vector (tokens));
        g_return_val_if_fail (pos.bar[1] >= -15 && pos.bar[1] <= 15,
                              free_vector (tokens));

        g_return_val_if_fail (parse_integer (tokens[31], &pos.turn, "turn"),
                              free_vector (tokens));
        g_return_val_if_fail ((pos.turn >= -1 && pos.turn <= 1),
                              free_vector (tokens));

        g_return_val_if_fail (parse_integer (tokens[32], &pos.dice[0][0], 
                                             "dice[0][0]"),
                              free_vector (tokens));
        g_return_val_if_fail (pos.dice[0][0] >= 0 && pos.dice[0][0] <= 6, 
                              free_vector (tokens));

        g_return_val_if_fail (parse_integer (tokens[33], &pos.dice[0][1], 
                                             "dice[0][1]"),
                              free_vector (tokens));
        g_return_val_if_fail (pos.dice[0][1] >= 0 && pos.dice[0][1] <= 6, 
                              free_vector (tokens));

        g_return_val_if_fail (parse_integer (tokens[34], &pos.dice[1][0], 
                                             "dice[1][0]"),
                              free_vector (tokens));
        g_return_val_if_fail (pos.dice[1][0] >= 0 && pos.dice[1][0] <= 6, 
                              free_vector (tokens));

        g_return_val_if_fail (parse_integer (tokens[35], &pos.dice[1][1], 
                                             "dice[1][1]"),
                              free_vector (tokens));
        g_return_val_if_fail (pos.dice[1][1] >= 0 && pos.dice[1][1] <= 6, 
                              free_vector (tokens));
        
        g_return_val_if_fail (parse_integer (tokens[36], &pos.cube, "cube"),
                              free_vector (tokens));
        g_return_val_if_fail (pos.cube, free_vector (tokens));
        
        g_return_val_if_fail (parse_integer (tokens[37], &pos.may_double[0], 
                              "may double 0"),
                              free_vector (tokens));
        g_return_val_if_fail (pos.may_double[0] == 0 || pos.may_double[0] == 1, 
                              free_vector (tokens));

        g_return_val_if_fail (parse_integer (tokens[38], &pos.may_double[1], 
                              "may double 0"),
                              free_vector (tokens));
        g_return_val_if_fail (pos.may_double[0] == 0 || pos.may_double[0] == 1, 
                              free_vector (tokens));

        g_strfreev (tokens);
        
        set_position (&pos);

return FALSE;        
        return TRUE;
}
