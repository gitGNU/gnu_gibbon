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

#include "gibbon-connection.h"
#include "gibbon-session.h"
#include "gibbon-prefs.h"
#include "gibbon-server-console.h"
#include "gibbon-player-list.h"
#include "gibbon-player-list-view.h"
#include "gibbon-cairoboard.h"
#include "gibbon-fibs-message.h"
#include "gibbon-shouts.h"
#include "gibbon-position.h"
#include "gibbon-board.h"

#define CLIP_WELCOME 1
#define CLIP_WHO_INFO 5
#define CLIP_WHO_INFO_END 6
#define CLIP_SAYS 12
#define CLIP_SHOUTS 13
#define CLIP_YOU_SAY 16
#define CLIP_YOU_SHOUT 17

static gint gibbon_session_clip_welcome (GibbonSession *self,
                                         const gchar *message);
static gint gibbon_session_clip_who_info (GibbonSession *self,
                                          const gchar *message);
static gint gibbon_session_clip_says (GibbonSession *self,
                                      const gchar *message);
static gint gibbon_session_clip_shouts (GibbonSession *self,
                                        const gchar *message);
static gint gibbon_session_clip_you_say (GibbonSession *self,
                                          const gchar *message);
static gint gibbon_session_clip_you_shout (GibbonSession *self,
                                           const gchar *message);
static gint gibbon_session_dispatch_clip_message (GibbonSession *self,
                                                  const gchar *message);
static gboolean gibbon_session_handle_board (GibbonSession *self,
                                             const gchar *board);

static gboolean free_vector (gchar **);
static gboolean parse_integer (const gchar *str, gint* result,
                               const gchar *what,
                               gint lower, gint upper);

struct _GibbonSessionPrivate {
        GibbonApp *app;
        GibbonConnection *connection;

        gchar *watching;
        gchar *opponent;

        GibbonPlayerList *player_list;
        GibbonPlayerListView *player_list_view;
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
        self->priv->player_list = NULL;
        self->priv->player_list_view = NULL;
}

static void
gibbon_session_finalize (GObject *object)
{
        GibbonSession *self = GIBBON_SESSION (object);

        G_OBJECT_CLASS (gibbon_session_parent_class)->finalize (object);

        self->priv->app = NULL;
        self->priv->connection = NULL;

        if (self->priv->watching)
                g_free (self->priv->watching);
        self->priv->watching = NULL;

        if (self->priv->opponent)
                g_free (self->priv->opponent);
        self->priv->opponent = NULL;

        if (self->priv->player_list)
                g_object_unref (self->priv->player_list);
        self->priv->player_list = NULL;

        if (self->priv->player_list_view)
                g_object_unref (self->priv->player_list_view);
        self->priv->player_list_view = NULL;
}

static void
gibbon_session_class_init (GibbonSessionClass *klass)
{
        GObjectClass* object_class = G_OBJECT_CLASS (klass);

        g_type_class_add_private (klass, sizeof (GibbonSessionPrivate));

        object_class->finalize = gibbon_session_finalize;
}

GibbonSession *
gibbon_session_new (GibbonApp *app, GibbonConnection *connection)
{
        GibbonSession *self = g_object_new (GIBBON_TYPE_SESSION, NULL);

        self->priv->connection = connection;
        self->priv->app = app;

        self->priv->player_list = gibbon_player_list_new ();
        self->priv->player_list_view =
                gibbon_player_list_view_new (app, self->priv->player_list);

        return self;
}

static gint
gibbon_session_dispatch_clip_message (GibbonSession *self,
                                      const gchar *message)
{
        unsigned long int code;
        gchar *endptr;
        gint retval = -1;

        g_return_val_if_fail (GIBBON_IS_SESSION (self), -1);
        
        code = strtoul (message, &endptr, 10);
        
        /* Skip whitespace.  */
        while (*endptr == ' ' || *endptr == '\t' || *endptr == '\r'
               || *endptr == '\f' || *endptr == '\v') {
                retval = 0;
                endptr++;
        }
        if (retval && *endptr)
                /* No whitespace following number.  */
                return retval;
        
        switch (code) {
                case CLIP_WELCOME:
                        retval = gibbon_session_clip_welcome (self, endptr);
                        break;
                case CLIP_WHO_INFO:
                        retval = gibbon_session_clip_who_info (self, endptr);
                        break;
                case CLIP_WHO_INFO_END: /* Ignored.  */
                        retval = CLIP_WHO_INFO_END;
                        break;
                case CLIP_SAYS:
                        retval = gibbon_session_clip_says (self, endptr);
                        break;
                case CLIP_SHOUTS:
                        retval = gibbon_session_clip_shouts (self, endptr);
                        break;
                case CLIP_YOU_SAY:
                        retval = gibbon_session_clip_you_say (self, endptr);
                        break;
                case CLIP_YOU_SHOUT:
                        retval = gibbon_session_clip_you_shout (self, endptr);
                        break;
                default:
                        retval = -1;
        }

        return retval;
}

