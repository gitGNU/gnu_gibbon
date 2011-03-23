/*
 * This file is part of gibbon.
 * Gibbon is a Gtk+ frontend for the First Internet Backgammon Server FIBS.
 * Copyright (C) 2009-2011 Guido Flohr, http://guido-flohr.net/.
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
 * SECTION:gibbon-server-console
 * @short_description: Class representing server output area!
 *
 * Since: 0.1.1
 *
 * Show server communication.
 **/

#include <glib.h>
#include <glib/gi18n.h>

#include <time.h>

#include "gibbon-prefs.h"
#include "gibbon-server-console.h"
#include "gibbon-signal.h"
#include "gibbon-connection.h"

static const char * const fibs_commands[] = {
                "about",
                "accept",
                "address",
                "autologin",
                "average",
                "away",
                "back",
                "beaver",
                "beginner",
                "blind",
                "board",
                "boardstyle",
                "bye",
                "client",
                "cls",
                "commands",
                "complaints",
                "countries",
                "crawford",
                "date",
                "dicetest",
                "double",
                "erase",
                "formula",
                "gag",
                "help",
                "hostnames",
                "invite",
                "join",
                "kibitz",
                "last",
                "leave",
                "look",
                "man",
                "message",
                "motd",
                "move",
                "names",
                "off",
                "oldboard",
                "oldmoves",
                "otter",
                "panic",
                "password",
                "pip",
                "raccoon",
                "ratings",
                "rawboard",
                "rawwho",
                "redouble",
                "reject",
                "resign",
                "roll",
                "rules",
                "save",
                "say",
                "set",
                "shout",
                "show",
                "shutdown",
                "sortwho",
                "stat",
                "tell",
                "tellx",
                "time",
                "timezones",
                "tinyfugue",
                "toggle",
                "unwatch",
                "version",
                "watch",
                "waitfor",
                "wave",
                "where",
                "whisper",
                "who",
                "whois",
                NULL
};

typedef struct _GibbonServerConsolePrivate GibbonServerConsolePrivate;
struct _GibbonServerConsolePrivate {
        GibbonApp *app;
        GtkTextView *text_view;
        GtkTextBuffer *buffer;

        GtkTextTag *raw_tag;
        GtkTextTag *sent_tag;
        GtkTextTag *received_tag;

        GibbonSignal *command_signal;

        gint max_recents;
        gint num_recents;
        GtkListStore *model;
};

#define GIBBON_SERVER_CONSOLE_PRIVATE(obj) (G_TYPE_INSTANCE_GET_PRIVATE ((obj), \
        GIBBON_TYPE_SERVER_CONSOLE, GibbonServerConsolePrivate))

G_DEFINE_TYPE (GibbonServerConsole, gibbon_server_console, G_TYPE_OBJECT)

static void _gibbon_server_console_print_raw (GibbonServerConsole *self,
                                              const gchar *string,
                                              GtkTextTag *tag,
                                              const gchar *prefix,
                                              gboolean linefeed);
static void gibbon_server_console_on_command (GibbonServerConsole *self,
                                              GtkEntry *entry);

static void 
gibbon_server_console_init (GibbonServerConsole *self)
{
        self->priv = G_TYPE_INSTANCE_GET_PRIVATE (self,
                GIBBON_TYPE_SERVER_CONSOLE, GibbonServerConsolePrivate);

        self->priv->app = NULL;
        self->priv->text_view = NULL;
        self->priv->buffer = NULL;

        self->priv->raw_tag = NULL;
        self->priv->sent_tag = NULL;
        self->priv->received_tag = NULL;

        self->priv->command_signal = NULL;

        self->priv->model = NULL;
        self->priv->max_recents = 100;
        self->priv->num_recents = 0;
}

