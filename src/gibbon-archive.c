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
 * SECTION:gibbon-archive
 * @short_description: Game archive.
 *
 * Since: 0.1.1
 *
 * Handling of archived games.
 **/

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <errno.h>
#include <string.h>
#include <sys/stat.h>

#include <glib.h>
#include <glib/gi18n.h>
#include <glib/gstdio.h>
#include <glib/gprintf.h>

#include "gibbon-archive.h"
#include "gibbon-connection.h"
#include "gibbon-database.h"

typedef struct _GibbonArchivePrivate GibbonArchivePrivate;
struct _GibbonArchivePrivate {
        GibbonApp *app;

        gchar *servers_directory;
        gchar *session_directory;

        GibbonDatabase *db;
        gint server_id;

        gchar *archive_directory;

        GHashTable *droppers;
};

#define GIBBON_ARCHIVE_PRIVATE(obj) (G_TYPE_INSTANCE_GET_PRIVATE ((obj), \
        GIBBON_TYPE_ARCHIVE, GibbonArchivePrivate))

G_DEFINE_TYPE (GibbonArchive, gibbon_archive, G_TYPE_OBJECT)

static void gibbon_archive_remove_from_droppers (GibbonArchive *self,
                                                 const gchar *player1,
                                                 const gchar *player2);

static void 
gibbon_archive_init (GibbonArchive *self)
{
        self->priv = G_TYPE_INSTANCE_GET_PRIVATE (self,
                GIBBON_TYPE_ARCHIVE, GibbonArchivePrivate);

        self->priv->app = NULL;
        self->priv->servers_directory = NULL;
        self->priv->db = NULL;
        self->priv->server_id = -1;
        self->priv->droppers = NULL;
}

static void
gibbon_archive_finalize (GObject *object)
{
        GibbonArchive *self = GIBBON_ARCHIVE (object);

        if (self->priv->servers_directory)
                g_free (self->priv->servers_directory);

        if (self->priv->session_directory)
                g_free (self->priv->session_directory);

        if (self->priv->droppers)
                g_hash_table_destroy (self->priv->droppers);

        G_OBJECT_CLASS (gibbon_archive_parent_class)->finalize(object);
}

static void
gibbon_archive_class_init (GibbonArchiveClass *klass)
{
        GObjectClass *object_class = G_OBJECT_CLASS (klass);
        
        g_type_class_add_private (klass, sizeof (GibbonArchivePrivate));

        object_class->finalize = gibbon_archive_finalize;
}

GibbonArchive *
gibbon_archive_new (GibbonApp *app)
{
        GibbonArchive *self;
        const gchar *documents_servers_directory;
        gboolean first_run = FALSE;
        gchar *db_path;
        mode_t mode;

        self = g_object_new (GIBBON_TYPE_ARCHIVE, NULL);

        self->priv->app = app;

        documents_servers_directory =
                g_get_user_special_dir (G_USER_DIRECTORY_DOCUMENTS);

        if (!documents_servers_directory)
                documents_servers_directory = g_get_home_dir ();

        if (!documents_servers_directory) {
                gibbon_app_display_error (app,
                                          _("Cannot determine documents"
                                            " servers_directory!"));
                g_object_unref (self);
                return NULL;
        }

        db_path = g_build_filename (documents_servers_directory,
                                    PACKAGE, "db.sqlite", NULL);
        self->priv->db = gibbon_database_new (app, db_path);
        g_free (db_path);

        if (!self->priv->db) {
                g_object_unref (self);
                return NULL;
        }

        self->priv->servers_directory = g_build_filename (documents_servers_directory,
                                                  PACKAGE, "servers", NULL);

        if (!g_file_test (self->priv->servers_directory, G_FILE_TEST_EXISTS))
                first_run = TRUE;

        mode = S_IRWXU | (S_IRWXG & ~S_IWGRP) | (S_IRWXO & ~S_IWOTH);
        if (0 != g_mkdir_with_parents (self->priv->servers_directory, mode)) {
                gibbon_app_display_error (app,
                                          _("Failed to create"
                                            " servers_directory `%s': %s!"),
                               self->priv->servers_directory,
                               strerror (errno));
                g_object_unref (self);
                return NULL;
        }

        self->priv->droppers = g_hash_table_new_full (g_str_hash, g_str_equal,
                                                      g_free, NULL);

        if (first_run)
                gibbon_app_display_info (app,
                                         _("You can import settings and saved"
                                           " games from your old client."
                                           " Check the menu `Extras' to see if"
                                           " your old client software is"
                                           " supported!"));

        return self;
}