gint
gibbon_session_process_server_line (GibbonSession *self,
                                    const gchar *line)
{
        g_return_val_if_fail (GIBBON_IS_SESSION (self), -1);

        if (line[0] >= '0' && line[0] <= '9') {
                return gibbon_session_dispatch_clip_message (self, line);
        } else if (0 == strncmp ("board:", line, 6)) {
                if (gibbon_session_handle_board (self, line + 6))
                        return 0;
        }

        return -1;
}

static gint
gibbon_session_clip_welcome (GibbonSession *self, 
                             const gchar *message)
{
        const gchar* login;
        gchar **tokens;
        GTimeVal last_login;
        gchar *last_login_str;
        gchar *reply;
        gchar *mail;
        GibbonServerConsole *console;
        GibbonPrefs *prefs;
        
        g_return_val_if_fail (GIBBON_IS_SESSION (self), FALSE);

        login = gibbon_connection_get_login (self->priv->connection);

        tokens = g_strsplit_set (message, GIBBON_SESSION_WHITESPACE, 4);
        
        last_login.tv_usec = 0;
        if (tokens[1])
                last_login.tv_sec = strtol (tokens[1], NULL, 10);
        
        if (!tokens[0] || strcmp (tokens[0], login)) {
                gibbon_connection_fatal (self->priv->connection,
                                         _("Parser expected `%s' as"
                                           " login: %s\n"),
                                         login, message);
                return -1;
        }

        if (last_login.tv_usec < 0) {
                gibbon_connection_fatal (self->priv->connection,
                                         _("Parser expected timestamp after"
                                           "login `%s': %s\n"),
                                           login, message);
                return -1;
        }

        if (!tokens[2]) {
                gibbon_connection_fatal (self->priv->connection,
                                         _("Parser expected address after"
                                           " timestamp: %s\n"),
                                           message);
                return -1;
        }

        if (tokens[3]) {
                gibbon_connection_fatal (self->priv->connection,
                                         _("Trailing garbage after address:"
                                           " %s\n"), message);
                return -1;
        }


        /* FIXME! Isn't there a better way to format a date and time
         * in glib?
         */
        last_login_str = ctime (&last_login.tv_sec);
        last_login_str[strlen (last_login_str) - 1] = 0;
                
        reply = g_strdup_printf (_("Last login on %s from %s."),
                                   last_login_str, tokens[2]);
        console = gibbon_app_get_server_console (self->priv->app);
        gibbon_server_console_print_info (console, reply);
        g_free (reply);
                
        gibbon_connection_queue_command (self->priv->connection,
                                         FALSE,
                                         "set boardstyle 3");

        prefs = gibbon_app_get_prefs (self->priv->app);
        mail = gibbon_prefs_get_string (prefs,
                                        GIBBON_PREFS_MAIL_ADDRESS);
        if (mail) {
                gibbon_connection_queue_command (self->priv->connection,
                                                 FALSE,
                                                 "address %s",
                                                 mail);
                g_free (mail);
        }
        
        g_strfreev (tokens);
        
        return CLIP_WELCOME;
}

static gint
gibbon_session_clip_who_info (GibbonSession *self, 
                              const gchar *message)
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

        tokens = g_strsplit_set (message, GIBBON_SESSION_WHITESPACE, 13);
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

        rating = g_ascii_strtod (tokens[5], NULL);
        g_return_val_if_fail (parse_integer (tokens[6], &experience,
                                             "experience", 0, G_MAXINT),
                              free_vector (tokens));
        
        idle = tokens[7];
        login = tokens[8];
        hostname = tokens[9];
        client = tokens[10];
        email = tokens[11];

        available = ready && !away && !opponent[0];
        
        gibbon_player_list_set (self->priv->player_list,
                                who, available, rating, experience,
                                opponent, watching, client, email);

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

        return CLIP_WHO_INFO;
}

