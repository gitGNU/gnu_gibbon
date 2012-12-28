/*
 * This file is part of gibbon.
 * Gibbon is a Gtk+ frontend for the First Internet Backgammon Server FIBS.
 * Copyright (C) 2009-2012 Guido Flohr, http://guido-flohr.net/.
 *
 * gibbon is free software: you can redistribute it and/or modify 
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * gibbon is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with gibbon.  If not, see <http://www.gnu.org/licenses/>.
 */

/**
 * SECTION:gibbon-app
 * @short_description: The Gibbon Application!
 *
 * Since: 0.1.0
 *
 * Class representing the running Gibbon application!
 **/

#include <glib/gi18n.h>
#include <gtk/gtk.h>
#include <gdk-pixbuf/gdk-pixbuf.h>

#include <stdlib.h>

#include "gibbon-app.h"
#include "gibbon-archive.h"
#include "gibbon-board.h"
#include "gibbon-cairoboard.h"
#include "gibbon-game-chat.h"
#include "gibbon-connection-dialog.h"
#include "gibbon-connection.h"
#include "gibbon-signal.h"
#include "gibbon-server-console.h"
#include "gibbon-shouts.h"
#include "gibbon-chat-view.h"
#include "gibbon-chat.h"
#include "gibbon-help.h"
#include "gibbon-player-list.h"
#include "gibbon-player-list-view.h"
#include "gibbon-inviter-list.h"
#include "gibbon-inviter-list-view.h"
#include "gibbon-session.h"
#include "gibbon-client-icons.h"
#include "gibbon-settings.h"
#include "gibbon-register-dialog.h"
#include "gibbon-match-list.h"
#include "gibbon-game-list-view.h"
#include "gibbon-move-list-view.h"
#include "gibbon-match-loader.h"
#include "gibbon-game.h"
#include "gibbon-analysis-view.h"
#include "gibbon-met.h"
#include "gibbon-java-fibs-importer.h"

enum gibbon_app_list_signal {
        NEW_MATCH,
        ACTION_ADDED,
        LAST_SIGNAL
};
static guint gibbon_app_list_signals[LAST_SIGNAL] = { 0 };

gchar *gibbon_app_pixmaps_directory = NULL;

typedef struct _GibbonAppPrivate GibbonAppPrivate;
struct _GibbonAppPrivate {
        GtkBuilder *builder;
        gchar *pixmaps_directory;
        GtkWidget *window;
        GtkWidget *statusbar;
        GibbonServerConsole *server_console;
        GibbonCairoboard *board;
        GibbonGameChat *game_chat;
        GibbonConnection *connection;
        GibbonShouts *shouts;

        GibbonSignal *connecting_signal;
        GibbonSignal *connected_signal;
        GibbonSignal *logged_in_signal;
        GibbonSignal *network_error_signal;
        GibbonSignal *disconnected_signal;

        GHashTable *chats;

        GibbonArchive *archive;

        GibbonPlayerList *player_list;
        GibbonPlayerListView *player_list_view;

        GibbonInviterList *inviter_list;
        GibbonInviterListView *inviter_list_view;

        GibbonClientIcons *client_icons;

        GibbonMatch *match;
        GibbonMatchList *match_list;
        GibbonGameListView *game_list_view;
        GibbonMoveListView *move_list_view;
        GibbonAnalysisView *analysis_view;

        GibbonMET *met;
};

#define GIBBON_APP_PRIVATE(obj) (G_TYPE_INSTANCE_GET_PRIVATE ((obj), \
        GIBBON_TYPE_APP, GibbonAppPrivate))

G_DEFINE_TYPE (GibbonApp, gibbon_app, G_TYPE_OBJECT)

static GtkBuilder *gibbon_app_get_builder (GibbonApp *self, const gchar *path);
static GibbonCairoboard *gibbon_app_init_board (GibbonApp *self,
                                                const gchar *board_filename);
static gboolean gibbon_app_init_match_list (GibbonApp *self,
                                            const gchar *filename);

static void gibbon_app_connect_signals (const GibbonApp *self);
static void gibbon_app_set_icon (const GibbonApp *self, const gchar *directory);

/* Signal handlers.  */
static void gibbon_app_on_connect_request (GibbonApp *self, GtkWidget *emitter);
static void gibbon_app_on_register_request (GibbonApp *self);
static gboolean gibbon_app_on_quit_request (GibbonApp *self,
                                            GtkWidget *emitter);
static void gibbon_app_on_open (GibbonApp *self, GtkWidget *emitter);
static void gibbon_app_on_connecting (GibbonApp *self,
                                      GibbonConnection *connection);
static void gibbon_app_on_connected (GibbonApp *self,
                                     GibbonConnection *connection);
static void gibbon_app_on_logged_in (GibbonApp *self,
                                     GibbonConnection *connection);
static void gibbon_app_on_network_error (GibbonApp *self,
                                         const gchar *error_msg);
static void gibbon_app_on_account_prefs (GibbonApp *self);
static void gibbon_app_on_java_fibs_import (GibbonApp *self);
static void gibbon_app_on_toggle_ready (GibbonApp *self);
static void gibbon_app_on_board_refresh (GibbonApp *self);
static void gibbon_app_on_board_leave (GibbonApp *self);
static void gibbon_app_on_board_resign (GibbonApp *self);
static void gibbon_app_on_board_undo (GibbonApp *self);
static void gibbon_app_on_board_accept (GibbonApp *self);
static void gibbon_app_on_board_reject (GibbonApp *self);
static void gibbon_app_on_action_selected (GibbonApp *self, gint action_no);

static GibbonApp *singleton = NULL;

static const char * const gibbon_app_icon_sizes[] = { "16x16", "22x22",
                "24x24", "32x32", "48x48", "128x128",
                /* 256x256 causes a Gdk warning "icons too large".  */
                /* "256x256", */
                "scalable", };

static void gibbon_app_init(GibbonApp *self)
{
        self->priv = G_TYPE_INSTANCE_GET_PRIVATE (self,
                        GIBBON_TYPE_APP, GibbonAppPrivate);

        self->priv->builder = NULL;
        self->priv->pixmaps_directory =  NULL;

        self->priv->board = NULL;
        self->priv->window = NULL;
        self->priv->statusbar = NULL;
        if (self->priv->server_console)
                g_object_unref(self->priv->server_console);
        self->priv->server_console = NULL;
        self->priv->game_chat = NULL;
        self->priv->connection = NULL;
        self->priv->shouts = NULL;

        self->priv->connecting_signal = NULL;
        self->priv->connected_signal = NULL;
        self->priv->logged_in_signal = NULL;
        self->priv->network_error_signal = NULL;
        self->priv->disconnected_signal = NULL;

        self->priv->chats = NULL;

        self->priv->archive = NULL;

        self->priv->player_list = NULL;
        self->priv->player_list_view = NULL;
        self->priv->inviter_list = NULL;
        self->priv->inviter_list_view = NULL;

        self->priv->client_icons = NULL;

        self->priv->match = NULL;
        self->priv->match_list = NULL;
        self->priv->game_list_view = NULL;
        self->priv->move_list_view = NULL;
        self->priv->analysis_view = NULL;
}

static void gibbon_app_finalize(GObject *object)
{
        GibbonApp *self = GIBBON_APP (object);

        gibbon_app_disconnect(self);

        if (self->priv->match)
                g_object_unref (self->priv->match);

        if (self->priv->client_icons)
                g_object_unref(self->priv->client_icons);

        if (self->priv->inviter_list_view)
                g_object_unref(self->priv->inviter_list_view);

        if (self->priv->inviter_list)
                g_object_unref(self->priv->inviter_list);

        if (self->priv->player_list_view)
                g_object_unref(self->priv->player_list_view);

        if (self->priv->player_list)
                g_object_unref(self->priv->player_list);

        if (self->priv->server_console)
                g_object_unref(self->priv->server_console);

        if (self->priv->archive)
                g_object_unref(self->priv->archive);

        if (self->priv->chats)
                g_hash_table_destroy(self->priv->chats);

        if (self->priv->analysis_view)
                g_object_unref (self->priv->analysis_view);

        if (self->priv->match_list)
                g_object_unref (self->priv->match_list);

        if (self->priv->window)
                gtk_widget_destroy (self->priv->window);

        G_OBJECT_CLASS (gibbon_app_parent_class)->finalize(object);
}

static void gibbon_app_class_init(GibbonAppClass *klass)
{
        GObjectClass *object_class = G_OBJECT_CLASS (klass);

        g_type_class_add_private(klass, sizeof(GibbonAppPrivate));

        gibbon_app_list_signals[NEW_MATCH] =
                g_signal_new ("new-match",
                              G_TYPE_FROM_CLASS (klass),
                              G_SIGNAL_RUN_FIRST,
                              0,
                              NULL, NULL,
                              g_cclosure_marshal_VOID__OBJECT,
                              G_TYPE_NONE,
                              1,
                              G_TYPE_OBJECT);

        gibbon_app_list_signals[ACTION_ADDED] =
                g_signal_new ("action-added",
                              G_TYPE_FROM_CLASS (klass),
                              G_SIGNAL_RUN_FIRST,
                              0,
                              NULL, NULL,
                              g_cclosure_marshal_VOID__OBJECT,
                              G_TYPE_NONE,
                              1,
                              G_TYPE_OBJECT);

        object_class->finalize = gibbon_app_finalize;
}