static void
gibbon_server_console_finalize (GObject *object)
{
        GibbonServerConsole *self = GIBBON_SERVER_CONSOLE (object);
        GtkTreePath *path;
        GtkTreeIter iter;
        GSList *list_iter;
        GSList *new_recents;
        guint i, max_recents;
        GibbonPrefs *prefs;
        gchar *data;

        if (self->priv->raw_tag)
                g_object_unref (self->priv->raw_tag);
        self->priv->raw_tag = NULL;

        if (self->priv->sent_tag)
                g_object_unref (self->priv->sent_tag);
        self->priv->sent_tag = NULL;

        if (self->priv->received_tag)
                g_object_unref (self->priv->received_tag);
        self->priv->received_tag = NULL;

        if (self->priv->command_signal)
                g_object_unref (self->priv->command_signal);
        self->priv->command_signal = NULL;

        if (self->priv->model) {
                new_recents = NULL;
                prefs = gibbon_app_get_prefs (self->priv->app);
                max_recents = gibbon_prefs_get_int (prefs,
                                                    GIBBON_PREFS_MAX_COMMANDS);
                if (max_recents)
                        max_recents = 100;
                if (self->priv->num_recents < max_recents)
                        self->priv->num_recents = max_recents;

                path = gtk_tree_path_new_first ();
                for (i = 0; path && i < self->priv->num_recents; ++i) {
                        if (!gtk_tree_model_get_iter (
                                        GTK_TREE_MODEL (self->priv->model),
                                        &iter, path))
                                break;
                        gtk_tree_model_get (GTK_TREE_MODEL (self->priv->model),
                                            &iter, 0, &data, -1);

                        if (!data)
                                break;
                        new_recents = g_slist_append (new_recents,
                                                      data);
                        gtk_tree_path_next (path);
                }
                if (new_recents)
                        gibbon_prefs_set_list (prefs, GIBBON_PREFS_COMMANDS,
                                               new_recents);

                list_iter = new_recents;
                while (list_iter) {
                        if (list_iter->data)
                                g_free (list_iter->data);
                        list_iter = list_iter->next;
                }
                g_slist_free (new_recents);
                g_object_unref (self->priv->model);
        }
        self->priv->model = NULL;

        self->priv->app = NULL;
        self->priv->text_view = NULL;
        self->priv->buffer = NULL;

        G_OBJECT_CLASS (gibbon_server_console_parent_class)->finalize(object);
}

static void
gibbon_server_console_class_init (GibbonServerConsoleClass *klass)
{
        GObjectClass *object_class = G_OBJECT_CLASS (klass);
        
        g_type_class_add_private (klass, sizeof (GibbonServerConsolePrivate));

        object_class->finalize = gibbon_server_console_finalize;
}

/**
 * gibbon_server_console_new:
 * @app: The #GibbonApp.
 *
 * Creates a new #GibbonServerConsole.
 *
 * Returns: The newly created #GibbonServerConsole or %NULL in case of failure.
 */
GibbonServerConsole *
gibbon_server_console_new (GibbonApp *app)
{
        GibbonServerConsole *self = g_object_new (GIBBON_TYPE_SERVER_CONSOLE,
                                                  NULL);
        PangoFontDescription *font_desc;
        GibbonPrefs *prefs;
        GObject *entry;
        GtkEntryCompletion *completion;
        GSList *recents;
        GtkTreeIter iter;
        gsize num_known;
        gsize i;

        self->priv->app = app;
        self->priv->text_view =
                GTK_TEXT_VIEW (gibbon_app_find_object (app,
                                                       "server_text_view",
                                                       GTK_TYPE_TEXT_VIEW));
        gtk_text_view_set_wrap_mode (self->priv->text_view,
                                     GTK_WRAP_NONE);
        self->priv->buffer = gtk_text_view_get_buffer (self->priv->text_view);

        self->priv->raw_tag =
                gtk_text_buffer_create_tag (self->priv->buffer, NULL,
                                            "foreground", "black",
                                            NULL);
        self->priv->sent_tag =
                gtk_text_buffer_create_tag (self->priv->buffer, NULL,
                                            "foreground", "blue",
                                            "style", PANGO_STYLE_ITALIC,
                                            NULL);
        self->priv->received_tag =
                gtk_text_buffer_create_tag (self->priv->buffer, NULL,
                                            "foreground", "green",
                                            "style", PANGO_STYLE_ITALIC,
                                            NULL);

        font_desc = pango_font_description_from_string ("monospace 10");
        gtk_widget_modify_font (GTK_WIDGET (self->priv->text_view),
                                font_desc);
        pango_font_description_free (font_desc);

        gtk_text_view_set_cursor_visible (self->priv->text_view, FALSE);

        entry = gibbon_app_find_object (app, "server-command-entry",
                                        GTK_TYPE_ENTRY);
        completion = gtk_entry_completion_new ();
        gtk_entry_completion_set_text_column(completion, 0);
        gtk_entry_set_completion (GTK_ENTRY (entry), completion);
        self->priv->model = gtk_list_store_new (1, G_TYPE_STRING);

        prefs = gibbon_app_get_prefs (app);
        self->priv->max_recents =
                gibbon_prefs_get_int (prefs,
                                      GIBBON_PREFS_MAX_COMMANDS);
        recents = gibbon_prefs_get_list (prefs,
                                         GIBBON_PREFS_COMMANDS);

        /* Upgrade to a doubly-linked list.  */
        while (recents) {
                if (recents->data) {
                        ++self->priv->num_recents;
                        gtk_list_store_append (self->priv->model, &iter);
                        gtk_list_store_set (self->priv->model, &iter,
                                            0, recents->data,
                                            -1);
                }
                recents = recents->next;
        }
        g_slist_free (recents);
        num_known = (sizeof fibs_commands) / (sizeof fibs_commands[0]);
        for (i = 0; i < num_known; ++i) {
                gtk_list_store_append (self->priv->model, &iter);
                gtk_list_store_set (self->priv->model, &iter,
                                    0, fibs_commands[i],
                                    -1);
        }

        gtk_entry_completion_set_model (completion,
                                        GTK_TREE_MODEL (self->priv->model));

        self->priv->command_signal =
                gibbon_signal_new (entry, "activate",
                                   G_CALLBACK (gibbon_server_console_on_command),
                                   G_OBJECT (self));

        return self;
}

