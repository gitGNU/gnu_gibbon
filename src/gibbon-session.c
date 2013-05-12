/*
 * This file is part of Gibbon, a graphical frontend to the First Internet 
 * Backgammon Server FIBS.
 * Copyright (C) 2009-2012 Guido Flohr, http://guido-flohr.net/.
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
#include <string.h>
#include <stdlib.h>

#include "gibbon-connection.h"
#include "gibbon-session.h"
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
#include "gibbon-clip-reader.h"
#include "gibbon-saved-info.h"
#include "gibbon-reliability.h"
#include "gibbon-client-icons.h"
#include "gibbon-country.h"
#include "gibbon-settings.h"
#include "gibbon-match-tracker.h"

typedef enum {
        GIBBON_SESSION_PLAYER_YOU = 0,
        GIBBON_SESSION_PLAYER_WATCHING = 1,
        GIBBON_SESSION_PLAYER_OPPONENT = 2,
        GIBBON_SESSION_PLAYER_OTHER = 3
} GibbonSessionPlayer;

typedef enum {
        GIBBON_SESSION_REGISTER_WAIT_INIT = 0,
        GIBBON_SESSION_REGISTER_WAIT_PROMPT = 1,
        GIBBON_SESSION_REGISTER_WAIT_PASSWORD_PROMPT = 2,
        GIBBON_SESSION_REGISTER_WAIT_PASSWORD2_PROMPT = 3,
        GIBBON_SESSION_REGISTER_WAIT_CONFIRMATION = 4
} GibbonSessionRegisterState;

struct GibbonSessionSavedCountCallbackInfo {
        GibbonSessionCallback callback;
        gchar *who;
        GObject *object;
        gpointer data;
};

#define GIBBON_SESSION_REPLY_TIMEOUT 2500

static gint gibbon_session_clip_welcome (GibbonSession *self, GSList *iter);
static gint gibbon_session_clip_own_info (GibbonSession *self, GSList *iter);
static gint gibbon_session_clip_who_info (GibbonSession *self, GSList *iter);
static gint gibbon_session_clip_who_info_end (GibbonSession *self,
                                              GSList *iter);
static gint gibbon_session_clip_login (GibbonSession *self, GSList *iter);
static gint gibbon_session_clip_logout (GibbonSession *self, GSList *iter);
static gint gibbon_session_clip_message (GibbonSession *self, GSList *iter);
static gint gibbon_session_clip_message_delivered (GibbonSession *self,
                                                   GSList *iter);
static gint gibbon_session_clip_message_saved (GibbonSession *self,
                                               GSList *iter);
static gint gibbon_session_clip_says (GibbonSession *self, GSList *iter);
static gint gibbon_session_clip_alerts (GibbonSession *self, GSList *iter);
static gint gibbon_session_clip_shouts (GibbonSession *self, GSList *iter);
static gint gibbon_session_clip_whispers (GibbonSession *self, GSList *iter);
static gint gibbon_session_clip_kibitzes (GibbonSession *self, GSList *iter);
static gint gibbon_session_clip_you_say (GibbonSession *self, GSList *iter);
static gint gibbon_session_clip_you_shout (GibbonSession *self, GSList *iter);
static gint gibbon_session_clip_you_whisper (GibbonSession *self, GSList *iter);
static gint gibbon_session_clip_you_kibitz (GibbonSession *self, GSList *iter);
static gint gibbon_session_handle_error (GibbonSession *self, GSList *iter);
static gint gibbon_session_handle_no_such_user (GibbonSession *self,
                                                GSList *iter);
static gint gibbon_session_handle_board (GibbonSession *self, GSList *iter);
static gint gibbon_session_handle_rolls (GibbonSession *self, GSList *iter);
static gint gibbon_session_handle_moves (GibbonSession *self, GSList *iter);
static gint gibbon_session_handle_invitation (GibbonSession *self,
                                              GSList *iter);
static gint gibbon_session_handle_youre_watching (GibbonSession *self,
                                                  GSList *iter);
static gint gibbon_session_handle_now_playing (GibbonSession *self,
                                               GSList *iter);
static gint gibbon_session_handle_invite_error (GibbonSession *self,
                                                GSList *iter);
static gint gibbon_session_handle_resume (GibbonSession *self, GSList *iter);
static gint gibbon_session_handle_win_match (GibbonSession *self,
                                             GSList *iter);
static gint gibbon_session_handle_async_win_match (GibbonSession *self,
                                                   GSList *iter);
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
static gint gibbon_session_handle_show_address (GibbonSession *self,
                                                GSList *iter);
static gint gibbon_session_handle_invalid_address (GibbonSession *self,
                                                   GSList *iter);
static gint gibbon_session_handle_left_game (GibbonSession *self,
                                             GSList *iter);
static gint gibbon_session_handle_cannot_move (GibbonSession *self,
                                               GSList *iter);
static gint gibbon_session_handle_doubles (GibbonSession *self, GSList *iter);
static gint gibbon_session_handle_accepts_double (GibbonSession *self,
                                                  GSList *iter);
static gint gibbon_session_handle_resigns (GibbonSession *self, GSList *iter);
static gint gibbon_session_handle_rejects (GibbonSession *self, GSList *iter);
static gint gibbon_session_handle_win_game (GibbonSession *self, GSList *iter);

static gchar *gibbon_session_decode_client (GibbonSession *self,
                                            const gchar *token);
static void gibbon_session_on_geo_ip_resolve (GibbonSession *self,
                                              const gchar *hostname,
                                              const GibbonCountry *country);
static gboolean gibbon_session_timeout (GibbonSession *self);
static void gibbon_session_registration_error (GibbonSession *self,
                                                 const gchar *msg);
static void gibbon_session_registration_success (GibbonSession *self);
static void gibbon_session_on_dice_picked_up (const GibbonSession *self);
static void gibbon_session_on_cube_turned (const GibbonSession *self);
static void gibbon_session_on_cube_taken (const GibbonSession *self);
static void gibbon_session_on_cube_dropped (const GibbonSession *self);
static void gibbon_session_on_resignation_accepted (const GibbonSession *self);
static void gibbon_session_on_resignation_rejected (const GibbonSession *self);

static void gibbon_session_check_expect_queues (GibbonSession *self,
                                                gboolean force);
static void gibbon_session_queue_who_request (GibbonSession *self,
                                              const gchar *who);
static void gibbon_session_unqueue_who_request (GibbonSession *self,
                                                const gchar *who);
static void gibbon_session_start_playing (GibbonSession *self,
                                          const gchar *player1,
                                          const gchar *player2,
                                          guint length,
                                          gboolean resumption);
static void gibbon_session_stop_playing (GibbonSession *self);
static void gibbon_session_clean_saved (const GibbonSession *self);
static void gibbon_session_update_tracker (GibbonSession *self);
static void gibbon_session_check_address (GibbonSession *self,
                                          const gchar *remote_address);
static void gibbon_session_auto_swap_dice (const GibbonSession *self,
                                           GibbonPosition *pos);

struct _GibbonSessionPrivate {
        GibbonApp *app;
        GibbonConnection *connection;
        GibbonCLIPReader *clip_reader;

        gchar *watching;
        gchar *opponent;
        gboolean available;

        gboolean rating_seen;
        gdouble rating;
        guint64 experience;

        /* This is the approved position.   It is the result of a "board"
         * message, or of a move, roll, or other game play action made.
         */
        GibbonPosition *position;
        gboolean direction;

        GibbonPlayerList *player_list;
        GibbonInviterList *inviter_list;

        GibbonMatchTracker *tracker;
        GibbonArchive *archive;

        gboolean guest_login;
        GibbonSessionRegisterState rstate;

        gboolean initialized;

        gboolean expect_boardstyle;
        gboolean set_boardstyle;
        gboolean expect_saved;
        gboolean saved_finished;
        gboolean expect_notify;
        gboolean expect_autoboard;
        GSList *expect_who_infos;
        GSList *expect_saved_counts;
        gboolean expect_address;

        gboolean address_checked;

        guint timeout_id;

        GHashTable *saved_games;

        guint dice_picked_up_handler;
        guint cube_turned_handler;
        guint cube_taken_handler;
        guint cube_dropped_handler;
        guint resignation_accepted_handler;
        guint resignation_rejected_handler;

        gboolean debug_board_state;
};

#define GIBBON_SESSION_PRIVATE(o) (G_TYPE_INSTANCE_GET_PRIVATE ((o), \
                                      GIBBON_TYPE_SESSION,           \
                                      GibbonSessionPrivate))
G_DEFINE_TYPE (GibbonSession, gibbon_session, G_TYPE_OBJECT);

static void
gibbon_session_init (GibbonSession *self)
{
        self->priv = G_TYPE_INSTANCE_GET_PRIVATE (self, 
                                                  GIBBON_TYPE_SESSION, 
                                                  GibbonSessionPrivate);

        self->priv->connection = NULL;
        self->priv->clip_reader = NULL;
        self->priv->watching = NULL;
        self->priv->opponent = NULL;
        self->priv->available = FALSE;
        self->priv->rating_seen = FALSE;
        self->priv->position = NULL;
        self->priv->direction = FALSE;
        self->priv->player_list = NULL;
        self->priv->inviter_list = NULL;
        self->priv->archive = NULL;
        self->priv->tracker = NULL;

        self->priv->guest_login = FALSE;
        self->priv->rstate = GIBBON_SESSION_REGISTER_WAIT_INIT;

        self->priv->initialized = FALSE;

        self->priv->expect_boardstyle = FALSE;
        self->priv->set_boardstyle = FALSE;
        self->priv->expect_saved = FALSE;
        self->priv->saved_finished = FALSE;
        self->priv->expect_notify = FALSE;
        self->priv->expect_autoboard = FALSE;
        self->priv->expect_who_infos = NULL;
        self->priv->expect_saved_counts = NULL;
        self->priv->expect_address = FALSE;

        self->priv->address_checked = FALSE;

        self->priv->saved_games =
                g_hash_table_new_full (g_str_hash,
                                       g_str_equal,
                                       g_free,
                                       (GDestroyNotify) gibbon_saved_info_free);

        self->priv->dice_picked_up_handler = 0;
        self->priv->cube_turned_handler = 0;
        self->priv->cube_taken_handler = 0;
        self->priv->cube_dropped_handler = 0;
        self->priv->resignation_accepted_handler = 0;
        self->priv->resignation_rejected_handler = 0;

        self->priv->debug_board_state = FALSE;
}

static void
gibbon_session_finalize (GObject *object)
{
        GibbonSession *self = GIBBON_SESSION (object);
        struct GibbonSessionSavedCountCallbackInfo *info;
        GSList *iter;
        const gchar *hostname;
        const gchar *login;
        guint port;

        G_OBJECT_CLASS (gibbon_session_parent_class)->finalize (object);

        /* We do this as soon as possible to prevent data loss.  */
        if (self->priv->tracker)
                g_object_unref (self->priv->tracker);

        if (self->priv->opponent) {
                hostname = gibbon_connection_get_hostname (self->priv->connection);
                port = gibbon_connection_get_port (self->priv->connection);
                login = gibbon_connection_get_login (self->priv->connection);
                gibbon_archive_save_drop (self->priv->archive, hostname, port,
                                          login, self->priv->opponent);
        }

        if (self->priv->dice_picked_up_handler)
                g_signal_handler_disconnect (gibbon_app_get_board (
                                             self->priv->app),
                                            self->priv->dice_picked_up_handler);
        if (self->priv->cube_turned_handler)
                g_signal_handler_disconnect (gibbon_app_get_board (
                                             self->priv->app),
                                             self->priv->cube_turned_handler);
        if (self->priv->cube_taken_handler)
                g_signal_handler_disconnect (gibbon_app_get_board (
                                             self->priv->app),
                                             self->priv->cube_taken_handler);
        if (self->priv->cube_dropped_handler)
                g_signal_handler_disconnect (gibbon_app_get_board (
                                             self->priv->app),
                                             self->priv->cube_dropped_handler);
        if (self->priv->resignation_accepted_handler)
                g_signal_handler_disconnect (gibbon_app_get_board (
                                             self->priv->app),
                                      self->priv->resignation_accepted_handler);
        if (self->priv->resignation_rejected_handler)
                g_signal_handler_disconnect (gibbon_app_get_board (
                                             self->priv->app),
                                      self->priv->resignation_rejected_handler);

        if (self->priv->timeout_id)
                g_source_remove (self->priv->timeout_id);

        if (self->priv->watching)
                g_free (self->priv->watching);

        if (self->priv->opponent)
                g_free (self->priv->opponent);

        if (self->priv->position)
                gibbon_position_free (self->priv->position);

        iter = self->priv->expect_who_infos;
        while (iter) {
                g_free (iter->data);
                iter = iter->next;
        }
        g_slist_free (self->priv->expect_who_infos);

        iter = self->priv->expect_saved_counts;
        while (iter) {
                info = (struct GibbonSessionSavedCountCallbackInfo *) iter->data;
                g_free (info->who);
                g_free (info);
                iter = iter->next;
        }
        g_slist_free (self->priv->expect_saved_counts);

        if (self->priv->saved_games)
                g_hash_table_destroy (self->priv->saved_games);

        if (self->priv->clip_reader)
                g_object_unref (self->priv->clip_reader);
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
        GibbonBoard *board;
        const gchar *hostname;
        guint port;
        const gchar *login;
        GError *error = NULL;

        self->priv->connection = connection;
        self->priv->clip_reader = gibbon_clip_reader_new ();
        self->priv->app = app;

        self->priv->debug_board_state = gibbon_debug ("board-state");

        if (self->priv->available) {
                gibbon_app_set_state_available (app);
        } else {
                gibbon_app_set_state_busy (app);
        }
        self->priv->player_list = gibbon_app_get_player_list (app);
        self->priv->inviter_list = gibbon_app_get_inviter_list (app);

        self->priv->archive = gibbon_app_get_archive (app);

        login = gibbon_connection_get_login (connection);
        hostname = gibbon_connection_get_hostname (connection);
        port = gibbon_connection_get_port (connection);

        if (!g_strcmp0 ("guest", login)) {
                self->priv->guest_login = TRUE;
        } else {
                if (gibbon_archive_get_rank (self->priv->archive,
                                             hostname, port, login,
                                             &self->priv->rating,
                                             &self->priv->experience,
                                             &error)) {
                        self->priv->rating_seen = TRUE;
                } else {
                        gibbon_app_display_error (app, _("Database Error"),
                                                  "%s", error->message);
                        g_error_free (error);
                        error = NULL;
                }
        }

        self->priv->position = gibbon_position_new ();


        board = gibbon_app_get_board (self->priv->app);
        self->priv->dice_picked_up_handler =
                g_signal_connect_swapped (G_OBJECT (board), "dice-picked-up",
                                  G_CALLBACK (gibbon_session_on_dice_picked_up),
                                          G_OBJECT (self));
        self->priv->cube_turned_handler =
                g_signal_connect_swapped (G_OBJECT (board), "cube-turned",
                                  G_CALLBACK (gibbon_session_on_cube_turned),
                                          G_OBJECT (self));
        self->priv->cube_taken_handler =
                g_signal_connect_swapped (G_OBJECT (board), "cube-taken",
                                  G_CALLBACK (gibbon_session_on_cube_taken),
                                          G_OBJECT (self));
        self->priv->cube_dropped_handler =
                g_signal_connect_swapped (G_OBJECT (board), "cube-dropped",
                                  G_CALLBACK (gibbon_session_on_cube_dropped),
                                          G_OBJECT (self));
        self->priv->resignation_accepted_handler =
             g_signal_connect_swapped (G_OBJECT (board), "resignation-accepted",
                            G_CALLBACK (gibbon_session_on_resignation_accepted),
                                          G_OBJECT (self));
        self->priv->resignation_rejected_handler =
             g_signal_connect_swapped (G_OBJECT (board), "resignation-rejected",
                            G_CALLBACK (gibbon_session_on_resignation_rejected),
                                          G_OBJECT (self));

        return self;
}