/**
 * gibbon_app_new:
 * @builder_path: Complete path to the builder XML file.
 * @pixmaps_dir: Path to the pixmaps.
 *
 * Creates a new #GibbonApp.
 *
 * Returns: The newly created #GibbonApp or %NULL in case of failure.
 */
GibbonApp *
gibbon_app_new(const gchar *builder_path, const gchar *pixmaps_directory,
               const gchar *data_dir, const gchar *match_file)
{
        GibbonApp *self;
        gchar *board_filename;
        GError *error = NULL;

        g_return_val_if_fail (singleton == NULL, singleton);

        app = self = g_object_new (GIBBON_TYPE_APP, NULL);

        self->priv->met = gibbon_met_new ();

        self->priv->builder = gibbon_app_get_builder(self, builder_path);
        if (!self->priv->builder) {
                g_object_unref(self);
                return NULL;
        }

        self->priv->pixmaps_directory =
                        gibbon_app_pixmaps_directory =
                                        g_strdup(pixmaps_directory);

        self->priv->window
                        = GTK_WIDGET (gibbon_app_find_object (self, "window",
                                                        GTK_TYPE_WINDOW));
        self->priv->statusbar = GTK_WIDGET (gibbon_app_find_object (self,
                                        "statusbar",
                                        GTK_TYPE_STATUSBAR));
        self->priv->server_console = gibbon_server_console_new(self);

        board_filename = g_build_filename(pixmaps_directory, "boards",
                        "default.svg", NULL);
        self->priv->board = gibbon_app_init_board(self, board_filename);
        g_free(board_filename);
        if (!self->priv->board) {
                g_object_unref(self);
                return NULL;
        }

        gtk_statusbar_push(GTK_STATUSBAR (self->priv->statusbar), 0,
                        _("Disconnected"));

        self->priv->game_chat = gibbon_game_chat_new(self);
        if (!self->priv->game_chat) {
                g_object_unref(self);
                return NULL;
        }

        self->priv->shouts = gibbon_shouts_new(self);
        if (!self->priv->shouts) {
                g_object_unref(self);
                return NULL;
        }

        self->priv->chats = g_hash_table_new_full(g_str_hash, g_str_equal,
                        g_free, g_object_unref);

        gibbon_app_set_state_disconnected(self);

        self->priv->archive = gibbon_archive_new(&error);
        if (!self->priv->archive) {
                gibbon_app_display_error (self, NULL, "%s", error->message);
                g_error_free (error);
                g_object_unref(self);
                return NULL;
        }

        self->priv->client_icons = gibbon_client_icons_new (pixmaps_directory);

        self->priv->player_list = gibbon_player_list_new();
        self->priv->player_list_view = gibbon_player_list_view_new(self,
                        self->priv->player_list);

        self->priv->inviter_list = gibbon_inviter_list_new();
        self->priv->inviter_list_view = gibbon_inviter_list_view_new(self,
                        self->priv->inviter_list);

        gibbon_app_set_icon(self, data_dir);

        if (!gibbon_app_init_match_list (self, match_file)) {
                g_object_unref (self);
                return NULL;
        }
        singleton = self;

        gibbon_app_connect_signals (self);

        return self;
}

void
gibbon_app_post_init (const GibbonApp *self)
{
        gint game_no;
        const GibbonGame *game;
        guint action_no;
        GtkWidget *widget;
        gint maxpos, minpos;

        g_return_if_fail (GIBBON_IS_APP (self));

        gibbon_analysis_view_fixup_layout (self->priv->analysis_view);

        widget = gibbon_app_find_widget (self, "rinner-vpaned",
                                         GTK_TYPE_PANED);
        g_object_get (widget,
                      "min-position", &minpos,
                      "max-position", &maxpos,
                      NULL);
        gtk_paned_set_position (GTK_PANED (widget),
                                minpos + (maxpos - minpos) / 4);

        widget = gibbon_app_find_widget (self, "router-vpaned",
                                         GTK_TYPE_PANED);
        g_object_get (widget,
                      "min-position", &minpos,
                      "max-position", &maxpos,
                      NULL);
        gtk_paned_set_position (GTK_PANED (widget),
                                minpos + 3 * (maxpos - minpos) / 4);

        /*
         * If a match was passed on the command line it is already loaded
         * and set to the last action in the match.  Make sure that the
         * analysis data displayed is up-to-date.
         */
        game_no = gibbon_match_list_get_active_game (self->priv->match_list);
        if (game_no < 0)
                return;

        if (!self->priv->match)
                return;

        game = gibbon_match_get_nth_game (self->priv->match, game_no);
        if (!game)
                return;

        action_no = gibbon_game_get_num_actions (game);
        gibbon_analysis_view_set_analysis (self->priv->analysis_view,
                                           game, action_no - 1);

}

gboolean
gibbon_app_init_match_list (GibbonApp *self, const gchar *match_file)
{
        GibbonMatchList *list = gibbon_match_list_new ();
        GError *error = NULL;
        GibbonMatchLoader *loader;
        GibbonMatch *match;
        GibbonGameListView *game_list_view;
        GObject *obj;
        GtkTreeView *move_list_view;
        GCallback callback;

        obj = gibbon_app_find_object (self, "combo-game-select",
                                      GTK_TYPE_COMBO_BOX);

        game_list_view = gibbon_game_list_view_new (GTK_COMBO_BOX (obj), list);
        self->priv->game_list_view = game_list_view;

        obj = gibbon_app_find_object (self, "move-list-view",
                                      GTK_TYPE_TREE_VIEW);
        move_list_view = GTK_TREE_VIEW (obj);
        self->priv->move_list_view = gibbon_move_list_view_new (move_list_view,
                                                                list);
        callback = G_CALLBACK (gibbon_move_list_view_on_new_match);
        g_signal_connect_swapped (G_OBJECT (self), "new-match", callback,
                                  (gpointer) self->priv->move_list_view);

        g_signal_connect_swapped (G_OBJECT (self->priv->move_list_view),
                                  "action-selected",
                                  G_CALLBACK (gibbon_app_on_action_selected),
                                  (gpointer) self);

        self->priv->match_list = list;

        g_signal_connect_swapped (G_OBJECT (self), "new-match",
                                  G_CALLBACK (gibbon_match_list_on_new_match),
                                  (gpointer) self->priv->match_list);
        self->priv->analysis_view = gibbon_analysis_view_new (self);

        if (match_file) {
                /*
                 * FIXME! The whole thing should be more generic so that
                 * whenever a new match is loaded the same things happen.
                 */
                loader = gibbon_match_loader_new ();
                match = gibbon_match_loader_read_match (loader, match_file,
                                                        &error);

                if (!match) {
                        if (error) {
                                gibbon_app_display_error (self, match_file,
                                                          "%s",
                                                          error->message);
                                g_error_free (error);
                        }
                        g_object_unref (loader);
                        g_object_unref (self);
                        return FALSE;
                }

                gibbon_app_set_match (self, match);

                g_object_unref (loader);
        }

        return TRUE;
}

void
gibbon_app_set_match (GibbonApp *self, GibbonMatch *match)
{
        GObject *obj;

        g_return_if_fail (GIBBON_IS_APP (self));
        g_return_if_fail (GIBBON_IS_MATCH (match));

        if (self->priv->match)
                g_object_unref (self->priv->match);
        self->priv->match = match;

        obj = gibbon_app_find_object (self, "notebook-info-area",
                                      GTK_TYPE_NOTEBOOK);
        gtk_notebook_set_current_page (GTK_NOTEBOOK (obj), 2);

        g_signal_emit (self, gibbon_app_list_signals[NEW_MATCH], 0, match);
}

GibbonMatch *
gibbon_app_get_match (GibbonApp *self)
{
        g_return_val_if_fail (GIBBON_IS_APP (self), NULL);

        return self->priv->match;
}

void
gibbon_app_activate_player_list (const GibbonApp *self)
{
        GObject *obj;

        g_return_if_fail (self != NULL);
        g_return_if_fail (GIBBON_IS_APP (self));

        obj = gibbon_app_find_object (self, "notebook-info-area",
                                      GTK_TYPE_NOTEBOOK);
        gtk_notebook_set_current_page (GTK_NOTEBOOK (obj), 0);
}

static GtkBuilder *
gibbon_app_get_builder(GibbonApp *self, const gchar *path)
{
        GtkBuilder *builder = gtk_builder_new();
        GError *error = NULL;

        if (!gtk_builder_add_from_file(builder, path, &error)) {
                gibbon_app_display_error(self, NULL, "%s.\n%s", error->message,
                                _("Do you need to pass the"
                                                " option `--data-dir'?\n"));
                g_object_unref(G_OBJECT (builder));
                return NULL;
        }

        return builder;
}

