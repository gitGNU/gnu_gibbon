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
 * SECTION:gibbon-java-fibs-importer
 * @short_description: Import settings and games from JavaFIBS!
 *
 * Since: 0.1.0
 *
 * Class that handles data import from JavaFIBS.
 **/

#include <gtk/gtk.h>
#include <glib/gi18n.h>
#include <gio/gio.h>

#include <errno.h>
#include <stdio.h>

#include "gibbon-app.h"
#include "gibbon-java-fibs-importer.h"
#include "gibbon-archive.h"
#include "gibbon-util.h"
#include "gibbon-java-fibs-reader.h"
#include "gibbon-jelly-fish-reader.h"
#include "gibbon-settings.h"
#include "gibbon-gmd-writer.h"

enum GibbonImportTaskType {
        GIBBON_IMPORT_GROUP = 0,
        GIBBON_IMPORT_RELATION,
        GIBBON_IMPORT_RANK,
        GIBBON_IMPORT_MATCH,
        GIBBON_IMPORT_SAVED
};

typedef struct _GibbonImportTask GibbonImportTask;
struct _GibbonImportTask {
        enum GibbonImportTaskType type;
        gpointer payload;
};

typedef struct _GibbonRank GibbonRank;
struct _GibbonRank {
        gint64 timestamp;
        gdouble rating;
        guint64 experience;
};

typedef struct _GibbonJavaFIBSImporterPrivate GibbonJavaFIBSImporterPrivate;
struct _GibbonJavaFIBSImporterPrivate {
        gboolean running;

        GibbonArchive *archive;

        gchar *directory;
        gchar *user;
        gchar *server;
        guint port;
        gchar *password;

        GtkWidget *progress;
        gchar *status;

        guint okay_handler;
        guint stop_handler;

        GThread *worker;
        GMutex mutex;
        GtkWidget *summary;
        gint jobs;
        GHashTable *matches;
        gint finished;
        guint finished_ratings;
        guint finished_friends;
        guint finished_villains;
        guint finished_matches;
        guint error_matches;
        guint timeout;
        gboolean cancelled;

        GSList *msg_queue;
        GSList *msg_tags;
        GtkWidget *msg_view;
        GtkTextBuffer *msg_buffer;

        GSList *tasks;
};

#define GIBBON_JAVA_FIBS_IMPORTER_PRIVATE(obj) (G_TYPE_INSTANCE_GET_PRIVATE ((obj), \
        GIBBON_TYPE_JAVA_FIBS_IMPORTER, GibbonJavaFIBSImporterPrivate))

G_DEFINE_TYPE (GibbonJavaFIBSImporter, gibbon_java_fibs_importer, G_TYPE_OBJECT)

static gboolean gibbon_java_fibs_importer_select_directory (
                GibbonJavaFIBSImporter *self);
static gboolean gibbon_java_fibs_importer_check_directory (
                GibbonJavaFIBSImporter *self);
static gboolean gibbon_java_fibs_importer_select_user (
                GibbonJavaFIBSImporter *self);
static gboolean gibbon_java_fibs_importer_read_prefs (GibbonJavaFIBSImporter
                                                      *self,
                                                      gchar *path_to_prefs,
                                                      gchar **server,
                                                      guint *port,
                                                      gchar **password,
                                                      GError **error);
static gchar *gibbon_java_fibs_importer_decrypt (GibbonJavaFIBSImporter *self,
                                                 guint32 *buffer,
                                                 gsize password_length);
static void gibbon_java_fibs_importer_on_stop (GibbonJavaFIBSImporter *self);
static void gibbon_java_fibs_importer_on_okay (GibbonJavaFIBSImporter *self);
static gpointer gibbon_java_fibs_importer_work (GibbonJavaFIBSImporter *self);
static gboolean gibbon_java_fibs_importer_poll (GibbonJavaFIBSImporter *self);
static void gibbon_java_fibs_importer_ready (GibbonJavaFIBSImporter *self);
static void gibbon_java_fibs_importer_summary (GibbonJavaFIBSImporter *self);
static gboolean gibbon_java_fibs_importer_collect_matches (
                GibbonJavaFIBSImporter *self);
static void gibbon_java_fibs_importer_mi_free (gchar **record);
static void gibbon_java_fibs_importer_group (GibbonJavaFIBSImporter *self,
                                             const gchar *name,
                                             guint *counter);
static void gibbon_java_fibs_importer_ranks (GibbonJavaFIBSImporter *self);
static void gibbon_java_fibs_importer_match (GibbonJavaFIBSImporter *self,
                                             gchar *stem, gchar **files);
static void gibbon_java_fibs_importer_output (GibbonJavaFIBSImporter *self,
                                              gchar *tag, const gchar *format,
                                              ...) G_GNUC_PRINTF (3, 4);
static void gibbon_java_fibs_importer_update (GibbonJavaFIBSImporter *self,
                                              gchar *tag, const gchar *message);
static void gibbon_java_fibs_importer_add_task (GibbonJavaFIBSImporter *self,
                                                enum GibbonImportTaskType type,
                                                gpointer payload);
static void gibbon_java_fibs_importer_save_group (GibbonJavaFIBSImporter *self,
                                                  gchar *group);
static void gibbon_java_fibs_importer_save_relation (GibbonJavaFIBSImporter *self,
                                                     gchar *record);
static void gibbon_java_fibs_importer_save_rank (GibbonJavaFIBSImporter *self,
                                                 GibbonRank *rank);
static void gibbon_java_fibs_importer_save_match (GibbonJavaFIBSImporter *self,
                                                  const GibbonMatch *match);
static void gibbon_java_fibs_importer_save_saved (GibbonJavaFIBSImporter *self,
                                                  const GibbonMatch *match);

static void gibbon_import_yyerror (GError **error, const gchar *msg);
static void gibbon_import_task_free (GibbonImportTask *task);

static void 
gibbon_java_fibs_importer_init (GibbonJavaFIBSImporter *self)
{
        self->priv = G_TYPE_INSTANCE_GET_PRIVATE (self,
                GIBBON_TYPE_JAVA_FIBS_IMPORTER, GibbonJavaFIBSImporterPrivate);

        self->priv->running = FALSE;

        self->priv->archive = NULL;
        self->priv->directory = NULL;
        self->priv->user = NULL;
        self->priv->server = NULL;
        self->priv->port = 0;
        self->priv->password = NULL;

        self->priv->progress = NULL;
        self->priv->status = NULL;

        self->priv->worker = NULL;
        self->priv->jobs = -1;
        self->priv->matches = NULL;
        self->priv->finished = 0;
        self->priv->finished_ratings = 0;
        self->priv->finished_friends = 0;
        self->priv->finished_villains = 0;
        self->priv->finished_matches = 0;
        self->priv->error_matches = 0;
        self->priv->timeout = 0;
        self->priv->cancelled = FALSE;

        self->priv->okay_handler = 0;
        self->priv->stop_handler = 0;

        self->priv->msg_queue = NULL;
        self->priv->msg_tags = NULL;
        self->priv->msg_view = NULL;
        self->priv->msg_buffer = NULL;

        self->priv->tasks = NULL;
}

