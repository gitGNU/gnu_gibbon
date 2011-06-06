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
#include "gibbon-player-list-view.h"
#include "gibbon-cairoboard.h"
#include "gibbon-fibs-message.h"
#include "gibbon-shouts.h"
#include "gibbon-position.h"
#include "gibbon-board.h"
#include "gibbon-game-chat.h"
#include "gibbon-archive.h"
#include "gibbon-util.h"
#include "gibbon-clip.h"

#define CLIP_WELCOME 1
#define CLIP_WHO_INFO 5
#define CLIP_WHO_INFO_END 6
#define CLIP_LOGIN 7
#define CLIP_LOGOUT 8
#define CLIP_SAYS 12
#define CLIP_SHOUTS 13
#define CLIP_WHISPERS 14
#define CLIP_KIBITZES 15
#define CLIP_YOU_SAY 16
#define CLIP_YOU_SHOUT 17
#define CLIP_YOU_WHISPER 18
#define CLIP_YOU_KIBITZ 19

typedef enum {
        GIBBON_SESSION_PLAYER_YOU = 0,
        GIBBON_SESSION_PLAYER_WATCHING = 1,
        GIBBON_SESSION_PLAYER_OPPONENT = 2,
        GIBBON_SESSION_PLAYER_OTHER = 3
} GibbonSessionPlayer;

#ifdef GIBBON_SESSION_DEBUG_BOARD_STATE
static const gchar *keys[] = {
                "player",
                "opponent",
                "match length",
                "player's score",
                "opponents's score",
                "home/bar",
                "point 0",
                "point 1",
                "point 2",
                "point 3",
                "point 4",
                "point 5",
                "point 6",
                "point 7",
                "point 8",
                "point 9",
                "point 10",
                "point 11",
                "point 12",
                "point 13",
                "point 14",
                "point 15",
                "point 16",
                "point 17",
                "point 18",
                "point 19",
                "point 20",
                "point 21",
                "point 22",
                "point 23",
                "home/bar",
                "turn",
                "player's die 0",
                "player's die 1",
                "opponent's die 0",
                "opponent's die 1",
                "doubling cube",
                "player may double",
                "opponent may double",
                "was doubled",
                "color",
                "direction",
                "home index",
                "bar index",
                "player's checkers on home",
                "opponent's checkers on home",
                "player's checkers on bar",
                "opponent's checkers on bar",
                "can move",
                "forced move",
                "did crawford",
                "redoubles",
                NULL
};

static void gibbon_session_dump_board (const GibbonSession *self,
                                       const gchar *raw,
                                       gchar **tokens);
static void gibbon_session_dump_position (const GibbonSession *self,
                                          const GibbonPosition *pos);
#endif /* #ifdef GIBBON_SESSION_DEBUG_BOARD_STATE */

static gint gibbon_session_handle_number (GibbonSession *self,
                                          const gchar *line,
                                          const gchar **tokens);
static gint gibbon_session_clip_welcome (GibbonSession *self, GSList *iter);
static gint gibbon_session_clip_who_info (GibbonSession *self,
                                          const gchar *line,
                                          const gchar **tokens);
static gint gibbon_session_clip_logout (GibbonSession *self,
                                        const gchar *line,
                                        const gchar **tokens);
static gint gibbon_session_clip_says (GibbonSession *self,
                                      const gchar *line,
                                      const gchar **tokens);
static gint gibbon_session_clip_shouts (GibbonSession *self,
                                        const gchar *line,
                                        const gchar **tokens);
static gint gibbon_session_clip_whispers (GibbonSession *self,
                                          const gchar *line,
                                          const gchar **tokens);
static gint gibbon_session_clip_kibitzes (GibbonSession *self,
                                          const gchar *line,
                                          const gchar **tokens);
static gint gibbon_session_clip_you_say (GibbonSession *self,
                                         const gchar *line,
                                         const gchar **tokens);
static gint gibbon_session_clip_you_shout (GibbonSession *self,
                                           const gchar *line,
                                           const gchar **tokens);