void gibbon_app_display_error(const GibbonApp* self, const gchar *headline,
                              const gchar *message_format, ...)
{
        va_list args;
        gchar *message;
        GtkWidget *dialog;

        va_start (args, message_format);
        message = g_strdup_vprintf(message_format, args);
        va_end (args);

        if (headline)
                dialog = gtk_message_dialog_new_with_markup(
                                GTK_WINDOW (self->priv->window),
                                GTK_DIALOG_DESTROY_WITH_PARENT,
                                GTK_MESSAGE_ERROR,
                                GTK_BUTTONS_CLOSE,
                                "<span weight='bold' size='larger'>"
                                "%s</span>\n%s",
                                headline, message);
        else
                dialog = gtk_message_dialog_new_with_markup(
                                GTK_WINDOW (self->priv->window),
                                GTK_DIALOG_DESTROY_WITH_PARENT,
                                GTK_MESSAGE_ERROR,
                                GTK_BUTTONS_CLOSE,
                                "%s", message);

        g_free(message);

        gtk_dialog_run(GTK_DIALOG (dialog));

        gtk_widget_destroy(GTK_WIDGET (dialog));
}

void gibbon_app_fatal_error(const GibbonApp* self, const gchar *headline,
                            const gchar *message_format, ...)
{
        va_list args;
        gchar *message;
        GtkWidget *dialog;

        va_start (args, message_format);
        message = g_strdup_vprintf(message_format, args);
        va_end (args);

        if (headline)
                dialog = gtk_message_dialog_new_with_markup(
                                GTK_WINDOW (self->priv->window),
                                GTK_DIALOG_DESTROY_WITH_PARENT,
                                GTK_MESSAGE_ERROR,
                                GTK_BUTTONS_CLOSE,
                                "<span weight='bold' size='larger'>"
                                "%s: %s</span>\n%s",
                                _("Fatal error"), headline, message);
        else
                dialog = gtk_message_dialog_new_with_markup(
                                GTK_WINDOW (self->priv->window),
                                GTK_DIALOG_DESTROY_WITH_PARENT,
                                GTK_MESSAGE_ERROR,
                                GTK_BUTTONS_CLOSE,
                                _("Fatal error: %s"), message);

        g_free(message);

        gtk_dialog_run(GTK_DIALOG (dialog));

        gtk_widget_destroy(GTK_WIDGET (dialog));

        gtk_main_quit ();
}

void gibbon_app_display_info(const GibbonApp *self, const gchar *headline,
                             const gchar *message_format, ...)
{
        va_list args;
        gchar *message;
        GtkWidget *dialog;

        va_start (args, message_format);
        message = g_strdup_vprintf(message_format, args);
        va_end (args);

        if (headline)
                dialog = gtk_message_dialog_new_with_markup(
                                GTK_WINDOW (self->priv->window),
                                GTK_DIALOG_DESTROY_WITH_PARENT,
                                GTK_MESSAGE_ERROR,
                                GTK_BUTTONS_CLOSE,
                                "<span weight='bold' size='larger'>"
                                "%s</span>\n%s",
                                headline, message);
        else
                dialog = gtk_message_dialog_new_with_markup(
                                GTK_WINDOW (self->priv->window),
                                GTK_DIALOG_DESTROY_WITH_PARENT,
                                GTK_MESSAGE_ERROR,
                                GTK_BUTTONS_CLOSE,
                                "%s", message);

        g_free(message);

        gtk_dialog_run(GTK_DIALOG (dialog));

        gtk_widget_destroy(GTK_WIDGET (dialog));
}

GObject *
gibbon_app_find_object(const GibbonApp *self, const gchar *id, GType type)
{
        GObject *obj;
        GType got_type;

        g_return_val_if_fail (GIBBON_IS_APP (self), NULL);
        g_return_val_if_fail (G_TYPE_IS_OBJECT (type), NULL);

        obj = gtk_builder_get_object(self->priv->builder, id);

        if (!obj) {
                /* TRANSLATORS: UI means user interface.  */
                gibbon_app_display_error(self, NULL,
                                _("Object `%s' not found in UI"
                                " definition!"), id);
                exit(-1);
        }

        if (!G_IS_OBJECT (obj)) {
                gibbon_app_display_error(
                                self, NULL,
                                _("Object `%s' is not a GObject!"),
                                id);
                exit(-1);
        }

        if (!G_TYPE_CHECK_INSTANCE_TYPE (obj, type)) {
                got_type = G_OBJECT_TYPE (obj);
                gibbon_app_display_error(
                                self, NULL,
                                _("Object `%s' is not of type `%s'"
                                                " but `%s'!"), id, g_type_name(
                                                type), g_type_name(got_type));
                exit(-1);
        }

        return obj;
}

static GibbonCairoboard *
gibbon_app_init_board(GibbonApp *self, const gchar *board_filename)
{
        GObject *board_vbox;
        GObject *dummy;
        GibbonCairoboard *board = gibbon_cairoboard_new(self, board_filename);

        if (!board)
                return NULL;

        gtk_widget_show(GTK_WIDGET (board));
        /* FIXME! This should occupy reasonable space by default!  Do
         * not hardcode the values.
         */
        gtk_widget_set_size_request(GTK_WIDGET (board), 294, 270);

        dummy = gibbon_app_find_object(self, "dummy-drawingarea",
                        GTK_TYPE_DRAWING_AREA);
        gtk_widget_destroy(GTK_WIDGET (dummy));

        board_vbox = gibbon_app_find_object(self, "board-vbox", GTK_TYPE_BOX);
        gtk_box_pack_start(GTK_BOX (board_vbox), GTK_WIDGET (board), TRUE,
                        TRUE, 0);

        return board;
}

GtkWidget *
gibbon_app_get_window(const GibbonApp *self)
{
        g_return_val_if_fail (GIBBON_IS_APP (self), NULL);

        return self->priv->window;
}

const gchar *
gibbon_app_get_pixmaps_directory(const GibbonApp *self)
{
        g_return_val_if_fail (GIBBON_IS_APP (self), NULL);

        return self->priv->pixmaps_directory;
}

static void gibbon_app_connect_signals(const GibbonApp *self)
{
        GObject* obj;

        obj = gibbon_app_find_object(self, "toolbar_quit_button",
                        GTK_TYPE_TOOL_BUTTON);
        g_signal_connect_swapped (obj, "clicked",
                        G_CALLBACK (gibbon_app_on_quit_request),
                        (gpointer) self);

        obj = gibbon_app_find_object(self, "open_menu_item",
                                     GTK_TYPE_MENU_ITEM);
        g_signal_connect_swapped (obj, "activate",
                        G_CALLBACK (gibbon_app_on_open),
                        (gpointer) self);

        obj = gibbon_app_find_object(self, "quit_menu_item",
                                     GTK_TYPE_MENU_ITEM);
        g_signal_connect_swapped (obj, "activate",
                        G_CALLBACK (gibbon_app_on_quit_request),
                        (gpointer) self);

        obj = gibbon_app_find_object(self, "connect_menu_item",
                        GTK_TYPE_MENU_ITEM);
        g_signal_connect_swapped (obj, "activate",
                        G_CALLBACK (gibbon_app_on_connect_request),
                        (gpointer) self);
        obj = gibbon_app_find_object(self, "toolbar_connect_button",
                        GTK_TYPE_TOOL_BUTTON);
        g_signal_connect_swapped (obj, "clicked",
                        G_CALLBACK (gibbon_app_on_connect_request),
                        (gpointer) self);

        obj = gibbon_app_find_object(self, "disconnect_menu_item",
                        GTK_TYPE_MENU_ITEM);
        g_signal_connect_swapped (obj, "activate",
                        G_CALLBACK (gibbon_app_disconnect),
                        (gpointer) self);
        obj = gibbon_app_find_object(self, "toolbar_disconnect_button",
                        GTK_TYPE_TOOL_BUTTON);
        g_signal_connect_swapped (obj, "clicked",
                        G_CALLBACK (gibbon_app_disconnect),
                        (gpointer) self);

        obj = gibbon_app_find_object(self, "toolbar-ready-button",
                        GTK_TYPE_TOOL_BUTTON);
        g_signal_connect_swapped (obj, "clicked",
                                  G_CALLBACK (gibbon_app_on_toggle_ready),
                                  (gpointer) self);

        obj = gibbon_app_find_object(self, "window", GTK_TYPE_WINDOW);
        g_signal_connect_swapped (obj, "delete-event",
                        G_CALLBACK (gibbon_app_on_quit_request),
                        (gpointer) self);

        obj = gibbon_app_find_object(self, "account-menu-item",
                        GTK_TYPE_MENU_ITEM);
        g_signal_connect_swapped (obj, "activate",
                        G_CALLBACK (gibbon_app_on_account_prefs),
                        (gpointer) self);

        obj = gibbon_app_find_object(self, "import-java-fibs-menu-item",
                        GTK_TYPE_MENU_ITEM);
        g_signal_connect_swapped (obj, "activate",
                        G_CALLBACK (gibbon_app_on_java_fibs_import),
                        (gpointer) self);

        obj = gibbon_app_find_object(self, "help-menu-item",
                                        GTK_TYPE_MENU_ITEM);
        g_signal_connect (obj, "activate",
                        G_CALLBACK (gibbon_help_show_help),
                        (gpointer) self);

        obj = gibbon_app_find_object(self, "about-menu-item",
                        GTK_TYPE_MENU_ITEM);
        g_signal_connect (obj, "activate",
                        G_CALLBACK (gibbon_help_show_about),
                        (gpointer) self);

        obj = gibbon_app_find_object(self, "board-refresh",
                                     GTK_TYPE_TOOL_BUTTON);
        g_signal_connect_swapped (obj, "clicked",
                                  G_CALLBACK (gibbon_app_on_board_refresh),
                                  (gpointer) self);

        obj = gibbon_app_find_object(self, "board-undo",
                                     GTK_TYPE_TOOL_BUTTON);
        g_signal_connect_swapped (obj, "clicked",
                                  G_CALLBACK (gibbon_app_on_board_undo),
                                  (gpointer) self);

        obj = gibbon_app_find_object(self, "board-resign",
                                     GTK_TYPE_TOOL_BUTTON);
        g_signal_connect_swapped (obj, "clicked",
                                  G_CALLBACK (gibbon_app_on_board_resign),
                                  (gpointer) self);

        obj = gibbon_app_find_object(self, "board-leave",
                                     GTK_TYPE_TOOL_BUTTON);
        g_signal_connect_swapped (obj, "clicked",
                                  G_CALLBACK (gibbon_app_on_board_leave),
                                  (gpointer) self);

        obj = gibbon_app_find_object(self, "board-accept",
                                     GTK_TYPE_TOOL_BUTTON);
        g_signal_connect_swapped (obj, "clicked",
                                  G_CALLBACK (gibbon_app_on_board_accept),
                                  (gpointer) self);

        obj = gibbon_app_find_object(self, "board-reject",
                                     GTK_TYPE_TOOL_BUTTON);
        g_signal_connect_swapped (obj, "clicked",
                                  G_CALLBACK (gibbon_app_on_board_reject),
                                  (gpointer) self);
}

