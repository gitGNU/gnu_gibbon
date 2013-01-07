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
 * SECTION:gibbon-server-console
 * @short_description: Class representing server output area!
 *
 * Since: 0.1.0
 *
 * Show server communication.
 **/

#include <glib.h>
#include <glib/gi18n.h>
#include <glib/gstdio.h>

#include <time.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>

#include "gibbon-settings.h"
#include "gibbon-server-console.h"
#include "gibbon-signal.h"
#include "gibbon-connection.h"

static const char * const fibs_commands[] = {
                "about",
                "accept",
                "address",
                "alert",
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
                "whois"
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
        GtkEntryCompletion *completion;
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
        self->priv->completion = NULL;
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
        gchar *data;
        GSettings *settings;

        self->priv->raw_tag = NULL;
        self->priv->sent_tag = NULL;
        self->priv->received_tag = NULL;

        if (self->priv->command_signal)
                g_object_unref (self->priv->command_signal);
        self->priv->command_signal = NULL;

        if (self->priv->model) {
                new_recents = NULL;
                settings = g_settings_new (GIBBON_DATA_RECENT_SCHEMA);
                max_recents =
                        gibbon_settings_get_uint (settings,
                                                  GIBBON_DATA_MAX_COMMANDS);
                if (!max_recents)
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
                        (void)
                        gibbon_settings_set_string_list (settings,
                                                         GIBBON_DATA_COMMANDS,
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

        if (self->priv->completion)
                g_object_unref (self->priv->completion);

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
        GSettings *settings;
        GObject *entry;
        GtkEntryCompletion *completion;
        gchar **recents;
        gchar **recent;
        GtkTreeIter iter;
        gsize num_known;
        gsize i;
        GHashTable *seen;

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
        completion = self->priv->completion = gtk_entry_completion_new ();
        gtk_entry_completion_set_text_column(completion, 0);
        gtk_entry_set_completion (GTK_ENTRY (entry), completion);
        self->priv->model = gtk_list_store_new (1, G_TYPE_STRING);

        settings = g_settings_new (GIBBON_DATA_RECENT_SCHEMA);

        self->priv->max_recents =
                        gibbon_settings_get_uint (settings,
                                                  GIBBON_DATA_MAX_COMMANDS);
        recents = g_settings_get_strv (settings,
                                       GIBBON_DATA_COMMANDS);
        g_object_unref (settings);

        seen = g_hash_table_new_full (g_str_hash, g_str_equal, g_free, NULL);
        recent = recents;
        while (*recent) {
                if (g_hash_table_lookup_extended (seen, *recent, NULL, NULL)) {
                        ++recent;
                        continue;
                }
                g_hash_table_add (seen, g_strdup (*recent));
                ++self->priv->num_recents;
                gtk_list_store_append (self->priv->model, &iter);
                gtk_list_store_set (self->priv->model, &iter, 0, *recent, -1);
                ++recent;
        }
        g_strfreev (recents);

        num_known = (sizeof fibs_commands) / (sizeof fibs_commands[0]);
        for (i = 0; i < num_known; ++i) {
                if (g_hash_table_lookup_extended (seen, fibs_commands[i],
                                                  NULL, NULL))
                        continue;
                g_hash_table_add (seen, g_strdup (fibs_commands[i]));
                gtk_list_store_append (self->priv->model, &iter);
                gtk_list_store_set (self->priv->model, &iter,
                                    0, fibs_commands[i],
                                    -1);
        }
        g_hash_table_destroy (seen);

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
        struct tm *now;
        GTimeVal timeval;
        gchar *timestamp = NULL;
        GSettings *settings;
        gchar *logfile;
        gchar *full_logfile;
        FILE *log;
        const gchar *_prefix;
        gchar *_linefeed;

        length = gtk_text_buffer_get_char_count (buffer);
        gtk_text_buffer_get_iter_at_offset (buffer, &start, length);
        gtk_text_buffer_place_cursor (buffer, &start);

        settings = g_settings_new (GIBBON_PREFS_DEBUG_SCHEMA);

        /* We abuse the prefix a little.  If prefix is empty it is ignored.
         * If it is NULL, we assume that this is the login and in this case
         * we never print a timestamp.
         */
        if (prefix
            && g_settings_get_boolean (settings,
                                       GIBBON_PREFS_DEBUG_TIMESTAMPS)) {
                g_get_current_time (&timeval);
                now = localtime ((time_t *) &timeval.tv_sec);
                timestamp = g_strdup_printf ("[%02d:%02d:%02d.%06ld] ",
                                             now->tm_hour,
                                             now->tm_min,
                                             now->tm_sec,
                                             timeval.tv_usec);
                gtk_text_buffer_insert_at_cursor (buffer, timestamp, -1);
        }

        if (prefix)
                gtk_text_buffer_insert_at_cursor (buffer, prefix, -1);
        gtk_text_buffer_insert_at_cursor (buffer, string, -1);
        if (linefeed)
                gtk_text_buffer_insert_at_cursor (buffer, "\n", -1);
        gtk_text_buffer_get_iter_at_offset (buffer, &start, length);
        gtk_text_buffer_get_end_iter (buffer, &end);
        gtk_text_buffer_apply_tag (buffer, tag, &start, &end);

        gtk_text_buffer_place_cursor (buffer, &end);

        gtk_text_view_scroll_to_mark (self->priv->text_view,
                gtk_text_buffer_get_insert (buffer),
                0.0, TRUE, 0.5, 1);

        logfile = g_settings_get_string (settings,
                                         GIBBON_PREFS_DEBUG_LOGFILE);
        if (logfile && *logfile) {
                _prefix = prefix ? prefix : "";
                _linefeed = linefeed ? "\n" : "";
                full_logfile = g_strdup_printf ("%s.%llu", logfile,
                                                (unsigned long long) getpid ());
                log = g_fopen (full_logfile, "a");
                if (!log || !fprintf (log, "%s%s%s%s", timestamp,
                                      _prefix, string, _linefeed)
                    || fclose (log)) {
                        g_critical(_("Unable to write to logfile `%s': %s.\n"),
                                   full_logfile, strerror (errno));
                }
        }
        g_free (logfile);
        g_free (timestamp);

        g_object_unref (settings);
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
        GSettings *settings;

        g_return_if_fail (GIBBON_IS_SERVER_CONSOLE (self));
        g_return_if_fail (string != NULL);

        settings = g_settings_new (GIBBON_PREFS_DEBUG_SCHEMA);

        if (g_settings_get_boolean (settings,
                                    GIBBON_PREFS_DEBUG_FIBS)) {
                _gibbon_server_console_print_raw (self, string,
                                self->priv->received_tag,
                                "<<< ", TRUE);
        }
}

void
gibbon_server_console_print_input (GibbonServerConsole *self,
                                   const gchar *string)
{
        GSettings *settings;

        g_return_if_fail (GIBBON_IS_SERVER_CONSOLE (self));
        g_return_if_fail (string != NULL);

        settings = g_settings_new (GIBBON_PREFS_DEBUG_SCHEMA);

        if (g_settings_get_boolean (settings,
                                    GIBBON_PREFS_DEBUG_FIBS)) {
                _gibbon_server_console_print_raw (self, string,
                                self->priv->sent_tag,
                                ">>> ", TRUE);
        }
        g_object_unref (settings);
}

static void
gibbon_server_console_on_command (GibbonServerConsole *self, GtkEntry *entry)
{
        gchar *trimmed;
        GibbonConnection *connection;
        gboolean valid;
        GtkTreeIter iter;
        gchar *data;
        GtkTreeModel *model;

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

        model = GTK_TREE_MODEL (self->priv->model);
        valid = gtk_tree_model_get_iter_first (model, &iter);
        while (valid) {
                gtk_tree_model_get (model, &iter, 0, &data, -1);

                if (!data || 0 == g_strcmp0 (data, trimmed)) {
                        (void) gtk_list_store_remove (self->priv->model, &iter);
                        --self->priv->num_recents;
                        g_free (data);
                        break;
                }

                g_free (data);

                valid = gtk_tree_model_iter_next (model, &iter);
        }

        gtk_list_store_prepend (self->priv->model, &iter);
        gtk_list_store_set (self->priv->model, &iter,
                            0, trimmed,
                            -1);
        ++self->priv->num_recents;

        gtk_entry_set_text (entry, "");
}