static gint gibbon_session_clip_you_whisper (GibbonSession *self,
                                             const gchar *line,
                                             const gchar **tokens);
static gint gibbon_session_clip_you_kibitz (GibbonSession *self,
                                            const gchar *line,
                                            const gchar **tokens);
static gboolean gibbon_session_handle_board (GibbonSession *self,
                                             const gchar **tokens);
static gboolean gibbon_session_handle_youre (GibbonSession *self,
                                               const gchar **tokens);
static gboolean gibbon_session_handle_youre_now (GibbonSession *self,
                                                 const gchar **tokens);
static gboolean gibbon_session_handle_youre_now_watching (GibbonSession *self,
                                                          const gchar **tokens);
static gchar *gibbon_session_decode_client (GibbonSession *self,
                                            const gchar *token);
static gboolean gibbon_session_handle_one_of_us (GibbonSession *self,
                                                 GibbonSessionPlayer player,
                                                 const gchar *line);
static gboolean gibbon_session_handle_someone (GibbonSession *self,
                                               const gchar *player,
                                               const gchar *line);
static gboolean gibbon_session_handle_someone_and (GibbonSession *self,
                                                   const gchar *player1,
                                                   const gchar *line);
static gboolean gibbon_session_handle_someone_wins (GibbonSession *self,
                                                    const gchar *player1,
                                                    const gchar *line);
static gboolean gibbon_session_handle_rolls (GibbonSession *self,
                                             GibbonSessionPlayer player,
                                             const gchar *line);
static gboolean gibbon_session_handle_moves (GibbonSession *self,
                                             GibbonSessionPlayer player,
                                             const gchar *line);

static gboolean parse_integer (const gchar *str, gint* result,
                               const gchar *what,
                               gint lower, gint upper);

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

        GibbonArchive *archive;
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
        self->priv->position = NULL;
        self->priv->player_list = NULL;
        self->priv->archive = NULL;
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
        guint64 code;

        g_return_val_if_fail (GIBBON_IS_SESSION (self), -1);
        g_return_val_if_fail (line != NULL, -1);

        values = gibbon_clip_parse (line);
        if (!values)
                return -1;

        iter = values;
        if (!gibbon_clip_get_uint64 (&iter, GIBBON_CLIP_TYPE_UINT, &code)) {
                gibbon_clip_free_result (iter);
                return -1;
        }

        switch (code) {
        case GIBBON_CLIP_CODE_UNHANDLED:
                break;
        case GIBBON_CLIP_CODE_WELCOME:
                retval = gibbon_session_clip_welcome (self, iter);
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
        gchar *mail;
        GibbonServerConsole *console;
        GibbonPrefs *prefs;

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
        
        return GIBBON_CLIP_CODE_WELCOME;
}

static gint
gibbon_session_clip_who_info (GibbonSession *self, 
                              const gchar *line,
                              const gchar **tokens)
{
        const gchar *who;
        const gchar *opponent;
        const gchar *watching;
        gboolean ready;
        gboolean available;
        gboolean away;
        gdouble rating;
        gint experience;
        const gchar *idle;
        const gchar *login;
        gchar *client;
        const gchar *email;
        const gchar *hostname;
        GibbonConnection *connection;
        GibbonArchive *archive;
        gdouble reliability;
        guint confidence;
        const gchar *account;
        guint port;
        const gchar *server;

        g_return_val_if_fail (GIBBON_IS_SESSION (self), FALSE);

        g_return_val_if_fail (13 == g_strv_length ((gchar **) tokens), -1);

        who = tokens[1];
        
        opponent = tokens[2];
        if (opponent[0] == '-' && opponent[1] == 0)
                opponent = "";
                
        watching = tokens[3];
        if (watching[0] == '-' && watching[1] == 0)
                watching = "";
                
        g_return_val_if_fail (tokens[4][0] == '0' || tokens[4][0] == '1',
                              -1);
        g_return_val_if_fail (tokens[4][1] == 0, -1);
        if (tokens[4][0] == '1') {
                ready = TRUE;
        } else {
                ready = FALSE;
        }
        
        g_return_val_if_fail (tokens[5][0] == '0' || tokens[5][0] == '1', -1);
        g_return_val_if_fail (tokens[5][1] == 0, -1);
        if (tokens[5][0] == '1') {
                away = TRUE;
        } else {
                away = FALSE;
        }

        rating = g_ascii_strtod (tokens[6], NULL);
        g_return_val_if_fail (parse_integer (tokens[7], &experience,
                                             "experience", 0, G_MAXINT),
                              -1);
        
        idle = tokens[8];
        login = tokens[9];
        hostname = tokens[10];
        client = gibbon_session_decode_client (self, tokens[11]);
        email = tokens[12];

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

        gibbon_player_list_set (self->priv->player_list,
                                who, available, rating, experience,
                                reliability, confidence,
                                opponent, watching, client, email);
        g_free (client);

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

        gibbon_archive_update_user (archive,
                                    gibbon_connection_get_hostname (connection),
                                    gibbon_connection_get_port (connection),
                                    who, rating, experience);

        return CLIP_WHO_INFO;
}