gint
gibbon_session_process_server_line (GibbonSession *self,
                                    const gchar *line)
{
        gint retval = -1;
        GSList *values, *iter;
        enum GibbonClipCode code;
        GTimeVal timeval;
        struct tm *now;

        g_return_val_if_fail (GIBBON_IS_SESSION (self), -1);
        g_return_val_if_fail (line != NULL, -1);

        if (self->priv->guest_login) {
                /* Ignore empty lines.  */
                if (!line[0])
                        return -1;
                if (self->priv->rstate
                    == GIBBON_SESSION_REGISTER_WAIT_PASSWORD_PROMPT) {
                        if (0 == strncmp ("Your name will be ", line, 18))
                                return -1;
                        if (0 == strncmp ("Type in no password ", line, 20))
                                return -1;
                }

                if (self->priv->rstate
                    == GIBBON_SESSION_REGISTER_WAIT_CONFIRMATION) {
                        if (0 == strncmp ("You are registered", line, 18)) {
                                gibbon_session_registration_success (self);
                                return -1;
                        }
                }
                if (self->priv->rstate > GIBBON_SESSION_REGISTER_WAIT_PROMPT)
                        gibbon_session_registration_error (self, line);
                return -1;
        }

        values = gibbon_clip_reader_parse (self->priv->clip_reader, line);
        if (!values)
                return -1;

        iter = values;
        if (!gibbon_clip_reader_get_int (self->priv->clip_reader, &iter,
                                         (gint *) &code)) {
                gibbon_clip_reader_free_result (self->priv->clip_reader,
                                                values);
                return -1;
        }

        switch (code) {
        case GIBBON_CLIP_UNHANDLED:
        case GIBBON_CLIP_UNKNOWN_MESSAGE:
                break;
        case GIBBON_CLIP_WELCOME:
                retval = gibbon_session_clip_welcome (self, iter);
                break;
        case GIBBON_CLIP_OWN_INFO:
                retval = gibbon_session_clip_own_info (self, iter);
                break;
        case GIBBON_CLIP_MOTD_START:
                retval = GIBBON_CLIP_MOTD_START;
                break;
        case GIBBON_CLIP_MOTD_END:
                retval = GIBBON_CLIP_MOTD_END;
                break;
        case GIBBON_CLIP_WHO_INFO:
                retval = gibbon_session_clip_who_info (self, iter);
                break;
        case GIBBON_CLIP_WHO_INFO_END:
                retval = gibbon_session_clip_who_info_end (self, iter);
                break;
        case GIBBON_CLIP_LOGIN:
                retval = gibbon_session_clip_login (self, iter);
                break;
        case GIBBON_CLIP_LOGOUT:
                retval = gibbon_session_clip_logout (self, iter);
                break;
        case GIBBON_CLIP_MESSAGE:
                retval = gibbon_session_clip_message (self, iter);
                break;
        case GIBBON_CLIP_MESSAGE_DELIVERED:
                retval = gibbon_session_clip_message_delivered (self, iter);
                break;
        case GIBBON_CLIP_MESSAGE_SAVED:
                retval = gibbon_session_clip_message_saved (self, iter);
                break;
        case GIBBON_CLIP_SAYS:
                retval = gibbon_session_clip_says (self, iter);
                break;
        case GIBBON_CLIP_SHOUTS:
                retval = gibbon_session_clip_shouts (self, iter);
                break;
        case GIBBON_CLIP_WHISPERS:
                retval = gibbon_session_clip_whispers (self, iter);
                break;
        case GIBBON_CLIP_KIBITZES:
                retval = gibbon_session_clip_kibitzes (self, iter);
                break;
        case GIBBON_CLIP_YOU_SAY:
                retval = gibbon_session_clip_you_say (self, iter);
                break;
        case GIBBON_CLIP_YOU_SHOUT:
                retval = gibbon_session_clip_you_shout (self, iter);
                break;
        case GIBBON_CLIP_YOU_WHISPER:
                retval = gibbon_session_clip_you_whisper (self, iter);
                break;
        case GIBBON_CLIP_YOU_KIBITZ:
                retval = gibbon_session_clip_you_kibitz (self, iter);
                break;
        case GIBBON_CLIP_ALERTS:
                retval = gibbon_session_clip_alerts (self, iter);
                break;
        case GIBBON_CLIP_ERROR:
                retval = gibbon_session_handle_error (self, iter);
                break;
        case GIBBON_CLIP_BOARD:
                if (self->priv->debug_board_state) {
                        g_get_current_time (&timeval);
                        now = localtime ((time_t *) &timeval.tv_sec);
                        g_printerr ("[%02d:%02d:%02d.%06ld] %s\n",
                                   now->tm_hour, now->tm_min, now->tm_sec,
                                   timeval.tv_usec, line);
                }
                retval = gibbon_session_handle_board (self, iter);
                break;
        case GIBBON_CLIP_ROLLS:
                retval = gibbon_session_handle_rolls (self, iter);
                break;
        case GIBBON_CLIP_MOVES:
                retval = gibbon_session_handle_moves (self, iter);
                break;
        case GIBBON_CLIP_START_GAME:
                /* Ignored.  */
                retval = GIBBON_CLIP_START_GAME;
                break;
        case GIBBON_CLIP_LEFT_GAME:
        case GIBBON_CLIP_DROPS_GAME:
                retval = gibbon_session_handle_left_game (self, iter);
                break;
        case GIBBON_CLIP_CANNOT_MOVE:
                retval = gibbon_session_handle_cannot_move (self, iter);
                break;
        case GIBBON_CLIP_DOUBLES:
                retval = gibbon_session_handle_doubles (self, iter);
                break;
        case GIBBON_CLIP_ACCEPTS_DOUBLE:
                retval = gibbon_session_handle_accepts_double (self, iter);
                break;
        case GIBBON_CLIP_RESIGNS:
                retval = gibbon_session_handle_resigns (self, iter);
                break;
        case GIBBON_CLIP_REJECTS_RESIGNATION:
                retval = gibbon_session_handle_rejects (self, iter);
                break;
        case GIBBON_CLIP_INVITATION:
                retval = gibbon_session_handle_invitation (self, iter);
                break;
        case GIBBON_CLIP_TYPE_JOIN:
                retval = GIBBON_CLIP_TYPE_JOIN;
                break;
        case GIBBON_CLIP_YOURE_WATCHING:
                retval = gibbon_session_handle_youre_watching (self, iter);
                break;
        case GIBBON_CLIP_NOW_PLAYING:
                retval = gibbon_session_handle_now_playing (self, iter);
                break;
        case GIBBON_CLIP_INVITE_ERROR:
                retval = gibbon_session_handle_invite_error (self, iter);
                break;
        case GIBBON_CLIP_RESUME:
                retval = gibbon_session_handle_resume (self, iter);
                break;
        case GIBBON_CLIP_RESUME_INFO_TURN:
        case GIBBON_CLIP_RESUME_INFO_MATCH_LENGTH:
        case GIBBON_CLIP_RESUME_INFO_POINTS:
                /* Ignored.  */
                retval = code;
                break;
        case GIBBON_CLIP_JOIN_OR_LEAVE:
                /*
                 * We always assume that player wants to continue.  There is
                 * no way to terminated an unlimited match, anyway.
                 */
                gibbon_connection_queue_command (self->priv->connection,
                                                 FALSE, "join");
                retval = GIBBON_CLIP_JOIN_OR_LEAVE;
                break;
        case GIBBON_CLIP_WIN_GAME:
                retval = gibbon_session_handle_win_game (self, iter);
                break;
        case GIBBON_CLIP_START_MATCH:
                retval = GIBBON_CLIP_START_MATCH;
                break;
        case GIBBON_CLIP_GAME_SCORE:
                /*
                 * Ignored.  We know the score already from the previous
                 * win message.
                 */
                retval = GIBBON_CLIP_GAME_SCORE;
                break;
        case GIBBON_CLIP_WAIT_JOIN_TOO:
                /* Ignore.  */
                retval = GIBBON_CLIP_WAIT_JOIN_TOO;
                break;
        case GIBBON_CLIP_INVITE_CONFIRMATION:
                /* Ignore.  */
                retval = GIBBON_CLIP_INVITE_CONFIRMATION;
                break;
        case GIBBON_CLIP_WIN_MATCH:
                retval = gibbon_session_handle_win_match (self, iter);
                break;
        case GIBBON_CLIP_STOP_WATCHING:
                gibbon_app_set_state_not_watching (self->priv->app);
                retval = GIBBON_CLIP_STOP_WATCHING;
                break;
        case GIBBON_CLIP_ASYNC_WIN_MATCH:
                retval = gibbon_session_handle_async_win_match (self, iter);
                break;
        case GIBBON_CLIP_RESUME_MATCH:
                retval = gibbon_session_handle_resume_match (self, iter);
                break;
        case GIBBON_CLIP_EMPTY:
                retval = GIBBON_CLIP_EMPTY;
                break;
        case GIBBON_CLIP_START_SETTINGS:
                retval = -1;
                break;
        case GIBBON_CLIP_SHOW_SETTING:
                retval = gibbon_session_handle_show_setting (self, iter);
                break;
        case GIBBON_CLIP_START_TOGGLES:
                retval = -1;
                break;
        case GIBBON_CLIP_SHOW_TOGGLE:
                retval = gibbon_session_handle_show_toggle (self, iter);
                break;
        case GIBBON_CLIP_SHOW_START_SAVED:
                if (self->priv->expect_saved) {
                        self->priv->expect_saved = FALSE;
                        gibbon_session_check_expect_queues (self, TRUE);
                }
                if (self->priv->saved_finished)
                        retval = -1;
                else
                        retval = GIBBON_CLIP_SHOW_START_SAVED;
                self->priv->expect_saved = FALSE;
                break;
        case GIBBON_CLIP_SHOW_SAVED:
                retval = gibbon_session_handle_show_saved (self, iter);
                break;
        case GIBBON_CLIP_SHOW_SAVED_NONE:
                if (self->priv->expect_saved) {
                        self->priv->expect_saved = FALSE;
                        gibbon_session_check_expect_queues (self, TRUE);
                }
                if (self->priv->saved_finished)
                        retval = -1;
                else
                        retval = GIBBON_CLIP_SHOW_START_SAVED;
                self->priv->expect_saved = FALSE;
                self->priv->saved_finished = FALSE;
                break;
        case GIBBON_CLIP_SHOW_SAVED_COUNT:
                retval = gibbon_session_handle_show_saved_count (self, iter);
                break;
        case GIBBON_CLIP_SHOW_ADDRESS:
                retval = gibbon_session_handle_show_address (self, iter);
                break;
        case GIBBON_CLIP_MOTD:
                /* FIXME! Returning -1 here causes the motto of the day to
                 * be displayed in the server console.  Instead, display it
                 * in a dialog box, with a checkbox that allows to redirect
                 * it to the server console in the future.
                 */
                retval = -1;
                break;
        case GIBBON_CLIP_NO_SUCH_USER:
                retval = gibbon_session_handle_no_such_user (self, iter);
                break;
        case GIBBON_CLIP_INVALID_ADDRESS:
                retval = gibbon_session_handle_invalid_address (self, iter);
                break;
        case GIBBON_CLIP_HEARD_YOU:
                /* Ignored.  */
                retval = code;
                break;
        }

        gibbon_clip_reader_free_result (self->priv->clip_reader, values);

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
        GibbonCLIPReader *clip_reader = self->priv->clip_reader;

        if (!gibbon_clip_reader_get_string (clip_reader, &iter, &login))
                return -1;

        expect = gibbon_connection_get_login (self->priv->connection);
        if (g_strcmp0 (expect, login))
                return -1;

        last_login.tv_usec = 0;
        if (!gibbon_clip_reader_get_int64 (clip_reader, &iter,
                                           (gint64 *) &tv_sec))
                return -1;
        last_login.tv_sec = tv_sec;
        
        if (!gibbon_clip_reader_get_string (clip_reader, &iter,
                                            &last_from))
                return -1;

        /* FIXME! Isn't there a better way to format a date and time
         * in glib?
         */
        last_login_str = ctime (&last_login.tv_sec);
        last_login_str[strlen (last_login_str) - 1] = 0;
                
        reply = g_strdup_printf (_("Last login on %s from `%s'."),
                                   last_login_str, last_from);
        console = gibbon_app_get_server_console (self->priv->app);
        gibbon_server_console_print_info (console, reply);
        g_free (reply);

        return GIBBON_CLIP_WELCOME;
}