static void
_gibbon_server_console_print_raw (GibbonServerConsole *self,
                                  const gchar *string,
                                  GtkTextTag *tag,
                                  const gchar *prefix,
                                  gboolean linefeed)
{
        GtkTextBuffer *buffer = self->priv->buffer;
        gint length;
        GtkTextIter start, end;
        GibbonPrefs *prefs = prefs;
        struct tm *now;
        GTimeVal timeval;
        gchar *timestamp;

        length = gtk_text_buffer_get_char_count (buffer);

        prefs = gibbon_app_get_prefs (self->priv->app);

        /* We abuse the prefix a little.  If prefix is empty it is ignored.
         * If it is NULL, we assume that this is the login and in this case
         * we never print a timestamp.
         */
        if (prefix
            && gibbon_prefs_get_boolean (prefs,
                                         GIBBON_PREFS_DEBUG_TIMESTAMPS)) {
                g_get_current_time (&timeval);
                now = localtime ((time_t *) &timeval.tv_sec);
                timestamp = g_strdup_printf ("[%02d:%02d.%07ld] ",
                                             now->tm_hour,
                                             now->tm_min,
                                             timeval.tv_usec);
                gtk_text_buffer_insert_at_cursor (buffer, timestamp, -1);
                g_free (timestamp);
        }

        if (prefix)
                gtk_text_buffer_insert_at_cursor (buffer, prefix, -1);
        gtk_text_buffer_insert_at_cursor (buffer, string, -1);
        if (linefeed)
                gtk_text_buffer_insert_at_cursor (buffer, "\n", -1);
        gtk_text_buffer_get_iter_at_offset (buffer, &start, length);
        gtk_text_buffer_get_end_iter (buffer, &end);
        gtk_text_buffer_apply_tag (buffer, tag, &start, &end);

        gtk_text_view_scroll_to_mark (self->priv->text_view,
                gtk_text_buffer_get_insert (buffer),
                0.0, TRUE, 0.5, 1);
}

void
gibbon_server_console_print_raw (GibbonServerConsole *self,
                                 const gchar *string)
{
        _gibbon_server_console_print_raw (self, string, self->priv->raw_tag,
                                          "", FALSE);
}

void
gibbon_server_console_print_info (GibbonServerConsole *self,
                                  const gchar *string)
{
        _gibbon_server_console_print_raw (self, string, self->priv->raw_tag,
                                          "", TRUE);
}

void
gibbon_server_console_print_login (GibbonServerConsole *self,
                                   const gchar *string)
{
        _gibbon_server_console_print_raw (self, string, self->priv->sent_tag,
                                          NULL, TRUE);
}

void
gibbon_server_console_print_output (GibbonServerConsole *self,
                                    const gchar *string)
{
        GibbonPrefs *prefs = prefs;

        prefs = gibbon_app_get_prefs (self->priv->app);

        if (gibbon_prefs_get_boolean (prefs,
                                      GIBBON_PREFS_DEBUG_SERVER_COMM)) {
                _gibbon_server_console_print_raw (self, string,
                                self->priv->received_tag,
                                "<<< ", TRUE);
        }
}

void
gibbon_server_console_print_input (GibbonServerConsole *self,
                                   const gchar *string)
{
        GibbonPrefs *prefs = prefs;

        prefs = gibbon_app_get_prefs (self->priv->app);

        if (gibbon_prefs_get_boolean (prefs,
                                      GIBBON_PREFS_DEBUG_SERVER_COMM)) {
                _gibbon_server_console_print_raw (self, string,
                                self->priv->sent_tag,
                                ">>> ", TRUE);
        }
}

static void
gibbon_server_console_on_command (GibbonServerConsole *self, GtkEntry *entry)
{
        gchar *trimmed;
        GibbonConnection *connection;
        GtkTreeIter iter;

        g_return_if_fail (GIBBON_IS_SERVER_CONSOLE (self));
        g_return_if_fail (GTK_IS_ENTRY (entry));

        connection = gibbon_app_get_connection (self->priv->app);
        if (!connection)
                return;

        trimmed = pango_trim_string (gtk_entry_get_text (entry));
        if (!*trimmed) {
                g_free (trimmed);
                return;
        }
        gibbon_connection_queue_command (connection, TRUE, "%s", trimmed);
        gtk_list_store_prepend (self->priv->model, &iter);
        gtk_list_store_set (self->priv->model, &iter,
                            0, trimmed,
                            -1);
        ++self->priv->num_recents;

        gtk_entry_set_text (entry, "");
}