static gint
gibbon_session_clip_logout (GibbonSession *self,
                            const gchar *line,
                            const gchar **tokens)
{
        const gchar *name;
        gchar *opponent;

        name = tokens[1];

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
                gibbon_archive_save_drop (self->priv->archive, name, opponent);
                g_free (opponent);
        }
        gibbon_player_list_remove (self->priv->player_list, name);

        return CLIP_LOGOUT;
}

static gint
gibbon_session_clip_says (GibbonSession *self,
                          const gchar *line,
                          const gchar **tokens)
{
        GibbonFIBSMessage *fibs_message;
        GibbonConnection *connection;
        const gchar *message;

        message = gibbon_skip_ws_tokens (line, tokens, 1);
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
                            const gchar *line,
                            const gchar **tokens)
{
        GibbonFIBSMessage *fibs_message;
        GibbonShouts *shouts;
        const gchar *message;

        g_return_val_if_fail (GIBBON_IS_SESSION (self), FALSE);

        message = gibbon_skip_ws_tokens (line, tokens, 1);
        fibs_message = gibbon_fibs_message_new (message);
        if (!fibs_message)
                return -1;

        shouts = gibbon_app_get_shouts (self->priv->app);
        gibbon_shouts_append_message (shouts, fibs_message);

        gibbon_fibs_message_free (fibs_message);

        return CLIP_SHOUTS;
}

static gint
gibbon_session_clip_whispers (GibbonSession *self,
                              const gchar *line,
                              const gchar **tokens)
{
        GibbonFIBSMessage *fibs_message;
        GibbonGameChat *game_chat;
        const gchar *message;

        g_return_val_if_fail (GIBBON_IS_SESSION (self), FALSE);

        message = gibbon_skip_ws_tokens (line, tokens, 1);
        fibs_message = gibbon_fibs_message_new (message);
        if (!fibs_message)
                return -1;

        game_chat = gibbon_app_get_game_chat (self->priv->app);
        gibbon_game_chat_append_message (game_chat, fibs_message);

        gibbon_fibs_message_free (fibs_message);

        return CLIP_WHISPERS;
}

static gint
gibbon_session_clip_kibitzes (GibbonSession *self,
                              const gchar *line,
                              const gchar **tokens)
{
        GibbonFIBSMessage *fibs_message;
        GibbonGameChat *game_chat;
        const gchar *message;

        g_return_val_if_fail (GIBBON_IS_SESSION (self), FALSE);

        message = gibbon_skip_ws_tokens (line, tokens, 1);
        fibs_message = gibbon_fibs_message_new (message);
        if (!fibs_message)
                return -1;

        game_chat = gibbon_app_get_game_chat (self->priv->app);
        gibbon_game_chat_append_message (game_chat, fibs_message);

        gibbon_fibs_message_free (fibs_message);

        return CLIP_KIBITZES;
}