static gboolean
gibbon_app_on_quit_request (GibbonApp *self, GtkWidget *emitter)
{
        gtk_main_quit ();

        return TRUE;
}

static void
gibbon_app_on_open (GibbonApp *self, GtkWidget *emitter)
{
        GtkWidget *dialog;
        gchar *filename;
        GtkFileFilter *filter;
        GibbonMatchLoader *loader;
        GibbonMatch *match;
        GError *error = NULL;
        const gchar *servers_directory;
        GSettings *settings;
        GVariant *variant;
        gchar *host, *login;
        guint16 port;
        gchar *buf;
        gchar *server_path;
        gchar *year_path;
        GDateTime *dt;
        gint i, year;

        dialog = gtk_file_chooser_dialog_new (_("Open File"),
                                              GTK_WINDOW (self->priv->window),
                                              GTK_FILE_CHOOSER_ACTION_OPEN,
                                              GTK_STOCK_CANCEL,
                                              GTK_RESPONSE_CANCEL,
                                              GTK_STOCK_OPEN,
                                              GTK_RESPONSE_ACCEPT,
                                              NULL);
        filter = gtk_file_filter_new ();
        gtk_file_filter_set_name (filter, _("GNUBG files (*.sgf)"));
        gtk_file_filter_add_pattern (filter, "*.sgf");
        gtk_file_filter_add_pattern (filter, "*.SGF");
        gtk_file_chooser_add_filter (GTK_FILE_CHOOSER (dialog), filter);

        filter = gtk_file_filter_new ();
        gtk_file_filter_set_name (filter, _("Gibbon files (*.gmd)"));
        gtk_file_filter_add_pattern (filter, "*.gmd");
        gtk_file_filter_add_pattern (filter, "*.GMD");
        gtk_file_chooser_add_filter (GTK_FILE_CHOOSER (dialog), filter);

        filter = gtk_file_filter_new ();
        gtk_file_filter_set_name (filter, _("JavaFIBS files (*.match)"));
        gtk_file_filter_add_pattern (filter, "*.match");
        gtk_file_filter_add_pattern (filter, "*.MATCH");
        gtk_file_chooser_add_filter (GTK_FILE_CHOOSER (dialog), filter);

        filter = gtk_file_filter_new ();
        gtk_file_filter_set_name (filter, _("JellyFish files (*.match)"));
        gtk_file_filter_add_pattern (filter, "*.match");
        gtk_file_filter_add_pattern (filter, "*.MATCH");
        gtk_file_chooser_add_filter (GTK_FILE_CHOOSER (dialog), filter);

        filter = gtk_file_filter_new ();
        gtk_file_filter_set_name (filter, _("All files"));
        gtk_file_filter_add_pattern (filter, "*");
        gtk_file_chooser_add_filter (GTK_FILE_CHOOSER (dialog), filter);

        settings = g_settings_new (GIBBON_PREFS_SERVER_SCHEMA);
        host = g_settings_get_string (settings, GIBBON_PREFS_SERVER_HOST);
        variant = g_settings_get_value (settings, GIBBON_PREFS_SERVER_PORT);
        login = g_settings_get_string (settings, GIBBON_PREFS_SERVER_LOGIN);
        dt = g_date_time_new_now_local ();

        if (host && variant && login && dt) {
                port = g_variant_get_uint16 (variant);
                servers_directory = gibbon_archive_get_servers_directory (
                                self->priv->archive);
                server_path = g_build_filename (servers_directory,
                                                host, NULL);
                if (port != 4321) {
                        buf = g_strdup_printf ("%s_%u", server_path, port);
                        g_free (server_path);
                        server_path = buf;
                }

                year = g_date_time_get_year (dt);
                for (i = 0; i < 3; ++i) {
                        buf = g_strdup_printf ("%04d", year - i);
                        year_path = g_build_filename (server_path, login,
                                                      buf, NULL);
                        g_free (buf);
                        gtk_file_chooser_add_shortcut_folder (
                                        GTK_FILE_CHOOSER (dialog), year_path,
                                        NULL);
                        g_free (year_path);
                }
                g_free (server_path);
        }

        g_free (host);
        if (variant)
                g_variant_unref (variant);
        g_free (login);
        if (dt)
                g_date_time_unref (dt);

        if (gtk_dialog_run (GTK_DIALOG (dialog)) == GTK_RESPONSE_ACCEPT) {
                filename = gtk_file_chooser_get_filename (
                                GTK_FILE_CHOOSER (dialog));
                /* g_printerr ("open filename %s\n", filename); */
                loader = gibbon_match_loader_new ();
                match = gibbon_match_loader_read_match (loader, filename,
                                                        &error);

                if (!match) {
                        if (error) {
                                gibbon_app_display_error (self, filename,
                                                          "%s",
                                                          error->message);
                                g_error_free (error);
                        }
                        g_object_unref (loader);
                        g_object_unref (self);
                } else {
                        gibbon_app_set_match (self, match);
                }
                g_object_unref (loader);
                g_free (filename);
        }

        gtk_widget_destroy (dialog);
}

static void
gibbon_app_on_register_request (GibbonApp *self)
{
        GibbonRegisterDialog *dialog;
        gint response;
        gchar *hostname;
        guint port;
        gchar *password;
        GSettings *settings;
        GVariant *variant;

        if (self->priv->connection) {
                gibbon_app_display_error (self, NULL,
                                          _("You cannot register an account"
                                            " while you are connected to a"
                                            " server!"));
                return;
        }

        dialog = gibbon_register_dialog_new (self);
        response = gtk_dialog_run (GTK_DIALOG (dialog));
        if (!gibbon_register_dialog_okay (dialog))
                response = GTK_RESPONSE_CANCEL;
        password = g_strdup (gibbon_register_dialog_get_password (dialog));

        gtk_widget_destroy (GTK_WIDGET (dialog));

        if (response != GTK_RESPONSE_OK) {
                g_free (password);
                return;
        }

        /* Clean up opened connection dialog and stale connection objects.
         * The latter should not happen.
         */
        gibbon_app_disconnect (self);
        gibbon_app_set_state_connecting (self);

        /*
         * We have a little race here.  We read the login, hostname, and port
         * from the GSettings, and not from the dialog.
         */
        settings = g_settings_new (GIBBON_PREFS_SERVER_SCHEMA);

        hostname = g_settings_get_string (settings,
                                          GIBBON_PREFS_SERVER_HOST);

        variant = g_settings_get_value (settings, GIBBON_PREFS_SERVER_PORT);
        port = g_variant_get_uint16 (variant);
        g_variant_unref (variant);

        g_object_unref (settings);

        self->priv->connection = gibbon_connection_new (self, hostname, port,
                                                        "guest", password);
        g_free (password);
        g_free (hostname);
        if (!self->priv->connection) {
                gibbon_app_disconnect (self);
                return;
        }

        self->priv->connecting_signal = gibbon_signal_new (
                        G_OBJECT (self->priv->connection), "connecting",
                        G_CALLBACK (gibbon_app_on_connecting),
                        G_OBJECT (self));
        self->priv->connected_signal = gibbon_signal_new (
                        G_OBJECT (self->priv->connection), "connected",
                        G_CALLBACK (gibbon_app_on_connected),
                        G_OBJECT (self));
        self->priv->logged_in_signal = gibbon_signal_new (
                        G_OBJECT (self->priv->connection), "logged_in",
                        G_CALLBACK (gibbon_app_on_logged_in),
                        G_OBJECT (self));
        self->priv->network_error_signal = gibbon_signal_new (
                        G_OBJECT (self->priv->connection),
                        "network-error",
                        G_CALLBACK (gibbon_app_on_network_error),
                        G_OBJECT (self));
        self->priv->disconnected_signal = gibbon_signal_new (
                        G_OBJECT (self->priv->connection),
                        "disconnected", G_CALLBACK (gibbon_app_disconnect),
                        G_OBJECT (self));

        if (!gibbon_connection_connect (self->priv->connection))
                gibbon_app_disconnect (self);
}

