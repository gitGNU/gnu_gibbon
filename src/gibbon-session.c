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
#include <string.h>

#include "gibbon-connection.h"
#include "gibbon-session.h"
#include "gibbon-prefs.h"
#include "gibbon-server-console.h"
#include "gibbon-player-list.h"
#include "gibbon-inviter-list.h"
#include "gibbon-cairoboard.h"
#include "gibbon-fibs-message.h"
#include "gibbon-shouts.h"
#include "gibbon-position.h"
#include "gibbon-board.h"
#include "gibbon-game-chat.h"
#include "gibbon-archive.h"
#include "gibbon-util.h"
#include "gibbon-clip.h"
#include "gibbon-saved-info.h"
#include "gibbon-reliability.h"
#include "gibbon-client-icons.h"

typedef enum {
        GIBBON_SESSION_PLAYER_YOU = 0,
        GIBBON_SESSION_PLAYER_WATCHING = 1,
        GIBBON_SESSION_PLAYER_OPPONENT = 2,
        GIBBON_SESSION_PLAYER_OTHER = 3
} GibbonSessionPlayer;

#ifdef GIBBON_SESSION_DEBUG_BOARD_STATE
static void gibbon_session_dump_position (const GibbonSession *self,
                                          const GibbonPosition *pos);
#endif /* #ifdef GIBBON_SESSION_DEBUG_BOARD_STATE */

static gint gibbon_session_clip_welcome (GibbonSession *self, GSList *iter);
static gint gibbon_session_clip_who_info (GibbonSession *self, GSList *iter);
static gint gibbon_session_clip_who_info_end (GibbonSession *self,
                                              GSList *iter);
static gint gibbon_session_clip_logout (GibbonSession *self, GSList *iter);
static gint gibbon_session_clip_message (GibbonSession *self, GSList *iter);
static gint gibbon_session_clip_message_delivered (GibbonSession *self,
                                                   GSList *iter);
static gint gibbon_session_clip_message_saved (GibbonSession *self,
                                               GSList *iter);
static gint gibbon_session_clip_says (GibbonSession *self, GSList *iter);
static gint gibbon_session_clip_shouts (GibbonSession *self, GSList *iter);
static gint gibbon_session_clip_whispers (GibbonSession *self, GSList *iter);
static gint gibbon_session_clip_kibitzes (GibbonSession *self, GSList *iter);
static gint gibbon_session_clip_you_say (GibbonSession *self, GSList *iter);
static gint gibbon_session_clip_you_shout (GibbonSession *self, GSList *iter);
static gint gibbon_session_clip_you_whisper (GibbonSession *self, GSList *iter);
static gint gibbon_session_clip_you_kibitz (GibbonSession *self, GSList *iter);
static gint gibbon_session_handle_board (GibbonSession *self, GSList *iter);
static gint gibbon_session_handle_bad_board (GibbonSession *self, GSList *iter);
static gint gibbon_session_handle_rolls (GibbonSession *self, GSList *iter);
static gint gibbon_session_handle_moves (GibbonSession *self, GSList *iter);
static gint gibbon_session_handle_invitation (GibbonSession *self,
                                              GSList *iter);
static gint gibbon_session_handle_youre_watching (GibbonSession *self,
                                                  GSList *iter);
static gint gibbon_session_handle_win_match (GibbonSession *self, GSList *iter);
static gint gibbon_session_handle_resume_match (GibbonSession *self,
                                                GSList *iter);
static gint gibbon_session_handle_show_setting (GibbonSession *self,
                                                GSList *iter);
static gint gibbon_session_handle_show_toggle (GibbonSession *self,
                                               GSList *iter);
static gint gibbon_session_handle_show_saved (GibbonSession *self,
                                              GSList *iter);
static gint gibbon_session_handle_show_saved_count (GibbonSession *self,
                                                    GSList *iter);

static gchar *gibbon_session_decode_client (GibbonSession *self,
                                            const gchar *token);
static gboolean gibbon_session_clear_expect_list (GibbonSession *self,
                                                  GSList **list,
                                                  const gchar *string);

struct _GibbonSessionPrivate {
        GibbonApp *app;
        GibbonConnection *connection;

        gchar *watching;
        gchar *opponent;

        /* This is the approved position.   It is the result of a "board"
         * message, or of a move, roll, or other game play action made.
         */
        GibbonPosition *position;

        GibbonPlayerList *player_list;
        GibbonInviterList *inviter_list;

        GibbonArchive *archive;

        gboolean initialized;

        GSList *expect_settings;
        GSList *expect_toggles;

        GHashTable *saved_games;
        gboolean saved_games_finished;
};

#define GIBBON_SESSION_PRIVATE(o) (G_TYPE_INSTANCE_GET_PRIVATE ((o), \
                                      GIBBON_TYPE_SESSION,           \
                                      GibbonSessionPrivate))
G_DEFINE_TYPE (GibbonSession, gibbon_session, G_TYPE_OBJECT);

static void
gibbon_session_init (GibbonSession *self)
{
        GSList *iter;

        self->priv = G_TYPE_INSTANCE_GET_PRIVATE (self, 
                                                  GIBBON_TYPE_SESSION, 
                                                  GibbonSessionPrivate);

        self->priv->connection = NULL;
        self->priv->watching = NULL;
        self->priv->opponent = NULL;
        self->priv->position = NULL;
        self->priv->player_list = NULL;
        self->priv->inviter_list = NULL;
        self->priv->archive = NULL;

        self->priv->initialized = FALSE;

        iter = NULL;
        iter = g_slist_prepend (iter, "boardstyle");
        iter = g_slist_prepend (iter, "linelength");
        iter = g_slist_prepend (iter, "pagelength");
        iter = g_slist_prepend (iter, "redoubles");
        iter = g_slist_prepend (iter, "sortwho");
        iter = g_slist_prepend (iter, "timezone");
        self->priv->expect_settings = iter;

        iter = NULL;
        iter = g_slist_prepend (iter, "wrap");
        iter = g_slist_prepend (iter, "telnet");
        iter = g_slist_prepend (iter, "silent");
        iter = g_slist_prepend (iter, "report");
        iter = g_slist_prepend (iter, "ready");
        iter = g_slist_prepend (iter, "ratings");
        iter = g_slist_prepend (iter, "notify");
        iter = g_slist_prepend (iter, "moves");
        iter = g_slist_prepend (iter, "moreboards");
        iter = g_slist_prepend (iter, "greedy");
        iter = g_slist_prepend (iter, "double");
        iter = g_slist_prepend (iter, "crawford");
        iter = g_slist_prepend (iter, "bell");
        iter = g_slist_prepend (iter, "automove");
        iter = g_slist_prepend (iter, "autodouble");
        iter = g_slist_prepend (iter, "autoboard");
        iter = g_slist_prepend (iter, "allowpip");
        self->priv->expect_toggles = iter;

        self->priv->saved_games =
                g_hash_table_new_full (g_str_hash,
                                       g_str_equal,
                                       g_free,
                                       (GDestroyNotify) gibbon_saved_info_free);
        self->priv->saved_games_finished = FALSE;
}