static gint
gibbon_session_clip_you_say (GibbonSession *self,
                             const gchar *line,
                             const gchar **tokens)
{
        GibbonFIBSMessage *fibs_message;
        GibbonConnection *connection;
        gchar *receiver;
        const gchar *message;

        g_return_val_if_fail (GIBBON_IS_SESSION (self), FALSE);

        message = gibbon_skip_ws_tokens (line, tokens, 1);
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
                               const gchar *line,
                               const gchar **tokens)
{
        GibbonFIBSMessage *fibs_message;
        GibbonConnection *connection;
        const gchar *message;

        g_return_val_if_fail (GIBBON_IS_SESSION (self), FALSE);

        connection = gibbon_app_get_connection (self->priv->app);
        if (!connection)
                return FALSE;

        message = gibbon_skip_ws_tokens (line, tokens, 1);
        fibs_message = g_malloc (sizeof *fibs_message);
        fibs_message->message = g_strdup (message);
        fibs_message->sender =
                g_strdup (gibbon_connection_get_login (connection));

        gibbon_app_show_shout (self->priv->app, fibs_message);

        gibbon_fibs_message_free (fibs_message);

        return CLIP_YOU_SHOUT;
}

static gint
gibbon_session_clip_you_whisper (GibbonSession *self,
                                 const gchar *line,
                                 const gchar **tokens)
{
        GibbonFIBSMessage *fibs_message;
        GibbonConnection *connection;
        const gchar *message;

        g_return_val_if_fail (GIBBON_IS_SESSION (self), FALSE);

        connection = gibbon_app_get_connection (self->priv->app);
        if (!connection)
                return FALSE;

        message = gibbon_skip_ws_tokens (line, tokens, 1);
        fibs_message = g_malloc (sizeof *fibs_message);
        fibs_message->message = g_strdup (message);
        fibs_message->sender =
                g_strdup (gibbon_connection_get_login (connection));

        gibbon_app_show_game_chat (self->priv->app, fibs_message);

        gibbon_fibs_message_free (fibs_message);

        return CLIP_YOU_WHISPER;
}