static void
gibbon_app_on_connect_request(GibbonApp *self, GtkWidget *emitter)
{
        GibbonConnectionDialog *dialog;
        gint response;
        gchar *password;
        guint16 port;
        GSettings *settings;
        gchar *hostname = NULL;
        gchar *login = NULL;
        GVariant *variant;

        dialog = gibbon_connection_dialog_new (self, FALSE);
        response = gtk_dialog_run (GTK_DIALOG (dialog));

        switch (response) {
        case GTK_RESPONSE_OK:
                break;
        case GIBBON_CONNECTION_DIALOG_RESPONSE_REGISTER:
                gtk_widget_destroy (GTK_WIDGET (dialog));
                gibbon_app_on_register_request (self);
                return;
        default:
                gtk_widget_destroy (GTK_WIDGET (dialog));
                return;
        }

        password = g_strdup (gibbon_connection_dialog_get_password (dialog));

        gtk_widget_destroy (GTK_WIDGET (dialog));

        settings = g_settings_new (GIBBON_PREFS_SERVER_SCHEMA);

        hostname = g_settings_get_string (settings,
                                          GIBBON_PREFS_SERVER_HOST);
        if (!hostname || !*hostname) {
                g_free (password);
                g_free (hostname);
                g_object_unref (settings);
                gibbon_app_display_error (self, NULL, _("No hostname given!"));
                return;
        }

        login = g_settings_get_string (settings,
                                       GIBBON_PREFS_SERVER_LOGIN);
        if (!login || !*login) {
                g_free (password);
                g_free (hostname);
                g_free (login);
                g_object_unref (settings);
                gibbon_app_display_error (self, NULL, _("No user name given!"));
                return;
        }
        if (0 == g_strcmp0 (login, "guest")) {
                g_free (password);
                g_free (hostname);
                g_free (login);
                g_object_unref (settings);
                gibbon_app_display_error (self, NULL,
                                          _("The login `%s' is reserved!"),
					  "guest");
                return;
        }

        variant = g_settings_get_value (settings, GIBBON_PREFS_SERVER_PORT);
        port = g_variant_get_uint16 (variant);
        g_variant_unref (variant);
        if (!port) {
                g_free (password);
                g_free (hostname);
                g_free (login);
                g_object_unref (settings);
                gibbon_app_display_error (self, NULL,
                                          _("Invalid port number!"
                                            " In doubt try the default"
                                            " port number 4321."));
                return;
        }

        if (!password || !*password) {
                g_free (password);
                g_free (hostname);
                g_free (login);
                g_object_unref (settings);
                gibbon_app_display_error (self, NULL,
                                          _("No password given!"));
                return;
        }

        /* Clean up opened connection dialog and stale connection objects.
         * The latter should not happen.
         */
        gibbon_app_disconnect (self);
        gibbon_app_set_state_connecting (self);

        self->priv->connection = gibbon_connection_new (self, hostname, port,
                                                        login, password);
        g_free (password);
        g_free (hostname);
        g_free (login);
        if (!self->priv->connection) {
                gibbon_app_disconnect (self);
                return;
        }

        self->priv->connecting_signal = gibbon_signal_new (
                        G_OBJECT (self->priv->connection), "connecting",
                        G_CALLBACK (gibbon_app_on_connecting),
                        G_OBJECT (self));
        self->priv->connected_signal = gibbon_signal_new (
                        G_OBJECT (self->priv->connection), "connected",
                        G_CALLBACK (gibbon_app_on_connected),
                        G_OBJECT (self));
        self->priv->logged_in_signal = gibbon_signal_new (
                        G_OBJECT (self->priv->connection), "logged_in",
                        G_CALLBACK (gibbon_app_on_logged_in),
                        G_OBJECT (self));
        self->priv->network_error_signal = gibbon_signal_new (
                        G_OBJECT (self->priv->connection),
                        "network-error",
                        G_CALLBACK (gibbon_app_on_network_error),
                        G_OBJECT (self));
        self->priv->disconnected_signal = gibbon_signal_new (
                        G_OBJECT (self->priv->connection),
                        "disconnected", G_CALLBACK (gibbon_app_disconnect),
                        G_OBJECT (self));

        if (!gibbon_connection_connect (self->priv->connection))
                gibbon_app_disconnect (self);
}

void
gibbon_app_on_board_refresh (GibbonApp *self)
{
        gibbon_connection_queue_command (self->priv->connection, FALSE,
                                         "board");
}

void
gibbon_app_on_board_undo (GibbonApp *self)
{
        GibbonSession *session = gibbon_connection_get_session(
                        self->priv->connection);

        if (session)
                gibbon_session_reset_position (session);
}

void
gibbon_app_on_board_resign (GibbonApp *self)
{
        GtkWidget *dialog;
        gint cube;
        gint result;
        GtkWidget *ca;
        GtkWidget *rb1, *rb2, *rb3;
        GSList *group = NULL;
        gchar *label;
        guint selected;
        GibbonSession *session;
        const GibbonPosition *position;

        session = gibbon_connection_get_session (self->priv->connection);
        if (!session)
                return;

        position = gibbon_session_get_position (session);
        if (!position)
                return;

        cube = position->cube;

        dialog = gtk_dialog_new_with_buttons (_("Resign"),
                                              GTK_WINDOW (self->priv->window),
                                              GTK_DIALOG_MODAL
                                              | GTK_DIALOG_DESTROY_WITH_PARENT,
                                              GTK_STOCK_OK,
                                              GTK_RESPONSE_ACCEPT,
                                              GTK_STOCK_CANCEL,
                                              GTK_RESPONSE_REJECT,
                                              NULL);

        ca = gtk_dialog_get_content_area (GTK_DIALOG (dialog));

        label = g_strdup_printf (g_dngettext (GETTEXT_PACKAGE,
                                              "_normal (one point)",
                                              "_normal (%d points)",
                                              cube),
                                 cube);
        rb1 = gtk_radio_button_new_with_mnemonic (group, label);
        g_free (label);
        gtk_box_pack_start (GTK_BOX (ca), rb1, TRUE, TRUE, 0);
        group = gtk_radio_button_get_group (GTK_RADIO_BUTTON (rb1));
        label = g_strdup_printf (g_dngettext (GETTEXT_PACKAGE,
                                              "_gammon (one point)",
                                              "_gammon (%d points)",
                                              cube * 2),
                                 cube * 2);
        rb2 = gtk_radio_button_new_with_mnemonic (group, label);
        g_free (label);
        gtk_box_pack_start (GTK_BOX (ca), rb2, TRUE, TRUE, 0);
        group = gtk_radio_button_get_group (GTK_RADIO_BUTTON (rb2));
        label = g_strdup_printf (g_dngettext (GETTEXT_PACKAGE,
                                              "_backgammon (one point)",
                                              "_backgammon (%d points)",
                                              cube * 3),
                                 cube * 3);
        rb3 = gtk_radio_button_new_with_mnemonic (group, label);
        g_free (label);
        gtk_box_pack_start (GTK_BOX (ca), rb3, TRUE, TRUE, 0);

        gtk_dialog_set_default_response (GTK_DIALOG (dialog),
                                         GTK_RESPONSE_ACCEPT);

        gtk_widget_show_all (dialog);

        result = gtk_dialog_run (GTK_DIALOG (dialog));

        if (gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (rb1)))
                selected = 1;
        else if (gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (rb2)))
                selected = 2;
        else if (gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (rb3)))
                selected = 3;
        else
                selected = 1;

        gtk_widget_destroy (dialog);

        if (result != GTK_RESPONSE_ACCEPT)
                return;

        gibbon_session_resign (session, selected);
}

void
gibbon_app_on_board_leave (GibbonApp *self)
{
        GtkWidget *dialog;
        gint response;

        dialog = gtk_message_dialog_new (GTK_WINDOW (self->priv->window),
                                         GTK_DIALOG_DESTROY_WITH_PARENT,
                                         GTK_MESSAGE_QUESTION,
                                         GTK_BUTTONS_YES_NO, "%s",
                                         _("Leaving in the middle of a game"
                                           " is normally considered bad"
                                           " behavior.  Do you really want to"
                                           " leave?"));

        response = gtk_dialog_run(GTK_DIALOG (dialog));

        gtk_widget_destroy(GTK_WIDGET (dialog));

        if (response == GTK_RESPONSE_YES)
                gibbon_connection_queue_command (self->priv->connection, FALSE,
                                                 "leave");
}