void
gibbon_archive_on_login (GibbonArchive *self, GibbonConnection *connection)
{
        guint port;
        const gchar *login;
        const gchar *host;
        gchar *session_directory;
        gchar *buf;
        mode_t mode;

        g_return_if_fail (GIBBON_IS_ARCHIVE (self));
        g_return_if_fail (GIBBON_IS_CONNECTION (connection));

        login = gibbon_connection_get_login (connection);
        host = gibbon_connection_get_hostname (connection);
        port = gibbon_connection_get_port (connection);

        session_directory = g_build_filename (self->priv->servers_directory,
                                              host, NULL);
        if (port != 4321) {
                buf = g_strdup_printf ("%s_%u", session_directory, port);
                g_free (session_directory);
                session_directory = buf;
        }
        buf = g_build_filename (session_directory, login, NULL);
        g_free (session_directory);

        if (self->priv->session_directory)
                g_free (self->priv->session_directory);
        self->priv->session_directory = buf;

        mode = S_IRWXU | (S_IRWXG & ~S_IWGRP) | (S_IRWXO & ~S_IWOTH);
        if (0 != g_mkdir_with_parents (self->priv->session_directory, mode)) {
                gibbon_app_display_error (self->priv->app,
                                          _("Failed to create"
                                            " directory `%s': %s!\n\n"
                                            "It will be impossible to save"
                                            " your matches and other data."),
                               self->priv->servers_directory,
                               strerror (errno));
        }

        self->priv->server_id = gibbon_database_update_server (self->priv->db,
                                                               host, port);
        gibbon_database_update_account (self->priv->db,
                                        self->priv->server_id, login);
}

void
gibbon_archive_update_user (GibbonArchive *self,
                            const gchar *hostname, guint port,
                            const gchar *login, gdouble rating,
                            gint experience)
{
        g_return_if_fail (GIBBON_IS_ARCHIVE (self));
        g_return_if_fail (hostname != NULL);
        g_return_if_fail (login != NULL);

        gibbon_database_update_user (self->priv->db,
                                     self->priv->server_id, login,
                                     rating, experience);
}

void
gibbon_archive_save_win (GibbonArchive *self,
                         const gchar *winner, const gchar *loser)
{
        g_return_if_fail (GIBBON_IS_ARCHIVE (self));
        g_return_if_fail (winner != NULL);
        g_return_if_fail (loser != NULL);

        gibbon_archive_remove_from_droppers (self, winner, loser);
        g_printerr ("TODO: Record win %s/%s.\n", winner, loser);
}

void
gibbon_archive_save_drop (GibbonArchive *self,
                          const gchar *dropper, const gchar *victim)
{
        g_return_if_fail (GIBBON_IS_ARCHIVE (self));
        g_return_if_fail (dropper != NULL);
        g_return_if_fail (victim != NULL);

        g_hash_table_insert (self->priv->droppers,
                             g_strdup_printf ("%s:%s", dropper, victim),
                             (gpointer) 1);

        g_printerr ("TODO: Record drop %s/%s.\n", dropper, victim);
}

void
gibbon_archive_save_resume (GibbonArchive *self,
                            const gchar *player1, const gchar *player2)
{
        gchar *key;

        g_return_if_fail (GIBBON_IS_ARCHIVE (self));
        g_return_if_fail (player1 != NULL);
        g_return_if_fail (player2 != NULL);

        key = g_alloca (strlen (player1) + strlen (player2) + 2);

        (void) sprintf (key, "%s:%s", player1, player2);
        if (g_hash_table_remove (self->priv->droppers, key)) {
                g_printerr ("TODO: Record resume %s/%s.\n", player1, player2);
                return;
        }

        (void) sprintf (key, "%s:%s", player2, player1);
        if (g_hash_table_remove (self->priv->droppers, key)) {
                g_printerr ("TODO: Record resume %s/%s.\n", player2, player1);
                return;
        }

        g_printerr ("Stale resume %s/%s.\n", player1, player2);
}

static void
gibbon_archive_remove_from_droppers (GibbonArchive *self,
                                     const gchar *player1, const gchar *player2)
{
        gchar *key = g_alloca (strlen (player1) + strlen (player2) + 2);

        (void) sprintf (key, "%s:%s", player1, player2);
        (void) g_hash_table_remove (self->priv->droppers, key);
        (void) sprintf (key, "%s:%s", player2, player1);
        (void) g_hash_table_remove (self->priv->droppers, key);
}