static gint
gibbon_session_clip_says (GibbonSession *self,
                             const gchar *message)
{
        GibbonFIBSMessage *fibs_message;
        GibbonConnection *connection;

        g_return_val_if_fail (GIBBON_IS_SESSION (self), FALSE);

        fibs_message = gibbon_fibs_message_new (message);
        if (!fibs_message)
                return -1;

        connection = gibbon_app_get_connection (self->priv->app);
        if (!connection)
                return -1;

        gibbon_app_show_message (self->priv->app,
                                 fibs_message->sender,
                                 fibs_message);

        gibbon_fibs_message_free (fibs_message);

        return CLIP_SAYS;
}

static gint
gibbon_session_clip_shouts (GibbonSession *self,
                           const gchar *message)
{
        GibbonFIBSMessage *fibs_message;
        GibbonShouts *shouts;

        g_return_val_if_fail (GIBBON_IS_SESSION (self), FALSE);

        fibs_message = gibbon_fibs_message_new (message);
        if (!fibs_message)
                return -1;

        shouts = gibbon_app_get_shouts (self->priv->app);
        gibbon_shouts_append_message (shouts, fibs_message);

        gibbon_fibs_message_free (fibs_message);

        return CLIP_SHOUTS;
}

static gint
gibbon_session_clip_you_say (GibbonSession *self,
                             const gchar *message)
{
        GibbonFIBSMessage *fibs_message;
        GibbonConnection *connection;
        gchar *receiver;

        g_return_val_if_fail (GIBBON_IS_SESSION (self), FALSE);

        fibs_message = gibbon_fibs_message_new (message);
        if (!fibs_message)
                return FALSE;

        connection = gibbon_app_get_connection (self->priv->app);
        if (!connection)
                return FALSE;

        /* Steal the receiver, and make it our sender.  */
        receiver = fibs_message->sender;
        fibs_message->sender =
                g_strdup (gibbon_connection_get_login (connection));

        gibbon_app_show_message (self->priv->app,
                                 receiver,
                                 fibs_message);

        gibbon_fibs_message_free (fibs_message);
        g_free (receiver);

        return CLIP_YOU_SAY;
}

static gint
gibbon_session_clip_you_shout (GibbonSession *self,
                               const gchar *message)
{
        GibbonFIBSMessage *fibs_message;
        GibbonConnection *connection;

        g_return_val_if_fail (GIBBON_IS_SESSION (self), FALSE);

        connection = gibbon_app_get_connection (self->priv->app);
        if (!connection)
                return FALSE;

        fibs_message = g_malloc (sizeof *fibs_message);
        fibs_message->message = g_strdup (message);
        fibs_message->sender =
                g_strdup (gibbon_connection_get_login (connection));

        gibbon_app_show_shout (self->priv->app, fibs_message);

        gibbon_fibs_message_free (fibs_message);

        return CLIP_YOU_SHOUT;
}

/* FIXME! Use g_ascii_strtoll in this function! */
/* FIXME! Pass upper and lower bounds as arguments. */
static gboolean
parse_integer (const gchar *str, gint *result, const gchar *what,
               gint lower, gint upper)
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
        if (*result < lower || *result > upper)
                return FALSE;
        
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

/*
 * This requires a little explanation.
 *
 * On FIBS, colors are X and O.  When necessary X is represented as -1,
 * and O as 1, zero is used for indicating none of these.  This mapping
 * is similar to that used in #GibbonPosition.
 *
 * The initial color for the two opponents is randomly chosen.
 *
 * The board is represented with 26 integer fields.
 *
 * Index #0 and index #25 are the home board and the bar for the player
 * on move.  Which one is home and which one is bar depends on the direction.
 * And the sign depends your color.  We therefore ignore them altogether
 * and rather rely on the explicit broken down values given at the end of
 * the board string because they are always positive.
 */