static gint
gibbon_session_clip_own_info (GibbonSession *self, GSList *iter)
{
        const gchar *login;
        gboolean allowpip, autoboard, autodouble, automove, away, bell,
                 crawford, dbl;
        guint64 experience;
        gboolean greedy, moreboards, moves, notify;
        gdouble rating;
        gboolean ratings, ready;
        gint64 redoubles;
        gboolean report, silent;
        const gchar *timezone;
        GibbonCLIPReader *clip_reader = self->priv->clip_reader;

        if (!gibbon_clip_reader_get_string (clip_reader, &iter, &login))
                return -1;
        if (!gibbon_clip_reader_get_boolean (clip_reader, &iter, &allowpip))
                return -1;
        if (!gibbon_clip_reader_get_boolean (clip_reader, &iter, &autoboard))
                return -1;
        if (!gibbon_clip_reader_get_boolean (clip_reader, &iter, &autodouble))
                return -1;
        if (!gibbon_clip_reader_get_boolean (clip_reader, &iter, &automove))
                return -1;
        if (!gibbon_clip_reader_get_boolean (clip_reader, &iter, &away))
                return -1;
        if (!gibbon_clip_reader_get_boolean (clip_reader, &iter, &bell))
                return -1;
        if (!gibbon_clip_reader_get_boolean (clip_reader, &iter, &crawford))
                return -1;
        if (!gibbon_clip_reader_get_boolean (clip_reader, &iter, &dbl))
                return -1;
        if (!gibbon_clip_reader_get_int64 (clip_reader, &iter,
                                           (gint64 *) &experience))
                return -1;
        if (!gibbon_clip_reader_get_boolean (clip_reader, &iter, &greedy))
                return -1;
        if (!gibbon_clip_reader_get_boolean (clip_reader, &iter, &moreboards))
                return -1;
        if (!gibbon_clip_reader_get_boolean (clip_reader, &iter, &moves))
                return -1;
        if (!gibbon_clip_reader_get_boolean (clip_reader, &iter,  &notify))
                return -1;
        if (!gibbon_clip_reader_get_double (clip_reader, &iter, &rating))
                return -1;
        if (!gibbon_clip_reader_get_boolean (clip_reader, &iter, &ratings))
                return -1;
        if (!gibbon_clip_reader_get_boolean (clip_reader, &iter, &ready))
                return -1;
        if (!gibbon_clip_reader_get_int64 (clip_reader, &iter, &redoubles))
                return -1;
        if (!gibbon_clip_reader_get_boolean (clip_reader, &iter, &report))
                return -1;
        if (!gibbon_clip_reader_get_boolean (clip_reader, &iter, &silent))
                return -1;
        if (!gibbon_clip_reader_get_string (clip_reader, &iter, &timezone))
                return -1;

        if (!notify)
                self->priv->expect_notify = TRUE;
        if (!autoboard)
                self->priv->expect_autoboard = TRUE;
        if (ready) {
                self->priv->available = TRUE;
                gibbon_app_set_state_available (self->priv->app);
        } else {
                self->priv->available = FALSE;
                gibbon_app_set_state_busy (self->priv->app);
        }
        return GIBBON_CLIP_OWN_INFO;
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
        const gchar *current_email;
        const gchar *hostname;
        GibbonCountry *country;
        GibbonArchive *archive;
        gdouble reliability;
        guint confidence;
        const gchar *account;
        guint port;
        const gchar *server;
        gboolean has_saved;
        GibbonSavedInfo *saved_info;
        GError *error = NULL;
        GibbonCLIPReader *clip_reader = self->priv->clip_reader;

        if (!gibbon_clip_reader_get_string (clip_reader, &iter, &who))
                return -1;

        gibbon_session_unqueue_who_request (self, who);

        if (!gibbon_clip_reader_get_string (clip_reader, &iter, &opponent))
                return -1;
        if (opponent[0] == '-' && opponent[1] == 0)
                opponent = "";
                
        if (!gibbon_clip_reader_get_string (clip_reader, &iter, &watching))
                return -1;
        if (watching[0] == '-' && watching[1] == 0)
                watching = "";

        if (!gibbon_clip_reader_get_boolean (clip_reader, &iter, &ready))
                return -1;

        if (!gibbon_clip_reader_get_boolean (clip_reader, &iter, &away))
                return -1;

        if (!gibbon_clip_reader_get_double (clip_reader, &iter, &rating))
                return -1;

        if (!gibbon_clip_reader_get_int64 (clip_reader, &iter,
                                           (gint64 *) &experience))
                return -1;

        iter = iter->next;
        iter = iter->next;
        
        if (!gibbon_clip_reader_get_string (clip_reader, &iter, &hostname))
                return -1;

        if (!gibbon_clip_reader_get_string (clip_reader, &iter, &raw_client))
                return -1;
        client = gibbon_session_decode_client (self, raw_client);

        if (!gibbon_clip_reader_get_string (clip_reader, &iter, &current_email))
                return -1;

        available = ready && !away && !opponent[0];

        account = gibbon_connection_get_login (self->priv->connection);
        server = gibbon_connection_get_hostname (self->priv->connection);
        port = gibbon_connection_get_port (self->priv->connection);

        if (!gibbon_archive_get_reliability (self->priv->archive,
                                             server, port, who,
                                             &reliability, &confidence,
                                             &error)) {
                gibbon_app_display_error (self->priv->app, _("Database Error"),
                                          "%s", error->message);
                g_error_free (error);
                error = NULL;
                confidence = 0;
                reliability = 0;
        }

        client_type = gibbon_get_client_type (client, who, server, port);
        client_icons = gibbon_app_get_client_icons (self->priv->app);
        client_icon = gibbon_client_icons_get_icon (client_icons, client_type);

        country = gibbon_archive_get_country (self->priv->archive, hostname,
                                              (GibbonGeoIPCallback)
                                              gibbon_session_on_geo_ip_resolve,
                                              self);

        saved_info = g_hash_table_lookup (self->priv->saved_games, who);
        has_saved = saved_info ? TRUE : FALSE;

        gibbon_player_list_set (self->priv->player_list,
                                who, has_saved, available, rating, experience,
                                reliability, confidence,
                                opponent, watching,
                                client, client_icon,
                                hostname, country, current_email);

        if (opponent && *opponent) {
                gibbon_inviter_list_remove (self->priv->inviter_list, who);
                gibbon_inviter_list_remove (self->priv->inviter_list, opponent);
        }

        if  (gibbon_inviter_list_exists (self->priv->inviter_list, who)) {
                /*
                 * If we have a saved unlimited match with that player, and
                 * we are now invited to a match of fixed lenght, we must
                 * take care not to overwrite the existing state that we
                 * calculated when receiving the invitation.
                 */
                has_saved = gibbon_inviter_list_get_has_saved (
                                self->priv->inviter_list, who);
                gibbon_inviter_list_set (self->priv->inviter_list, who,
                                         has_saved, rating, experience,
                                         reliability, confidence,
                                         client, client_icon,
                                         hostname, country, current_email);
        }

        g_free (client);

        archive = gibbon_app_get_archive (self->priv->app);

        if (!g_strcmp0 (who, account)) {
                if (!opponent[0])
                        opponent = NULL;
                if (!watching[0])
                        watching = NULL;
                if (g_strcmp0 (opponent, self->priv->opponent)) {
                        gibbon_session_stop_playing (self);
                        self->priv->opponent = g_strdup (opponent);
                }
                if (g_strcmp0 (watching, self->priv->watching)) {
                        gibbon_session_stop_playing (self);
                        self->priv->watching = g_strdup (watching);
                }

                if (!self->priv->rating_seen
                    || rating != self->priv->rating
                    || experience != self->priv->experience) {
                        self->priv->rating_seen = TRUE;
                        self->priv->rating = rating;
                        self->priv->experience = experience;
                        if (!gibbon_archive_update_rank (archive, server, port,
                                                         account, rating,
                                                         experience, NULL,
                                                         &error)) {
                                gibbon_app_display_error (self->priv->app,
                                                          _("Database Failure"),
                                                          "%s",
                                                          error->message);
                                g_error_free (error);
                                error = NULL;
                        }
                }
        }

        gibbon_archive_update_user (archive, server, port, account,
                                    rating, experience);

        if (!g_strcmp0 (account, who))
                gibbon_session_check_address (self, current_email);

        return GIBBON_CLIP_WHO_INFO;
}

static gint
gibbon_session_clip_who_info_end (GibbonSession *self,
                                  GSList *iter)
{
        const gchar *login;

        if (!self->priv->initialized) {
                self->priv->initialized = TRUE;

                self->priv->expect_boardstyle = TRUE;
                self->priv->expect_saved = TRUE;

                /*
                 * Make sure that we see a who info for ourselves.
                 */
                login = gibbon_connection_get_login (self->priv->connection);
                gibbon_session_queue_who_request (self, login);

                gibbon_session_check_expect_queues (self, TRUE);
        }

        return GIBBON_CLIP_WHO_INFO_END;
}


static gint
gibbon_session_clip_login (GibbonSession *self, GSList *iter)
{
        const gchar *name;

        if (!gibbon_clip_reader_get_string (self->priv->clip_reader, &iter,
                                            &name))
                return -1;

        gibbon_session_queue_who_request (self, name);

        return GIBBON_CLIP_LOGIN;
}

static gint
gibbon_session_clip_logout (GibbonSession *self, GSList *iter)
{
        const gchar *hostname;
        guint port;
        const gchar *name;
        gchar *opponent;
        GibbonSavedInfo *info;
        guint match_length;
        guint scores[2];

        if (!gibbon_clip_reader_get_string (self->priv->clip_reader, &iter,
                                            &name))
                return -1;

        gibbon_session_unqueue_who_request (self, name);

        opponent = gibbon_player_list_get_opponent (self->priv->player_list,
                                                    name);
        if (opponent) {
                gibbon_session_queue_who_request (self, opponent);

                hostname = gibbon_connection_get_hostname (self->priv->connection);
                port = gibbon_connection_get_port (self->priv->connection);
                gibbon_archive_save_drop (self->priv->archive,
                                          hostname, port, name, opponent);
                g_free (opponent);
        }
        gibbon_player_list_remove (self->priv->player_list, name);
        gibbon_inviter_list_remove (self->priv->inviter_list, name);

        if (0 == g_strcmp0 (name, self->priv->watching)) {
                g_free (self->priv->position->status);
                self->priv->position->status =
                    g_strdup_printf (_("%s logged out!"), name);
                gibbon_session_stop_playing (self);
                gibbon_app_display_info (self->priv->app, NULL,
                                         _("Player `%s' logged out!"),
                                         name);
        } else if (0 == g_strcmp0 (name, self->priv->opponent)) {
                g_free (self->priv->position->status);
                self->priv->position->status =
                    g_strdup_printf (_("%s logged out!"), name);
                if (!self->priv->watching) {
                        match_length = self->priv->position->match_length;
                        scores[0] = self->priv->position->scores[0];
                        scores[1] = self->priv->position->scores[1];
                        info = gibbon_saved_info_new (name, match_length,
                                                      scores[0], scores[1]);

                        g_hash_table_insert (self->priv->saved_games,
                                             (gpointer) g_strdup (name),
                                             (gpointer) info);
                        gibbon_app_display_info (self->priv->app, NULL,
                                                 _("Player `%s' logged out!"),
                                                 name);
                }
                gibbon_session_stop_playing (self);
        }

        return GIBBON_CLIP_LOGOUT;
}

static gint
gibbon_session_clip_message (GibbonSession *self, GSList *iter)
{
       const gchar *sender;
       const gchar *message;
       gint64 when;
       GTimeVal last_login;
       gchar *last_login_str;
       GibbonCLIPReader *clip_reader = self->priv->clip_reader;

       if (!gibbon_clip_reader_get_string (clip_reader, &iter, &sender))
               return -1;

       if (!gibbon_clip_reader_get_int64 (clip_reader, &iter, &when))
               return -1;

       if (!gibbon_clip_reader_get_string (clip_reader, &iter, &message))
               return -1;

       last_login_str = ctime (&last_login.tv_sec);
       last_login_str[strlen (last_login_str) - 1] = 0;

       gibbon_app_display_info (self->priv->app, NULL,
                                _("User `%s' left you a message at %s: %s"),
                                sender, last_login_str, message);

       return GIBBON_CLIP_MESSAGE;
}

static gint
gibbon_session_clip_message_delivered (GibbonSession *self, GSList *iter)
{
       const gchar *recipient;

       if (!gibbon_clip_reader_get_string (self->priv->clip_reader, &iter,
                                           &recipient))
               return -1;

       gibbon_app_display_info (self->priv->app, NULL,
                                _("Your message for user `%s' has been"
                                  " delivered!"),
                                recipient);

       return GIBBON_CLIP_MESSAGE_DELIVERED;
}

static gint
gibbon_session_clip_message_saved (GibbonSession *self, GSList *iter)
{
       const gchar *recipient;

       if (!gibbon_clip_reader_get_string (self->priv->clip_reader, &iter,
                                           &recipient))
               return -1;

       gibbon_app_display_info (self->priv->app, NULL,
                                _("User `%s' is not logged in.  Your message"
                                  " has been saved!"),
                                recipient);

       return GIBBON_CLIP_MESSAGE_DELIVERED;
}

static gint
gibbon_session_clip_says (GibbonSession *self, GSList *iter)
{
        GibbonFIBSMessage *fibs_message;
        GibbonConnection *connection;
        const gchar *message;
        const gchar *sender;
        GibbonGameChat *game_chat;

        connection = gibbon_app_get_connection (self->priv->app);
        if (!connection)
                return -1;

        if (!gibbon_clip_reader_get_string (self->priv->clip_reader, &iter,
                                            &sender))
                return -1;

        if (!gibbon_clip_reader_get_string (self->priv->clip_reader, &iter,
                                            &message))
                return -1;

        fibs_message = gibbon_fibs_message_new (sender, message);

        if (self->priv->opponent && !self->priv->watching
            && 0 == g_strcmp0 (self->priv->opponent, sender)) {
                game_chat = gibbon_app_get_game_chat (self->priv->app);
                if (!game_chat) {
                        gibbon_fibs_message_free (fibs_message);
                        return -1;
                }
                gibbon_game_chat_append_message (game_chat, fibs_message);
        } else {
                gibbon_app_show_message (self->priv->app,
                                         fibs_message->sender,
                                         fibs_message);
        }

        gibbon_fibs_message_free (fibs_message);

        return GIBBON_CLIP_SAYS;
}

static gint
gibbon_session_clip_alerts (GibbonSession *self, GSList *iter)
{
        GibbonFIBSMessage *fibs_message;
        GibbonConnection *connection;
        const gchar *message;
        const gchar *sender;
        gchar *headline;

        if (GIBBON_CLIP_SAYS != gibbon_session_clip_says (self, iter))
                return -1;

        connection = gibbon_app_get_connection (self->priv->app);
        if (!connection)
                return -1;

        if (!gibbon_clip_reader_get_string (self->priv->clip_reader, &iter,
                                            &sender))
                return -1;

        if (!gibbon_clip_reader_get_string (self->priv->clip_reader, &iter,
                                            &message))
                return -1;

        fibs_message = gibbon_fibs_message_new (sender, message);

        headline = g_strdup_printf (_("Message from administrator `%s':"),
                                    fibs_message->sender);
        gibbon_app_display_info (self->priv->app, headline, "%s",
                                 gibbon_fibs_message_formatted (fibs_message));
        g_free (headline);
        gibbon_fibs_message_free (fibs_message);

        gibbon_app_activate_player_list (self->priv->app);

        return GIBBON_CLIP_ALERTS;
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

        if (!gibbon_clip_reader_get_string (self->priv->clip_reader, &iter,
                                            &sender))
                return -1;

        if (!gibbon_clip_reader_get_string (self->priv->clip_reader, &iter,
                                            &message))
                return -1;

        fibs_message = gibbon_fibs_message_new (sender, message);

        gibbon_shouts_append_message (shouts, fibs_message);

        gibbon_fibs_message_free (fibs_message);

        return GIBBON_CLIP_SHOUTS;
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

        if (!gibbon_clip_reader_get_string (self->priv->clip_reader, &iter,
                                            &sender))
                return -1;

        if (!gibbon_clip_reader_get_string (self->priv->clip_reader, &iter,
                                            &message))
                return -1;

        fibs_message = gibbon_fibs_message_new (sender, message);

        gibbon_game_chat_append_message (game_chat, fibs_message);

        gibbon_fibs_message_free (fibs_message);

        return GIBBON_CLIP_WHISPERS;
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

        if (!gibbon_clip_reader_get_string (self->priv->clip_reader, &iter,
                                            &sender))
                return -1;

        if (!gibbon_clip_reader_get_string (self->priv->clip_reader, &iter,
                                            &message))
                return -1;

        fibs_message = gibbon_fibs_message_new (sender, message);

        gibbon_game_chat_append_message (game_chat, fibs_message);

        gibbon_fibs_message_free (fibs_message);

        return GIBBON_CLIP_KIBITZES;
}