static void
gibbon_java_fibs_importer_finalize (GObject *object)
{
        GibbonJavaFIBSImporter *self = GIBBON_JAVA_FIBS_IMPORTER (object);
        GObject *obj;

        if (self->priv->stop_handler) {
                obj = gibbon_app_find_object (app, "java-fibs-importer-stop",
                                              GTK_TYPE_BUTTON);
                g_signal_handler_disconnect (obj, self->priv->stop_handler);
        }

        if (self->priv->okay_handler) {
                obj = gibbon_app_find_object (app, "java-fibs-importer-ok",
                                              GTK_TYPE_BUTTON);
                g_signal_handler_disconnect (obj, self->priv->okay_handler);
        }

        if (self->priv->timeout)
                g_source_remove (self->priv->timeout);

        if (self->priv->worker)
                g_thread_join (self->priv->worker);

        if (self->priv->archive)
                g_object_unref (self->priv->archive);
        obj = gibbon_app_find_object (app, "import-java-fibs-menu-item",
                                      GTK_TYPE_MENU_ITEM);
        gtk_widget_set_sensitive (GTK_WIDGET (obj), TRUE);

        g_free (self->priv->status);

        g_free (self->priv->directory);
        g_free (self->priv->user);
        g_free (self->priv->server);
        g_free (self->priv->password);
        if (self->priv->matches)
                g_hash_table_destroy (self->priv->matches);

        if (self->priv->msg_buffer)
                g_object_unref (self->priv->msg_buffer);
        g_slist_free_full (self->priv->msg_queue, g_free);
        g_slist_free_full (self->priv->msg_tags, g_free);
        g_slist_free_full (self->priv->tasks,
                           (GDestroyNotify) gibbon_import_task_free);

        G_OBJECT_CLASS (gibbon_java_fibs_importer_parent_class)->finalize(object);
}

static void
gibbon_java_fibs_importer_class_init (GibbonJavaFIBSImporterClass *klass)
{
        GObjectClass *object_class = G_OBJECT_CLASS (klass);
        
        g_type_class_add_private (klass, sizeof (GibbonJavaFIBSImporterPrivate));

        object_class->finalize = gibbon_java_fibs_importer_finalize;
}

GibbonJavaFIBSImporter *
gibbon_java_fibs_importer_new ()
{
        GibbonJavaFIBSImporter *self =
                        g_object_new (GIBBON_TYPE_JAVA_FIBS_IMPORTER, NULL);
        GtkTextTagTable *tags;
        GtkTextTag *error_tag;

        self->priv->progress = gibbon_app_find_widget (
                        app, "java-fibs-importer-progressbar",
                        GTK_TYPE_PROGRESS_BAR);
        gtk_progress_bar_set_fraction (GTK_PROGRESS_BAR (self->priv->progress),
                                       0.0);
        gtk_progress_bar_pulse (GTK_PROGRESS_BAR (self->priv->progress));
        gtk_progress_bar_set_text (GTK_PROGRESS_BAR (self->priv->progress),
                                   NULL);

        self->priv->summary = gibbon_app_find_widget (
                        app, "java-fibs-importer-summary",
                        GTK_TYPE_LABEL);

        self->priv->matches = g_hash_table_new_full (
                        g_str_hash,
                        g_str_equal,
                        g_free,
                        (GDestroyNotify) gibbon_java_fibs_importer_mi_free);

        self->priv->msg_view = gibbon_app_find_widget (
                        app, "java-fibs-importer-text-view",
                        GTK_TYPE_TEXT_VIEW);
        tags = gtk_text_tag_table_new ();
        error_tag = gtk_text_tag_new ("error");
        g_object_set (G_OBJECT (error_tag),
                      "foreground", "red",
                      NULL);
        gtk_text_tag_table_add (tags, error_tag);
        self->priv->msg_buffer = gtk_text_buffer_new (tags);

        gtk_text_view_set_buffer (GTK_TEXT_VIEW (self->priv->msg_view),
                                  self->priv->msg_buffer);

        return self;
}

void
gibbon_java_fibs_importer_run (GibbonJavaFIBSImporter *self)
{
        GtkWidget *dialog;
        gint reply;
        GtkWidget *widget;
        GError *error = NULL;
        guint id;
        GSettings *settings;

        g_return_if_fail (GIBBON_IS_JAVA_FIBS_IMPORTER (self));

        if (self->priv->running)
                return;

        self->priv->archive = gibbon_archive_new (&error);
        if (!self->priv->archive) {
                gibbon_app_fatal_error (app, NULL, "%s", error->message);
                /* NOTREACHED */
        }

        self->priv->running = TRUE;

        dialog = gtk_message_dialog_new_with_markup (
                                GTK_WINDOW (gibbon_app_get_window (app)),
                                GTK_DIALOG_DESTROY_WITH_PARENT,
                                GTK_MESSAGE_QUESTION,
                                GTK_BUTTONS_OK_CANCEL,
                                "<span weight='bold' size='larger'>"
                                "%s</span>\n%s",
                                _("Import from JavaFIBS"),
                                _("In order to import your saved matches and"
                                  " settings from JavaFIBS you first have to"
                                  " select the JavaFIBS installation"
                                  " directory (that is the directory that"
                                  " contains the .jar file)."));

        reply = gtk_dialog_run(GTK_DIALOG (dialog));
        gtk_widget_destroy(GTK_WIDGET (dialog));

        if (reply != GTK_RESPONSE_OK) {
                g_object_unref (self);
                return;
        }

        if (!gibbon_java_fibs_importer_select_directory (self)) {
                g_object_unref (self);
                return;
        }

        if (!gibbon_archive_login (self->priv->archive, self->priv->server,
                                   self->priv->port, self->priv->user,
                                   &error)) {
                gibbon_app_display_error (app, NULL, "%s",
                                          error->message);
                g_error_free (error);
                g_object_unref (self);
                return;
        }

        settings = g_settings_new (GIBBON_PREFS_SERVER_SCHEMA);
        g_settings_delay (settings);

        g_settings_set_string (settings, GIBBON_PREFS_SERVER_HOST,
                               self->priv->server);
        g_settings_set_value (settings, GIBBON_PREFS_SERVER_PORT,
                              g_variant_new_uint16 (self->priv->port));
        g_settings_set_string (settings, GIBBON_PREFS_SERVER_LOGIN,
                               self->priv->user);
        if (self->priv->password) {
                g_settings_set_string (settings, GIBBON_PREFS_SERVER_PASSWORD,
                                       self->priv->password);
                g_settings_set_boolean (settings,
                                        GIBBON_PREFS_SERVER_SAVE_PASSWORD,
                                        TRUE);
        } /* No else.  Leave current settings untouched.  */

        g_settings_apply (settings);
        g_object_unref (settings);

        dialog = gibbon_app_find_widget (app, "java-fibs-importer-dialog",
                                         GTK_TYPE_DIALOG);

        widget = gibbon_app_find_widget (app, "java-fibs-importer-stop",
                                         GTK_TYPE_BUTTON);
        self->priv->stop_handler = g_signal_connect_swapped (
                                G_OBJECT (widget), "clicked",
                                G_CALLBACK (gibbon_java_fibs_importer_on_stop),
                                self);
        gtk_widget_set_sensitive (widget, TRUE);

        widget = gibbon_app_find_widget (app, "java-fibs-importer-ok",
                                         GTK_TYPE_BUTTON);
        self->priv->okay_handler =
                        g_signal_connect_swapped (
                                        G_OBJECT (widget), "clicked",
                                        G_CALLBACK (gibbon_java_fibs_importer_on_okay),
                                        self);
        gtk_widget_set_sensitive (widget, FALSE);

        g_mutex_init (&self->priv->mutex);
        id = g_timeout_add_full (G_PRIORITY_DEFAULT_IDLE, 1,
                                 (GSourceFunc) gibbon_java_fibs_importer_poll,
                                 self, NULL);
        self->priv->timeout = id;

        gibbon_java_fibs_importer_summary (self);

        gtk_widget_show_all (dialog);

        if (!g_thread_try_new ("java-fibs-importer-worker",
                               (GThreadFunc) gibbon_java_fibs_importer_work,
                               self, &error)) {
                gibbon_app_display_error (app, _("System Error"),
                                          _("Error creating new thread: %s!"),
                                            error->message);
                g_object_unref (self);
                gtk_widget_hide (dialog);

                return;
        }
}