static void
gibbon_session_finalize (GObject *object)
{
        GibbonSession *self = GIBBON_SESSION (object);

        G_OBJECT_CLASS (gibbon_session_parent_class)->finalize (object);

        if (self->priv->watching)
                g_free (self->priv->watching);

        if (self->priv->opponent)
                g_free (self->priv->opponent);

        if (self->priv->position)
                gibbon_position_free (self->priv->position);

        if (self->priv->expect_settings)
                g_slist_free (self->priv->expect_settings);
        if (self->priv->expect_toggles)
                g_slist_free (self->priv->expect_toggles);

        if (self->priv->saved_games)
                g_hash_table_destroy (self->priv->saved_games);
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

        self->priv->player_list = gibbon_app_get_player_list (app);
        self->priv->inviter_list = gibbon_app_get_inviter_list (app);

        self->priv->position = gibbon_position_new ();

        self->priv->archive = gibbon_app_get_archive (app);

        return self;
}

gint
gibbon_session_process_server_line (GibbonSession *self,
                                    const gchar *line)
{
        gint retval = -1;
        GSList *values, *iter;
        enum GibbonClipCode code;

        g_return_val_if_fail (GIBBON_IS_SESSION (self), -1);
        g_return_val_if_fail (line != NULL, -1);

        values = gibbon_clip_parse (line);
        if (!values)
                return -1;

        iter = values;
        if (!gibbon_clip_get_int (&iter, GIBBON_CLIP_TYPE_UINT,
                                  (gint *) &code)) {
                gibbon_clip_free_result (iter);
                return -1;
        }

        switch (code) {
        case GIBBON_CLIP_CODE_UNHANDLED:
                break;
        case GIBBON_CLIP_CODE_WELCOME:
                retval = gibbon_session_clip_welcome (self, iter);
                break;
        case GIBBON_CLIP_CODE_OWN_INFO:
                /* FIXME! Parse settings! */
                retval = GIBBON_CLIP_CODE_OWN_INFO;
                break;
        case GIBBON_CLIP_CODE_MOTD:
                retval = GIBBON_CLIP_CODE_MOTD;
                break;
        case GIBBON_CLIP_CODE_MOTD_END:
                retval = GIBBON_CLIP_CODE_MOTD_END;
                break;
        case GIBBON_CLIP_CODE_WHO_INFO:
                retval = gibbon_session_clip_who_info (self, iter);
                break;
        case GIBBON_CLIP_CODE_WHO_INFO_END:
                retval = gibbon_session_clip_who_info_end (self, iter);
                break;
        case GIBBON_CLIP_CODE_LOGIN:
                retval = GIBBON_CLIP_CODE_LOGIN;
                break;
        case GIBBON_CLIP_CODE_LOGOUT:
                retval = gibbon_session_clip_logout (self, iter);
                break;
        case GIBBON_CLIP_CODE_MESSAGE:
                retval = gibbon_session_clip_message (self, iter);
                break;
        case GIBBON_CLIP_CODE_MESSAGE_DELIVERED:
                retval = gibbon_session_clip_message_delivered (self, iter);
                break;
        case GIBBON_CLIP_CODE_MESSAGE_SAVED:
                retval = gibbon_session_clip_message_saved (self, iter);
                break;
        case GIBBON_CLIP_CODE_SAYS:
                retval = gibbon_session_clip_says (self, iter);
                break;
        case GIBBON_CLIP_CODE_SHOUTS:
                retval = gibbon_session_clip_shouts (self, iter);
                break;
        case GIBBON_CLIP_CODE_WHISPERS:
                retval = gibbon_session_clip_whispers (self, iter);
                break;
        case GIBBON_CLIP_CODE_KIBITZES:
                retval = gibbon_session_clip_kibitzes (self, iter);
                break;
        case GIBBON_CLIP_CODE_YOU_SAY:
                retval = gibbon_session_clip_you_say (self, iter);
                break;
        case GIBBON_CLIP_CODE_YOU_SHOUT:
                retval = gibbon_session_clip_you_shout (self, iter);
                break;
        case GIBBON_CLIP_CODE_YOU_WHISPER:
                retval = gibbon_session_clip_you_whisper (self, iter);
                break;
        case GIBBON_CLIP_CODE_YOU_KIBITZ:
                retval = gibbon_session_clip_you_kibitz (self, iter);
                break;
        case GIBBON_CLIP_CODE_BOARD:
                retval = gibbon_session_handle_board (self, iter);
                break;
        case GIBBON_CLIP_CODE_BAD_BOARD:
                retval = gibbon_session_handle_bad_board (self, iter);
                break;
        case GIBBON_CLIP_CODE_ROLLS:
                retval = gibbon_session_handle_rolls (self, iter);
                break;
        case GIBBON_CLIP_CODE_MOVES:
                retval = gibbon_session_handle_moves (self, iter);
                break;
        case GIBBON_CLIP_CODE_INVITATION:
                retval = gibbon_session_handle_invitation (self, iter);
                break;
        case GIBBON_CLIP_CODE_TYPE_JOIN:
                retval = GIBBON_CLIP_CODE_TYPE_JOIN;
                break;
        case GIBBON_CLIP_CODE_YOURE_WATCHING:
                retval = gibbon_session_handle_youre_watching (self, iter);
                break;
        case GIBBON_CLIP_CODE_START_MATCH:
                retval = GIBBON_CLIP_CODE_START_MATCH;
                break;
        case GIBBON_CLIP_CODE_WIN_MATCH:
                retval = gibbon_session_handle_win_match (self, iter);
                break;
        case GIBBON_CLIP_CODE_RESUME_MATCH:
                retval = gibbon_session_handle_resume_match (self, iter);
                break;
        case GIBBON_CLIP_CODE_EMPTY:
                retval = GIBBON_CLIP_CODE_EMPTY;
                break;
        case GIBBON_CLIP_CODE_START_SETTINGS:
                if (self->priv->expect_settings)
                        retval = GIBBON_CLIP_CODE_START_SETTINGS;
                else
                        retval = -1;
                break;
        case GIBBON_CLIP_CODE_SHOW_SETTING:
                retval = gibbon_session_handle_show_setting (self, iter);
                break;
        case GIBBON_CLIP_CODE_START_TOGGLES:
                if (self->priv->expect_toggles)
                        retval = GIBBON_CLIP_CODE_START_TOGGLES;
                else
                        retval = -1;
                break;
        case GIBBON_CLIP_CODE_SHOW_TOGGLE:
                retval = gibbon_session_handle_show_toggle (self, iter);
                break;
        case GIBBON_CLIP_CODE_SHOW_START_SAVED:
                if (self->priv->saved_games_finished)
                        retval = -1;
                else
                        retval = GIBBON_CLIP_CODE_SHOW_START_SAVED;
                break;
        case GIBBON_CLIP_CODE_SHOW_SAVED:
                retval = gibbon_session_handle_show_saved (self, iter);
                break;
        case GIBBON_CLIP_CODE_SHOW_SAVED_NONE:
                if (self->priv->saved_games_finished)
                        retval = -1;
                else
                        retval = GIBBON_CLIP_CODE_SHOW_SAVED_NONE;
                self->priv->saved_games_finished = TRUE;
                break;
        case GIBBON_CLIP_CODE_SHOW_SAVED_COUNT:
                retval = gibbon_session_handle_show_saved_count (self, iter);
                break;
        }

        gibbon_clip_free_result (values);

        return retval;
}