static gint
gibbon_session_clip_you_kibitz (GibbonSession *self,
                                const gchar *line,
                                const gchar **tokens)
{
        GibbonFIBSMessage *fibs_message;
        GibbonConnection *connection;
        const gchar *message;

        g_return_val_if_fail (GIBBON_IS_SESSION (self), FALSE);

        connection = gibbon_app_get_connection (self->priv->app);
        if (!connection)
                return FALSE;

        message = gibbon_skip_ws_tokens (line, tokens, 1);
        fibs_message = g_malloc (sizeof *fibs_message);
        fibs_message->message = g_strdup (message);
        fibs_message->sender =
                g_strdup (gibbon_connection_get_login (connection));

        gibbon_app_show_game_chat (self->priv->app, fibs_message);

        gibbon_fibs_message_free (fibs_message);

        return CLIP_YOU_KIBITZ;
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
gibbon_session_handle_board (GibbonSession *self, const gchar **tokens)
{
        GibbonPosition *pos;
        GibbonPositionSide turn, color, direction;
        gint may_double[2];
        GibbonBoard *board;
        gint i;
        gint dice[4];
        GibbonConnection *connection;
                        
        g_return_val_if_fail (GIBBON_IS_SESSION (self), FALSE);
        g_return_val_if_fail (tokens, FALSE);

        pos = gibbon_position_new ();
        if (self->priv->position) {
                if (self->priv->position->game_info)
                        pos->game_info =
                                g_strdup (self->priv->position->game_info);
                if (self->priv->position->status)
                        pos->status = g_strdup (self->priv->position->status);
        }

#ifdef GIBBON_SESSION_DEBUG_BOARD_STATE
        gibbon_session_dump_board (self, string, tokens);
#endif

        g_return_val_if_fail (52 == g_strv_length ((gchar **) tokens), -1);

        g_return_val_if_fail (parse_integer (tokens[31], &turn,
                              "turn", -1, 1), -1);
        g_return_val_if_fail (parse_integer (tokens[40], &color,
                              "color", -1, 1), -1);
        g_return_val_if_fail (parse_integer (tokens[41], &direction,
                              "direction", -1, 1), -1);
        if (!direction) {
                g_critical (_("Invalid direction 0 in board state.\n"));
                return FALSE;
        }

        if (g_strcmp0 ("You", tokens[0])) {
                pos->players[0] = g_strdup (tokens[0]);
                g_free (self->priv->watching);
                self->priv->watching = g_strdup (pos->players[0]);
        } else {
                connection = gibbon_app_get_connection (self->priv->app);
                pos->players[0] =
                        g_strdup (gibbon_connection_get_login (connection));
                g_free (self->priv->watching);
                self->priv->watching = NULL;
        }
        g_return_val_if_fail (pos->players[0][0], -1);
        
        pos->players[1] = g_strdup (tokens[1]);
        g_return_val_if_fail (pos->players[1][0], -1);

        if (g_strcmp0 (self->priv->opponent, pos->players[1])) {
                g_free (self->priv->opponent);
                self->priv->opponent = g_strdup (pos->players[1]);
        }
        
        g_return_val_if_fail (parse_integer (tokens[2], &pos->match_length,
                              "match length", 0, G_MAXINT), -1);

        g_return_val_if_fail (parse_integer (tokens[3], &pos->scores[0],
                                             "score0", 0, G_MAXINT), -1);
        g_return_val_if_fail (parse_integer (tokens[4], &pos->scores[1],
                                             "score1", 0, G_MAXINT), -1);
        
        if (direction == GIBBON_POSITION_SIDE_BLACK) {
                for (i = 6; i < 30; ++i) {
                        g_return_val_if_fail (parse_integer (tokens[i],
                                                             &pos->points[i - 6],
                                                             "checker", -15, 15),
                                              -1);
                        pos->points[i - 6] *= color;
                }
        } else {
                for (i = 6; i < 30; ++i) {
                        g_return_val_if_fail (parse_integer (tokens[i],
                                                             &pos->points[29 - i],
                                                             "checker", -15, 15),
                                              -1);
                        pos->points[29 - i] *= color;
                }

        }
        
        g_return_val_if_fail (parse_integer (tokens[32], &dice[0],
                                             "dice[0]", 0, 6), -1);
        g_return_val_if_fail (parse_integer (tokens[33], &dice[1],
                                             "dice[1]", 0, 6), -1);
        g_return_val_if_fail (parse_integer (tokens[34], &dice[2],
                                             "dice[2]", 0, 6), -1);
        g_return_val_if_fail (parse_integer (tokens[35], &dice[3],
                                             "dice[3]", 0, 6), -1);
        g_return_val_if_fail (parse_integer (tokens[36], &pos->cube,
                                             "cube", 0, G_MAXINT), -1);
        g_return_val_if_fail (parse_integer (tokens[37], &may_double[0],
                                             "may double 0", 0, 1), -1);
        g_return_val_if_fail (parse_integer (tokens[38], &may_double[1],
                                             "may double 0", 0, 1), -1);

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
                                             0, 15), -1);

        g_return_val_if_fail (parse_integer (tokens[47], &pos->bar[1], "bar1",
                                             0, 15), -1);

        
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
                g_printerr ("Board positions are equal.\n");
                gibbon_position_free (pos);
        } else {
                g_printerr ("Board positions are not equal.\n");
                gibbon_board_set_position (GIBBON_BOARD (board), pos);
        }

#ifdef GIBBON_SESSION_DEBUG_BOARD_STATE
        gibbon_session_dump_position (self, self->priv->position);
#endif

        return TRUE;
}

static gboolean
gibbon_session_handle_youre (GibbonSession *self,
                             const gchar **tokens)
{
        if (!tokens[1])
                return FALSE;

        if (!g_strcmp0 ("now", tokens[1]))
                return gibbon_session_handle_youre_now (self, tokens);

        return FALSE;
}

static gboolean
gibbon_session_handle_youre_now (GibbonSession *self,
                                 const gchar **tokens)
{
        if (!tokens[2])
                return FALSE;

        if (!g_strcmp0 ("watching", tokens[2]))
                return gibbon_session_handle_youre_now_watching (self, tokens);

        return FALSE;
}