static gboolean
gibbon_java_fibs_importer_select_directory (GibbonJavaFIBSImporter *self)
{
        GtkWidget *dialog;
        gint reply;
        gchar *path;

        dialog = gtk_file_chooser_dialog_new (
                        _("Select JavaFIBS Directory"),
                        GTK_WINDOW (gibbon_app_get_window (app)),
                        GTK_FILE_CHOOSER_ACTION_SELECT_FOLDER,
                        GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
                        GTK_STOCK_OPEN, GTK_RESPONSE_OK,
                        NULL);
        gtk_file_chooser_set_create_folders (GTK_FILE_CHOOSER (dialog), FALSE);
        gtk_file_chooser_set_select_multiple (GTK_FILE_CHOOSER (dialog), FALSE);
        gtk_file_chooser_add_shortcut_folder (GTK_FILE_CHOOSER (dialog),
                                              "/home/guido/java/JavaFIBS2001.test",
                                              NULL);
        reply = gtk_dialog_run(GTK_DIALOG (dialog));
        if (reply != GTK_RESPONSE_OK) {
                gtk_widget_destroy (dialog);
                return FALSE;
        }

        path = gtk_file_chooser_get_filename (GTK_FILE_CHOOSER (dialog));
        gtk_widget_destroy (dialog);

        self->priv->directory = path;

        return gibbon_java_fibs_importer_check_directory (self);
}

static gboolean
gibbon_java_fibs_importer_check_directory (GibbonJavaFIBSImporter *self)
{
        gchar *path_to_last_user;
        GError *error = NULL;
        gchar *buffer;
        gsize bytes_read;

        /*
         * If we cannot find the file user/last.user
         */
        path_to_last_user = g_build_filename (self->priv->directory,
                                              "user", "last.user", NULL);
        if (!g_file_test (path_to_last_user, G_FILE_TEST_EXISTS)) {
                gibbon_app_display_error (app, _("Not a JavaFIBS Installation"),
                                          _("The directory `%s' does not look"
                                            " like a JavaFIBS installation. "
                                            " The file `%s' is missing."),
                                            self->priv->directory,
                                            path_to_last_user);
                g_free (path_to_last_user);
                return gibbon_java_fibs_importer_select_directory (self);
        }

        if (!gibbon_slurp_file (path_to_last_user, &buffer, &bytes_read,
                                NULL, &error)) {
                gibbon_app_display_error (app, _("Read Error"),
                                          _("Error reading `%s': %s!"),
                                            path_to_last_user,
                                            error->message);
                g_free (path_to_last_user);
                g_error_free (error);
                /* No point falling back to last step here.  */;
                return FALSE;
        }

        g_free (path_to_last_user);

        buffer = g_realloc (buffer, bytes_read + 1);
        buffer[bytes_read] = 0;

        self->priv->user = g_strdup (gibbon_trim (buffer));
        if (!*self->priv->user) {
                g_free (self->priv->user);
                self->priv->user = NULL;
        }
        g_free (buffer);

        return gibbon_java_fibs_importer_select_user (self);
}

static gboolean
gibbon_java_fibs_importer_select_user (GibbonJavaFIBSImporter *self)
{
        gchar *path_to_user = g_build_filename (self->priv->directory,
                                                "user", NULL);
        GError *error = NULL;
        GDir *dir = g_dir_open (path_to_user, 0, &error);
        const gchar *user_dir;
        gchar *path_to_prefs;
        gchar *server; guint port; gchar *password;
        GtkListStore *store;
        GtkTreeIter iter;
        gsize n_users = 0;
        gint active = -1;
        gchar *text;
        GtkWidget *window, *dialog, *content_area, *label, *combo;
        GtkCellRenderer *cell;
        gint response;

        if (!dir) {
                gibbon_app_display_error (app, NULL,
                                          _("Error opening directory `%s': %s!"),
                                            path_to_user,
                                            error->message);
                g_free (path_to_user);
                g_error_free (error);
                /* No point falling back to last step here.  */;
                return FALSE;
        }
        store = gtk_list_store_new (5,
                                    G_TYPE_STRING,
                                    G_TYPE_STRING,
                                    G_TYPE_STRING,
                                    G_TYPE_UINT,
                                    G_TYPE_STRING);

        do {
                user_dir = g_dir_read_name (dir);
                if (!user_dir)
                        break;
                if (user_dir[0] == '.')
                        continue;
                path_to_prefs = g_build_filename (path_to_user, user_dir,
                                                  "preferences", NULL);
                if (!g_file_test (path_to_prefs, G_FILE_TEST_EXISTS))
                        continue;

                if (!gibbon_java_fibs_importer_read_prefs (self, path_to_prefs,
                                                           &server, &port,
                                                           &password, NULL))
                        continue;
                gtk_list_store_append (store, &iter);
                text = g_strdup_printf (_("%s on %s port %u"),
                                        user_dir, server, port);
                gtk_list_store_set (store, &iter,
                                    0, text,
                                    1, user_dir,
                                    2, server,
                                    3, port,
                                    4, password,
                                    -1);
                g_free (text);

                if (!g_strcmp0 (user_dir, self->priv->user))
                        active = n_users;

                ++n_users;
        } while (user_dir != NULL);

        g_dir_close (dir);

        if (!n_users) {
                gibbon_app_display_error (app, NULL,
                                          _("No user data found in `%s'!"),
                                            path_to_user);
                g_object_unref (store);
                g_free (path_to_user);
                return FALSE;
        } else if (n_users == 1) {
                /* Iter was initialized above.  */
                g_free (self->priv->user);
                gtk_tree_model_get (GTK_TREE_MODEL (store), &iter,
                                    1, &self->priv->user,
                                    2, &self->priv->server,
                                    3, &self->priv->port,
                                    4, &self->priv->password,
                                    -1);
                g_object_unref (store);
                g_free (path_to_user);

                return TRUE;
        }

        window = gibbon_app_get_window (app);
        dialog = gtk_dialog_new_with_buttons (_("Select Account"),
                                              GTK_WINDOW (window),
                                              GTK_DIALOG_DESTROY_WITH_PARENT
                                              | GTK_DIALOG_MODAL,
                                              GTK_STOCK_CANCEL,
                                              GTK_RESPONSE_CANCEL,
                                              GTK_STOCK_OK,
                                              GTK_RESPONSE_OK,
                                              NULL);

        content_area = gtk_dialog_get_content_area (GTK_DIALOG (dialog));

        label = gtk_label_new (_("Multiple accounts found.   Please select"
                                 " one identity:"));
        gtk_label_set_line_wrap (GTK_LABEL (label), TRUE);
        gtk_container_add (GTK_CONTAINER (content_area), label);

        combo = gtk_combo_box_new_with_model (GTK_TREE_MODEL (store));
        cell = gtk_cell_renderer_text_new ();
        gtk_cell_layout_pack_start (GTK_CELL_LAYOUT (combo), cell, TRUE);
        gtk_cell_layout_set_attributes (GTK_CELL_LAYOUT (combo), cell,
                                        "text", 0,
                                        NULL);

        gtk_combo_box_set_entry_text_column (GTK_COMBO_BOX (combo), 0);
        gtk_combo_box_set_active (GTK_COMBO_BOX (combo), active);

        gtk_container_add (GTK_CONTAINER (content_area), combo);

        gtk_widget_show_all (dialog);

        gtk_dialog_set_default_response (GTK_DIALOG (dialog), GTK_RESPONSE_OK);
        response = gtk_dialog_run (GTK_DIALOG (dialog));
        active = gtk_combo_box_get_active (GTK_COMBO_BOX (combo));

        if (response != GTK_RESPONSE_OK
            || !gtk_combo_box_get_active_iter (GTK_COMBO_BOX (combo), &iter)) {
                gtk_widget_destroy (dialog);
                g_object_unref (store);
                g_free (path_to_user);
                return FALSE;
        }

        gtk_widget_destroy (dialog);

        gtk_tree_model_get (GTK_TREE_MODEL (store), &iter,
                            1, &self->priv->user,
                            2, &self->priv->server,
                            3, &self->priv->port,
                            4, &self->priv->password,
                            -1);

        g_object_unref (store);
        g_free (path_to_user);

        return TRUE;
}