static gint
gibbon_session_clip_welcome (GibbonSession *self, GSList *iter)
{
        const gchar *login;
        const gchar *expect;
        gint64 tv_sec;
        const gchar *last_from;
        GTimeVal last_login;
        gchar *last_login_str;
        gchar *reply;
        GibbonServerConsole *console;

        if (!gibbon_clip_get_string (&iter, GIBBON_CLIP_TYPE_NAME, &login))
                return -1;

        expect = gibbon_connection_get_login (self->priv->connection);
        if (g_strcmp0 (expect, login))
                return -1;

        last_login.tv_usec = 0;
        if (!gibbon_clip_get_uint64 (&iter, GIBBON_CLIP_TYPE_TIMESTAMP,
                                     (guint64 *) &tv_sec))
                return -1;
        last_login.tv_sec = tv_sec;
        
        if (!gibbon_clip_get_string (&iter, GIBBON_CLIP_TYPE_STRING,
                                     &last_from))
                return -1;

        /* FIXME! Isn't there a better way to format a date and time
         * in glib?
         */
        last_login_str = ctime (&last_login.tv_sec);
        last_login_str[strlen (last_login_str) - 1] = 0;
                
        reply = g_strdup_printf (_("Last login on %s from %s."),
                                   last_login_str, last_from);
        console = gibbon_app_get_server_console (self->priv->app);
        gibbon_server_console_print_info (console, reply);
        g_free (reply);

        return GIBBON_CLIP_CODE_WELCOME;
}

static gint
gibbon_session_clip_who_info (GibbonSession *self, 
                              GSList *iter)
{
        const gchar *who;
        const gchar *opponent;
        const gchar *watching;
        gboolean ready;
        gboolean available;
        gboolean away;
        gdouble rating;
        guint64 experience;
        const gchar *raw_client;
        gchar *client;
        enum GibbonClientType client_type;
        GibbonClientIcons *client_icons;
        GdkPixbuf *client_icon;
        const gchar *email;
        const gchar *hostname;
        GibbonConnection *connection;
        GibbonArchive *archive;
        gdouble reliability;
        guint confidence;
        const gchar *account;
        guint port;
        const gchar *server;

        if (!gibbon_clip_get_string (&iter, GIBBON_CLIP_TYPE_NAME, &who))
                return -1;

        if (!gibbon_clip_get_string (&iter, GIBBON_CLIP_TYPE_NAME, &opponent))
                return -1;
        if (opponent[0] == '-' && opponent[1] == 0)
                opponent = "";
                
        if (!gibbon_clip_get_string (&iter, GIBBON_CLIP_TYPE_NAME, &watching))
                return -1;
        if (watching[0] == '-' && watching[1] == 0)
                watching = "";

        if (!gibbon_clip_get_boolean (&iter, GIBBON_CLIP_TYPE_BOOLEAN,
                                      &ready))
                return -1;

        if (!gibbon_clip_get_boolean (&iter, GIBBON_CLIP_TYPE_BOOLEAN,
                                      &away))
                return -1;

        if (!gibbon_clip_get_double (&iter, GIBBON_CLIP_TYPE_DOUBLE,
                                     &rating))
                return -1;

        if (!gibbon_clip_get_uint64 (&iter, GIBBON_CLIP_TYPE_UINT,
                                     &experience))
                return -1;

        iter = iter->next;
        iter = iter->next;
        
        if (!gibbon_clip_get_string (&iter, GIBBON_CLIP_TYPE_STRING,
                                     &hostname))
                return -1;

        if (!gibbon_clip_get_string (&iter, GIBBON_CLIP_TYPE_STRING,
                                     &raw_client))
                return -1;
        client = gibbon_session_decode_client (self, raw_client);

        if (!gibbon_clip_get_string (&iter, GIBBON_CLIP_TYPE_STRING,
                                     &email))
                return -1;

        available = ready && !away && !opponent[0];

        account = gibbon_connection_get_login (self->priv->connection);
        server = gibbon_connection_get_hostname (self->priv->connection);
        port = gibbon_connection_get_port (self->priv->connection);

        if (!gibbon_archive_get_reliability (self->priv->archive,
                                             server, port, who,
                                             &reliability, &confidence)) {
                confidence = 123;
                reliability = 4.56;
        }

        client_type = gibbon_get_client_type (client, who, server, port);
        client_icons = gibbon_app_get_client_icons (self->priv->app);
        client_icon = gibbon_client_icons_get_icon (client_icons, client_type);
        gibbon_player_list_set (self->priv->player_list,
                                who, available, rating, experience,
                                reliability, confidence,
                                opponent, watching,
                                client, client_icon,
                                hostname, email);

        if  (gibbon_inviter_list_exists (self->priv->inviter_list, who)) {
                if (opponent) {
                        gibbon_inviter_list_remove (self->priv->inviter_list,
                                                    who);
                } else {
                        gibbon_inviter_list_set (self->priv->inviter_list, who,
                                                 rating, experience,
                                                 reliability, confidence,
                                                 client, hostname, email);
                }
        }

        g_free (client);

        if (opponent && *opponent)
                gibbon_inviter_list_remove (self->priv->inviter_list, who);

        if (!g_strcmp0 (who, account)) {
                if (!opponent[0])
                        opponent = NULL;
                if (!watching[0])
                        watching = NULL;
                if (g_strcmp0 (opponent, self->priv->opponent)) {
                        g_free (self->priv->opponent);
                        self->priv->opponent = g_strdup (opponent);
                }
                if (g_strcmp0 (watching, self->priv->watching)) {
                        g_free (self->priv->watching);
                        self->priv->watching = g_strdup (watching);
                }
        }

        archive = gibbon_app_get_archive (self->priv->app);
        connection = self->priv->connection;

        gibbon_archive_update_user_full (archive, server, port, account,
                                         rating, experience);

        return GIBBON_CLIP_CODE_WHO_INFO;
}

static gint
gibbon_session_clip_who_info_end (GibbonSession *self,
                                  GSList *iter)
{
        GibbonPrefs *prefs;
        gchar *mail;

        if (!self->priv->initialized) {
                self->priv->initialized = TRUE;
                gibbon_connection_queue_command (self->priv->connection,
                                                 FALSE,
                                                 "show saved");
                gibbon_connection_queue_command (self->priv->connection,
                                                 FALSE,
                                                 "set");
                gibbon_connection_queue_command (self->priv->connection,
                                                 FALSE,
                                                 "toggle");

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

        }

        return GIBBON_CLIP_CODE_WHO_INFO_END;
}