static gboolean
gibbon_session_handle_youre_now_watching (GibbonSession *self,
                                          const gchar **tokens)
{
        gchar *player;
        gsize length;

        if (!tokens[3])
                return FALSE;

        player = g_strdup (tokens[3]);
        length = strlen (player);
        if ('.' == player[length - 1])
                player[length - 1] = 0;

        g_free (self->priv->watching);
        self->priv->watching = g_strdup (player);
        g_free (self->priv->opponent);
        self->priv->opponent = NULL;

        return TRUE;
}

/* FIXME! Use g_ascii_strtoll in this function! */
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
gibbon_session_dump_board (const GibbonSession *self,
                           const gchar *raw,
                           gchar **tokens)
{
        int i = 0;

        g_printerr ("=== Board ===\n");
        g_printerr ("board:%s\n", raw);
        for (i = 0; keys[i]; ++i)
                g_printerr ("%s (%s)\n", keys[i], tokens[i]);
}

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
gibbon_session_handle_one_of_us (GibbonSession *self,
                                 GibbonSessionPlayer player,
                                 const gchar *line)
{
        if (0 == strncmp ("rolls ", line, 6)) {
                if (player == GIBBON_SESSION_PLAYER_OPPONENT
                    || player == GIBBON_SESSION_PLAYER_WATCHING)
                        return gibbon_session_handle_rolls (self, player,
                                                            line + 6);
                return FALSE;
        } else if (0 == strncmp ("moves ", line, 6)) {
                if (player == GIBBON_SESSION_PLAYER_OPPONENT
                    || player == GIBBON_SESSION_PLAYER_WATCHING)
                        return gibbon_session_handle_moves (self, player,
                                                            line + 6);
                return FALSE;
        }

        return FALSE;
}

static gboolean
gibbon_session_handle_someone (GibbonSession *self,
                               const gchar *player,
                               const gchar *line)

{
        if (0 == strncmp ("and ", line, 4)) {
                return gibbon_session_handle_someone_and (self, player,
                                                          line + 4);
        } else if (0 == strncmp ("wins a ", line, 7)) {
                return gibbon_session_handle_someone_wins (self, player,
                                                           line + 7);
        }

        return FALSE;
}

static gboolean
gibbon_session_handle_someone_and (GibbonSession *self,
                                   const gchar *player1,
                                   const gchar *line)
{
        gchar **tokens;
        gchar *player2;

        tokens = g_strsplit (line, " ", 0);
        if (!tokens)
                return FALSE;

        player2 = tokens[0];
        if (!player2) {
                g_strfreev (tokens);
                return FALSE;
        }

        if (!gibbon_player_list_exists (self->priv->player_list, player2)) {
                g_strfreev (tokens);
                return FALSE;
        }
        if (0 == g_strcmp0 ("start", tokens[1])
            && 0 == g_strcmp0 ("a", tokens[2])
            && '1' <= tokens[3][0]
            && '9' >= tokens[3][0]
            && 0 == g_strcmp0 ("point", tokens[4])
            && 0 == g_strcmp0 ("match.", tokens[5])) {
                /* Ignore.  */
                g_strfreev (tokens);
                return TRUE;
        } else if (0 == g_strcmp0 ("are", tokens[1])
                   && 0 == g_strcmp0 ("resuming", tokens[2])
                   && 0 == g_strcmp0 ("their", tokens[3])
                   && '1' <= tokens[4][0]
                   && '9' >= tokens[4][0]
                   && 0 == g_strcmp0 ("match.", tokens[5])) {
                /* Give the dropper a little compensation.  */
                gibbon_archive_save_resume (self->priv->archive,
                                            player1, player2);
                g_strfreev (tokens);
                return TRUE;
        }

        g_strfreev (tokens);
        return FALSE;
}