static gboolean
gibbon_java_fibs_importer_read_prefs (GibbonJavaFIBSImporter *self,
                                      gchar *path_to_prefs,
                                      gchar **server,
                                      guint *port,
                                      gchar **password,
                                      GError **error)
{
        gsize filesize;
        gchar *buffer;
        guint32 *ui32;
        guint32 password_length, server_length;
        gchar *server_utf16;
        gsize bytes_read, bytes_written;

        if (!gibbon_slurp_file (path_to_prefs, &buffer, &filesize,
                                NULL, error))
                return FALSE;

        if (filesize < 8) {
                g_set_error_literal (error, GIBBON_ERROR, -1,
                                     _("premature end of file"));
                g_free (buffer);
                return FALSE;
        }

        ui32 = (guint32 *) buffer;
        ++ui32;
        password_length = GUINT32_FROM_BE (*ui32);
        ++ui32;

        if (filesize < 8 + 4 * password_length) {
                g_set_error_literal (error, GIBBON_ERROR, -1,
                                     _("premature end of file"));
                g_free (buffer);
                return FALSE;
        }
        if (password_length) {
                *password = gibbon_java_fibs_importer_decrypt (self,
                                                               ui32,
                                                               password_length);
                ui32 += password_length;
        } else {
                *password = NULL;
        }

        server_length = GUINT32_FROM_BE (*ui32);
        ++ui32;

        if (filesize < 16 + 4 * password_length + 2 * server_length) {
                g_set_error_literal (error, GIBBON_ERROR, -1,
                                     _("premature end of file"));
                g_free (buffer);
                return FALSE;
        }
        if (!server_length) {
                g_set_error_literal (error, GIBBON_ERROR, -1,
                                     _("no server name"));
                g_free (buffer);
                return FALSE;
        }

        server_utf16 = (gchar *) ui32;
        *server = g_convert (server_utf16, 2 * server_length,
                             "utf-8", "utf-16be",
                             &bytes_read, &bytes_written, error);
        if (!*server) {
                g_free (buffer);
                return FALSE;
        }

        if (bytes_read != 2 * server_length
            || bytes_written != server_length) {
                g_set_error_literal (error, GIBBON_ERROR, 1,
                                     _("error converting hostname"));
                return FALSE;
        }
        server_utf16 += 2 * server_length;
        ui32 = (guint32 *) server_utf16;

        *port = GUINT32_FROM_BE (*ui32);

        g_free (buffer);

        return TRUE;
}

static gchar *
gibbon_java_fibs_importer_decrypt (GibbonJavaFIBSImporter *self,
                                   guint32 *buffer,
                                   gsize password_length)
{
        gchar *password = g_malloc (password_length + 1);
        guchar *dest;
        guint32 s;
        guint32 *src;
        gint i;

        /*
         * This routine only "decrypts" passwords with characters in the
         * range from 32 to 127.  Other characters are not allowed anyway.
         *
         * BTW, this is not really encryption.  JavaFIBS scrambles passwords
         * by converting characters to floats.  The restriction to codes
         * 32-127 makes parsing the floats very simple.
         */
        src = buffer;
        dest = (guchar *) password;
        for (i = 0; i < password_length; ++i) {
                s = *src++;
                s = GUINT32_FROM_BE (s);
                if ((s & 0xff000000) != 0x42000000) {
                        g_free (password);
                        return NULL;
                }
                s = s & 0xff0000;
                s >>= 16;

                if (s < 0x80) {
                        if (s & 0x3) {
                                g_free (password);
                                return NULL;
                        }
                        *dest++ = (s / 4) + 32;
                } else {
                        if (s & 0x1) {
                                g_free (password);
                                return NULL;
                        }
                        *dest++ = (s / 2);
                }
        }
        *dest = 0;

        return password;
}

static void
gibbon_java_fibs_importer_on_stop (GibbonJavaFIBSImporter *self)
{
        gibbon_java_fibs_importer_ready (self);

        g_mutex_lock (&self->priv->mutex);
        self->priv->cancelled = TRUE;
        g_mutex_unlock (&self->priv->mutex);
}

static void
gibbon_java_fibs_importer_on_okay (GibbonJavaFIBSImporter *self)
{
        GtkWidget *dialog;

        dialog = gibbon_app_find_widget (app, "java-fibs-importer-dialog",
                                         GTK_TYPE_DIALOG);
        gtk_widget_hide (dialog);

        g_object_unref (self);
}

static gpointer
gibbon_java_fibs_importer_work (GibbonJavaFIBSImporter *self)
{
        guint jobs;
        GHashTableIter iter;
        gchar *stem;
        gchar **files;

        g_mutex_lock (&self->priv->mutex);
        g_free (self->priv->status);
        self->priv->status = g_strdup (_("Collecting data."));
        g_mutex_unlock (&self->priv->mutex);

        /*
         * The first 4 jobs are always importing the ratings, and the list
         * for friends and villains.
         */
        if (!gibbon_java_fibs_importer_collect_matches (self))
                return NULL;

        jobs = 3 + 2 * g_hash_table_size (self->priv->matches);

        g_mutex_lock (&self->priv->mutex);
        self->priv->jobs = jobs;
        g_mutex_unlock (&self->priv->mutex);

        gibbon_java_fibs_importer_group (self, "friends",
                                         &self->priv->finished_friends);
        g_mutex_lock (&self->priv->mutex);
        ++self->priv->finished;
        g_mutex_unlock (&self->priv->mutex);

        gibbon_java_fibs_importer_group (self, "villains",
                                         &self->priv->finished_villains);
        g_mutex_lock (&self->priv->mutex);
        ++self->priv->finished;
        g_mutex_unlock (&self->priv->mutex);

        /* Import ratings.  */
        gibbon_java_fibs_importer_ranks (self);
        g_mutex_lock (&self->priv->mutex);
        ++self->priv->finished;
        g_mutex_unlock (&self->priv->mutex);

        g_hash_table_iter_init (&iter, self->priv->matches);
        while (g_hash_table_iter_next (&iter, (gpointer) &stem,
                                       (gpointer )&files)) {
                g_mutex_lock (&self->priv->mutex);
                if (self->priv->cancelled) {
                        g_mutex_unlock (&self->priv->mutex);
                        return NULL;
                }
                g_mutex_unlock (&self->priv->mutex);

                gibbon_java_fibs_importer_match (self, stem, files);
        }

        g_mutex_lock (&self->priv->mutex);
        g_free (self->priv->status);
        self->priv->status = g_strdup ("");
        g_mutex_unlock (&self->priv->mutex);

        return NULL;
}

/*
 * In fact, the only real work that the worker thread does is parsing the
 * matches.  All the rest, especially updating the database, is done by the
 * main thread.  This is necessary because SQLite is "thread-safe" in that
 * sense that you can only use it from one thread, even if you protect the
 * database against concurrent access with mutexes.
 *
 * However, we assume here that the database activity is fast, and will not
 * block very long.
 */