static gint
gibbon_session_clip_logout (GibbonSession *self, GSList *iter)
{
        const gchar *hostname;
        guint port;

        const gchar *name;
        gchar *opponent;

        if (!gibbon_clip_get_string (&iter, GIBBON_CLIP_TYPE_NAME, &name))
                return -1;

        if (0 == g_strcmp0 (name, self->priv->watching)) {
                g_free (self->priv->position->status);
                self->priv->position->status =
                    g_strdup_printf (_("%s logged out!"), name);
                g_free (self->priv->watching);
                self->priv->watching = NULL;
                g_free (self->priv->opponent);
                self->priv->opponent = NULL;
        } else if (0 == g_strcmp0 (name, self->priv->opponent)) {
                g_free (self->priv->position->status);
                self->priv->position->status =
                    g_strdup_printf (_("%s logged out."), name);
                g_free (self->priv->opponent);
                self->priv->opponent = NULL;
                if (!self->priv->watching)
                        gibbon_app_display_info (self->priv->app,
                                                 _("%s logged out!"),
                                                 name);
        }

        opponent = gibbon_player_list_get_opponent (self->priv->player_list,
                                                    name);
        if (opponent) {
                hostname = gibbon_connection_get_hostname (self->priv->connection);
                port = gibbon_connection_get_port (self->priv->connection);
                gibbon_archive_save_drop (self->priv->archive,
                                          hostname, port, name, opponent);
                g_free (opponent);
        }
        gibbon_player_list_remove (self->priv->player_list, name);
        gibbon_inviter_list_remove (self->priv->inviter_list, name);

        return GIBBON_CLIP_CODE_LOGOUT;
}

static gint
gibbon_session_clip_message (GibbonSession *self, GSList *iter)
{
       const gchar *sender;
       const gchar *message;
       gint64 when;
       GTimeVal last_login;
       gchar *last_login_str;

       if (!gibbon_clip_get_string (&iter, GIBBON_CLIP_TYPE_NAME, &sender))
               return -1;

       if (!gibbon_clip_get_int64 (&iter, GIBBON_CLIP_TYPE_TIMESTAMP, &when))
               return -1;

       if (!gibbon_clip_get_string (&iter, GIBBON_CLIP_TYPE_STRING, &message))
               return -1;

       last_login_str = ctime (&last_login.tv_sec);
       last_login_str[strlen (last_login_str) - 1] = 0;

       gibbon_app_display_info (self->priv->app,
                                _("User `%s' left you a message at %s: %s"),
                                sender, last_login_str, message);

       return GIBBON_CLIP_CODE_MESSAGE;
}

static gint
gibbon_session_clip_message_delivered (GibbonSession *self, GSList *iter)
{
       const gchar *recipient;

       if (!gibbon_clip_get_string (&iter, GIBBON_CLIP_TYPE_NAME, &recipient))
               return -1;

       gibbon_app_display_info (self->priv->app,
                                _("Your message for user `%s' has been"
                                  " delivered!"),
                                recipient);

       return GIBBON_CLIP_CODE_MESSAGE_DELIVERED;
}

static gint
gibbon_session_clip_message_saved (GibbonSession *self, GSList *iter)
{
       const gchar *recipient;

       if (!gibbon_clip_get_string (&iter, GIBBON_CLIP_TYPE_NAME, &recipient))
               return -1;

       gibbon_app_display_info (self->priv->app,
                                _("User `%s' is not logged in.  Your message"
                                  " has been saved!"),
                                recipient);

       return GIBBON_CLIP_CODE_MESSAGE_DELIVERED;
}

static gint
gibbon_session_clip_says (GibbonSession *self, GSList *iter)
{
        GibbonFIBSMessage *fibs_message;
        GibbonConnection *connection;
        const gchar *message;
        const gchar *sender;

        connection = gibbon_app_get_connection (self->priv->app);
        if (!connection)
                return -1;

        if (!gibbon_clip_get_string (&iter, GIBBON_CLIP_TYPE_NAME, &sender))
                return -1;

        if (!gibbon_clip_get_string (&iter, GIBBON_CLIP_TYPE_STRING, &message))
                return -1;

        fibs_message = gibbon_fibs_message_new (sender, message);

        gibbon_app_show_message (self->priv->app,
                                 fibs_message->sender,
                                 fibs_message);

        gibbon_fibs_message_free (fibs_message);

        return GIBBON_CLIP_CODE_SAYS;
}

static gint
gibbon_session_clip_shouts (GibbonSession *self, GSList *iter)
{
        GibbonFIBSMessage *fibs_message;
        GibbonShouts *shouts;
        const gchar *sender;
        const gchar *message;

        shouts = gibbon_app_get_shouts (self->priv->app);
        if (!shouts)
                return -1;

        if (!gibbon_clip_get_string (&iter, GIBBON_CLIP_TYPE_NAME, &sender))
                return -1;

        if (!gibbon_clip_get_string (&iter, GIBBON_CLIP_TYPE_STRING, &message))
                return -1;

        fibs_message = gibbon_fibs_message_new (sender, message);

        gibbon_shouts_append_message (shouts, fibs_message);

        gibbon_fibs_message_free (fibs_message);

        return GIBBON_CLIP_CODE_SHOUTS;
}

static gint
gibbon_session_clip_whispers (GibbonSession *self, GSList *iter)
{
        GibbonFIBSMessage *fibs_message;
        GibbonGameChat *game_chat;
        const gchar *sender;
        const gchar *message;

        game_chat = gibbon_app_get_game_chat (self->priv->app);
        if (!game_chat)
                return -1;

        if (!gibbon_clip_get_string (&iter, GIBBON_CLIP_TYPE_NAME, &sender))
                return -1;

        if (!gibbon_clip_get_string (&iter, GIBBON_CLIP_TYPE_STRING, &message))
                return -1;

        fibs_message = gibbon_fibs_message_new (sender, message);

        gibbon_game_chat_append_message (game_chat, fibs_message);

        gibbon_fibs_message_free (fibs_message);

        return GIBBON_CLIP_CODE_WHISPERS;
}

static gint
gibbon_session_clip_kibitzes (GibbonSession *self, GSList *iter)
{
        GibbonFIBSMessage *fibs_message;
        GibbonGameChat *game_chat;
        const gchar *sender;
        const gchar *message;

        game_chat = gibbon_app_get_game_chat (self->priv->app);
        if (!game_chat)
                return -1;

        if (!gibbon_clip_get_string (&iter, GIBBON_CLIP_TYPE_NAME, &sender))
                return -1;

        if (!gibbon_clip_get_string (&iter, GIBBON_CLIP_TYPE_STRING, &message))
                return -1;

        fibs_message = gibbon_fibs_message_new (sender, message);

        gibbon_game_chat_append_message (game_chat, fibs_message);

        gibbon_fibs_message_free (fibs_message);

        return GIBBON_CLIP_CODE_KIBITZES;
}