static gint
gibbon_session_clip_you_say (GibbonSession *self, GSList *iter)
{
        GibbonFIBSMessage *fibs_message;
        GibbonConnection *connection;
        const gchar *sender;
        const gchar *recipient;
        const gchar *message;
        GibbonGameChat *game_chat;

        connection = gibbon_app_get_connection (self->priv->app);
        if (!connection)
                return -1;

        sender = gibbon_connection_get_login (connection);
        if (!sender)
                return -1;

        if (!gibbon_clip_reader_get_string (self->priv->clip_reader, &iter,
                                            &recipient))
                return -1;

        if (!gibbon_clip_reader_get_string (self->priv->clip_reader, &iter,
                                            &message))
                return -1;

        fibs_message = gibbon_fibs_message_new (sender, message);

        if (self->priv->opponent && !self->priv->watching
            && 0 == g_strcmp0 (self->priv->opponent, recipient)) {
                game_chat = gibbon_app_get_game_chat (self->priv->app);
                if (!game_chat) {
                        gibbon_fibs_message_free (fibs_message);
                        return -1;
                }
                gibbon_game_chat_append_message (game_chat, fibs_message);
        } else {
                gibbon_app_show_message (self->priv->app, recipient,
                                         fibs_message);
        }

        gibbon_fibs_message_free (fibs_message);

        return GIBBON_CLIP_YOU_SAY;
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

        if (!gibbon_clip_reader_get_string (self->priv->clip_reader, &iter,
                                            &message))
                return -1;

        sender = gibbon_connection_get_login (connection);
        fibs_message = gibbon_fibs_message_new (sender, message);

        gibbon_app_show_shout (self->priv->app, fibs_message);

        gibbon_fibs_message_free (fibs_message);

        return GIBBON_CLIP_YOU_SHOUT;
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

        if (!gibbon_clip_reader_get_string (self->priv->clip_reader, &iter,
                                            &message))
                return -1;

        fibs_message = gibbon_fibs_message_new (sender, message);

        gibbon_app_show_game_chat (self->priv->app, fibs_message);

        gibbon_fibs_message_free (fibs_message);

        return GIBBON_CLIP_YOU_WHISPER;
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

        if (!gibbon_clip_reader_get_string (self->priv->clip_reader, &iter,
                                            &message))
                return -1;

        fibs_message = gibbon_fibs_message_new (sender, message);

        gibbon_app_show_game_chat (self->priv->app, fibs_message);

        gibbon_fibs_message_free (fibs_message);

        return GIBBON_CLIP_YOU_KIBITZ;
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
        gint tmp;
        GibbonConnection *connection;
        const gchar *login = NULL;
        const GibbonPosition *current;

        pos = gibbon_position_copy (g_value_get_boxed (iter->data));
        iter = iter->next;
        self->priv->direction = g_value_get_boolean (iter->data);

        if (!g_strcmp0 ("You", pos->players[0])) {
                connection = gibbon_app_get_connection (self->priv->app);
                login = gibbon_connection_get_login (connection);
                pos->players[0] = g_strdup (login);
                if (self->priv->watching)
                        gibbon_session_stop_playing (self);
        }
        
        if (g_strcmp0 (self->priv->opponent, pos->players[1])) {
                g_free (self->priv->opponent);
                self->priv->opponent = g_strdup (pos->players[1]);
        }
        
        if (pos->match_length > 99)
                pos->match_length = 0;

        /*
         * FIXME! We have to call gibbon_match_tracker_set_crawford() if
         * this is the crawford game.
         */

        if (!memcmp (pos->points, (gibbon_position_initial ())->points,
                     sizeof (pos->points))) {
                /*
                 * Correct the opening roll so that it matches our expectations.
                 */
                if (pos->turn == GIBBON_POSITION_SIDE_WHITE) {
                        if (pos->dice[0] < pos->dice[1]) {
                                tmp = pos->dice[0];
                                pos->dice[0] = pos->dice[1];
                                pos->dice[1] = tmp;
                        }
                } else if (pos->turn == GIBBON_POSITION_SIDE_BLACK) {
                        if (pos->dice[0] > pos->dice[1]) {
                                tmp = pos->dice[0];
                                pos->dice[0] = pos->dice[1];
                                pos->dice[1] = tmp;
                        }
                }
        }

        if (self->priv->position)
                gibbon_position_free (self->priv->position);

        gibbon_session_auto_swap_dice (self, pos);
        self->priv->position = gibbon_position_copy (pos);

        board = gibbon_app_get_board (self->priv->app);
        if (gibbon_position_equals_technically (pos,
                                           gibbon_board_get_position (board))) {
                gibbon_position_free (pos);
        } else {
                gibbon_board_set_position (board, pos);
                gibbon_position_free (pos);
        }

        pos = self->priv->position;
        if (pos->may_double[0]
            && !pos->dice[0]
            && self->priv->position->turn == GIBBON_POSITION_SIDE_WHITE)
                gibbon_app_set_state_may_double (self->priv->app, TRUE);
        else
                gibbon_app_set_state_may_double (self->priv->app, FALSE);

        if (self->priv->debug_board_state) {
                g_printerr ("Board state:\n");
                gibbon_position_dump (self->priv->position);
                GibbonMatch *match = gibbon_app_get_match (app);
                if (match) {
                        current = gibbon_match_get_current_position (match);
                        if (current) {
                                g_printerr ("Current match position:\n");
                                gibbon_position_dump (current);
                        }
                }
        }

        /*
         * Is this our position?  There is no need to check whether player 2
         * is our currently stored opponent, since this is forced above
         * already.
         */
        if ((login && !self->priv->watching && self->priv->opponent
             && !g_strcmp0 (login, pos->players[0]))
            || (self->priv->watching
                && !g_strcmp0 (self->priv->watching, pos->players[0]))) {
                /*
                 * When resuming a match, we initially do not know the length
                 * of the match, and the creation of the tracker object was
                 * delayed until we know the match length from the board state.
                 * In the unlikely case that a match action was performed
                 * before we get the board, our match completion code will
                 * now catch up with the gap.  Well, hopefully. ;)
                 */
                if (!self->priv->tracker)
                        gibbon_session_start_playing (self,
                                                      pos->players[0],
                                                      pos->players[1],
                                                      pos->match_length,
                                                      TRUE);

                g_printerr ("updating tracker from %s:%d\n", __FILE__, __LINE__);
                gibbon_session_update_tracker (self);
        }

        return GIBBON_CLIP_BOARD;
}

static gboolean
gibbon_session_handle_no_such_user (GibbonSession *self, GSList *iter)
{
        const gchar *who;

        if (!gibbon_clip_reader_get_string (self->priv->clip_reader, &iter,
                                            &who))
                return -1;

        gibbon_session_unqueue_who_request (self, who);
        gibbon_player_list_remove (self->priv->player_list, who);
        gibbon_inviter_list_remove (self->priv->inviter_list, who);

        return GIBBON_CLIP_ERROR;
}

static gint
gibbon_session_handle_youre_watching (GibbonSession *self, GSList *iter)
{
        const gchar *player;

        if (!gibbon_clip_reader_get_string (self->priv->clip_reader, &iter,
                                            &player))
                return -1;

        gibbon_session_stop_playing (self);

        self->priv->watching = g_strdup (player);

        return GIBBON_CLIP_YOURE_WATCHING;
}

static gint
gibbon_session_handle_now_playing (GibbonSession *self, GSList *iter)
{
        GibbonBoard *board;
        const gchar *opponent;
        guint length;
        const gchar *player;

        if (!gibbon_clip_reader_get_string (self->priv->clip_reader, &iter,
                                            &opponent))
                return -1;
        if (!gibbon_clip_reader_get_int (self->priv->clip_reader, &iter,
                                        (gint*) &length))
                return -1;

        g_free (self->priv->opponent);
        g_free (self->priv->watching);
        self->priv->opponent = g_strdup (opponent);

        if (self->priv->position)
                gibbon_position_free (self->priv->position);
        self->priv->position = gibbon_position_new ();

        board = gibbon_app_get_board (self->priv->app);
        gibbon_position_reset_unused_dice (self->priv->position);
        gibbon_board_set_position (GIBBON_BOARD (board), self->priv->position);

        gibbon_app_set_state_playing (self->priv->app);

        gibbon_inviter_list_clear (self->priv->inviter_list);
        g_hash_table_remove (self->priv->saved_games, opponent);

        player = gibbon_connection_get_login (self->priv->connection);

        gibbon_session_start_playing (self, player, opponent, length, FALSE);

        return GIBBON_CLIP_NOW_PLAYING;
}

static gint
gibbon_session_handle_invite_error (GibbonSession *self, GSList *iter)
{
        const gchar *player;
        const gchar *message;

        if (!gibbon_clip_reader_get_string (self->priv->clip_reader, &iter,
                                            &player))
                return -1;
        if (!gibbon_clip_reader_get_string (self->priv->clip_reader, &iter,
                                            &message))
                return -1;

        gibbon_inviter_list_remove (self->priv->inviter_list, player);
        gibbon_app_display_error (self->priv->app, NULL, "%s", message);

        return GIBBON_CLIP_INVITE_ERROR;
}

static gint
gibbon_session_handle_resume (GibbonSession *self, GSList *iter)
{
        GibbonBoard *board;
        const gchar *player;
        const gchar *hostname;
        const gchar *login;
        guint port;

        if (!gibbon_clip_reader_get_string (self->priv->clip_reader, &iter,
                                            &player))
                return -1;

        gibbon_session_stop_playing (self);
        self->priv->opponent = g_strdup (player);

        hostname = gibbon_connection_get_hostname (self->priv->connection);
        port = gibbon_connection_get_port (self->priv->connection);
        login = gibbon_connection_get_login (self->priv->connection);
        gibbon_archive_save_resume (self->priv->archive, hostname, port,
                                    login, player);

        if (self->priv->position)
                gibbon_position_free (self->priv->position);
        self->priv->position = gibbon_position_new ();

        board = gibbon_app_get_board (self->priv->app);
        gibbon_position_reset_unused_dice (self->priv->position);
        gibbon_board_set_position (GIBBON_BOARD (board), self->priv->position);

        gibbon_connection_queue_command (self->priv->connection, FALSE,
                                         "board");
        gibbon_app_set_state_playing (self->priv->app);

        return GIBBON_CLIP_RESUME;
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
#ifndef HAVE_INDEX
# define index(str, c) memchr (str, c, strlen (str))
#endif
                underscore = index (retval, '_');
                if (underscore && underscore != client)
                        *underscore = ' ';
                return retval;
        }
}

static gboolean
gibbon_session_handle_win_match (GibbonSession *self, GSList *iter)
{
        const gchar *hostname;
        guint port;
        const gchar *login;

        if (!self->priv->opponent)
                return -1;
        if (self->priv->watching)
                return -1;
        hostname = gibbon_connection_get_hostname (self->priv->connection);
        port = gibbon_connection_get_port (self->priv->connection);
        login = gibbon_connection_get_login (self->priv->connection);

        gibbon_session_queue_who_request (self, self->priv->opponent);
        gibbon_session_queue_who_request (self, login);

        gibbon_archive_save_win (self->priv->archive, hostname, port,
                                 self->priv->opponent, login);

        return GIBBON_CLIP_WIN_MATCH;
}

static gboolean
gibbon_session_handle_async_win_match (GibbonSession *self, GSList *iter)
{
        const gchar *hostname;
        guint port;
        const gchar *player1;
        const gchar *player2;

        if (!gibbon_clip_reader_get_string (self->priv->clip_reader, &iter,
                                            &player1))
                return -1;
        gibbon_session_queue_who_request (self, player1);
        if (!gibbon_clip_reader_get_string (self->priv->clip_reader, &iter,
                                            &player2))
                return -1;
        gibbon_session_queue_who_request (self, player2);

        hostname = gibbon_connection_get_hostname (self->priv->connection);
        port = gibbon_connection_get_port (self->priv->connection);

        gibbon_archive_save_win (self->priv->archive, hostname, port,
                                 player1, player2);

        return GIBBON_CLIP_ASYNC_WIN_MATCH;
}

static gboolean
gibbon_session_handle_resume_match (GibbonSession *self, GSList *iter)
{
        const gchar *hostname;
        guint port;
        const gchar *player1;
        const gchar *player2;

        if (!gibbon_clip_reader_get_string (self->priv->clip_reader, &iter,
                                            &player1))
                return -1;
        gibbon_session_queue_who_request (self, player1);
        if (!gibbon_clip_reader_get_string (self->priv->clip_reader, &iter,
                                            &player2))
                return -1;
        gibbon_session_queue_who_request (self, player2);

        hostname = gibbon_connection_get_hostname (self->priv->connection);
        port = gibbon_connection_get_port (self->priv->connection);

        gibbon_archive_save_resume (self->priv->archive, hostname, port,
                                    player1, player2);

        return GIBBON_CLIP_RESUME_MATCH;
}

static gboolean
gibbon_session_handle_rolls (GibbonSession *self, GSList *iter)
{
        const gchar *who;
        gint64 dice[2];
        GibbonPosition *pos;
        GibbonCLIPReader *clip_reader = self->priv->clip_reader;

        if (!gibbon_clip_reader_get_string (clip_reader, &iter, &who))
                return -1;

        if (!gibbon_clip_reader_get_int64 (clip_reader, &iter, &dice[0]))
                return -1;
        if (!gibbon_clip_reader_get_int64 (clip_reader, &iter, &dice[1]))
                return -1;

        if (0 == g_strcmp0 ("You", who)) {
                self->priv->position->dice[0] = dice[0];
                self->priv->position->dice[1] = dice[1];
                g_free (self->priv->position->status);
                self->priv->position->status =
                        g_strdup_printf (_("You roll %u and %u."),
                                         (guint) dice[0], (guint) dice[1]);
        } else if (0 == g_strcmp0 (self->priv->opponent, who)) {
                self->priv->position->dice[0] = dice[0];
                self->priv->position->dice[1] = dice[1];
                g_free (self->priv->position->status);
                self->priv->position->status =
                                g_strdup_printf (_("%s rolls %u and %u."),
                                                 self->priv->opponent,
                                                 (guint) dice[0],
                                                 (guint) dice[1]);
        } else if (0 == g_strcmp0 (self->priv->watching, who)) {
                self->priv->position->dice[0] = dice[0];
                self->priv->position->dice[1] = dice[1];
                g_free (self->priv->position->status);
                self->priv->position->status =
                        g_strdup_printf (_("%s rolls %u and %u."),
                                         self->priv->watching,
                                         (guint) dice[0], (guint) dice[1]);
        } else {
                return -1;
        }

        gibbon_session_auto_swap_dice (self, self->priv->position);

        pos = self->priv->position;

        gibbon_board_set_position (gibbon_app_get_board (self->priv->app), pos);

g_printerr ("updating tracker from %s:%d\n", __FILE__, __LINE__);
        gibbon_session_update_tracker (self);

        return GIBBON_CLIP_ROLLS;
}