static gboolean
gibbon_java_fibs_importer_poll (GibbonJavaFIBSImporter *self)
{
        gdouble fraction;
        GibbonImportTask *task;
        guint done = 0;

        g_mutex_lock (&self->priv->mutex);

        gibbon_java_fibs_importer_summary (self);

        if (self->priv->jobs <= 0)
                gtk_progress_bar_pulse (GTK_PROGRESS_BAR (self->priv->progress));
        else {
                fraction = (gdouble) self->priv->finished / self->priv->jobs;
                gtk_progress_bar_set_fraction (
                                GTK_PROGRESS_BAR (self->priv->progress),
                                fraction);
        }

        if (self->priv->status) {
                gtk_progress_bar_set_text (
                                GTK_PROGRESS_BAR (self->priv->progress),
                                self->priv->status);
                g_free (self->priv->status);
                self->priv->status = NULL;
        }

        self->priv->msg_queue = g_slist_reverse (self->priv->msg_queue);
        self->priv->msg_tags = g_slist_reverse (self->priv->msg_tags);

        while (self->priv->msg_queue) {
                gibbon_java_fibs_importer_update (self,
                                                  self->priv->msg_tags->data,
                                                  self->priv->msg_queue->data);
                g_free (self->priv->msg_queue->data);
                g_free (self->priv->msg_tags->data);
                self->priv->msg_queue =
                                g_slist_remove_link (self->priv->msg_queue,
                                                     self->priv->msg_queue);
                self->priv->msg_tags =
                                g_slist_remove_link (self->priv->msg_tags,
                                                     self->priv->msg_tags);        }

        g_mutex_unlock (&self->priv->mutex);

        while (self->priv->tasks) {
                g_mutex_lock (&self->priv->mutex);
                task = (GibbonImportTask *) self->priv->tasks->data;
                self->priv->tasks = g_slist_remove_link (self->priv->tasks,
                                                         self->priv->tasks);
                g_mutex_unlock (&self->priv->mutex);
                switch (task->type) {
                case GIBBON_IMPORT_GROUP:
                        gibbon_java_fibs_importer_save_group (self,
                                                              task->payload);
                        break;
                case GIBBON_IMPORT_RELATION:
                        gibbon_java_fibs_importer_save_relation (
                                        self, task->payload);
                        break;
                case GIBBON_IMPORT_RANK:
                        gibbon_java_fibs_importer_save_rank (
                                        self, task->payload);
                        break;
                case GIBBON_IMPORT_MATCH:
                        gibbon_java_fibs_importer_save_match (
                                        self, task->payload);
                        break;
                case GIBBON_IMPORT_SAVED:
                        gibbon_java_fibs_importer_save_saved (
                                        self, task->payload);
                        break;
                }
                gibbon_import_task_free (task);
                if (++done > 0)
                        break;
        }

        g_mutex_lock (&self->priv->mutex);

        /*
         * We have to check again for status updates.  They might have been
         * sent by ourselves.
         */
        if (self->priv->status) {
                gtk_progress_bar_set_text (
                                GTK_PROGRESS_BAR (self->priv->progress),
                                self->priv->status);
                g_free (self->priv->status);
                self->priv->status = NULL;
        }

        gibbon_java_fibs_importer_summary (self);

        self->priv->msg_queue = g_slist_reverse (self->priv->msg_queue);
        self->priv->msg_tags = g_slist_reverse (self->priv->msg_tags);

        while (self->priv->msg_queue) {
                gibbon_java_fibs_importer_update (self,
                                                  self->priv->msg_tags->data,
                                                  self->priv->msg_queue->data);
                g_free (self->priv->msg_queue->data);
                g_free (self->priv->msg_tags->data);
                self->priv->msg_queue =
                                g_slist_remove_link (self->priv->msg_queue,
                                                     self->priv->msg_queue);
                self->priv->msg_tags =
                                g_slist_remove_link (self->priv->msg_tags,
                                                     self->priv->msg_tags);
        }

        if (self->priv->cancelled) {
                gibbon_java_fibs_importer_ready (self);
                g_mutex_unlock (&self->priv->mutex);

                gibbon_java_fibs_importer_summary (self);

                return FALSE;
        }

        if (!self->priv->tasks && self->priv->jobs >= 0
            && self->priv->finished >= self->priv->jobs) {
                gibbon_java_fibs_importer_ready (self);
                g_mutex_unlock (&self->priv->mutex);

                gibbon_java_fibs_importer_summary (self);
                gtk_progress_bar_set_fraction (
                                GTK_PROGRESS_BAR (self->priv->progress),
                                1.0f);

                return FALSE;
        }

        g_mutex_unlock (&self->priv->mutex);

        return TRUE;
}

static void
gibbon_java_fibs_importer_ready (GibbonJavaFIBSImporter *self)
{
        GtkWidget *button;

        button = gibbon_app_find_widget (app, "java-fibs-importer-stop",
                                         GTK_TYPE_BUTTON);
        gtk_widget_set_sensitive (button, FALSE);
        button = gibbon_app_find_widget (app, "java-fibs-importer-ok",
                                         GTK_TYPE_BUTTON);
        gtk_widget_set_sensitive (button, TRUE);
}

static void
gibbon_java_fibs_importer_summary (GibbonJavaFIBSImporter *self)
{
        const gchar *password;
        gchar *summary;
        gchar *ratings;
        gchar *friends;
        gchar *villains;
        gchar *matches;
        gchar *errors;
        gchar *escaped;

        password = self->priv->password ?
                 _("Password recovered.") : _("Password not recovered.");

        ratings = g_strdup_printf (g_dngettext (GETTEXT_PACKAGE,
                                                _("one rating record"),
                                                _("%u rating records"),
                                                self->priv->finished_ratings),
                                   self->priv->finished_ratings);
        friends = g_strdup_printf (g_dngettext (GETTEXT_PACKAGE,
                                                _("one friend"),
                                                _("%u friends"),
                                                self->priv->finished_friends),
                                   self->priv->finished_friends);
        villains = g_strdup_printf (g_dngettext (GETTEXT_PACKAGE,
                                                 _("one villain"),
                                                 _("%u villains"),
                                                 self->priv->finished_villains),
                                    self->priv->finished_villains);
        matches = g_strdup_printf (
                        g_dngettext (GETTEXT_PACKAGE,
                                     _("one match successfully imported."),
                                     _("%u matches successfully imported."),
                                     self->priv->finished_matches),
                        self->priv->finished_matches);
        errors = g_strdup_printf (
                        g_dngettext (GETTEXT_PACKAGE,
                                     _("one match had errors."),
                                     _("%u matches had errors."),
                                     self->priv->error_matches),
                        self->priv->error_matches);

        summary = g_strdup_printf ("%s\n%s\n%s\n%s\n%s\n%s",
                                   password, ratings, friends, villains,
                                   matches, errors);
        escaped = g_markup_escape_text (summary, -1);
        g_free (summary);

        gtk_label_set_text (GTK_LABEL (self->priv->summary), escaped);

        g_free (escaped);
}

static gboolean
gibbon_java_fibs_importer_collect_matches (GibbonJavaFIBSImporter *self)
{
        gchar *directory;
        GDir *dir;
        GError *error = NULL;
        const gchar *filename;
        const gchar *last_dot;
        gchar **record;
        gchar *stem;

        directory = g_build_filename (self->priv->directory,
                                      "matches", "internal",
                                      NULL);
        dir = g_dir_open (directory, 0, &error);
        if (dir) {
                while ((filename = g_dir_read_name (dir)) != NULL) {
                        last_dot = rindex (filename, '.');
                        if (!last_dot)
                                continue;
                        if (g_ascii_strcasecmp (last_dot, ".match"))
                                continue;
                        stem = g_strdup (filename);
                        stem[strlen (stem) - 6] = 0;
                        record = g_malloc (2 * sizeof filename);
                        record[0] = g_strdup (filename);
                        record[1] = NULL;
                        g_hash_table_insert (self->priv->matches, stem, record);
                }
        } else {
                /*
                 * FIXME! This error is unlikely but has to be reported to the
                 * main thread somehow later.
                 */
                gibbon_java_fibs_importer_output (self, "error",
                                                  "%s", error->message);
                g_error_free (error);
                return FALSE;
        }
        g_free (directory);

        directory = g_build_filename (self->priv->directory,
                                      "matches", "jellyfish",
                                      NULL);
        dir = g_dir_open (directory, 0, &error);
        if (dir) {
                while ((filename = g_dir_read_name (dir)) != NULL) {
                        last_dot = rindex (filename, '.');
                        if (!last_dot)
                                continue;
                        if (g_ascii_strcasecmp (last_dot, ".mat"))
                                continue;
                        stem = g_strdup (filename);
                        stem[strlen (stem) - 4] = 0;
                        record = (gchar **) g_hash_table_lookup (
                                        self->priv->matches, stem);
                        if (!record) {
                                record = g_malloc (2 * sizeof filename);
                                record[0] = NULL;
                                g_hash_table_insert (self->priv->matches, stem,
                                                     record);
                        }
                        record[1] = g_strdup (filename);
                }
        } else {
                /*
                 * We can ignore this error.  Maybe the directory was never
                 * created, when the user never converted a match to
                 * JellyFish.
                 */
                g_error_free (error);
        }
        g_free (directory);

        return TRUE;
}

