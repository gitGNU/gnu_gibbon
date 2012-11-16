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

#include "gibbon-app.h"
#include "gibbon-java-fibs-importer.h"
#include "gibbon-archive.h"
#include "gibbon-util.h"

typedef struct _GibbonJavaFIBSImporterPrivate GibbonJavaFIBSImporterPrivate;
struct _GibbonJavaFIBSImporterPrivate {
        GibbonArchive *archive;
        gchar *directory;
        gchar *user;
        gchar *server;
        guint port;
        gchar *password;
};

#define GIBBON_JAVA_FIBS_IMPORTER_PRIVATE(obj) (G_TYPE_INSTANCE_GET_PRIVATE ((obj), \
        GIBBON_TYPE_JAVA_FIBS_IMPORTER, GibbonJavaFIBSImporterPrivate))

G_DEFINE_TYPE (GibbonJavaFIBSImporter, gibbon_java_fibs_importer, G_TYPE_OBJECT)

static void gibbon_java_fibs_importer_select_directory (
                GibbonJavaFIBSImporter *self);
static void gibbon_java_fibs_importer_check_directory (
                GibbonJavaFIBSImporter *self);
static void gibbon_java_fibs_importer_select_user (
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

static void 
gibbon_java_fibs_importer_init (GibbonJavaFIBSImporter *self)
{
        self->priv = G_TYPE_INSTANCE_GET_PRIVATE (self,
                GIBBON_TYPE_JAVA_FIBS_IMPORTER, GibbonJavaFIBSImporterPrivate);

        self->priv->archive = NULL;
        self->priv->directory = NULL;
        self->priv->user = NULL;
        self->priv->server = NULL;
        self->priv->port = 0;
        self->priv->password = NULL;
}

static void
gibbon_java_fibs_importer_finalize (GObject *object)
{
        GibbonJavaFIBSImporter *self = GIBBON_JAVA_FIBS_IMPORTER (object);
        GObject *obj;

        if (self->priv->archive)
                g_object_unref (self->priv->archive);
        obj = gibbon_app_find_object (app, "import-java-fibs-menu-item",
                                      GTK_TYPE_MENU_ITEM);
        gtk_widget_set_sensitive (GTK_WIDGET (obj), TRUE);

        g_free (self->priv->directory);
        g_free (self->priv->user);
        g_free (self->priv->server);
        g_free (self->priv->password);

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
gibbon_java_fibs_importer_new (GibbonApp *app)
{
        GibbonJavaFIBSImporter *self =
                        g_object_new (GIBBON_TYPE_JAVA_FIBS_IMPORTER, NULL);

        return self;
}

void
gibbon_java_fibs_importer_run (GibbonJavaFIBSImporter *self)
{
        GtkWidget *dialog;
        gint reply;

        g_return_if_fail (GIBBON_IS_JAVA_FIBS_IMPORTER (self));

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

        gibbon_java_fibs_importer_select_directory (self);

        g_object_unref (self);
}

static void
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
        g_printerr ("Remove line %s:%d!\n", __FILE__, __LINE__ + 1);
        gtk_file_chooser_add_shortcut_folder (GTK_FILE_CHOOSER (dialog),
                                              "/home/guido/java/JavaFIBS2001",
                                              NULL);
        reply = gtk_dialog_run(GTK_DIALOG (dialog));
        if (reply != GTK_RESPONSE_OK) {
                gtk_widget_destroy (dialog);
                return;
        }

        path = gtk_file_chooser_get_filename (GTK_FILE_CHOOSER (dialog));
        gtk_widget_destroy (dialog);

        self->priv->directory = path;

        gibbon_java_fibs_importer_check_directory (self);
}

static void
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
                gibbon_java_fibs_importer_select_directory (self);
                return;
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
                return;
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

        gibbon_java_fibs_importer_select_user (self);
}

static
void gibbon_java_fibs_importer_select_user (GibbonJavaFIBSImporter *self)
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

        if (!dir) {
                gibbon_app_display_error (app, NULL,
                                          _("Error opening directory `%s': %s!"),
                                            path_to_user,
                                            error->message);
                g_free (path_to_user);
                g_error_free (error);
                /* No point falling back to last step here.  */;
                return;
        }
        store = gtk_list_store_new (4,
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
                gtk_list_store_set (store, &iter,
                                    0, user_dir,
                                    1, server,
                                    2, port,
                                    3, password,
                                    -1);
                ++n_users;
        } while (user_dir != NULL);

        g_dir_close (dir);

        if (!n_users) {
                gibbon_app_display_error (app, NULL,
                                          _("No user data found in `%s'!"),
                                            path_to_user);
                g_object_unref (store);
                g_free (path_to_user);
                return;
        } else if (n_users == 1) {
                /* Iter was initialized above.  */
                g_free (self->priv->user);
                gtk_tree_model_get (GTK_TREE_MODEL (store), &iter,
                                    0, &self->priv->user,
                                    1, &self->priv->server,
                                    2, &self->priv->port,
                                    3, &self->priv->password,
                                    -1);
                g_object_unref (store);
                g_free (path_to_user);
                /* TODO: Call next step.  */
                return;
        }

        g_object_unref (store);
        g_free (path_to_user);
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
                g_set_error_literal (error, 0, -1, _("premature end of file"));
                g_free (buffer);
                return FALSE;
        }

        ui32 = (guint32 *) buffer;
        ++ui32;
        password_length = GUINT32_FROM_BE (*ui32);
        ++ui32;

        if (filesize < 8 + 4 * password_length) {
                g_set_error_literal (error, 0, -1, _("premature end of file"));
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
                g_set_error_literal (error, 0, -1, _("premature end of file"));
                g_free (buffer);
                return FALSE;
        }
        if (!server_length) {
                g_set_error_literal (error, 0, -1, _("no server name"));
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
                g_set_error_literal (error, 0, 1, _("error converting hostname"));
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