static gboolean
gibbon_session_handle_board (GibbonSession *self, const gchar *string)
{
        gchar **tokens;
        GibbonPosition *pos;
        GibbonPositionSide turn, color, direction;
        gint may_double[2];
        GibbonCairoboard *board;
        gint i;
        gint dice[4];
                        
        g_return_val_if_fail (GIBBON_IS_SESSION (self), FALSE);
        g_return_val_if_fail (string, FALSE);

        pos = gibbon_position_new ();
        
        tokens = g_strsplit (string, ":", 99);

        g_return_val_if_fail (tokens, FALSE);
        
        for (i = 0; i <= 38; ++i)
                g_return_val_if_fail (tokens[i], free_vector (tokens));

        g_return_val_if_fail (parse_integer (tokens[31], &turn,
                              "turn", -1, 1),
                free_vector (tokens));
        g_return_val_if_fail (parse_integer (tokens[40], &color,
                              "color", -1, 1),
                free_vector (tokens));
        g_return_val_if_fail (parse_integer (tokens[41], &direction,
                              "direction", -1, 1),
                free_vector (tokens));

        /* FIXME! This must be replaced by the real name if playing.
         * Otherwise it is always the string "You".
         */
        pos->players[0] = g_strdup (tokens[0]);
        g_return_val_if_fail (pos->players[0][0], free_vector (tokens));
        
        pos->players[1] = g_strdup (tokens[1]);
        g_return_val_if_fail (pos->players[1][0], free_vector (tokens));
        
        g_return_val_if_fail (parse_integer (tokens[2], &pos->match_length,
                              "match length", 0, G_MAXINT),
                              free_vector (tokens));

        g_return_val_if_fail (parse_integer (tokens[3], &pos->scores[0],
                                             "score0", 0, G_MAXINT),
                              free_vector (tokens));
        g_return_val_if_fail (parse_integer (tokens[4], &pos->scores[1],
                                             "score1", 0, G_MAXINT),
                              free_vector (tokens));
        
        if (direction == GIBBON_POSITION_SIDE_BLACK) {
                for (i = 6; i < 30; ++i) {
                        g_return_val_if_fail (parse_integer (tokens[i],
                                                             &pos->points[i - 6],
                                                             "checker", -15, 15),
                                              free_vector (tokens));
                        pos->points[i - 6] *= color;
                }
        } else {
                for (i = 29; i >= 6; --i) {
                        g_return_val_if_fail (parse_integer (tokens[i],
                                                             &pos->points[i - 6],
                                                             "checker", -15, 15),
                                              free_vector (tokens));
                        pos->points[i - 6] *= color;
                }

        }
        
        g_return_val_if_fail (parse_integer (tokens[32], &dice[0],
                                             "dice[0]", 0, 6),
                              free_vector (tokens));
        g_return_val_if_fail (parse_integer (tokens[33], &dice[1],
                                             "dice[1]", 0, 6),
                              free_vector (tokens));
        g_return_val_if_fail (parse_integer (tokens[34], &dice[2],
                                             "dice[2]", 0, 6),
                              free_vector (tokens));
        g_return_val_if_fail (parse_integer (tokens[35], &dice[3],
                                             "dice[3]", 0, 6),
                              free_vector (tokens));
        g_return_val_if_fail (parse_integer (tokens[36], &pos->cube,
                                             "cube", 0, G_MAXINT),
                              free_vector (tokens));
        g_return_val_if_fail (parse_integer (tokens[37], &may_double[0],
                                             "may double 0", 0, 1),
                                             free_vector (tokens));
        g_return_val_if_fail (parse_integer (tokens[38], &may_double[1],
                                             "may double 0", 0, 1),
                                             free_vector (tokens));

        pos->may_double[0] = may_double[0] ? TRUE : FALSE;
        pos->may_double[1] = may_double[1] ? TRUE : FALSE;

        /* FIXME! It is better to rely on the moves displayed.  The dice from
         * the board command should only be checked for consistency.
         */
        if (turn == color) {
                pos->dice[0] = dice[0];
                pos->dice[1] = dice[1];
        } else if (turn) {
                pos->dice[0] = -dice[2];
                pos->dice[1] = -dice[3];
        }

        g_return_val_if_fail (parse_integer (tokens[46], &pos->bar[0], "bar0",
                                             0, 15),
                              free_vector (tokens));

        g_return_val_if_fail (parse_integer (tokens[47], &pos->bar[1], "bar1",
                                             0, 15),
                              free_vector (tokens));

        g_strfreev (tokens);
        
        if (pos->match_length &&
            (pos->scores[0] == pos->match_length - 1
             || pos->scores[1] == pos->match_length - 1)
            && !pos->may_double[0]
            && !pos->may_double[1]) {
                pos->game_info = g_strdup (_("Crawford game"));
        }

        board = gibbon_app_get_board (self->priv->app);
        gibbon_board_set_position (GIBBON_BOARD (board), pos);

        return TRUE;
}