static void
gibbon_java_fibs_importer_mi_free (gchar **record)
{
        if (record) {
                g_free (record[0]);
                g_free (record[1]);
                g_free (record);
        }
}

static void
gibbon_java_fibs_importer_group (GibbonJavaFIBSImporter *self,
                                 const gchar *name, guint *counter)
{
        gchar *path;
        gchar *buffer;
        gsize bytes_read;
        GError *error = NULL;
        gchar **lines;
        gsize i, num_lines;
        gchar *line;
        gchar **tokens;
        gchar *relation;

        path = g_build_filename (self->priv->directory, "user",
                                 self->priv->user, name, NULL);
        if (!g_file_test (path, G_FILE_TEST_EXISTS)) {
                gibbon_java_fibs_importer_output (self, NULL,
                                                  _("No group `%s'.\n"),
                                                  name);
                g_free (path);
                return;
        }

        gibbon_java_fibs_importer_output (self, NULL, _("Reading `%s'.\n"),
                                          path);

        if (!gibbon_slurp_file (path, &buffer, &bytes_read, NULL, &error)) {
                gibbon_java_fibs_importer_output (self, "error",
                                                  /*
                                                   * TRANSLATORS: The first
                                                   * placeholder is a filename,
                                                   * the second an error
                                                   * occurring for that file.
                                                   */
                                                  _("%s: %s!\n"),
                                                  path, error->message);
                g_free (path);
                g_error_free (error);
                return;
        }

        if (!bytes_read) {
                gibbon_java_fibs_importer_output (self, NULL,
                                                  _("No group `%s'.\n"),
                                                  name);
                g_free (path);
                return;
        }

        gibbon_java_fibs_importer_add_task (self, GIBBON_IMPORT_GROUP,
                                            g_strdup (name));

        buffer = g_realloc (buffer, bytes_read + 1);
        buffer[bytes_read] = 0;
        lines = g_strsplit_set (buffer, "\012\015", -1);
        g_free (buffer);

        num_lines = g_strv_length (lines);
        for (i = 0; i < num_lines; ++i) {
                line = gibbon_trim (lines[i]);
                if (!*line)
                        continue;

                tokens = g_strsplit_set (line, " \t", 3);
                if (!tokens || !*tokens[0]) {
                        g_strfreev (tokens);
                        continue;
                }

                relation = g_strdup_printf ("%s/%s", name, tokens[0]);
                gibbon_java_fibs_importer_add_task (self,
                                                    GIBBON_IMPORT_RELATION,
                                                    relation);

                g_strfreev (tokens);
        }

        g_strfreev (lines);
        g_free (path);
}

static void
gibbon_java_fibs_importer_ranks (GibbonJavaFIBSImporter *self)
{
        gchar *path;
        gchar *buffer;
        gsize bytes_read;
        GError *error = NULL;
        gchar **lines;
        gsize i, num_lines;
        gchar *line;
        gchar **tokens;
        gint64 timestamp;
        gdouble rating;
        guint64 experience;
        gchar *endptr;
        gdouble last_rating = 0.0f;
        guint64 last_experience = 0;
        GibbonRank *rank;

        path = g_build_filename (self->priv->directory, "user",
                                 self->priv->user, "ratings", NULL);
        if (!g_file_test (path, G_FILE_TEST_EXISTS)) {
                gibbon_java_fibs_importer_output (self, NULL,
                                                  _("No saved ratings.\n"));
                g_free (path);
                return;
        }

        gibbon_java_fibs_importer_output (self, NULL, _("Reading `%s'.\n"),
                                          path);

        if (!gibbon_slurp_file (path, &buffer, &bytes_read, NULL, &error)) {
                gibbon_java_fibs_importer_output (self, "error",
                                                  /*
                                                   * TRANSLATORS: The first
                                                   * placeholder is a filename,
                                                   * the second an error
                                                   * occurring for that file.
                                                   */
                                                  _("%s: %s!\n"),
                                                  path, error->message);
                g_free (path);
                g_error_free (error);
                return;
        }

        if (!bytes_read) {
                gibbon_java_fibs_importer_output (self, NULL,
                                                  _("No saved ratings.\n"));
                g_free (path);
                return;
        }

        buffer = g_realloc (buffer, bytes_read + 1);
        buffer[bytes_read] = 0;
        lines = g_strsplit_set (buffer, "\012\015", -1);
        g_free (buffer);

        num_lines = g_strv_length (lines);
        for (i = 0; i < num_lines; ++i) {
                line = gibbon_trim (lines[i]);
                if (!*line)
                        continue;

                tokens = g_strsplit_set (line, " \t", 3);
                if (!tokens || !*tokens[0]) {
                        g_strfreev (tokens);
                        continue;
                }

                if (!tokens[1] || !tokens[2]) {
                        g_strfreev (tokens);
                        continue;
                }

                errno = 0;
                timestamp = g_ascii_strtoll (tokens[0], &endptr, 10);
                if (errno || !endptr || *endptr) {
                        g_strfreev (tokens);
                        continue;
                }

                rating = g_ascii_strtod (tokens[1], &endptr);
                if (errno || !endptr || *endptr) {
                        g_strfreev (tokens);
                        continue;
                }

                experience = g_ascii_strtoull (tokens[2], &endptr, 10);
                if (errno || !endptr || *endptr) {
                        g_strfreev (tokens);
                        continue;
                }

                g_strfreev (tokens);

                if (rating != last_rating || experience != last_experience
                    || i == 0) {
                        rank = g_malloc (sizeof *rank);
                        rank->timestamp = timestamp;
                        rank->rating = rating;
                        rank->experience = experience;
                        g_mutex_lock (&self->priv->mutex);
                        ++self->priv->jobs;
                        g_mutex_unlock (&self->priv->mutex);
                        gibbon_java_fibs_importer_add_task (self,
                                                            GIBBON_IMPORT_RANK,
                                                            rank);
                }

                last_rating = rating;
                last_experience = experience;
        }

        g_strfreev (lines);
        g_free (path);
}

static void
gibbon_java_fibs_importer_output (GibbonJavaFIBSImporter *self,
                                  gchar *tag, const gchar *format, ...)
{
        va_list args;
        gchar *message;

        va_start (args, format);
        message = g_strdup_vprintf(format, args);
        va_end (args);

        g_mutex_lock (&self->priv->mutex);
        self->priv->msg_queue = g_slist_prepend (self->priv->msg_queue,
                                                 message);
        self->priv->msg_tags = g_slist_prepend (self->priv->msg_tags,
                                                g_strdup (tag));
        g_mutex_unlock (&self->priv->mutex);
}

static void
gibbon_java_fibs_importer_update (GibbonJavaFIBSImporter *self,
                                  gchar *tag, const gchar *message)
{
        GtkTextBuffer *buffer;
        gint length;
        GtkTextIter start, end;

        buffer = self->priv->msg_buffer;
        length = gtk_text_buffer_get_char_count (buffer);

        gtk_text_buffer_insert_at_cursor (buffer, message, -1);

        if (tag) {
                gtk_text_buffer_get_iter_at_offset (buffer, &start, length);
                gtk_text_buffer_get_end_iter (buffer, &end);
                gtk_text_buffer_apply_tag_by_name (buffer, tag, &start, &end);
        }
        gtk_text_view_scroll_to_mark (GTK_TEXT_VIEW (self->priv->msg_view),
                gtk_text_buffer_get_insert (buffer),
                0.0, TRUE, 0.5, 1);
}