static gint
gibbon_session_handle_moves (GibbonSession *self, GSList *iter)
{
        GibbonMove *move;
        GibbonMovement *movement;
        gint i;
        GibbonPositionSide side;
        gchar *pretty_move;
        guint *dice;
        const gchar *player;
        guint num_moves;
        GibbonBoard *board;
        GibbonPosition *target_position;
        GibbonCLIPReader *clip_reader = self->priv->clip_reader;

        if (!gibbon_clip_reader_get_string (clip_reader, &iter, &player))
                return -1;

        num_moves = g_slist_length (iter) >> 1;

        if (g_strcmp0 (player, self->priv->opponent))
                side = GIBBON_POSITION_SIDE_WHITE;
        else
                side = GIBBON_POSITION_SIDE_BLACK;

        move = gibbon_move_new (0, 0, num_moves);

        for (i = 0; i < num_moves; ++i) {
                movement = move->movements + move->number++;
                if (!gibbon_clip_reader_get_int (clip_reader, &iter,
                                                 &movement->from)) {
                        g_object_unref (move);
                        return -1;
                }
                if (!gibbon_clip_reader_get_int (clip_reader, &iter,
                                                 &movement->to)) {
                        g_object_unref (move);
                        return -1;
                }
        }

        pretty_move = gibbon_position_format_move (self->priv->position, move,
                                                   side,
                                                   !self->priv->direction);
        g_free (self->priv->position->status);
        self->priv->position->status = NULL;
        dice = self->priv->position->dice;

        if (0 == g_strcmp0 (self->priv->opponent, player)) {
                self->priv->position->status =
                                g_strdup_printf (_("%u%u: %s moves %s."),
                                                 dice[0], dice[1],
                                                 self->priv->opponent,
                                                 pretty_move);
        } else if (0 == g_strcmp0 ("You", player)) {
                self->priv->position->status =
                                g_strdup_printf (_("%u%u: You move %s."),
                                                 dice[0], dice[1],
                                                 pretty_move);
        } else if (0 == g_strcmp0 (self->priv->watching, player)) {
                self->priv->position->status =
                                g_strdup_printf (_("%u%u: %s moves %s."),
                                                dice[0], dice[1],
                                                self->priv->watching,
                                                pretty_move);
        } else {
                g_object_unref (move);
                return -1;
        }

        if (!gibbon_position_apply_move (self->priv->position, move,
                                         side, !self->priv->direction)) {
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
                g_object_unref (move);
                return -1;
        }
        g_free (pretty_move);

        self->priv->position->dice[0] = 0;
        self->priv->position->dice[1] = 0;

        board = gibbon_app_get_board (self->priv->app);
        gibbon_position_reset_unused_dice (self->priv->position);
        if (0 == g_strcmp0 ("You", player)) {
                gibbon_board_set_position (board,
                                           self->priv->position);
        } else {
                target_position = gibbon_position_copy (self->priv->position);
                gibbon_board_animate_move (board, move,
                                           -self->priv->position->turn,
                                           target_position);
        }

        g_printerr ("updating tracker from %s:%d\n", __FILE__, __LINE__);
        gibbon_session_update_tracker (self);

        g_object_unref (move);

        return GIBBON_CLIP_MOVES;
}