static gint
gibbon_session_clip_you_say (GibbonSession *self, GSList *iter)
{
        GibbonFIBSMessage *fibs_message;
        GibbonConnection *connection;
        const gchar *sender;
        const gchar *receiver;
        const gchar *message;

        connection = gibbon_app_get_connection (self->priv->app);
        if (!connection)
                return -1;

        sender = gibbon_connection_get_login (connection);
        if (!sender)
                return -1;

        if (!gibbon_clip_get_string (&iter, GIBBON_CLIP_TYPE_NAME, &receiver))
                return -1;

        if (!gibbon_clip_get_string (&iter, GIBBON_CLIP_TYPE_STRING, &message))
                return -1;

        fibs_message = gibbon_fibs_message_new (sender, message);

        gibbon_app_show_message (self->priv->app,
                                 receiver,
                                 fibs_message);

        gibbon_fibs_message_free (fibs_message);

        return GIBBON_CLIP_CODE_YOU_SAY;
}

static gint
gibbon_session_clip_you_shout (GibbonSession *self, GSList *iter)
{
        GibbonFIBSMessage *fibs_message;
        GibbonConnection *connection;
        const gchar *sender;

        const gchar *message;

        connection = gibbon_app_get_connection (self->priv->app);
        if (!connection)
                return FALSE;

        if (!gibbon_clip_get_string (&iter, GIBBON_CLIP_TYPE_STRING, &message))
                return -1;

        sender = gibbon_connection_get_login (connection);
        fibs_message = gibbon_fibs_message_new (sender, message);

        gibbon_app_show_shout (self->priv->app, fibs_message);

        gibbon_fibs_message_free (fibs_message);

        return GIBBON_CLIP_CODE_YOU_SHOUT;
}

static gint
gibbon_session_clip_you_whisper (GibbonSession *self, GSList *iter)
{
        GibbonFIBSMessage *fibs_message;
        GibbonConnection *connection;
        const gchar *sender;
        const gchar *message;

        connection = gibbon_app_get_connection (self->priv->app);
        if (!connection)
                return -1;

        sender = gibbon_connection_get_login (connection);
        if (!sender)
                return -1;

        if (!gibbon_clip_get_string (&iter, GIBBON_CLIP_TYPE_STRING, &message))
                return -1;

        fibs_message = gibbon_fibs_message_new (sender, message);

        gibbon_app_show_game_chat (self->priv->app, fibs_message);

        gibbon_fibs_message_free (fibs_message);

        return GIBBON_CLIP_CODE_YOU_WHISPER;
}