static void
gibbon_import_task_free (GibbonImportTask *task)
{
        if (task) {
                switch (task->type) {
                case GIBBON_IMPORT_GROUP:
                        g_free (task->payload);
                        break;
                case GIBBON_IMPORT_RELATION:
                        g_free (task->payload);
                        break;
                case GIBBON_IMPORT_RANK:
                        g_free (task->payload);
                        break;
                case GIBBON_IMPORT_MATCH:
                        g_object_unref (task->payload);
                        break;
                case GIBBON_IMPORT_SAVED:
                        g_object_unref (task->payload);
                        break;
                }
        }
}

static void
gibbon_java_fibs_importer_add_task (GibbonJavaFIBSImporter *self,
                                    enum GibbonImportTaskType type,
                                    gpointer payload)
{
        GibbonImportTask *task = g_malloc (sizeof *task);

        g_mutex_lock (&self->priv->mutex);

        task->type = type;
        task->payload = payload;
        self->priv->tasks = g_slist_append (self->priv->tasks, task);

        g_mutex_unlock (&self->priv->mutex);
}

static void
gibbon_java_fibs_importer_save_group (GibbonJavaFIBSImporter *self,
                                      gchar *group)
{
        GError *error = NULL;

        gibbon_java_fibs_importer_output (self, NULL,
                                          _("Creating/checking group `%s'.\n"),
                                          "friends");

        if (!gibbon_archive_create_group (self->priv->archive,
                                          self->priv->server,
                                          self->priv->port,
                                          self->priv->user,
                                          group, &error)) {
                gibbon_java_fibs_importer_output (self, "error",
                                                  "%s.\n", error->message);
                g_error_free (error);
        }
}

static void
gibbon_java_fibs_importer_save_relation (GibbonJavaFIBSImporter *self,
                                         gchar *group)
{
        gchar *user;
        GError *error = NULL;

        user = group + 1;
        while (*user && *user != '/')
                ++user;

        *user++ = 0;

        gibbon_java_fibs_importer_output (self, NULL,
                                          _("Importing user `%s' into group"
                                            " `%s'.\n"),
                                          user, group);

        if (!gibbon_archive_create_relation (self->priv->archive,
                                             self->priv->server,
                                             self->priv->port,
                                             self->priv->user,
                                             group, user,
                                             &error)) {
                gibbon_java_fibs_importer_output (self, "error",
                                                  "%s\n",
                                                  error->message);
                g_error_free (error);
        }
}

static void
gibbon_java_fibs_importer_save_rank (GibbonJavaFIBSImporter *self,
                                     GibbonRank *rank)
{
        GError *error = NULL;
        GDateTime *dt;
        gchar *formatted;

        g_mutex_lock (&self->priv->mutex);
        ++self->priv->finished;
        g_mutex_unlock (&self->priv->mutex);

        dt = g_date_time_new_from_unix_local (rank->timestamp / 1000);
        if (!dt) {
                gibbon_java_fibs_importer_output (self, "error",
                                                  _("Timestamp %lld out of"
                                                    " range!"),
                                                  (long long) rank->timestamp);
                return;
        }

        formatted = g_date_time_format (dt, "%c");
        gibbon_java_fibs_importer_output (self, NULL,
                                          _("Importing rating %.2f with"
                                            " experience %llu at %s.\n"),
                                          rank->rating,
                                          (unsigned long long) rank->experience,
                                          formatted);
        g_free (formatted);

        if (!gibbon_archive_update_rank (self->priv->archive,
                                         self->priv->server, self->priv->port,
                                         self->priv->user,
                                         rank->rating, rank->experience, dt,
                                         &error)) {
                g_date_time_unref (dt);
                gibbon_java_fibs_importer_output (self, "error",
                                                  "%s\n",
                                                  error->message);
                g_error_free (error);
        }
        g_date_time_unref (dt);

        g_mutex_lock (&self->priv->mutex);
        ++self->priv->finished_ratings;
        g_mutex_unlock (&self->priv->mutex);
}

static void
gibbon_java_fibs_importer_match (GibbonJavaFIBSImporter *self,
                                 gchar *stem, gchar **files)
{
        GibbonMatchReader *reader;
        GError *error = NULL;
        GibbonMatch *match = NULL;
        gchar *filename;
        gchar *ptr;
        GDateTime *dt;
        gint64 timestamp = G_MININT64;
        gint file_index = -1;
        GFile *file;
        GFileInfo *file_info;
        gchar *query = G_FILE_ATTRIBUTE_TIME_CHANGED
                       "," G_FILE_ATTRIBUTE_TIME_CHANGED_USEC;
        guint year, month, mday, hour, minute, seconds;
        guint factor;
        gint i;
        gchar *location;
        enum GibbonImportTaskType type = GIBBON_IMPORT_SAVED;

        if (files[0]) {
                file_index = 0;
                filename = g_build_filename (self->priv->directory, "matches",
                                             "internal", files[0], NULL);
                gibbon_java_fibs_importer_output (self, NULL,
                                                  _("Parsing `%s'.\n"),
                                                   filename);
                reader = GIBBON_MATCH_READER (gibbon_java_fibs_reader_new (
                                (GibbonMatchReaderErrorFunc)
                                gibbon_import_yyerror, &error));
                match = gibbon_match_reader_parse (reader, filename);
                if (!match) {
                        gibbon_java_fibs_importer_output (self, "error",
                                                          "%s\n",
                                                          error->message);
                        g_error_free (error);
                        error = NULL;
                } else if (!gibbon_match_get_current_game (match)) {
                        gibbon_java_fibs_importer_output (self, "error",
                                                          _("Empty or"
                                                            " incomplete match"
                                                            "file!\n"));
                        g_object_unref (match);
                        match = NULL;
                }
                g_free (filename);
                g_object_unref (reader);
        }

        if (!match && files[1]) {
                if (!files[0])
                        file_index = 1;
                filename = g_build_filename (self->priv->directory, "matches",
                                             "jellyfish", files[1], NULL);
                gibbon_java_fibs_importer_output (self, NULL,
                                                  _("Parsing `%s'.\n"),
                                                   filename);
                reader = GIBBON_MATCH_READER (gibbon_jelly_fish_reader_new (
                                (GibbonMatchReaderErrorFunc)
                                gibbon_import_yyerror, &error));
                match = gibbon_match_reader_parse (reader, filename);
                if (!match) {
                        gibbon_java_fibs_importer_output (self, "error",
                                                          "%s\n",
                                                          error->message);
                        g_error_free (error);
                        error = NULL;
                } else if (!gibbon_match_get_current_game (match)) {
                        gibbon_java_fibs_importer_output (self, "error",
                                                          _("Empty or"
                                                            " incomplete match"
                                                            "file!\n"));
                        g_object_unref (match);
                        match = NULL;
                }
                g_free (filename);
                g_object_unref (reader);
        }

        g_mutex_lock (&self->priv->mutex);
        ++self->priv->finished;
        g_mutex_unlock (&self->priv->mutex);

        if (!match) {
                g_mutex_lock (&self->priv->mutex);
                ++self->priv->finished;
                ++self->priv->error_matches;
                g_mutex_unlock (&self->priv->mutex);
                return;
        }

        /*
         * If the two opponents cannot be deduced from the scores, assign it
         * to the current user.
         */
        if (!g_strcmp0 ("You", gibbon_match_get_white (match)))
                gibbon_match_set_white (match, self->priv->user);

        /*
         * Try to extract the date and time of the match from the filename.
         */
        ptr = stem + strlen (stem) - 1;
        while (*ptr >= '0' && *ptr <= '9') {
                --ptr;
        }

        if (*ptr == '_' && 15 <= strlen (ptr)
            && 0 < sscanf (ptr, "_%04d%02d%02d%02d%02d%2d",
                        &year, &month, &mday, &hour, &minute, &seconds)) {
                dt = g_date_time_new_local (year, month, mday, hour, minute,
                                            seconds);
                if (dt) {
                        timestamp = 1000000 * g_date_time_to_unix (dt);
                        g_date_time_unref (dt);

                        factor = 100000;
                        for (i = 0; i < 6 && ptr[15 + i]; ++i) {
                                timestamp += (ptr[15 + i] - '0') * factor;
                                factor /= 10;
                        }
                        type = GIBBON_IMPORT_MATCH;
                }
        }

        /*
         * Use the creation date of the file.
         */
        if (timestamp == G_MININT64) {
                if (file_index == 0)
                        filename = g_build_filename (self->priv->directory,
                                                     "matches", "internal",
                                                     files[0], NULL);
                else
                        filename = g_build_filename (self->priv->directory,
                                                     "matches", "jellyfish",
                                                     files[1], NULL);
                file = g_file_new_for_path (filename);
                g_free (filename);
                file_info = g_file_query_info (file, query,
                                G_FILE_QUERY_INFO_NONE, NULL, NULL);
                if (file_info) {
                        if (g_file_info_has_attribute (
                                        file_info,
                                        G_FILE_ATTRIBUTE_TIME_CHANGED))
                                timestamp = 1000000
                                        * g_file_info_get_attribute_uint64 (
                                            file_info,
                                            G_FILE_ATTRIBUTE_TIME_CHANGED)
                                + g_file_info_get_attribute_uint32 (
                                            file_info,
                                            G_FILE_ATTRIBUTE_TIME_CHANGED_USEC);
                        g_object_unref (file_info);
                }
        }

        /*
         * If all of the above fails fall back to the current time.
         */
        if (timestamp == G_MININT64)
                timestamp = g_get_real_time ();

        gibbon_match_set_start_time (match, timestamp);

        if (self->priv->port == 1234)
                location = g_strdup_printf ("x-fibs://%s", self->priv->server);
        else
                location = g_strdup_printf ("x-fibs://%s:%u",
                                            self->priv->server,
                                            self->priv->port);
        gibbon_match_set_location (match, location);
        g_free (location);

        /*
         * We could try to guess the current rating here.  But since the
         * timestamp may involve a whole lot of guess work, we better
         * omit that.
         */
        gibbon_java_fibs_importer_add_task (self, type, match);
}