static gboolean
gibbon_session_handle_someone_wins (GibbonSession *self,
                                    const gchar *player1,
                                    const gchar *line)
{
        gchar **tokens;

        tokens = g_strsplit_set (line, " ", 0);
        if (!tokens)
                return FALSE;

        if ('1' <= tokens[0][0]
            && '9' >= tokens[0][0]
            && 0 == g_strcmp0 ("point", tokens[1])
            && 0 == g_strcmp0 ("match", tokens[2])
            && 0 == g_strcmp0 ("against", tokens[3])
            && tokens[4]
            && gibbon_player_list_exists (self->priv->player_list,
                                          tokens[4])) {
                gibbon_archive_save_win (self->priv->archive, player1,
                                         tokens[4]);
                g_strfreev (tokens);
                return TRUE;
        }

        g_strfreev (tokens);
        return FALSE;
}

static gboolean
gibbon_session_handle_rolls (GibbonSession *self, GibbonSessionPlayer player,
                               const gchar *line)
{
        guint dice[2];

        if (*line >= '1' && *line <= '6')
                dice[0] = *line++ - '0';
        else
                return FALSE;

        if (strncmp (line, " and ", 5))
                return FALSE;

        line += 5;

        if (*line >= '1' && *line <= '6')
                dice[1] = *line++ - '0';
        else
                return FALSE;

        if (*line++ != '.')
                return FALSE;

        if (*line)
                return FALSE;

        switch (player) {
                case GIBBON_SESSION_PLAYER_OTHER:
                        return FALSE;
                case GIBBON_SESSION_PLAYER_OPPONENT:
                        self->priv->position->dice[0] = -dice[0];
                        self->priv->position->dice[1] = -dice[1];
                        g_free (self->priv->position->game_info);
                        self->priv->position->status =
                                g_strdup_printf (_("%s rolls %u and %u."),
                                                 self->priv->opponent,
                                                 dice[0], dice[1]);
                        break;
                case GIBBON_SESSION_PLAYER_YOU:
                        self->priv->position->dice[0] = dice[0];
                        self->priv->position->dice[1] = dice[1];
                        self->priv->position->status =
                                g_strdup_printf (_("You roll %u and %u."),
                                                 dice[0], dice[1]);
                        break;
                case GIBBON_SESSION_PLAYER_WATCHING:
                        self->priv->position->dice[0] = dice[0];
                        self->priv->position->dice[1] = dice[1];
                        self->priv->position->status =
                                g_strdup_printf (_("%s rolls %u and %u."),
                                                 self->priv->watching,
                                                 dice[0], dice[1]);
                        break;
        }

        gibbon_board_set_position (gibbon_app_get_board (self->priv->app),
                                   gibbon_position_copy (self->priv->position));

        return TRUE;
}