void
gibbon_app_on_board_accept (GibbonApp *self)
{
        GibbonSession *session = gibbon_connection_get_session(
                        self->priv->connection);

        if (session)
                gibbon_session_accept_request (session);
}

void
gibbon_app_on_board_reject (GibbonApp *self)
{
        GibbonSession *session = gibbon_connection_get_session(
                        self->priv->connection);

        if (session)
                gibbon_session_reject_request (session);
}

void
gibbon_app_set_state_disconnected (GibbonApp *self)
{
        GObject* obj;

        gibbon_app_set_state_not_playing (self);

        obj = gibbon_app_find_object(self, "toolbar_connect_button",
                        GTK_TYPE_TOOL_BUTTON);
        gtk_widget_set_sensitive(GTK_WIDGET (obj), TRUE);
        gtk_widget_set_visible(GTK_WIDGET (obj), TRUE);

        obj = gibbon_app_find_object(self, "toolbar_disconnect_button",
                        GTK_TYPE_TOOL_BUTTON);
        gtk_widget_set_sensitive(GTK_WIDGET (obj), FALSE);
        gtk_widget_set_visible(GTK_WIDGET (obj), FALSE);

        obj = gibbon_app_find_object(self, "toolbar-ready-button",
                        GTK_TYPE_TOOL_BUTTON);
        gtk_widget_set_sensitive(GTK_WIDGET (obj), FALSE);

        obj = gibbon_app_find_object(self, "connect_menu_item",
                        GTK_TYPE_IMAGE_MENU_ITEM);
        gtk_widget_set_sensitive(GTK_WIDGET (obj), TRUE);

        obj = gibbon_app_find_object(self, "disconnect_menu_item",
                        GTK_TYPE_IMAGE_MENU_ITEM);
        gtk_widget_set_sensitive(GTK_WIDGET (obj), FALSE);

        obj = gibbon_app_find_object(self, "server-command-entry",
                        GTK_TYPE_ENTRY);
        gtk_editable_set_editable(GTK_EDITABLE (obj), FALSE);

        obj = gibbon_app_find_object(self, "game-chat-entry", GTK_TYPE_ENTRY);
        gtk_editable_set_editable(GTK_EDITABLE (obj), FALSE);

        obj = gibbon_app_find_object(self, "shout-entry", GTK_TYPE_ENTRY);
        gtk_editable_set_editable(GTK_EDITABLE (obj), FALSE);

        obj = gibbon_app_find_object(self, "board-refresh", GTK_TYPE_TOOL_BUTTON);
        gtk_widget_set_sensitive(GTK_WIDGET (obj), TRUE);
        obj = gibbon_app_find_object(self, "board-undo", GTK_TYPE_TOOL_BUTTON);
        gtk_widget_set_sensitive(GTK_WIDGET (obj), TRUE);
        obj = gibbon_app_find_object(self, "board-resign", GTK_TYPE_TOOL_BUTTON);
        gtk_widget_set_sensitive(GTK_WIDGET (obj), TRUE);
        obj = gibbon_app_find_object(self, "board-accept", GTK_TYPE_TOOL_BUTTON);
        gtk_widget_set_sensitive(GTK_WIDGET (obj), TRUE);
        obj = gibbon_app_find_object(self, "board-reject", GTK_TYPE_TOOL_BUTTON);
        gtk_widget_set_sensitive(GTK_WIDGET (obj), TRUE);
        obj = gibbon_app_find_object(self, "board-double", GTK_TYPE_TOOL_BUTTON);
        gtk_widget_set_sensitive(GTK_WIDGET (obj), TRUE);
        obj = gibbon_app_find_object(self, "board-fullscreen", GTK_TYPE_TOOL_BUTTON);
        gtk_widget_set_sensitive(GTK_WIDGET (obj), TRUE);
        obj = gibbon_app_find_object(self, "board-record", GTK_TYPE_TOOL_BUTTON);
        gtk_widget_set_sensitive(GTK_WIDGET (obj), TRUE);
        obj = gibbon_app_find_object(self, "board-leave", GTK_TYPE_TOOL_BUTTON);
        gtk_widget_set_sensitive(GTK_WIDGET (obj), TRUE);
}

void
gibbon_app_set_state_connecting(GibbonApp *self)
{
        GObject* obj;

        obj = gibbon_app_find_object(self, "toolbar_connect_button",
                        GTK_TYPE_TOOL_BUTTON);
        gtk_widget_set_sensitive(GTK_WIDGET (obj), FALSE);
        gtk_widget_set_visible(GTK_WIDGET (obj), FALSE);

        obj = gibbon_app_find_object(self, "toolbar_disconnect_button",
                        GTK_TYPE_TOOL_BUTTON);
        gtk_widget_set_sensitive(GTK_WIDGET (obj), TRUE);
        gtk_widget_set_visible(GTK_WIDGET (obj), TRUE);

        obj = gibbon_app_find_object(self, "toolbar-ready-button",
                        GTK_TYPE_TOOL_BUTTON);
        gtk_widget_set_sensitive(GTK_WIDGET (obj), FALSE);

        obj = gibbon_app_find_object(self, "connect_menu_item",
                        GTK_TYPE_IMAGE_MENU_ITEM);
        gtk_widget_set_sensitive(GTK_WIDGET (obj), FALSE);

        obj = gibbon_app_find_object(self, "disconnect_menu_item",
                        GTK_TYPE_IMAGE_MENU_ITEM);
        gtk_widget_set_sensitive(GTK_WIDGET (obj), TRUE);
}

void
gibbon_app_set_state_playing (const GibbonApp *self)
{
        GObject *obj;

        obj = gibbon_app_find_object (self, "board-refresh",
                                      GTK_TYPE_TOOL_BUTTON);
        gtk_widget_set_sensitive(GTK_WIDGET (obj), TRUE);

        obj = gibbon_app_find_object (self, "board-undo",
                                      GTK_TYPE_TOOL_BUTTON);
        gtk_widget_set_sensitive(GTK_WIDGET (obj), TRUE);

        obj = gibbon_app_find_object (self, "board-leave",
                                      GTK_TYPE_TOOL_BUTTON);
        gtk_widget_set_sensitive(GTK_WIDGET (obj), TRUE);

        obj = gibbon_app_find_object (self, "board-resign",
                                      GTK_TYPE_TOOL_BUTTON);
        gtk_widget_set_sensitive(GTK_WIDGET (obj), TRUE);

        obj = gibbon_app_find_object (self, "open_menu_item",
                                      GTK_TYPE_MENU_ITEM);
        gtk_widget_set_sensitive(GTK_WIDGET (obj), FALSE);
}

void
gibbon_app_set_state_watching (const GibbonApp *self)
{
        GObject *obj;

        obj = gibbon_app_find_object (self, "board-refresh",
                                      GTK_TYPE_TOOL_BUTTON);
        gtk_widget_set_sensitive(GTK_WIDGET (obj), TRUE);

        obj = gibbon_app_find_object (self, "open_menu_item",
                                      GTK_TYPE_MENU_ITEM);
        gtk_widget_set_sensitive(GTK_WIDGET (obj), FALSE);
}

void
gibbon_app_set_state_not_playing (const GibbonApp *self)
{
        GObject *obj;

        obj = gibbon_app_find_object (self, "board-refresh",
                                      GTK_TYPE_TOOL_BUTTON);
        gtk_widget_set_sensitive(GTK_WIDGET (obj), FALSE);

        obj = gibbon_app_find_object (self, "board-undo",
                                      GTK_TYPE_TOOL_BUTTON);
        gtk_widget_set_sensitive(GTK_WIDGET (obj), FALSE);

        obj = gibbon_app_find_object (self, "board-leave",
                                      GTK_TYPE_TOOL_BUTTON);
        gtk_widget_set_sensitive(GTK_WIDGET (obj), FALSE);

        obj = gibbon_app_find_object (self, "board-resign",
                                      GTK_TYPE_TOOL_BUTTON);
        gtk_widget_set_sensitive(GTK_WIDGET (obj), FALSE);

        obj = gibbon_app_find_object (self, "open_menu_item",
                                      GTK_TYPE_MENU_ITEM);
        gtk_widget_set_sensitive(GTK_WIDGET (obj), TRUE);
}

void
gibbon_app_set_state_not_watching (const GibbonApp *self)
{
        GObject *obj;

        obj = gibbon_app_find_object (self, "board-refresh",
                                      GTK_TYPE_TOOL_BUTTON);
        gtk_widget_set_sensitive(GTK_WIDGET (obj), FALSE);

        obj = gibbon_app_find_object (self, "open_menu_item",
                                      GTK_TYPE_MENU_ITEM);
        gtk_widget_set_sensitive(GTK_WIDGET (obj), TRUE);
}