static void
gibbon_java_fibs_importer_save_match (GibbonJavaFIBSImporter *self,
                                      const GibbonMatch *match)
{
        gchar *formatted;
        GDateTime *dt;
        gint64 timestamp;
        GError *error = NULL;

        g_mutex_lock (&self->priv->mutex);
        ++self->priv->finished;
        g_mutex_unlock (&self->priv->mutex);

        timestamp = gibbon_match_get_start_time (match);
        dt = g_date_time_new_from_unix_local (timestamp / 1000000);
        formatted = g_date_time_format (dt, "%c");
        g_date_time_unref (dt);

        if (gibbon_match_get_length > 0) {
                gibbon_java_fibs_importer_output (
                                self, NULL,
                                _("Storing %llu-point match between %s and %s"
                                  " from %s in database.\n"),
                                  (unsigned long long)
                                  gibbon_match_get_length (match),
                                  gibbon_match_get_white (match),
                                  gibbon_match_get_black (match),
                                  formatted);
        } else {
                gibbon_java_fibs_importer_output (
                                self, NULL,
                                _("Storing unlimited match between %s and %s"
                                  " from %s in database.\n"),
                                  gibbon_match_get_white (match),
                                  gibbon_match_get_black (match),
                                  formatted);
        }
        g_free (formatted);

        if (!gibbon_archive_save_match (self->priv->archive,
                                        self->priv->server, self->priv->port,
                                        self->priv->user, match,
                                        &error)) {
                gibbon_java_fibs_importer_output (self, "error",
                                                  "%s",
                                                  error->message);
                g_error_free (error);

                g_mutex_lock (&self->priv->mutex);
                ++self->priv->error_matches;
                g_mutex_unlock (&self->priv->mutex);
        } else {
                g_mutex_lock (&self->priv->mutex);
                ++self->priv->finished_matches;
                g_mutex_unlock (&self->priv->mutex);
        }
}

static void
gibbon_java_fibs_importer_save_saved (GibbonJavaFIBSImporter *self,
                                      const GibbonMatch *match)
{
        gchar *formatted;
        GDateTime *dt;
        gint64 timestamp;
        gchar *saved_name;
        GFile *file;
        GError *error = NULL;
        GFileOutputStream *fout;
        GOutputStream *out;
        GibbonMatchWriter *writer;

        g_mutex_lock (&self->priv->mutex);
        ++self->priv->finished;
        g_mutex_unlock (&self->priv->mutex);

        timestamp = gibbon_match_get_start_time (match);
        dt = g_date_time_new_from_unix_local (timestamp / 1000000);
        formatted = g_date_time_format (dt, "%c");
        g_date_time_unref (dt);

        if (gibbon_match_get_length > 0) {
                gibbon_java_fibs_importer_output (
                                self, NULL,
                                _("Storing saved %llu-point match between %s"
                                  " and %s from %s in database.\n"),
                                  (unsigned long long)
                                  gibbon_match_get_length (match),
                                  gibbon_match_get_white (match),
                                  gibbon_match_get_black (match),
                                  formatted);
        } else {
                gibbon_java_fibs_importer_output (
                                self, NULL,
                                _("Storing saved unlimited match between %s"
                                  " and %s from %s in database.\n"),
                                  gibbon_match_get_white (match),
                                  gibbon_match_get_black (match),
                                  formatted);
        }
        g_free (formatted);

        /*
         * If the saved match already exists, keep it untouched.
         */
        saved_name = gibbon_archive_get_saved_name (
                        self->priv->archive,
                        gibbon_match_get_white (match),
                        gibbon_match_get_black (match),
                        NULL);
        if (saved_name && g_file_test (saved_name, G_FILE_TEST_EXISTS)) {
                gibbon_java_fibs_importer_output (self, NULL,
                                                  "%s",
                                                  _("Skipped (already"
                                                    " exists).\n"));
                g_free (saved_name);
                return;
        }

        file = g_file_new_for_path (saved_name);
        g_free (saved_name);

        fout = g_file_replace (file, NULL, FALSE, G_FILE_COPY_OVERWRITE,
                               NULL, &error);
        g_object_unref (file);
        if (!fout) {
                gibbon_java_fibs_importer_output (self, "error",
                                                  "%s",
                                                  error->message);
                g_error_free (error);
                g_mutex_lock (&self->priv->mutex);
                ++self->priv->error_matches;
                g_mutex_unlock (&self->priv->mutex);
                return;
        }

        out = G_OUTPUT_STREAM (fout);

        writer = GIBBON_MATCH_WRITER (gibbon_gmd_writer_new ());
        if (gibbon_match_writer_write_stream (writer, out, match, &error)) {
                g_mutex_lock (&self->priv->mutex);
                ++self->priv->finished_matches;
                g_mutex_unlock (&self->priv->mutex);
        } else {
                gibbon_java_fibs_importer_output (self, "error",
                                                  "%s",
                                                  error->message);
                g_error_free (error);

                g_mutex_lock (&self->priv->mutex);
                ++self->priv->error_matches;
                g_mutex_unlock (&self->priv->mutex);
        }

        g_object_unref (writer);
        g_object_unref (out);
}

static void
gibbon_import_yyerror (GError **error, const gchar *msg)
{
        /* We can only report the first error.  */
        if (*error)
                return;

        g_set_error_literal (error, GIBBON_ERROR, -1, msg);
}