static gboolean
gibbon_session_handle_moves (GibbonSession *self, GibbonSessionPlayer player,
                             const gchar *line)
{
        GibbonMove *move = g_alloca (sizeof move->number
                                     + 4 * sizeof *move->movements
                                     + sizeof move->status);
        GibbonMovement *movement;
        gchar **tokens;
        gchar *token;
        gchar *endptr;
        gint i;
        guint64 from, to;
        GibbonPositionSide side;
        gchar *pretty_move;
        gint *dice;

        move->number = 0;

        if (player == GIBBON_SESSION_PLAYER_OPPONENT)
                side = GIBBON_POSITION_SIDE_BLACK;
        else
                side = GIBBON_POSITION_SIDE_WHITE;

        tokens = g_strsplit_set (line, " .", 0);
        if (!tokens)
                return FALSE;

        for (i = 0; tokens[i] && *tokens[i]; ++i) {
                if (i >= 4) {
                        g_critical ("Too many tokens in move '%s'.\n", line);
                        g_strfreev (tokens);
                        return FALSE;
                }
                token = tokens[i];
                movement = move->movements + move->number++;
                if (0 == strncmp ("bar", token, 3)) {
                        movement->from = -1;
                        endptr = token + 3;
                } else {
                        errno = 0;
                        from = g_ascii_strtoull (token, &endptr, 10);
                        if (errno) {
                                g_critical ("Error parsing token #%d in"
                                            " move '%s': %s!\n",
                                            i, line, strerror (errno));
                                g_strfreev (tokens);
                                return FALSE;
                        }
                        if (from == 0 || from > 24) {
                                g_critical ("Error parsing token #%d in"
                                            " move '%s': Starting point out"
                                            " of range!\n",
                                            i, line);
                                g_strfreev (tokens);
                                return FALSE;
                        }
                        movement->from = from;
                }

                if ('-' != *endptr) {
                        g_critical ("Error parsing token #%d in"
                                    " move '%s': Expected '-'!\n",
                                    i, line);
                        g_strfreev (tokens);
                        return FALSE;
                }

                token = endptr + 1;

                if (0 == strncmp ("off", token, 3)) {
                        movement->to = -1;
                        endptr = token + 3;
                } else {
                        errno = 0;
                        to = g_ascii_strtoull (token, &endptr, 10);
                        if (errno) {
                                g_critical ("Error parsing token #%d in"
                                            " move '%s': %s!\n",
                                            i, line, strerror (errno));
                                g_strfreev (tokens);
                                return FALSE;
                        }
                        if (to == 0 || to > 24) {
                                g_critical ("Error parsing token #%d in"
                                            " move '%s': Landing point out"
                                            " of range (%s)!\n",
                                            i, line, token);
                                g_strfreev (tokens);
                                return FALSE;
                        }
                        movement->to = to;
                }

                if (*endptr) {
                        g_critical ("Error parsing token #%d in"
                                    " move '%s': Trailing garbage!\n",
                                    i, line);
                        g_strfreev (tokens);
                        return FALSE;
                }

                /* Now repair the move according to our notion.  The
                 * direction of the move is actually giving in the board
                 * state but that is even more unreliable because we
                 * could have missed it.
                 *
                 * First assign bar and home accordingly.
                 */
                if  (movement->from == -1) {
                        if (movement->to <= 6) {
                                movement->from = 0;
                        } else {
                                movement->from = 25;
                        }
                }
                if (movement->to == -1) {
                        if (movement->from <= 6) {
                                movement->to = 0;
                        } else {
                                movement->to = 25;
                        }
                }

                /* At this point, the direction is determined.  We want white
                 * to move from his 24 to his ace point, and black the
                 * other way round.
                 */
                if ((side == GIBBON_POSITION_SIDE_WHITE
                     && movement->from < movement->to)
                    || (side == GIBBON_POSITION_SIDE_BLACK
                        && movement->from > movement->to)) {
                        movement->from = 25 - movement->from;
                        movement->to = 25 - movement->to;
                }
        }

        g_strfreev (tokens);

        pretty_move = gibbon_position_format_move (self->priv->position, move,
                                                   side, FALSE);
        g_free (self->priv->position->status);
        self->priv->position->status = NULL;
        dice = self->priv->position->dice;
        switch (player) {
                case GIBBON_SESSION_PLAYER_OTHER:
                        g_free (pretty_move);
                        return FALSE;
                case GIBBON_SESSION_PLAYER_OPPONENT:
                        self->priv->position->status =
                            g_strdup_printf (_("%d%d: %s moves %s."),
                                             abs (dice[0]),
                                             abs (dice[1]),
                                             self->priv->opponent,
                                             pretty_move);
                        break;
                case GIBBON_SESSION_PLAYER_YOU:
                        self->priv->position->status =
                            g_strdup_printf (_("%d%d: You move %s."),
                                             abs (dice[0]),
                                             abs (dice[1]),
                                             pretty_move);
                        break;
                case GIBBON_SESSION_PLAYER_WATCHING:
                        self->priv->position->status =
                            g_strdup_printf (_("%d%d: %s moves %s."),
                                             abs (dice[0]),
                                             abs (dice[1]),
                                             self->priv->watching,
                                             pretty_move);
                        break;
        }
        if (!gibbon_position_apply_move (self->priv->position, move,
                                         side, FALSE)) {
                g_critical ("Error applying move %s to position.",
                            pretty_move);
                g_critical ("Move received from FIBS: %s.", line);
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

        return TRUE;
}