void
gibbon_app_set_state_expect_response (const GibbonApp *self)
{
        GObject *obj;

        obj = gibbon_app_find_object (self, "board-accept",
                                      GTK_TYPE_TOOL_BUTTON);
        gtk_widget_set_sensitive(GTK_WIDGET (obj), TRUE);

        obj = gibbon_app_find_object (self, "board-reject",
                                      GTK_TYPE_TOOL_BUTTON);
        gtk_widget_set_sensitive(GTK_WIDGET (obj), TRUE);
}

void
gibbon_app_set_state_may_double (const GibbonApp *self, gboolean state)
{
        GObject *obj;

        obj = gibbon_app_find_object (self, "board-double",
                                      GTK_TYPE_TOOL_BUTTON);
        gtk_widget_set_sensitive(GTK_WIDGET (obj), state);
}

const gchar *
gibbon_app_get_entry_text(const GibbonApp *self, const gchar *id)
{
        GtkWidget *entry;

        g_return_val_if_fail (GIBBON_IS_APP (self), NULL);

        entry = GTK_WIDGET (gibbon_app_find_object (self, id, GTK_TYPE_ENTRY));

        return gtk_entry_get_text(GTK_ENTRY (entry));
}

const gchar *
gibbon_app_get_trimmed_entry_text(const GibbonApp *self, const gchar *id)
{
        GtkWidget *entry;
        gchar *trimmed;

        g_return_val_if_fail (GIBBON_IS_APP (self), NULL);

        entry = GTK_WIDGET (gibbon_app_find_object (self, id, GTK_TYPE_ENTRY));

        trimmed = pango_trim_string(gtk_entry_get_text(GTK_ENTRY (entry)));
        gtk_entry_set_text(GTK_ENTRY (entry), trimmed);
        g_free(trimmed);

        return gtk_entry_get_text(GTK_ENTRY (entry));
}

void gibbon_app_disconnect(GibbonApp *self)
{
        if (self->priv->connecting_signal)
                g_object_unref(self->priv->connecting_signal);
        self->priv->connecting_signal = NULL;

        if (self->priv->connected_signal)
                g_object_unref(self->priv->connected_signal);
        self->priv->connected_signal = NULL;

        if (self->priv->logged_in_signal)
                g_object_unref(self->priv->logged_in_signal);
        self->priv->logged_in_signal = NULL;

        if (self->priv->network_error_signal)
                g_object_unref(self->priv->network_error_signal);
        self->priv->network_error_signal = NULL;

        if (self->priv->disconnected_signal)
                g_object_unref(self->priv->disconnected_signal);
        self->priv->disconnected_signal = NULL;

        if (self->priv->connection)
                g_object_unref(self->priv->connection);
        self->priv->connection = NULL;

        gibbon_shouts_set_my_name(self->priv->shouts, NULL);
        gibbon_game_chat_set_my_name(self->priv->game_chat, NULL);
        if (self->priv->player_list)
                gibbon_player_list_clear(self->priv->player_list);
        if (self->priv->inviter_list)
                gibbon_inviter_list_clear(self->priv->inviter_list);

        gibbon_app_set_state_disconnected(self);
}

static void gibbon_app_on_connecting(GibbonApp *self, GibbonConnection *conn)
{
        g_return_if_fail (GIBBON_IS_CONNECTION (conn));

        gchar *msg = g_strdup_printf(_("Connecting with %s port %d."),
                        gibbon_connection_get_hostname(conn),
                        gibbon_connection_get_port(conn));
        GtkStatusbar *statusbar =
                GTK_STATUSBAR (gibbon_app_find_object (self, "statusbar",
                                                       GTK_TYPE_STATUSBAR));

        gtk_statusbar_pop(statusbar, 0);
        gtk_statusbar_push(statusbar, 0, msg);
        g_free(msg);
}

static void gibbon_app_on_connected(GibbonApp *self, GibbonConnection *conn)
{
        g_return_if_fail (GIBBON_IS_CONNECTION (conn));

        gchar *msg = g_strdup_printf(_("Connected with %s port %d."),
                        gibbon_connection_get_hostname(conn),
                        gibbon_connection_get_port(conn));
        GtkStatusbar
                        *statusbar =
                                        GTK_STATUSBAR (gibbon_app_find_object (self, "statusbar",
                                                                        GTK_TYPE_STATUSBAR));

        gtk_statusbar_pop(statusbar, 0);
        gtk_statusbar_push(statusbar, 0, msg);
        g_free(msg);
}

static void gibbon_app_on_logged_in(GibbonApp *self, GibbonConnection *conn)
{
        GObject *obj;
        GError *error = NULL;

        g_return_if_fail (GIBBON_IS_CONNECTION (conn));

        gchar *msg = g_strdup_printf(_("Logged in as %s on %s port %d."),
                        gibbon_connection_get_login(conn),
                        gibbon_connection_get_hostname(conn),
                        gibbon_connection_get_port(conn));
        GtkStatusbar
                        *statusbar =
                                        GTK_STATUSBAR (gibbon_app_find_object (self, "statusbar",
                                                                        GTK_TYPE_STATUSBAR));

        gtk_statusbar_pop(statusbar, 0);
        gtk_statusbar_push(statusbar, 0, msg);
        g_free(msg);

        obj = gibbon_app_find_object(self, "server-command-entry",
                        GTK_TYPE_ENTRY);
        gtk_editable_set_editable(GTK_EDITABLE (obj), TRUE);

        obj = gibbon_app_find_object(self, "game-chat-entry", GTK_TYPE_ENTRY);
        gtk_editable_set_editable(GTK_EDITABLE (obj), TRUE);

        obj = gibbon_app_find_object(self, "shout-entry", GTK_TYPE_ENTRY);
        gtk_editable_set_editable(GTK_EDITABLE (obj), TRUE);

        obj = gibbon_app_find_object(self, "toolbar-ready-button",
                        GTK_TYPE_TOOL_BUTTON);
        gtk_widget_set_sensitive(GTK_WIDGET (obj), TRUE);

        gibbon_shouts_set_my_name(self->priv->shouts,
                        gibbon_connection_get_login(conn));
        gibbon_game_chat_set_my_name(self->priv->game_chat,
                        gibbon_connection_get_login(conn));

        if (!gibbon_archive_login(self->priv->archive,
                        gibbon_connection_get_hostname(conn),
                        gibbon_connection_get_port(conn),
                        gibbon_connection_get_login(conn),
                        &error)) {
            gibbon_app_fatal_error (self, NULL, "%s", error->message);
        }
}

static void gibbon_app_on_network_error(GibbonApp *self, const gchar *message)
{
        gibbon_app_display_error(self, NULL, "%s", message);
        gibbon_app_disconnect(self);
}

GtkImage *
gibbon_app_load_scaled_image(const GibbonApp *self, const gchar *path,
                             gint width, gint height)
{
        GError *error = NULL;
        GtkImage *image;
        GdkPixbuf *pixbuf;

        g_return_val_if_fail (GIBBON_IS_APP (self), NULL);

        pixbuf = gdk_pixbuf_new_from_file_at_scale(path, width, height, FALSE,
                        &error);

        if (!pixbuf) {
                gibbon_app_display_error(
                                self, NULL,
                                _("Error loading image `%s': %s!"),
                                path, error->message);
                return NULL;
        }

        image = GTK_IMAGE (gtk_image_new ());
        gtk_image_set_from_pixbuf(image, pixbuf);
        g_object_unref(pixbuf);

        gtk_widget_show(GTK_WIDGET (image));
        return image;
}

GibbonServerConsole *
gibbon_app_get_server_console(const GibbonApp *self)
{
        g_return_val_if_fail (GIBBON_IS_APP (self), NULL);

        return self->priv->server_console;
}

GibbonBoard *
gibbon_app_get_board(const GibbonApp *self)
{
        g_return_val_if_fail (GIBBON_IS_APP (self), NULL);

        return GIBBON_BOARD (self->priv->board);
}

GibbonConnection *
gibbon_app_get_connection(const GibbonApp *self)
{
        g_return_val_if_fail (GIBBON_IS_APP (self), NULL);

        return self->priv->connection;
}

struct _GibbonSession *
gibbon_app_get_session(const GibbonApp *self)
{
        g_return_val_if_fail (GIBBON_IS_APP (self), NULL);

        return gibbon_connection_get_session(self->priv->connection);
}

GibbonShouts *
gibbon_app_get_shouts(const GibbonApp *self)
{
        g_return_val_if_fail (GIBBON_IS_APP (self), NULL);

        return self->priv->shouts;
}

GibbonGameChat *
gibbon_app_get_game_chat(const GibbonApp *self)
{
        g_return_val_if_fail (GIBBON_IS_APP (self), NULL);

        return self->priv->game_chat;
}

static void gibbon_app_on_account_prefs(GibbonApp *self)
{
        GibbonConnectionDialog *dialog;
        guint response;

        g_return_if_fail (GIBBON_IS_APP (self));

        dialog = gibbon_connection_dialog_new (self, TRUE);
        response = gtk_dialog_run (GTK_DIALOG (dialog));
        gtk_widget_destroy (GTK_WIDGET (dialog));

        if (response == GIBBON_CONNECTION_DIALOG_RESPONSE_REGISTER)
                gibbon_app_on_register_request (self);
}