static gint
gibbon_session_clip_you_kibitz (GibbonSession *self, GSList *iter)
{
        GibbonFIBSMessage *fibs_message;
        GibbonConnection *connection;
        const gchar *sender;
        const gchar *message;

        connection = gibbon_app_get_connection (self->priv->app);
        if (!connection)
                return -1;

        sender = gibbon_connection_get_login (connection);
        if (!sender)
                return -1;

        if (!gibbon_clip_get_string (&iter, GIBBON_CLIP_TYPE_STRING, &message))
                return -1;

        fibs_message = gibbon_fibs_message_new (sender, message);

        gibbon_app_show_game_chat (self->priv->app, fibs_message);

        gibbon_fibs_message_free (fibs_message);

        return GIBBON_CLIP_CODE_YOU_KIBITZ;
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
gibbon_session_handle_board (GibbonSession *self, GSList *iter)
{
        GibbonPosition *pos;
        GibbonBoard *board;
        gint i;
        GibbonConnection *connection;
        const gchar *str;
        gint retval = -1;

        pos = gibbon_position_new ();
        if (self->priv->position) {
                if (self->priv->position->game_info)
                        pos->game_info =
                                g_strdup (self->priv->position->game_info);
                if (self->priv->position->status)
                        pos->status = g_strdup (self->priv->position->status);
        }

        if (!gibbon_clip_get_string (&iter, GIBBON_CLIP_TYPE_NAME, &str))
                goto bail_out_board;

        if (g_strcmp0 ("You", str)) {
                pos->players[0] = g_strdup (str);
                g_free (self->priv->watching);
                self->priv->watching = g_strdup (pos->players[0]);
        } else {
                connection = gibbon_app_get_connection (self->priv->app);
                pos->players[0] =
                        g_strdup (gibbon_connection_get_login (connection));
                g_free (self->priv->watching);
                self->priv->watching = NULL;
        }
        
        if (!gibbon_clip_get_string (&iter, GIBBON_CLIP_TYPE_NAME,
                                     (const gchar **) &pos->players[1]))
                goto bail_out_board;

        if (g_strcmp0 (self->priv->opponent, pos->players[1])) {
                g_free (self->priv->opponent);
                self->priv->opponent = g_strdup (pos->players[1]);
        }
        
        if (!gibbon_clip_get_uint (&iter, GIBBON_CLIP_TYPE_UINT,
                                   &pos->match_length))
                goto bail_out_board;

        if (!gibbon_clip_get_uint (&iter, GIBBON_CLIP_TYPE_UINT,
                                   &pos->scores[0]))
                goto bail_out_board;
        if (!gibbon_clip_get_uint (&iter, GIBBON_CLIP_TYPE_UINT,
                                   &pos->scores[1]))
                goto bail_out_board;

        for (i = 0; i < 24; ++i) {
                if (!gibbon_clip_get_int (&iter, GIBBON_CLIP_TYPE_INT,
                                          &pos->points[i]))
                        goto bail_out_board;
                g_printerr (" %d", pos->points[i]);
        }
        g_printerr ("!\n");

        if (!gibbon_clip_get_int (&iter, GIBBON_CLIP_TYPE_INT,
                                  &pos->dice[0]))
                goto bail_out_board;
        if (!gibbon_clip_get_int (&iter, GIBBON_CLIP_TYPE_INT,
                                  &pos->dice[1]))
                goto bail_out_board;
        
        if (!gibbon_clip_get_uint (&iter, GIBBON_CLIP_TYPE_UINT,
                                   &pos->cube))
                goto bail_out_board;

        if (!gibbon_clip_get_boolean (&iter, GIBBON_CLIP_TYPE_BOOLEAN,
                                      &pos->may_double[0]))
                goto bail_out_board;
        if (!gibbon_clip_get_boolean (&iter, GIBBON_CLIP_TYPE_BOOLEAN,
                                      &pos->may_double[1]))
                goto bail_out_board;

        if (!gibbon_clip_get_uint (&iter, GIBBON_CLIP_TYPE_UINT,
                                   &pos->bar[0]))
                goto bail_out_board;
        if (!gibbon_clip_get_uint (&iter, GIBBON_CLIP_TYPE_UINT,
                                   &pos->bar[1]))
                goto bail_out_board;
        
        if (pos->match_length &&
            (pos->scores[0] == pos->match_length - 1
             || pos->scores[1] == pos->match_length - 1)
            && !pos->may_double[0]
            && !pos->may_double[1]) {
                pos->game_info = g_strdup (_("Crawford game"));
        }

        if (gibbon_position_equals_technically (pos, self->priv->position)) {
                g_printerr ("Session positions are equal.\n");
        } else {
                g_printerr ("Session positions are not equal.\n");
                if (self->priv->position)
                        gibbon_position_free (self->priv->position);
                self->priv->position = gibbon_position_copy (pos);
        }

        board = gibbon_app_get_board (self->priv->app);
        if (gibbon_position_equals_technically (pos,
                                           gibbon_board_get_position (board))) {
                gibbon_position_free (pos);
        } else {
                gibbon_board_set_position (GIBBON_BOARD (board), pos);
        }

#ifdef GIBBON_SESSION_DEBUG_BOARD_STATE
        gibbon_session_dump_position (self, self->priv->position);
#endif

        return GIBBON_CLIP_CODE_BOARD;

bail_out_board:
        gibbon_position_free (pos);

        return retval;
}

static gboolean
gibbon_session_handle_bad_board (GibbonSession *self, GSList *iter)
{
        const gchar *board;
        const gchar *other;

        if (!gibbon_clip_get_string (&iter, GIBBON_CLIP_TYPE_STRING, &board))
                return -1;
        if (!gibbon_clip_get_string (&iter, GIBBON_CLIP_TYPE_STRING, &other))
                return -1;

        if (0 > gibbon_session_process_server_line (self, board))
                return -1;
        if (0 > gibbon_session_process_server_line (self, other))
                return -1;

        return GIBBON_CLIP_CODE_BAD_BOARD;
}

static gint
gibbon_session_handle_youre_watching (GibbonSession *self, GSList *iter)
{
        const gchar *player;

        if (!gibbon_clip_get_string (&iter, GIBBON_CLIP_TYPE_NAME, &player))
                return -1;

        g_free (self->priv->watching);
        self->priv->watching = g_strdup (player);
        g_free (self->priv->opponent);
        self->priv->opponent = NULL;

        return TRUE;
}

static gchar *
gibbon_session_decode_client (GibbonSession *self, const gchar *client)
{
        gchar *retval;
        gchar *underscore;

        if (client[0] >= 60 && client[0] <= 63
            && 20 == strlen (client)
            && client[1] >= 'A'
            && client[1] <= 'Z'
            && client[2] >= 'A'
            && client[2] <= 'Z') {
                if (client[19] >= 33 && client[19] <= 39)
                        return g_strdup ("JavaFIBS");
                else if (!(strcmp ("=NTTourney1rganizer2", client)))
                        return g_strdup ("TourneyBot");
                else
                        return g_strdup (client);
        } else {
                retval = g_strdup (client);
                underscore = index (retval, '_');
                if (underscore && underscore != client)
                        *underscore = ' ';
                return retval;
        }
}

#ifdef GIBBON_SESSION_DEBUG_BOARD_STATE
static void
gibbon_session_dump_position (const GibbonSession *self,
                              const GibbonPosition *pos)
{
        gint i;

        g_printerr ("=== Position ===\n");
        g_printerr ("Opponent: %s, %d/%d points, %u pips\n",
                    pos->players[1], pos->scores[1], pos->match_length,
                    gibbon_position_get_pip_count (pos,
                                                   GIBBON_POSITION_SIDE_BLACK));
        g_printerr ("\
  +-13-14-15-16-17-18-------19-20-21-22-23-24-+ negative: black or X\n");
        g_printerr ("  |");
        for (i = 12; i < 18; ++i)
                if (pos->points[i])
                        g_printerr ("%+3d", pos->points[i]);
                else
                        g_printerr ("%s", "   ");
        g_printerr (" |%+3d|", pos->bar[1]);
        for (i = 18; i < 24; ++i)
                if (pos->points[i])
                        g_printerr ("%+3d", pos->points[i]);
                else
                        g_printerr ("%s", "   ");
        g_printerr (" | May double: %s\n", pos->may_double[1] ? "yes" : "no");
        g_printerr (" v| dice: %+d : %+d     ",
                    pos->dice[0], pos->dice[1]);
        g_printerr ("|BAR|                   | ");
        g_printerr (" Cube: %d\n", pos->cube);
        g_printerr ("  |");
        for (i = 11; i >= 6; --i)
                if (pos->points[i])
                        g_printerr ("%+3d", pos->points[i]);
                else
                        g_printerr ("%s", "   ");
        g_printerr (" |%+3d|", pos->bar[0]);
        for (i = 5; i >= 0; --i)
                if (pos->points[i])
                        g_printerr ("%+3d", pos->points[i]);
                else
                        g_printerr ("%s", "   ");
        g_printerr (" | May double: %s\n", pos->may_double[0] ? "yes" : "no");
        g_printerr ("\
  +-12-11-10--9--8--7--------6--5--4--3--2--1-+ positive: white or O\n");
        g_printerr ("Player: %s, %d/%d points, %u pips\n",
                    pos->players[0], pos->scores[0], pos->match_length,
                    gibbon_position_get_pip_count (pos,
                                                   GIBBON_POSITION_SIDE_WHITE));
        g_printerr ("Game info: %s\n", pos->game_info);
        g_printerr ("Status: %s\n", pos->status);
}
#endif

static gboolean
gibbon_session_handle_win_match (GibbonSession *self, GSList *iter)
{
        const gchar *hostname;
        guint port;
        const gchar *player1;
        const gchar *player2;

        if (!gibbon_clip_get_string (&iter, GIBBON_CLIP_TYPE_NAME, &player1))
                return -1;
        if (!gibbon_clip_get_string (&iter, GIBBON_CLIP_TYPE_NAME, &player2))
                return -1;

        hostname = gibbon_connection_get_hostname (self->priv->connection);
        port = gibbon_connection_get_port (self->priv->connection);

        gibbon_archive_save_win (self->priv->archive, hostname, port,
                                 player1, player2);

        return GIBBON_CLIP_CODE_WIN_MATCH;
}

static gboolean
gibbon_session_handle_resume_match (GibbonSession *self, GSList *iter)
{
        const gchar *hostname;
        guint port;
        const gchar *player1;
        const gchar *player2;

        if (!gibbon_clip_get_string (&iter, GIBBON_CLIP_TYPE_NAME, &player1))
                return -1;
        if (!gibbon_clip_get_string (&iter, GIBBON_CLIP_TYPE_NAME, &player2))
                return -1;

        hostname = gibbon_connection_get_hostname (self->priv->connection);
        port = gibbon_connection_get_port (self->priv->connection);

        gibbon_archive_save_resume (self->priv->archive, hostname, port,
                                    player1, player2);

        return GIBBON_CLIP_CODE_RESUME_MATCH;
}

static gboolean
gibbon_session_handle_rolls (GibbonSession *self, GSList *iter)
{
        const gchar *who;
        guint dice[2];

        if (!gibbon_clip_get_string (&iter, GIBBON_CLIP_TYPE_NAME, &who))
                return -1;

        if (!gibbon_clip_get_uint (&iter, GIBBON_CLIP_TYPE_UINT, &dice[0]))
                return -1;
        if (!gibbon_clip_get_uint (&iter, GIBBON_CLIP_TYPE_UINT, &dice[1]))
                return -1;

        if (0 == g_strcmp0 ("You", who)) {
                self->priv->position->dice[0] = dice[0];
                self->priv->position->dice[1] = dice[1];
                self->priv->position->status =
                        g_strdup_printf (_("You roll %u and %u."),
                                         dice[0], dice[1]);
        } else if (0 == g_strcmp0 (self->priv->opponent, who)) {
                self->priv->position->dice[0] = -dice[0];
                self->priv->position->dice[1] = -dice[1];
                g_free (self->priv->position->game_info);
                self->priv->position->status =
                                g_strdup_printf (_("%s rolls %u and %u."),
                                                 self->priv->opponent,
                                                 dice[0], dice[1]);
        } else if (0 == g_strcmp0 (self->priv->watching, who)) {
                self->priv->position->dice[0] = dice[0];
                self->priv->position->dice[1] = dice[1];
                self->priv->position->status =
                        g_strdup_printf (_("%s rolls %u and %u."),
                                         self->priv->watching,
                                         dice[0], dice[1]);
        } else {
                return -1;
        }

        gibbon_board_set_position (gibbon_app_get_board (self->priv->app),
                                   gibbon_position_copy (self->priv->position));

        return GIBBON_CLIP_CODE_ROLLS;
}

static gboolean
gibbon_session_handle_moves (GibbonSession *self, GSList *iter)
{
        GibbonMove *move;
        GibbonMovement *movement;
        gint i;
        GibbonPositionSide side;
        gchar *pretty_move;
        gint *dice;
        const gchar *player;
        guint num_moves;

        if (!gibbon_clip_get_string (&iter, GIBBON_CLIP_TYPE_STRING, &player))
                return -1;

        if (!gibbon_clip_get_uint (&iter, GIBBON_CLIP_TYPE_UINT, &num_moves))
                return -1;

        if (g_strcmp0 (player, self->priv->opponent))
                side = GIBBON_POSITION_SIDE_WHITE;
        else
                side = GIBBON_POSITION_SIDE_BLACK;

        move = g_alloca (sizeof move->number
                         + num_moves * sizeof *move->movements
                         + sizeof move->status);
        move->number = 0;

        for (i = 0; i < num_moves; ++i) {
                movement = move->movements + move->number++;
                if (!gibbon_clip_get_int (&iter, GIBBON_CLIP_TYPE_UINT,
                                          &movement->from))
                        return -1;
                if (!gibbon_clip_get_int (&iter, GIBBON_CLIP_TYPE_UINT,
                                          &movement->from))
                        return -1;
        }

        pretty_move = gibbon_position_format_move (self->priv->position, move,
                                                   side, FALSE);
        g_free (self->priv->position->status);
        self->priv->position->status = NULL;
        dice = self->priv->position->dice;

        if (0 == g_strcmp0 (self->priv->opponent, player)) {
                self->priv->position->status =
                                g_strdup_printf (_("%d%d: %s moves %s."),
                                                 abs (dice[0]),
                                                 abs (dice[1]),
                                                 self->priv->opponent,
                                                 pretty_move);
        } else if (0 == g_strcmp0 ("You", player)) {
                self->priv->position->status =
                                g_strdup_printf (_("%d%d: You move %s."),
                                                 abs (dice[0]),
                                                 abs (dice[1]),
                                                 pretty_move);
        } else if (0 == g_strcmp0 (self->priv->watching, player)) {
                self->priv->position->status =
                                g_strdup_printf (_("%d%d: %s moves %s."),
                                                abs (dice[0]),
                                                abs (dice[1]),
                                                self->priv->watching,
                                                pretty_move);
        } else {
                return -1;
        }

        if (!gibbon_position_apply_move (self->priv->position, move,
                                         side, FALSE)) {
                g_critical ("Error applying move %s to position.",
                            pretty_move);
                g_critical ("Parsed numbers:");
                for (i = 0; i < move->number; ++i) {
                        g_critical ("    %d/%d",
                                    move->movements[i].from,
                                    move->movements[i].to);
                }
                if (side == GIBBON_POSITION_SIDE_WHITE)
                        g_critical ("White on move.");
                else
                        g_critical ("Black on move.");
                gibbon_position_free (self->priv->position);
                self->priv->position = gibbon_position_new ();
                self->priv->position->status =
                    g_strdup_printf (_("Error applying move %s to position.\n"),
                                     pretty_move);
                g_free (pretty_move);
                return FALSE;
        }

        g_free (pretty_move);

        self->priv->position->dice[0] = 0;
        self->priv->position->dice[1] = 0;
        gibbon_board_set_position (gibbon_app_get_board (self->priv->app),
                                   gibbon_position_copy (self->priv->position));

        return GIBBON_CLIP_CODE_MOVES;
}

static gboolean
gibbon_session_handle_invitation (GibbonSession *self, GSList *iter)
{
        const gchar *opponent;
        guint length;
        GtkListStore *store;
        GtkTreeIter tree_iter;
        GibbonPlayerList *pl;
        gdouble rating = 1500.0;
        guint experience = 0;
        GibbonReliability *rel = NULL;
        gdouble reliability = 0;
        guint confidence = 0;
        gchar *client = NULL;
        gchar *hostname = NULL;
        gchar *email = NULL;

        if (!gibbon_clip_get_string (&iter, GIBBON_CLIP_TYPE_NAME,
                                     &opponent))
                return -1;

        if (!gibbon_clip_get_uint (&iter, GIBBON_CLIP_TYPE_UINT,
                                   &length))
                return -1;

        pl = self->priv->player_list;
        store = gibbon_player_list_get_store (pl);

        g_return_val_if_fail (store != NULL, -1);

        if (gibbon_player_list_get_iter (pl, opponent, &tree_iter)) {
                gtk_tree_model_get (GTK_TREE_MODEL (store), &tree_iter,
                                    GIBBON_PLAYER_LIST_COL_RATING, &rating,
                                    GIBBON_PLAYER_LIST_COL_EXPERIENCE,
                                            &experience,
                                    GIBBON_PLAYER_LIST_COL_RELIABILITY,
                                            &rel,
                                    GIBBON_PLAYER_LIST_COL_CLIENT, &client,
                                    GIBBON_PLAYER_LIST_COL_HOSTNAME, &hostname,
                                    GIBBON_PLAYER_LIST_COL_EMAIL, &email,
                                    -1);
                if (rel) {
                        reliability = rel->value;
                        confidence = rel->confidence;
                        gibbon_reliability_free (rel);
                }

        } else {
                /*
                 * FIBS never sent a who info for that user.  Try to force
                 * the information about the inviter.
                 */
                gibbon_connection_queue_command (self->priv->connection,
                                                 FALSE,
                                                 "rawwho %s", opponent);
        }

        gibbon_inviter_list_set (self->priv->inviter_list,
                                 opponent,
                                 rating,
                                 experience,
                                 reliability,
                                 confidence,
                                 client,
                                 hostname,
                                 email);

        g_free (client);
        g_free (hostname);
        g_free (email);

        /* Get the saved count.  */
        gibbon_connection_queue_command (self->priv->connection,
                                         FALSE,
                                         "show savedcount %s", opponent);

        return GIBBON_CLIP_CODE_INVITATION;
}

static gint
gibbon_session_handle_show_setting (GibbonSession *self, GSList *iter)
{
        gint retval = -1;
        const gchar *key;
        const gchar *value;

        if (!gibbon_clip_get_string (&iter, GIBBON_CLIP_TYPE_STRING, &key))
                return -1;
        if (!gibbon_clip_get_string (&iter, GIBBON_CLIP_TYPE_STRING, &value))
                return -1;

        if (gibbon_session_clear_expect_list (self,
                                              &self->priv->expect_settings,
                                              key))
                retval = GIBBON_CLIP_CODE_SHOW_SETTING;

        /* The only setting we are interested in is "boardstyle".  */
        if (0 == g_strcmp0 ("boardstyle", key)
            && (value[0] != '3' || value[1])) {
                /*
                 * Somebody changed the boardstyle.  This is assumed an error.
                 * We will therefore completely show the communication with
                 * FIBS and neither hide the command sent, nor will we
                 * try to hide the reply.
                 */
                gibbon_connection_queue_command (self->priv->connection,
                                                 TRUE,
                                                 "set boardstyle 3");
        }

        return retval;
}

static gint
gibbon_session_handle_show_toggle (GibbonSession *self, GSList *iter)
{
        gint retval = -1;
        const gchar *key;
        gboolean value;

        if (!gibbon_clip_get_string (&iter, GIBBON_CLIP_TYPE_STRING, &key))
                return -1;
        if (!gibbon_clip_get_boolean (&iter, GIBBON_CLIP_TYPE_BOOLEAN, &value))
                return -1;

        if (gibbon_session_clear_expect_list (self,
                                              &self->priv->expect_toggles,
                                              key))
                retval = GIBBON_CLIP_CODE_SHOW_TOGGLE;

        if (0 == g_strcmp0 ("notify", key)) {
                if (!value) {
                        gibbon_connection_queue_command (self->priv->connection,
                                                         TRUE,
                                                         "toggle notify");
                }
        }

        return retval;
}

static gint
gibbon_session_handle_show_saved (GibbonSession *self, GSList *iter)
{
        const gchar *opponent;
        guint match_length, scores[2];
        GibbonSavedInfo *info;

        if  (self->priv->saved_games_finished)
                return -1;

        if (!gibbon_clip_get_string (&iter, GIBBON_CLIP_TYPE_NAME, &opponent))
                return -1;
        if (!gibbon_clip_get_uint (&iter, GIBBON_CLIP_TYPE_UINT, &match_length))
                return -1;
        if (!gibbon_clip_get_uint (&iter, GIBBON_CLIP_TYPE_UINT, &scores[0]))
                return -1;
        if (!gibbon_clip_get_uint (&iter, GIBBON_CLIP_TYPE_UINT, &scores[1]))
                return -1;

        info = gibbon_saved_info_new (opponent, match_length,
                                      scores[0], scores[1]);

        g_hash_table_insert (self->priv->saved_games,
                             (gpointer) opponent, (gpointer) info);

        return GIBBON_CLIP_CODE_SHOW_SAVED;
}

static gint
gibbon_session_handle_show_saved_count (GibbonSession *self, GSList *iter)
{
        const gchar *who;
        guint count;

        if (!gibbon_clip_get_string (&iter, GIBBON_CLIP_TYPE_NAME, &who))
                return -1;

        /*
         * If the user is not an active inviter we guess that the command
         * was sent manually.
         */
        if (!gibbon_inviter_list_exists (self->priv->inviter_list, who))
                return -1;

        /*
         * Same, when we know that user's saved count already.
         */
        if (gibbon_inviter_list_get_saved_count (self->priv->inviter_list,
                                                 who) >= 0)
                return -1;

        if (!gibbon_clip_get_uint (&iter, GIBBON_CLIP_TYPE_UINT, &count))
                return -1;

        gibbon_inviter_list_set_saved_count (self->priv->inviter_list, who,
                                             count);

        return GIBBON_CLIP_CODE_SHOW_SAVED_COUNT;
}

static gboolean
gibbon_session_clear_expect_list (GibbonSession *self, GSList **list,
                                  const gchar *key)
{
        GSList *iter = *list;

        while (iter) {
                if (0 == g_strcmp0 (iter->data, key)) {
                        *list = g_slist_remove (*list, iter->data);
                        return TRUE;
                }
                iter = iter->next;
        }

        return FALSE;
}

void
gibbon_session_configure_player_menu (const GibbonSession *self,
                                      const gchar *player,
                                      GtkMenu *menu)
{
        gboolean is_self;
        gboolean am_playing;
        GObject *item;
        gboolean sensitive;
        gboolean is_occupied = FALSE;

        g_return_if_fail (GIBBON_IS_SESSION (self));
        g_return_if_fail (player != NULL);
        g_return_if_fail (GTK_IS_MENU (menu));

        if (g_strcmp0 (player,
                        gibbon_connection_get_login (self->priv->connection)))
                is_self = FALSE;
        else
                is_self = TRUE;

        am_playing = self->priv->opponent && !self->priv->watching;

        if (!is_self
            && !gibbon_player_list_get_available (self->priv->player_list,
                                                  player))
                        is_occupied = TRUE;

        item = gibbon_app_find_object (self->priv->app,
                                       "invite_player_menu_item",
                                       GTK_TYPE_MENU_ITEM);
        if (is_self || am_playing || is_occupied)
                sensitive = FALSE;
        else
                sensitive = TRUE;
        gtk_widget_set_sensitive (GTK_WIDGET (item), sensitive);

        item = gibbon_app_find_object (self->priv->app,
                                       "look_player_menu_item",
                                        GTK_TYPE_MENU_ITEM);
        if (is_self)
                sensitive = FALSE;
        else
                sensitive = TRUE;
        gtk_widget_set_sensitive (GTK_WIDGET (item), sensitive);

        item = gibbon_app_find_object (self->priv->app,
                                      "watch_player_menu_item",
                                      GTK_TYPE_MENU_ITEM);
        if (is_self || am_playing)
                sensitive = FALSE;
        else
                sensitive = TRUE;
        gtk_widget_set_sensitive (GTK_WIDGET (item), sensitive);
        if (self->priv->watching
            && 0 == g_strcmp0 (player, self->priv->watching)) {
                gtk_menu_item_set_label (GTK_MENU_ITEM (item), _("Stop"
                                                                 " watching"));
        } else {
                gtk_menu_item_set_label (GTK_MENU_ITEM (item), _("Watch"));
        }

        item = gibbon_app_find_object (self->priv->app,
                                       "tell-player-menu-item",
                                       GTK_TYPE_MENU_ITEM);
        /*
         * We do not let the user select the "chat" menu item for the
         * current opponent.  The game chat entry is already open for that
         * purpose.
         */
        if (is_self
            || (am_playing && 0 == g_strcmp0 (player, self->priv->opponent)))
                sensitive = FALSE;
        else
                sensitive = TRUE;
        gtk_widget_set_sensitive (GTK_WIDGET (item), sensitive);
}

const gchar * const
gibbon_session_get_watching (const GibbonSession *self)
{
        g_return_val_if_fail (GIBBON_IS_SESSION (self), NULL);

        return self->priv->watching;
}

const GibbonSavedInfo const *
gibbon_session_get_saved (const GibbonSession *self, const gchar *who)
{
        g_return_val_if_fail (GIBBON_IS_SESSION (self), NULL);
        g_return_val_if_fail (who != NULL, NULL);

        return g_hash_table_lookup (self->priv->saved_games, who);
}