static gboolean
gibbon_session_handle_invitation (GibbonSession *self, GSList *iter)
{
        const gchar *opponent;
        gint length;
        GtkListStore *store;
        GtkTreeIter tree_iter;
        GibbonPlayerList *pl;
        gdouble rating = 1500.0;
        guint experience = 0;
        GibbonReliability *rel = NULL;
        gdouble reliability = 0;
        guint confidence = 0;
        gchar *client = NULL;
        enum GibbonClientType client_type;
        GibbonClientIcons *client_icons;
        GdkPixbuf *client_icon;
        gchar *hostname = NULL;
        gchar *email = NULL;
        const GibbonCountry *country;
        guint port;
        const gchar *server;
        GibbonConnection *connection;
        GibbonSavedInfo *saved_info;
        gboolean has_saved;
        struct GibbonSessionSavedCountCallbackInfo *info;

        if (!gibbon_clip_reader_get_string (self->priv->clip_reader, &iter,
                                            &opponent))
                return -1;

        if (!gibbon_clip_reader_get_int (self->priv->clip_reader, &iter,
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
                gibbon_session_queue_who_request (self, opponent);
        }

        country = gibbon_archive_get_country (self->priv->archive, hostname,
                                              (GibbonGeoIPCallback)
                                              gibbon_session_on_geo_ip_resolve,
                                              self);

        connection = gibbon_app_get_connection (self->priv->app);
        server = gibbon_connection_get_hostname (connection);
        port = gibbon_connection_get_port (connection);
        client_type = gibbon_get_client_type (client, opponent, server, port);
        client_icons = gibbon_app_get_client_icons (self->priv->app);
        client_icon = gibbon_client_icons_get_icon (client_icons, client_type);

        saved_info = g_hash_table_lookup (self->priv->saved_games, opponent);

        if (saved_info && length < 0) {
                has_saved = TRUE;
        } else {
                has_saved = FALSE;
        }
        gibbon_inviter_list_set (self->priv->inviter_list,
                                 opponent,
                                 has_saved,
                                 rating,
                                 experience,
                                 reliability,
                                 confidence,
                                 client,
                                 client_icon,
                                 hostname,
                                 country,
                                 email);

        g_free (client);
        g_free (hostname);
        g_free (email);

        if (length < 0)
                length = 0;
        gibbon_inviter_list_set_match_length (self->priv->inviter_list,
                                              opponent, length);

        /* Get the saved count.  */
        info = g_malloc (sizeof *info);
        info->callback = NULL;
        info->data = NULL;
        info->who = g_strdup (opponent);
        info->object = NULL;

        self->priv->expect_saved_counts =
                        g_slist_append (self->priv->expect_saved_counts, info);
        gibbon_session_check_expect_queues (self, FALSE);

        return GIBBON_CLIP_INVITATION;
}

static gint
gibbon_session_handle_show_setting (GibbonSession *self, GSList *iter)
{
        gint retval = -1;
        const gchar *key;
        const gchar *value;
        gboolean check_queues = FALSE;
        gboolean force_queues = FALSE;

        if (!gibbon_clip_reader_get_string (self->priv->clip_reader, &iter,
                                            &key))
                return -1;
        if (!gibbon_clip_reader_get_string (self->priv->clip_reader, &iter,
                                            &value))
                return -1;

        /* The only setting we are interested in is "boardstyle".  */
        /*
         * FIXME! There are more settings that have a mandatory value for us.
         */
        if (0 == g_strcmp0 ("boardstyle", key)) {
                if (self->priv->expect_boardstyle) {
                        gibbon_session_clean_saved (self);
                        self->priv->saved_finished = TRUE;
                        check_queues = TRUE;
                        force_queues = TRUE;
                }
                if (value[0] != '3' || value[1]) {
                        /*
                         * Somebody changed the boardstyle.  This is assumed an
                         * error.  We will therefore completely show the
                         * communication with FIBS and neither hide the command
                         * sent, nor will we  try to hide the reply.
                         */
                        self->priv->expect_boardstyle = TRUE;
                        check_queues = TRUE;
                        force_queues = TRUE;
                } else if (self->priv->expect_boardstyle) {
                        self->priv->expect_boardstyle = FALSE;
                        retval = GIBBON_CLIP_SHOW_SETTING;
                        check_queues = TRUE;
                }
        }

        if (check_queues)
                gibbon_session_check_expect_queues (self, force_queues);

        return retval;
}

static gint
gibbon_session_handle_show_toggle (GibbonSession *self, GSList *iter)
{
        const gchar *key;
        gboolean value;
        gboolean check_queue = FALSE;

        if (!gibbon_clip_reader_get_string (self->priv->clip_reader, &iter,
                                            &key))
                return -1;
        if (!gibbon_clip_reader_get_boolean (self->priv->clip_reader, &iter,
                                             &value))
                return -1;

        if (0 == g_strcmp0 ("notify", key)) {
                if (self->priv->expect_notify)
                        check_queue = TRUE;
                self->priv->expect_notify = !value;
                if (check_queue || !value)
                        gibbon_session_check_expect_queues (self, TRUE);
        } else if (0 == g_strcmp0 ("autoboard", key)) {
                if (self->priv->expect_autoboard)
                        check_queue = TRUE;
                self->priv->expect_autoboard = !value;
                if (check_queue || !value)
                        gibbon_session_check_expect_queues (self, TRUE);
        } else if (0 == g_strcmp0 ("ready", key)) {
                self->priv->available = value;
                if (value) {
                        gibbon_app_set_state_available (self->priv->app);
                } else {
                        gibbon_app_set_state_busy (self->priv->app);
                }
        }

        return -1;
}

static gint
gibbon_session_handle_show_saved (GibbonSession *self, GSList *iter)
{
        const gchar *opponent;
        guint match_length, scores[2];
        GibbonSavedInfo *info;
        GibbonCLIPReader *clip_reader = self->priv->clip_reader;

        if (self->priv->expect_saved)
                gibbon_session_check_expect_queues (self, TRUE);
        self->priv->expect_saved = FALSE;

        if (!gibbon_clip_reader_get_string (clip_reader, &iter, &opponent))
                return -1;
        if (!gibbon_clip_reader_get_int (clip_reader, &iter,
                                         (gint *) &match_length))
                return -1;
        if (!gibbon_clip_reader_get_int (clip_reader, &iter,
                                         (gint *) &scores[0]))
                return -1;
        if (!gibbon_clip_reader_get_int (clip_reader, &iter,
                                         (gint *) &scores[1]))
                return -1;

        info = gibbon_saved_info_new (opponent, match_length,
                                      scores[0], scores[1]);

        g_hash_table_insert (self->priv->saved_games,
                             (gpointer) g_strdup (opponent), (gpointer) info);
        gibbon_player_list_update_has_saved (self->priv->player_list,
                                             opponent, TRUE);
        gibbon_inviter_list_update_has_saved (self->priv->inviter_list,
                                              opponent, TRUE);

        if (self->priv->saved_finished)
                return -1;
        else
                return GIBBON_CLIP_SHOW_SAVED;
}

static gint
gibbon_session_handle_show_saved_count (GibbonSession *self, GSList *iter)
{
        const gchar *who;
        guint count;
        GSList *iter2;
        struct GibbonSessionSavedCountCallbackInfo *info;

        if (!gibbon_clip_reader_get_string (self->priv->clip_reader, &iter,
                                            &who))
                return -1;

        if (!gibbon_clip_reader_get_int (self->priv->clip_reader, &iter,
                                         (gint *) &count))
                return -1;

        /*
         * Are we currently waiting for a saved count for a player we want
         * to invite?
         */
        iter2 = self->priv->expect_saved_counts;
        while (iter2) {
                info = (struct GibbonSessionSavedCountCallbackInfo *) iter2->data;
                if (0 == g_strcmp0 (info->who, who)) {
                        self->priv->expect_saved_counts =
                                g_slist_remove (self->priv->expect_saved_counts,
                                                iter2->data);
                        if (info->callback)
                                info->callback (info->object, info->who,
                                                count, info->data);
                        g_free (info->who);
                        g_free (info);
                        break;
                }
                iter2 = iter2->next;
        }
        gibbon_session_check_expect_queues (self, FALSE);

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

        gibbon_inviter_list_set_saved_count (self->priv->inviter_list, who,
                                             count);

        return GIBBON_CLIP_SHOW_SAVED_COUNT;
}

static gint
gibbon_session_handle_show_address (GibbonSession *self, GSList *iter)
{
        const gchar *address;

        self->priv->saved_finished = TRUE;

        if (!gibbon_clip_reader_get_string (self->priv->clip_reader, &iter,
                                            &address)) {
                self->priv->expect_address = FALSE;
                return -1;
        }

        gibbon_session_check_address (self, address);

        if  (self->priv->expect_address) {
                self->priv->expect_address = FALSE;
                return GIBBON_CLIP_SHOW_ADDRESS;
        }

        return -1;
}

static gint
gibbon_session_handle_invalid_address (GibbonSession *self, GSList *iter)
{
        const gchar *address;

        /*
         * If the command was entered manually there is no need to display
         * an error message.  It is already visible in the server console.
         */
        if (!self->priv->expect_address) {
                self->priv->expect_address = FALSE;
                return -1;
        }

        self->priv->expect_address = FALSE;

        if (!gibbon_clip_reader_get_string (self->priv->clip_reader, &iter,
                                            &address))
                return -1;

        gibbon_app_display_error(self->priv->app, NULL,
                                 _("The email address `%s' was rejected by"
                                   " the server!"), address);

        return GIBBON_CLIP_INVALID_ADDRESS;
}

static gint
gibbon_session_handle_left_game (GibbonSession *self, GSList *iter)
{
        GibbonSavedInfo *info;
        const gchar *who;
        guint match_length, scores[2];
        const gchar *hostname;
        const gchar *login;
        guint port;

        if (self->priv->watching)
                return -1;

        if (!gibbon_clip_reader_get_string (self->priv->clip_reader, &iter,
                                            &who))
                return -1;

        if (0 == g_strcmp0 (who, self->priv->opponent)) {
                gibbon_app_set_state_not_playing (self->priv->app);
                match_length = self->priv->position->match_length;
                scores[0] = self->priv->position->scores[0];
                scores[1] = self->priv->position->scores[1];
                info = gibbon_saved_info_new (who, match_length,
                                              scores[0], scores[1]);
                g_hash_table_insert (self->priv->saved_games,
                                     (gpointer) g_strdup (who),
                                     (gpointer) info);
                g_free (self->priv->position->status);
                self->priv->position->status = g_strdup_printf (_("Your"
                                                                  " opponent %s"
                                                                  " has left"
                                                                  " the"
                                                                  " match."),
                                                                who);
                gibbon_app_display_error(self->priv->app,
                                         _("Your opponent has left the game."),
                                         _("Your opponent %s has left the"
                                           " the match.  The server will save"
                                           " it for some time.  You can try to"
                                           " resume it at a later time by"
                                           " inviting %s."), who, who);
        } else if (0 == g_strcmp0 (who, "You")) {
                gibbon_app_set_state_not_playing (self->priv->app);
                self->priv->position->status = g_strdup_printf ("You have left"
                                                                " the match.");
        } else {
                return -1;
        }

        if (!self->priv->opponent)
                return GIBBON_CLIP_LEFT_GAME;

        hostname = gibbon_connection_get_hostname (self->priv->connection);
        port = gibbon_connection_get_port (self->priv->connection);
        login = gibbon_connection_get_login (self->priv->connection);
        gibbon_session_queue_who_request (self, login);
        gibbon_session_queue_who_request (self, self->priv->opponent);
        gibbon_archive_save_drop (self->priv->archive, hostname, port,
                                  login, self->priv->opponent);

        g_free (self->priv->opponent);
        self->priv->opponent = NULL;

        return GIBBON_CLIP_LEFT_GAME;
}

static gint
gibbon_session_handle_cannot_move (GibbonSession *self, GSList *iter)
{
        const gchar *who;
        gboolean must_fade = FALSE;
        GibbonBoard *board;

        if (!gibbon_clip_reader_get_string (self->priv->clip_reader, &iter,
                                            &who))
                return -1;

        if (0 == g_strcmp0 (self->priv->opponent, who)) {
                self->priv->position->turn = GIBBON_POSITION_SIDE_WHITE;
                g_free (self->priv->position->status);
                self->priv->position->status =
                                g_strdup_printf (_("%s cannot move!"), who);
                if (!self->priv->watching)
                        must_fade = TRUE;
        } else if (0 == g_strcmp0 (self->priv->watching, who)) {
                self->priv->position->turn = GIBBON_POSITION_SIDE_BLACK;
                g_free (self->priv->position->status);
                self->priv->position->status =
                                g_strdup_printf (_("%s cannot move!"), who);
        } else if (0 == g_strcmp0 ("You", who)) {
                self->priv->position->turn = GIBBON_POSITION_SIDE_WHITE;
                g_free (self->priv->position->status);
                self->priv->position->status = g_strdup (_("You cannot move!"));
        } else {
                return -1;
        }

        board = gibbon_app_get_board (self->priv->app);
        gibbon_position_reset_unused_dice (self->priv->position);
        gibbon_board_set_position (board, self->priv->position);
        if (must_fade) {
                gibbon_board_fade_out_dice (board);
                /*
                 * We anticipate the board change already.
                 */
                self->priv->position->dice[0] = 0;
                self->priv->position->dice[1] = 0;
                gibbon_position_reset_unused_dice (self->priv->position);
        }

        return GIBBON_CLIP_CANNOT_MOVE;
}

static gint
gibbon_session_handle_doubles (GibbonSession *self, GSList *iter)
{
        const gchar *who;

        if (!gibbon_clip_reader_get_string (self->priv->clip_reader, &iter,
                                            &who))
                return -1;

        if (0 == g_strcmp0 (self->priv->opponent, who)) {
                self->priv->position->cube_turned = GIBBON_POSITION_SIDE_BLACK;
                if (!self->priv->watching)
                        gibbon_app_set_state_expect_response (self->priv->app);
        } else if (0 == g_strcmp0 (self->priv->watching, who)) {
                self->priv->position->cube_turned = GIBBON_POSITION_SIDE_WHITE;
        } else if (0 == g_strcmp0 ("You", who)) {
                self->priv->position->cube_turned = GIBBON_POSITION_SIDE_WHITE;
        } else {
                return -1;
        }

        gibbon_position_reset_unused_dice (self->priv->position);
        gibbon_board_set_position (gibbon_app_get_board (self->priv->app),
                                   self->priv->position);

        g_printerr ("updating tracker from %s:%d\n", __FILE__, __LINE__);
        gibbon_session_update_tracker (self);

        return GIBBON_CLIP_DOUBLES;
}

static gint
gibbon_session_handle_accepts_double (GibbonSession *self, GSList *iter)
{
        const gchar *who;

        if (!gibbon_clip_reader_get_string (self->priv->clip_reader, &iter,
                                            &who))
                return -1;

        self->priv->position->cube_turned = GIBBON_POSITION_SIDE_NONE;

        g_free (self->priv->position->status);
        if (0 == g_strcmp0 (self->priv->opponent, who)) {
                self->priv->position->status =
                    g_strdup_printf ("Opponent %s accepted the cube.",
                                     self->priv->opponent);
                self->priv->position->may_double[0] = FALSE;
                self->priv->position->may_double[1] = TRUE;
        } else if (0 == g_strcmp0 (self->priv->watching, who)) {
                self->priv->position->status =
                    g_strdup_printf ("Player %s accepted the cube.",
                                     self->priv->watching);
                self->priv->position->may_double[0] = TRUE;
                self->priv->position->may_double[1] = FALSE;
        } else if (0 == g_strcmp0 ("You", who)) {
                self->priv->position->status =
                    g_strdup_printf ("You accepted the cube.");
                self->priv->position->may_double[0] = TRUE;
                self->priv->position->may_double[1] = FALSE;
        } else {
                return -1;
        }

        self->priv->position->cube <<= 1;
        gibbon_position_reset_unused_dice (self->priv->position);
        gibbon_board_set_position (gibbon_app_get_board (self->priv->app),
                                   self->priv->position);

        return GIBBON_CLIP_ACCEPTS_DOUBLE;
}

static gint
gibbon_session_handle_resigns (GibbonSession *self, GSList *iter)
{
        const gchar *who;
        guint points;

        if (!self->priv->position->cube)
                return -1;

        if (!gibbon_clip_reader_get_string (self->priv->clip_reader, &iter,
                                            &who))
                return -1;

        if (!gibbon_clip_reader_get_int (self->priv->clip_reader, &iter,
                                         (gint *) &points))
                return -1;

        if (points < 1 || points > 3)
                return -1;

        if (0 == g_strcmp0 ("You", who)) {
                self->priv->position->resigned = points;
        } else if (0 == g_strcmp0 (self->priv->opponent, who)) {
                self->priv->position->resigned = -points;
        } else if (0 == g_strcmp0 (self->priv->watching, who)) {
                self->priv->position->resigned = points;
        } else {
                return -1;
        }

        gibbon_position_reset_unused_dice (self->priv->position);
        gibbon_board_set_position (gibbon_app_get_board (self->priv->app),
                                   self->priv->position);

        g_printerr ("updating tracker from %s:%d\n", __FILE__, __LINE__);
        gibbon_session_update_tracker (self);

        return GIBBON_CLIP_RESIGNS;
}

static gint
gibbon_session_handle_rejects (GibbonSession *self, GSList *iter)
{
        self->priv->position->resigned = 0;

        gibbon_position_reset_unused_dice (self->priv->position);
        gibbon_board_set_position (gibbon_app_get_board (self->priv->app),
                                   self->priv->position);

        g_printerr ("updating tracker from %s:%d\n", __FILE__, __LINE__);
        gibbon_session_update_tracker (self);

        return GIBBON_CLIP_REJECTS_RESIGNATION;
}

static gint
gibbon_session_handle_win_game (GibbonSession *self, GSList *iter)
{
        const gchar *who;
        guint points;
        gint score, i;
        GibbonPosition *try;
        GibbonMove *move;
        gboolean reverse;

        if (!gibbon_clip_reader_get_string (self->priv->clip_reader, &iter,
                                            &who))
                return -1;
        if (!gibbon_clip_reader_get_int (self->priv->clip_reader, &iter,
                                         (gint *) &points))
                return -1;

        if (0 == g_strcmp0 (who, self->priv->opponent)) {
                self->priv->position->scores[1] += points;
                score = -points;
                g_free (self->priv->position->status);
                self->priv->position->status =
                                g_strdup_printf (g_dngettext (GETTEXT_PACKAGE,
                                                              "%s has won the"
                                                              " game and one"
                                                              " point!",
                                                              "%s has won the"
                                                              " game and %d"
                                                              " points!",
                                                              points),
                                                              who, points);
        } else if (0 == g_strcmp0 (who, "You")) {
                self->priv->position->scores[0] += points;
                score = points;
                g_free (self->priv->position->status);
                self->priv->position->status =
                                g_strdup_printf (g_dngettext (GETTEXT_PACKAGE,
                                                              "You have won the"
                                                              " game and one"
                                                              " point!",
                                                              "You have won the"
                                                              " game and %d"
                                                              " points!",
                                                              points),
                                                              points);
        } else if (0 == g_strcmp0 (who, self->priv->watching)) {
                self->priv->position->scores[0] += points;
                score = points;
                g_free (self->priv->position->status);
                self->priv->position->status =
                                g_strdup_printf (g_dngettext (GETTEXT_PACKAGE,
                                                              "%s has won the"
                                                              " game and one"
                                                              " point!",
                                                              "%s has won the"
                                                              " game and %d"
                                                              " points!",
                                                              points),
                                                              who, points);
        } else {
                return -1;
        }

        /*
         * If the last recorded action was a roll we may have missed the
         * winning move.  We detect the situation here, and "correct" the
         * position instead.
         */
        if (!self->priv->position->cube_turned
            && !self->priv->position->resigned
            && self->priv->position->dice[0]) {
                try = gibbon_position_copy (self->priv->position);
                for (i = 0; i < 24; ++i) {
                        /*
                         * If the product of the score and the "number" of
                         * checkers on a point is positive, the point was
                         * occupied by the winner.
                         */
                        if (score * try->points[i] > 0)
                                try->points[i] = 0;
                }

                move = gibbon_position_check_move (self->priv->position, try,
                                                   score);
                if (move->status == GIBBON_MOVE_LEGAL) {
                        reverse = score < 0 ? TRUE : FALSE;

                        (void)
                        gibbon_position_apply_move (self->priv->position, move,
                                                    score, reverse);

                        /*
                         * Reset the score as apply_move() has also set it.
                         */
                        self->priv->position->scores[0] = try->scores[0];
                        self->priv->position->scores[1] = try->scores[1];
                }
                gibbon_position_free (try);
                g_object_unref (move);
        }

        self->priv->position->dice[0] = self->priv->position->dice[1] = 0;
        gibbon_position_reset_unused_dice (self->priv->position);
        self->priv->position->score = score;
        gibbon_board_set_position (gibbon_app_get_board (self->priv->app),
                                   self->priv->position);

        g_printerr ("updating tracker from %s:%d\n", __FILE__, __LINE__);
        gibbon_session_update_tracker (self);

        return GIBBON_CLIP_WIN_GAME;
}

static gint
gibbon_session_handle_error (GibbonSession *self, GSList *iter)
{
        gint64 subcode;
        const gchar *msg;

        if (!gibbon_clip_reader_get_int64 (self->priv->clip_reader, &iter,
                                            &subcode))
                                     return -1;
        if (!gibbon_clip_reader_get_string (self->priv->clip_reader, &iter,
                                            &msg))
                                     return -1;
        gibbon_app_display_error (self->priv->app,
                                  _("Error message from server"),
                                  "%s", msg);

        return GIBBON_CLIP_ERROR;
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
        gboolean is_available = TRUE;

        g_return_if_fail (GIBBON_IS_SESSION (self));
        g_return_if_fail (player != NULL);
        g_return_if_fail (GTK_IS_MENU (menu));

        if (g_strcmp0 (player,
                        gibbon_connection_get_login (self->priv->connection)))
                is_self = FALSE;
        else
                is_self = TRUE;

        am_playing = self->priv->opponent && !self->priv->watching;

        /*
         * The check for self->priv->ready prevents a gratuitous lookup
         * of the other player's state.
         */
        if (!is_self && self->priv->available
            && !gibbon_player_list_get_available (self->priv->player_list,
                                                  player))
                        is_available = FALSE;

        item = gibbon_app_find_object (self->priv->app,
                                       "invite_player_menu_item",
                                       GTK_TYPE_MENU_ITEM);
        if (is_self || am_playing || !self->priv->available || !is_available)
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

static void
gibbon_session_on_geo_ip_resolve (GibbonSession *self,
                                  const gchar *hostname,
                                  const GibbonCountry *country)
{
        /* Silently fail for timed out sessions.  */
        if (!GIBBON_IS_SESSION (self))
                return;

        if (self->priv->player_list)
                gibbon_player_list_update_country (self->priv->player_list,
                                                   hostname, country);
        if (self->priv->inviter_list)
                gibbon_inviter_list_update_country (self->priv->inviter_list,
                                                    hostname, country);
}

void
gibbon_session_get_saved_count (GibbonSession *self, gchar *who,
                                GibbonSessionCallback callback,
                                GObject *object, gpointer data)
{
        struct GibbonSessionSavedCountCallbackInfo *info;

        g_return_if_fail (GIBBON_IS_SESSION (self));
        g_return_if_fail (who != NULL);
        g_return_if_fail (callback != NULL);
        g_return_if_fail (G_IS_OBJECT (object));

        info = g_malloc (sizeof *info);

        info->callback = callback;
        info->who = who;
        info->object = object;
        info->data = data;

        /*
         * Yes, prepending to a singly linked list is inefficient but this
         * list is always tiny.
         */
        self->priv->expect_saved_counts =
                        g_slist_append (self->priv->expect_saved_counts, info);
        gibbon_connection_queue_command (self->priv->connection, FALSE,
                                         "show savedcount %s", who);
}

static gboolean
gibbon_session_timeout (GibbonSession *self)
{
        if (self->priv->guest_login) {
                switch (self->priv->rstate) {
                case GIBBON_SESSION_REGISTER_WAIT_INIT:
                        /* Give another round.  */
                        self->priv->rstate =
                            GIBBON_SESSION_REGISTER_WAIT_PROMPT;
                        break;
                default:
                        if (self->priv->timeout_id > 0)
                                g_source_remove (self->priv->timeout_id);
                        self->priv->timeout_id = 0;
                        gibbon_app_display_error (self->priv->app, NULL, "%s",
                                                  _("Timeout during"
                                                    " during registration!"));
                        gibbon_app_disconnect (self->priv->app);
                        return FALSE;
                }
        } else if (self->priv->expect_boardstyle
                   || self->priv->expect_saved
                   || self->priv->expect_notify
                   || self->priv->expect_autoboard
                   || self->priv->expect_saved_counts
                   || self->priv->expect_who_infos
                   || self->priv->expect_address) {
                gibbon_session_check_expect_queues (self, TRUE);
                return TRUE;
        }

        self->priv->timeout_id = 0;

        return FALSE;
}

static void
gibbon_session_check_expect_queues (GibbonSession *self, gboolean force)
{
        GSettings *settings;
        gchar *mail;
        struct GibbonSessionSavedCountCallbackInfo *info;

        if (!force && self->priv->timeout_id)
                return;

        if (self->priv->expect_saved) {
                gibbon_connection_queue_command (self->priv->connection,
                                                 FALSE,
                                                 "show saved");
        } else if (self->priv->expect_boardstyle) {
                gibbon_connection_queue_command (self->priv->connection,
                                                 self->priv->set_boardstyle,
                                                 "set boardstyle 3");
                self->priv->set_boardstyle = TRUE;
        } else if (self->priv->expect_notify) {
                gibbon_connection_queue_command (self->priv->connection, TRUE,
                                                 "toggle notify");
        } else if (self->priv->expect_autoboard) {
                gibbon_connection_queue_command (self->priv->connection, TRUE,
                                                 "toggle autoboard");
        } else if (self->priv->expect_saved_counts) {
                info = self->priv->expect_saved_counts->data;
                gibbon_connection_queue_command (self->priv->connection, FALSE,
                                                 "show savedcount %s",
                                                 info->who);
        } else if (self->priv->expect_who_infos) {
                gibbon_connection_queue_command (self->priv->connection, FALSE,
                                                 "rawwho %s", (gchar *)
                                            self->priv->expect_who_infos->data);
        } else if (self->priv->expect_address) {
                settings = g_settings_new (GIBBON_PREFS_SERVER_SCHEMA);
                mail = g_settings_get_string (settings,
                                              GIBBON_PREFS_SERVER_ADDRESS);
                g_object_unref (settings);
                if (mail && *mail) {
                        gibbon_connection_queue_command (self->priv->connection,
                                                         FALSE,
                                                         "address %s",
                                                         mail);
                } else {
                        self->priv->expect_address = FALSE;
                }
                g_free (mail);
        } else {
                return;
        }

        if (!self->priv->timeout_id)
                self->priv->timeout_id =
                                g_timeout_add (GIBBON_SESSION_REPLY_TIMEOUT,
                                           (GSourceFunc) gibbon_session_timeout,
                                               (gpointer) self);
}

void
gibbon_session_handle_prompt (GibbonSession *self)
{
        gchar *login;
        GSettings *settings;

        g_return_if_fail (GIBBON_IS_SESSION (self));
        g_return_if_fail (self->priv->guest_login);

        if (!self->priv->guest_login) {
                if (self->priv->timeout_id > 0)
                        g_source_remove (self->priv->timeout_id);
                self->priv->timeout_id = 0;
                gibbon_app_display_error (self->priv->app, NULL, "%s",
                                          _("Unexpected reply from server!"));
                gibbon_app_disconnect (self->priv->app);
                return;
        }

        if (self->priv->rstate != GIBBON_SESSION_REGISTER_WAIT_INIT
            && self->priv->rstate != GIBBON_SESSION_REGISTER_WAIT_PROMPT) {
                if (self->priv->timeout_id > 0)
                        g_source_remove (self->priv->timeout_id);
                self->priv->timeout_id = 0;
                gibbon_app_display_error (self->priv->app, NULL, "%s",
                                          _("Registration failed.  Please see"
                                            " server console for details!"));
                gibbon_app_disconnect (self->priv->app);
                return;
        }

        settings = g_settings_new (GIBBON_PREFS_SERVER_SCHEMA);
        login = g_settings_get_string (settings, GIBBON_PREFS_SERVER_LOGIN);
        g_object_unref (settings);

        /* Restart timer.  */
        if (self->priv->timeout_id)
                g_source_remove (self->priv->timeout_id);
        self->priv->timeout_id =
                g_timeout_add (GIBBON_SESSION_REPLY_TIMEOUT,
                               (GSourceFunc) gibbon_session_timeout,
                               (gpointer) self);

        self->priv->rstate = GIBBON_SESSION_REGISTER_WAIT_PASSWORD_PROMPT;

        gibbon_connection_queue_command (self->priv->connection, TRUE,
                                         "name %s", login);

        g_free (login);
}

void
gibbon_session_handle_pw_prompt (GibbonSession *self)
{
        g_return_if_fail (GIBBON_IS_SESSION (self));
        g_return_if_fail (self->priv->guest_login);

        if (!self->priv->guest_login) {
                if (self->priv->timeout_id > 0)
                        g_source_remove (self->priv->timeout_id);
                self->priv->timeout_id = 0;
                gibbon_app_display_error (self->priv->app, NULL, "%s",
                                          _("Unexpected reply from server!"));
                gibbon_app_disconnect (self->priv->app);
                return;
        }

        switch (self->priv->rstate) {
        case GIBBON_SESSION_REGISTER_WAIT_PASSWORD_PROMPT:
        case GIBBON_SESSION_REGISTER_WAIT_PASSWORD2_PROMPT:
                ++self->priv->rstate;
                break;
        default:
                if (self->priv->timeout_id > 0)
                        g_source_remove (self->priv->timeout_id);
                self->priv->timeout_id = 0;
                gibbon_app_display_error (self->priv->app, NULL, "%s",
                                          _("Registration failed.  Please see"
                                            " server console for details!"));
                gibbon_app_disconnect (self->priv->app);
                return;
        }

        /* Restart timer.  */
        if (self->priv->timeout_id)
                g_source_remove (self->priv->timeout_id);
        self->priv->timeout_id =
                g_timeout_add (GIBBON_SESSION_REPLY_TIMEOUT,
                               (GSourceFunc) gibbon_session_timeout,
                               (gpointer) self);

        gibbon_connection_send_password (self->priv->connection, TRUE);
}

static void
gibbon_session_registration_error (GibbonSession *self, const gchar *_line)
{
        gchar *line;
        gchar *freeable;
        gchar *expect;
        GSettings *settings;
        gchar *login;

        if (self->priv->timeout_id > 0)
                g_source_remove (self->priv->timeout_id);
        self->priv->timeout_id = 0;

        line = freeable = g_strdup (_line);

        if (line[0] == '>' && line[1] == ' ')
                line += 2;
        if (line[0] == '*' && line[1] == '*' && line[2] == ' ')
                line += 3;

        settings = g_settings_new (GIBBON_PREFS_SERVER_SCHEMA);
        login = g_settings_get_string (settings, GIBBON_PREFS_SERVER_LOGIN);
        g_object_unref (settings);
        expect = g_strdup_printf ("Please use another name. '%s' is"
                                  " already used by someone else.", login);
        if (0 == g_strcmp0 (expect, line)) {
                g_free (freeable);
                line = freeable = g_strdup_printf (_("Please use another"
                                                     " name.  The name `%s' is"
                                                     " already used by"
                                                     " someone else on that"
                                                     " server!"), login);
        } else {
                line = g_strdup_printf (_("Registration failure: %s!"), line);
                g_free (freeable);
                freeable = line;
        }
        g_free (login);
        g_free (expect);

        gibbon_app_display_error (self->priv->app, NULL, "%s", line);
        g_free (freeable);

        gibbon_app_disconnect (self->priv->app);
}

static void
gibbon_session_registration_success (GibbonSession *self)
{
        GSettings *settings;
        const gchar *hostname;
        guint port;
        gchar *login;
        GError *error = NULL;

        if (self->priv->timeout_id)
                g_source_remove (self->priv->timeout_id);
        self->priv->timeout_id = 0;

        settings = g_settings_new (GIBBON_PREFS_SERVER_SCHEMA);
        login = g_settings_get_string (settings, GIBBON_PREFS_SERVER_LOGIN);
        g_object_unref (settings);
        hostname = gibbon_connection_get_hostname (self->priv->connection);
        port = gibbon_connection_get_port (self->priv->connection);
        if (!gibbon_archive_login (self->priv->archive, hostname, port,
                                   login, &error)) {
                gibbon_app_fatal_error (self->priv->app, _("Database Failure"),
                                        "%s", error->message);
                /* NOTREACHED */
        }
        g_free (login);

        gibbon_app_display_info (self->priv->app,
                                 _("Registration successful!"),
                                 "%s",
                                 _("Please do not forget your password! It"
                                   " cannot be recovered."
                                   "\n"
                                   "You have to connect to the server again to"
                                   " start playing."));
        gibbon_app_disconnect (self->priv->app);
}

void
gibbon_session_set_available (GibbonSession *self, gboolean available)
{
        g_return_if_fail (GIBBON_IS_SESSION (self));

        self->priv->available = available;

        gibbon_connection_queue_command (self->priv->connection, FALSE,
                                         "toggle ready");
}

void
gibbon_session_reply_to_invite (GibbonSession *self, const gchar *who,
                                gboolean reply, guint match_length)
{
        GtkWidget *dialog;
        GtkWidget *window = NULL;
        GtkWidget *entry;
        GtkWidget *content_area;
        gint response;
        const gchar *message;
        GibbonSavedInfo *saved_info;

        g_return_if_fail (GIBBON_IS_SESSION (self));
        g_return_if_fail (who != NULL);
        g_return_if_fail (*who);

        gibbon_inviter_list_remove (self->priv->inviter_list, who);

        if (reply
            && (saved_info = g_hash_table_lookup (self->priv->saved_games,
                                                  who))
            && !saved_info->match_length && match_length) {
                window = gibbon_app_get_window (self->priv->app);
                dialog = gtk_message_dialog_new_with_markup(
                                GTK_WINDOW (window),
                                GTK_DIALOG_DESTROY_WITH_PARENT,
                                GTK_MESSAGE_ERROR,
                                GTK_BUTTONS_YES_NO,
                                "<span weight='bold' size='larger'>"
                                "%s</span>\n%s",
                                _("This will end your saved match!"),
                                _("You still have a saved match of unlimited"
                                  " length with that player.  If you accept"
                                  " the invitation, your saved match will be"
                                  " terminated.\n"
                                  "Do you really want to accept the"
                                  " invitation?"));
                gtk_dialog_set_default_response (GTK_DIALOG (dialog),
                                                 GTK_RESPONSE_NO);
                response = gtk_dialog_run (GTK_DIALOG (dialog));
                gtk_widget_destroy (dialog);
                if (response == GTK_RESPONSE_NO)
                        return;
        }

        if (reply) {
                gibbon_connection_queue_command (self->priv->connection,
                                                 FALSE,
                                                 "join %s", who);
                return;
        }

        if (!window)
                window = gibbon_app_get_window (self->priv->app);

        dialog = gtk_dialog_new_with_buttons (_("Decline Invitation"),
                                              GTK_WINDOW (window),
                                              GTK_DIALOG_MODAL
                                              | GTK_DIALOG_DESTROY_WITH_PARENT,
                                              _("Decline with message"),
                                              GTK_RESPONSE_OK,
                                              _("Ignore"),
                                              GTK_RESPONSE_CANCEL,
                                              NULL);
        gtk_dialog_set_default_response (GTK_DIALOG (dialog),
                                         GTK_RESPONSE_OK);

        /*
         * TODO: We should store a list of recently sent decline messages and
         * allow the user to select one.
         */
        entry = gtk_entry_new ();
        /*
         * This string is not translated on purpose!
         */
        gtk_entry_set_text (GTK_ENTRY (entry),
                            "Not now.  Thanks for the invitation!");

        content_area = gtk_dialog_get_content_area (GTK_DIALOG (dialog));
        gtk_box_pack_start (GTK_BOX (content_area),
                            GTK_WIDGET (entry), TRUE, TRUE, 0);

        gtk_widget_show_all (dialog);

        response = gtk_dialog_run (GTK_DIALOG (dialog));
        if (response == GTK_RESPONSE_CANCEL) {
                gtk_widget_destroy (dialog);
                return;
        }

        message = gtk_entry_get_text (GTK_ENTRY (entry));
        gibbon_connection_queue_command (self->priv->connection, FALSE,
                                         "tellx %s %s", who, message);

        gtk_widget_destroy (dialog);
}

static void
gibbon_session_on_dice_picked_up (const GibbonSession *self)
{
        const GibbonPosition *pos;
        GibbonBoard *board;
        GibbonMove *move;
        gchar *fibs_move;

        g_return_if_fail (GIBBON_IS_SESSION (self));

        if (self->priv->watching)
                return;
        if (!self->priv->opponent)
                return;

        if (GIBBON_POSITION_SIDE_WHITE != self->priv->position->turn)
                return;

        /*
         * If it is our turn, and the dice are 0, we have to roll or double.
         * If the opponent cannot move, then the value on the dice will be
         * negative, and we also have to roll or double.
         */
        if (self->priv->position->dice[0] <= 0) {
                gibbon_connection_queue_command (self->priv->connection, FALSE,
                                                 "roll");
                return;
        }

        board = gibbon_app_get_board (self->priv->app);
        pos = gibbon_board_get_position (board);

        move = gibbon_position_check_move (self->priv->position, pos,
                                           GIBBON_POSITION_SIDE_WHITE);

        /*
         * If not all dice were used, and the move was not legal, we assume
         * that the user wants to switch the dice.
         */
        if (move->status != GIBBON_MOVE_LEGAL && pos->unused_dice[0]) {
                g_object_unref (move);
                self->priv->position->dice_swapped =
                                !self->priv->position->dice_swapped;
                gibbon_board_set_position (board, self->priv->position);
                return;
        }

        switch (move->status) {
        case GIBBON_MOVE_LEGAL:
                break;
        case GIBBON_MOVE_ILLEGAL:
                gibbon_app_display_error (self->priv->app, NULL, "%s",
                                          _("Illegal move!"));
                break;
        case GIBBON_MOVE_TOO_MANY_MOVES:
                /* Cannot happen.  */
                gibbon_app_display_error (self->priv->app, NULL, "%s",
                                          _("You moved too many checkers!"));
                break;
        case GIBBON_MOVE_BLOCKED:
                /* Cannot happen.  */
                gibbon_app_display_error (self->priv->app, NULL, "%s",
                                          _("You cannot move to a point that"
                                            " is blocked by two or more of"
                                            " your opponent's checkers!"));
                break;
        case GIBBON_MOVE_USE_ALL:
                gibbon_app_display_error (self->priv->app, NULL, "%s",
                                          _("You have to use both dice!"));
                break;
        case GIBBON_MOVE_USE_HIGHER:
                gibbon_app_display_error (self->priv->app, NULL, "%s",
                                          _("You have to use the higher die!"));
                break;
        case GIBBON_MOVE_TRY_SWAP:
                gibbon_app_display_error (self->priv->app, NULL, "%s",
                                          _("You have to use both dice if"
                                            " possible (hint: try to use"
                                            " the other die first)!"));
                break;
        case GIBBON_MOVE_PREMATURE_BEAR_OFF:
                gibbon_app_display_error (self->priv->app, NULL, "%s",
                                          _("You must move all checkers into"
                                            " your home board before you can"
                                            " start bearing-off!"));
                break;
        case GIBBON_MOVE_ILLEGAL_WASTE:
                gibbon_app_display_error (self->priv->app, NULL, "%s",
                                          _("You must move your checkers from"
                                            " the higher points first!"));
                break;
        case GIBBON_MOVE_DANCING:
                gibbon_app_display_error (self->priv->app, NULL, "%s",
                                          _("You must bring in checkers from"
                                            " the bar first!"));
                break;
        }

        if (move->status != GIBBON_MOVE_LEGAL) {
                g_object_unref (move);
                gibbon_position_reset_unused_dice (self->priv->position);
                gibbon_board_set_position (board, self->priv->position);
                return;
        }

        fibs_move = gibbon_position_fibs_move (self->priv->position,
                                               move,
                                               GIBBON_POSITION_SIDE_WHITE,
                                               !self->priv->direction);
        g_object_unref (move);
        gibbon_connection_queue_command (self->priv->connection, FALSE,
                                         "move %s", fibs_move);
        g_free (fibs_move);

        self->priv->position = gibbon_position_copy (pos);
        self->priv->position->turn = GIBBON_POSITION_SIDE_BLACK;
        self->priv->position->dice[0] = 0;
        self->priv->position->dice[1] = 0;
        gibbon_position_reset_unused_dice (self->priv->position);
        gibbon_board_set_position (board, self->priv->position);
}

static void
gibbon_session_on_cube_turned (const GibbonSession *self)
{
        GibbonBoard *board;

        g_return_if_fail (GIBBON_IS_SESSION (self));

        if (self->priv->watching)
                return;
        if (!self->priv->opponent)
                return;

        if (GIBBON_POSITION_SIDE_WHITE != self->priv->position->turn)
                return;
        if (!self->priv->position->may_double[0])
                return;
        if (self->priv->position->dice[0])
                return;
        self->priv->position->cube_turned = GIBBON_POSITION_SIDE_WHITE;

        gibbon_connection_queue_command (self->priv->connection, FALSE,
                                         "double");
        board = gibbon_app_get_board (self->priv->app);
        gibbon_position_reset_unused_dice (self->priv->position);
        gibbon_board_set_position (board, self->priv->position);
}

static void
gibbon_session_on_cube_taken (const GibbonSession *self)
{
        GibbonBoard *board;

        g_return_if_fail (GIBBON_IS_SESSION (self));

        if (self->priv->watching)
                return;
        if (!self->priv->opponent)
                return;

        if (GIBBON_POSITION_SIDE_BLACK != self->priv->position->turn)
                return;
        if (GIBBON_POSITION_SIDE_BLACK != self->priv->position->cube_turned)
                return;
        self->priv->position->cube_turned = GIBBON_POSITION_SIDE_NONE;
        self->priv->position->may_double[0] = FALSE;
        self->priv->position->may_double[1] = TRUE;
        gibbon_connection_queue_command (self->priv->connection, FALSE,
                                         "accept");
        board = gibbon_app_get_board (self->priv->app);
        gibbon_position_reset_unused_dice (self->priv->position);
        gibbon_board_set_position (board, self->priv->position);
}

static void
gibbon_session_on_cube_dropped (const GibbonSession *self)
{
        GibbonBoard *board;

        g_return_if_fail (GIBBON_IS_SESSION (self));

        if (self->priv->watching)
                return;
        if (!self->priv->opponent)
                return;

        if (GIBBON_POSITION_SIDE_BLACK != self->priv->position->turn)
                return;
        if (GIBBON_POSITION_SIDE_BLACK != self->priv->position->cube_turned)
                return;
        self->priv->position->cube_turned = GIBBON_POSITION_SIDE_NONE;
        gibbon_connection_queue_command (self->priv->connection, FALSE,
                                         "reject");
        board = gibbon_app_get_board (self->priv->app);
        gibbon_position_reset_unused_dice (self->priv->position);
        gibbon_board_set_position (board, self->priv->position);
}

static void
gibbon_session_on_resignation_accepted (const GibbonSession *self)
{
        GibbonBoard *board;

        g_return_if_fail (GIBBON_IS_SESSION (self));

        if (self->priv->watching)
                return;
        if (!self->priv->opponent)
                return;

        if (self->priv->position->resigned >= 0)
                return;
        self->priv->position->resigned = 0;
        gibbon_connection_queue_command (self->priv->connection, FALSE,
                                         "accept");
        board = gibbon_app_get_board (self->priv->app);
        gibbon_position_reset_unused_dice (self->priv->position);
        gibbon_board_set_position (board, self->priv->position);
}

static void
gibbon_session_on_resignation_rejected (const GibbonSession *self)
{
        GibbonBoard *board;

        g_return_if_fail (GIBBON_IS_SESSION (self));

        if (self->priv->watching)
                return;
        if (!self->priv->opponent)
                return;

        if (self->priv->position->resigned >= 0)
                return;
        self->priv->position->resigned = 0;
        gibbon_connection_queue_command (self->priv->connection, FALSE,
                                         "reject");
        board = gibbon_app_get_board (self->priv->app);
        gibbon_position_reset_unused_dice (self->priv->position);
        gibbon_board_set_position (board, self->priv->position);
}

void
gibbon_session_reset_position (GibbonSession *self)
{
        GibbonBoard *board;

        g_return_if_fail (GIBBON_IS_SESSION (self));

        board = gibbon_app_get_board (self->priv->app);
        gibbon_position_reset_unused_dice (self->priv->position);
        gibbon_session_auto_swap_dice (self, self->priv->position);
        gibbon_board_set_position (board, self->priv->position);
}

void
gibbon_session_accept_request (GibbonSession *self)
{
        gibbon_connection_queue_command (self->priv->connection, FALSE,
                                         "accept");
}

void
gibbon_session_reject_request (GibbonSession *self)
{
        gibbon_connection_queue_command (self->priv->connection, FALSE,
                                         "reject");
}

const GibbonPosition *
gibbon_session_get_position (const GibbonSession *self)
{
        g_return_val_if_fail (GIBBON_IS_SESSION (self), NULL);

        return self->priv->position;
}

void
gibbon_session_resign (GibbonSession *self, guint value)
{
        GibbonBoard *board;
        gchar value_string;

        g_return_if_fail (GIBBON_IS_SESSION (self));

        if (self->priv->watching)
                return;
        if (!self->priv->opponent)
                return;

        self->priv->position->resigned = value;
        switch (value) {
                case 1:
                default:
                        value_string = 'n';
                        break;
                case 2:
                        value_string = 'g';
                        break;
                case 3:
                        value_string = 'b';
                        break;
        }

        gibbon_connection_queue_command (self->priv->connection, FALSE,
                                         "resign %c", value_string);
        board = gibbon_app_get_board (self->priv->app);
        gibbon_position_reset_unused_dice (self->priv->position);
        gibbon_board_set_position (board, self->priv->position);

        g_printerr ("updating tracker from %s:%d\n", __FILE__, __LINE__);
        gibbon_session_update_tracker (self);
}

static void
gibbon_session_queue_who_request (GibbonSession *self, const gchar *who)
{
        GSList *iter;

        /* Restart timer.  */
        if (self->priv->timeout_id)
                g_source_remove (self->priv->timeout_id);
        self->priv->timeout_id =
                g_timeout_add (GIBBON_SESSION_REPLY_TIMEOUT,
                               (GSourceFunc) gibbon_session_timeout,
                               (gpointer) self);

        iter = self->priv->expect_who_infos;
        while (iter) {
                if (0 == g_strcmp0 (who, iter->data))
                        return;
                iter = iter->next;
        }

        self->priv->expect_who_infos =
                        g_slist_prepend (self->priv->expect_who_infos,
                                         g_strdup (who));
}

static void
gibbon_session_unqueue_who_request (GibbonSession *self, const gchar *who)
{
        GSList *iter;

        iter = self->priv->expect_who_infos;
        while (iter) {
                if (0 == g_strcmp0 (who, iter->data)) {
                        g_free (iter->data);
                        self->priv->expect_who_infos =
                                g_slist_remove (self->priv->expect_who_infos,
                                                iter->data);
                        gibbon_session_unqueue_who_request (self, who);
                        return;
                }
                iter = iter->next;
        }
}

static void
gibbon_session_start_playing (GibbonSession *self,
                              const gchar *player, const gchar *opponent,
                              guint length, gboolean resumption)
{
        gdouble rating;
        guint experience;
        GibbonReliability *rel;
        GtkTreeIter tree_iter;
        GtkListStore *store;
        gchar *rank;

        if (self->priv->tracker)
                g_object_unref (self->priv->tracker);
        self->priv->tracker = gibbon_match_tracker_new (player, opponent,
                                                        length, resumption);

        store = gibbon_player_list_get_store (self->priv->player_list);

        if (gibbon_player_list_get_iter (self->priv->player_list, player,
                                         &tree_iter)) {
                gtk_tree_model_get (GTK_TREE_MODEL (store), &tree_iter,
                                    GIBBON_PLAYER_LIST_COL_RATING, &rating,
                                    GIBBON_PLAYER_LIST_COL_EXPERIENCE,
                                            &experience,
                                    GIBBON_PLAYER_LIST_COL_RELIABILITY,
                                            &rel,
                                    -1);

                if (rel && rel->confidence) {
                        rank = g_strdup_printf ("%f %u (FIBS), %f %u (Gibbon)",
                                                 rating, experience,
                                                 rel->value, rel->confidence);
                } else {
                        rank = g_strdup_printf ("%f %u (FIBS)",
                                                 rating, experience);
                }

                gibbon_match_tracker_store_rank (self->priv->tracker, rank,
                                                 GIBBON_POSITION_SIDE_WHITE);

                g_free (rank);
                if (rel) gibbon_reliability_free (rel);
        }

        if (gibbon_player_list_get_iter (self->priv->player_list, opponent,
                                         &tree_iter)) {
                gtk_tree_model_get (GTK_TREE_MODEL (store), &tree_iter,
                                    GIBBON_PLAYER_LIST_COL_RATING, &rating,
                                    GIBBON_PLAYER_LIST_COL_EXPERIENCE,
                                            &experience,
                                    GIBBON_PLAYER_LIST_COL_RELIABILITY,
                                            &rel,
                                    -1);

                if (rel && rel->confidence) {
                        rank = g_strdup_printf ("%f %u (FIBS), %f %u (Gibbon)",
                                                 rating, experience,
                                                 rel->value, rel->confidence);
                } else {
                        rank = g_strdup_printf ("%f %u (FIBS)",
                                                 rating, experience);
                }

                gibbon_match_tracker_store_rank (self->priv->tracker, rank,
                                                 GIBBON_POSITION_SIDE_BLACK);

                g_free (rank);
                if (rel) gibbon_reliability_free (rel);
        }
}

static void
gibbon_session_stop_playing (GibbonSession *self)
{
        g_free (self->priv->watching);
        self->priv->watching = NULL;
        g_free (self->priv->opponent);
        self->priv->opponent = NULL;

        if (self->priv->tracker) {
                g_object_unref (self->priv->tracker);
                self->priv->tracker = NULL;
        }
}

static void
gibbon_session_clean_saved (const GibbonSession *self)
{
        const GibbonConnection *connection = self->priv->connection;
        const gchar *hostname = gibbon_connection_get_hostname (connection);
        guint port = gibbon_connection_get_port (connection);
        const gchar *login = gibbon_connection_get_login (connection);
        GHashTable *saved = gibbon_archive_get_saved (self->priv->archive,
                                                      hostname, port, login);
        GHashTableIter iter;
        const gchar *saved_name;
        const GibbonSavedInfo *local_record;
        const GibbonSavedInfo *remote_record;
        gchar *path;
        gchar *saved_directory;

        saved_directory = gibbon_archive_get_saved_directory (
                        self->priv->archive, NULL);
        if (!saved_directory)
                return;

        g_hash_table_iter_init (&iter, saved);
        while (g_hash_table_iter_next (&iter, (gpointer) &saved_name,
                                       (gpointer) &local_record)) {
                remote_record = g_hash_table_lookup (self->priv->saved_games,
                                                     local_record->opponent);
                if (gibbon_saved_info_equals (local_record, remote_record))
                        continue;
                /*
                 * Locally saved match is stale.  Archive it.
                 */
                path = g_build_filename (saved_directory, saved_name, NULL);
                (void) gibbon_archive_archive_match_file (self->priv->archive,
                                                          NULL, path, NULL);
                g_free (path);
        }

        g_hash_table_destroy (saved);
        g_free (saved_directory);
}

static void
gibbon_session_update_tracker (GibbonSession *self)
{
        GibbonPositionSide winner;

        if (!self->priv->tracker)
                return;

        gibbon_match_tracker_update (self->priv->tracker, self->priv->position);

        winner = gibbon_match_over (gibbon_app_get_match (app));
        if (!winner)
                return;

        gibbon_session_stop_playing (self);
}

static void
gibbon_session_check_address (GibbonSession *self, const gchar *remote_email)
{
        GSettings *settings;
        gchar *local_email;

        settings = g_settings_new (GIBBON_PREFS_SERVER_SCHEMA);
        local_email = g_settings_get_string (settings,
                                             GIBBON_PREFS_SERVER_ADDRESS);
        g_object_unref (settings);

        if (!local_email || !*local_email) {
                g_free (local_email);
                local_email = g_strdup ("-");
        }

        if (!g_strcmp0 (local_email, remote_email)) {
                g_free (local_email);
                self->priv->address_checked = TRUE;
                return;
        }

        if (self->priv->address_checked) {
                /*
                 * Address has changed on the server.  Write it back to
                 * GSettings.
                 */
                if (!g_strcmp0 ("-", remote_email))
                        remote_email = "";
                settings = g_settings_new (GIBBON_PREFS_SERVER_SCHEMA);
                g_settings_set_string (settings, GIBBON_PREFS_SERVER_ADDRESS,
                                       remote_email);
                g_object_unref (settings);
        } else {
                /*
                 * Send user configured address to the server.
                 */
                gibbon_connection_queue_command (
                                self->priv->connection,
                                FALSE,
                                "address %s",
                                local_email);
                self->priv->expect_address = TRUE;
        }

        g_free (local_email);

        self->priv->address_checked = TRUE;
}

static void
gibbon_session_auto_swap_dice (const GibbonSession *self, GibbonPosition *pos)
{
        GSettings *settings;

        g_printerr ("auto swapping dice\n");
        pos->dice_swapped = FALSE;

        if (pos->dice[0] < pos->dice[1]) {
                g_printerr ("lower is left\n");
                settings = g_settings_new (GIBBON_PREFS_MATCH_SCHEMA);
                if (g_settings_get_boolean (settings,
                                            GIBBON_PREFS_MATCH_AUTO_SWAP))
                        g_printerr ("auto-swap active!\n");
                else
                        g_printerr ("auto-swap inactive!\n");
                if (g_settings_get_boolean (settings,
                                            GIBBON_PREFS_MATCH_AUTO_SWAP))
                        pos->dice_swapped = TRUE;
                g_object_unref (settings);
        }

        if (GTK_TEXT_DIR_RTL == gtk_widget_get_default_direction ())
                g_printerr ("rtl!\n");
        if (GTK_TEXT_DIR_RTL == gtk_widget_get_default_direction ())
                pos->dice_swapped = !pos->dice_swapped;

        gibbon_position_reset_unused_dice (pos);

        g_printerr ("Dice after auto swap: %u%d (%u%u)\n",
                        pos->dice[0], pos->dice[1],
                        pos->unused_dice[0], pos->unused_dice[1]);
}