void gibbon_app_start_chat(GibbonApp *self, const gchar *who)
{
        GibbonChatView *view;
        GibbonChat *chat;
        const gchar *me;

        g_return_if_fail (GIBBON_IS_APP (self));
        g_return_if_fail (self->priv->connection != NULL);

        if (g_hash_table_lookup(self->priv->chats, who))
                return;

        me = gibbon_connection_get_login(self->priv->connection);
        chat = gibbon_chat_new(self, me);
        view = gibbon_chat_view_new(self, who, chat);
        g_hash_table_insert(self->priv->chats, g_strdup(who), view);

        /* The chat view has a reference to the chat.  */
        g_object_unref(chat);
}

void gibbon_app_close_chat(GibbonApp *self, const gchar *who)
{
        g_return_if_fail (GIBBON_IS_APP (self));
        g_return_if_fail (self->priv->connection != NULL);

        g_hash_table_remove(self->priv->chats, who);
}

void gibbon_app_show_message(GibbonApp *self, const gchar *peer,
                             const GibbonFIBSMessage *message)
{
        GibbonChatView *view;

        g_return_if_fail (GIBBON_IS_APP (self));
        g_return_if_fail (self->priv->connection != NULL);
        g_return_if_fail (message != NULL);

        gibbon_app_start_chat(self, peer);
        view = GIBBON_CHAT_VIEW (g_hash_table_lookup (self->priv->chats,
                                        peer));
        g_return_if_fail (GIBBON_IS_CHAT_VIEW (view));

        gibbon_chat_view_append_message(view, message);
}

void gibbon_app_show_shout(GibbonApp *self, const GibbonFIBSMessage *message)
{
        g_return_if_fail (GIBBON_IS_APP (self));
        g_return_if_fail (self->priv->connection != NULL);
        g_return_if_fail (message != NULL);

        gibbon_shouts_append_message(self->priv->shouts, message);
}

void gibbon_app_show_game_chat(GibbonApp *self,
                               const GibbonFIBSMessage *message)
{
        g_return_if_fail (GIBBON_IS_APP (self));
        g_return_if_fail (self->priv->connection != NULL);
        g_return_if_fail (message != NULL);

        gibbon_game_chat_append_message(self->priv->game_chat, message);
}

GibbonArchive *
gibbon_app_get_archive(const GibbonApp *self)
{
        g_return_val_if_fail (GIBBON_IS_APP (self), NULL);

        return self->priv->archive;
}

GibbonPlayerList *
gibbon_app_get_player_list(const GibbonApp *self)
{
        g_return_val_if_fail (GIBBON_IS_APP (self), NULL);

        return self->priv->player_list;
}

GibbonInviterList *
gibbon_app_get_inviter_list(const GibbonApp *self)
{
        g_return_val_if_fail (GIBBON_IS_APP (self), NULL);

        return self->priv->inviter_list;
}

GibbonMatchList *
gibbon_app_get_match_list(const GibbonApp *self)
{
        g_return_val_if_fail (GIBBON_IS_APP (self), NULL);

        return self->priv->match_list;
}

void gibbon_app_configure_player_menu(const GibbonApp *self,
                                      const gchar *player, GtkMenu *menu)
{
        gibbon_session_configure_player_menu(gibbon_connection_get_session(
                        self->priv->connection), player, menu);
}

static void
gibbon_app_set_icon (const GibbonApp *self, const gchar *data_dir)
{
        GList *list = NULL;
        gint i;
        const gchar *size;
        const gchar * const pngname = PACKAGE ".png";
        const gchar * const svgname = PACKAGE ".svg";
        gchar *filename;
        gboolean is_svg;
        GdkPixbuf *pixbuf;

        for (i = 0; i < (sizeof gibbon_app_icon_sizes)
                        / (sizeof gibbon_app_icon_sizes[0]); ++i) {
                size = gibbon_app_icon_sizes[i];
                is_svg = !g_strcmp0("scalable", size);
                filename = g_build_filename(data_dir, "icons", "hicolor", size,
                                "apps", is_svg ? svgname : pngname, NULL);
                pixbuf = gdk_pixbuf_new_from_file(filename, NULL);
                if (pixbuf)
                        list = g_list_prepend(list, pixbuf);
                g_free(filename);
        }

        gtk_window_set_default_icon_list(list);
        g_list_foreach (list, (GFunc) g_object_unref, NULL);
        g_list_free (list);
}

GibbonClientIcons *
gibbon_app_get_client_icons(const GibbonApp *self)
{
        g_return_val_if_fail (GIBBON_IS_APP (self), NULL);

        return self->priv->client_icons;
}

void
gibbon_app_set_state_available (const GibbonApp *self)
{
        GtkWidget *ready_button;

        g_return_if_fail (GIBBON_IS_APP (self));

        ready_button = gibbon_app_find_widget (self, "toolbar-ready-button",
                                               GTK_TYPE_TOOL_BUTTON);

        gtk_tool_button_set_label (GTK_TOOL_BUTTON (ready_button), _("Ready"));
        gtk_tool_button_set_stock_id (GTK_TOOL_BUTTON (ready_button),
                                      GTK_STOCK_YES);
        gtk_tool_item_set_tooltip_text (GTK_TOOL_ITEM (ready_button),
                                        _("Click to set yourself busy."));
}

void
gibbon_app_set_state_busy (const GibbonApp *self)
{
        GtkWidget *ready_button;

        g_return_if_fail (GIBBON_IS_APP (self));

        ready_button = gibbon_app_find_widget (self, "toolbar-ready-button",
                                               GTK_TYPE_TOOL_BUTTON);

        gtk_tool_button_set_label (GTK_TOOL_BUTTON (ready_button), _("Busy"));
        gtk_tool_button_set_stock_id (GTK_TOOL_BUTTON (ready_button),
                                      GTK_STOCK_STOP);
        gtk_tool_item_set_tooltip_text (GTK_TOOL_ITEM (ready_button),
                                        _("Click to set yourself ready to"
                                          " play."));
}

static void
gibbon_app_on_toggle_ready (GibbonApp *self)
{
        GtkWidget *button;
        const gchar *stock_id;
        GibbonSession *session;

        session = gibbon_connection_get_session (self->priv->connection);
        g_return_if_fail (GIBBON_IS_SESSION (session));

        button = gibbon_app_find_widget (self, "toolbar-ready-button",
                                         GTK_TYPE_TOOL_BUTTON);
        stock_id = gtk_tool_button_get_stock_id (GTK_TOOL_BUTTON (button));
        if (0 == g_strcmp0 (GTK_STOCK_YES, stock_id)) {
                gibbon_app_set_state_busy (self);
                gibbon_session_set_available (session, FALSE);
        } else {
                gibbon_app_set_state_available (self);
                gibbon_session_set_available (session, TRUE);
        }
}

static void
gibbon_app_on_action_selected (GibbonApp *self, gint action_no)
{
        gint game_no;
        const GibbonGame *game;
        const GibbonPosition *current;
        GibbonPosition *pos;

        game_no = gibbon_match_list_get_active_game (self->priv->match_list);
        if (game_no < 0)
                return;

        if (!self->priv->match)
                return;

        game = gibbon_match_get_nth_game (self->priv->match, game_no);
        if (!game)
                return;

        if (action_no < 0)
                current = gibbon_game_get_initial_position (game);
        else
                current = gibbon_game_get_nth_position (game, action_no);
        if (!current)
                return;

        pos = gibbon_position_copy (current);
        gibbon_position_reset_unused_dice (pos);
        gibbon_board_set_position (GIBBON_BOARD (self->priv->board), pos);
        gibbon_position_free (pos);

        gibbon_analysis_view_set_analysis (self->priv->analysis_view,
                                           game, action_no);
}

const struct _GibbonMET *
gibbon_app_get_met (const GibbonApp *self)
{
        g_return_val_if_fail (GIBBON_IS_APP (self), NULL);

        return self->priv->met;
}

static void
gibbon_app_on_java_fibs_import (GibbonApp *self)
{
        GObject *obj;
        GibbonJavaFIBSImporter *importer;

        obj = gibbon_app_find_object (self, "import-java-fibs-menu-item",
                                      GTK_TYPE_MENU_ITEM);
        gtk_widget_set_sensitive (GTK_WIDGET (obj), FALSE);

        importer = gibbon_java_fibs_importer_new ();
        gibbon_java_fibs_importer_run (importer);
}

gboolean
gibbon_app_add_action (GibbonApp *self, GibbonPositionSide side,
                       GibbonGameAction *action,
                       gint64 timestamp, GError **error)
{
        gibbon_return_val_if_fail (GIBBON_IS_APP (self), FALSE, error);
        gibbon_return_val_if_fail (GIBBON_IS_GAME_ACTION (action), FALSE,
                                   error);

        if (!gibbon_match_add_action (self->priv->match, side, action,
                                      timestamp, error))
                return FALSE;


        g_signal_emit (self, gibbon_app_list_signals[ACTION_ADDED], 0, action);

        return TRUE;
}
