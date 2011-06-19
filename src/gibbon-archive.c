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
 *
 * One of the things handled here is the detection and evaluation of the
 * so-called "droppers", i.e. players on FIBS that leave a game, while it
 * is going on.  This is sometimes due to network problems.  Some people
 * hope to save a higher rating.
 *
 * Every player that finishes a match is assigned one point.  For each drop,
 * the dropper gets a malus of one point.  A resume is honored with 0.75
 * points but only if the associated drop happened while our session.
 * Otherwise it is impossible to find out who of the two opponents was
 * the dropper.
 *
 * This gives a malus to both types of droppers which is exactly what we want.
 * It could be argued that only finished matches that happened during our
 * session should be honored, just like only resumes after drops are honored.
 * But the interval between a drop and a resume is necessarily shorter than
 * the interface between start of a match and regular end.  And besides,
 * the result bias from the little injustice does not affect players that
 * stay long on the server.
 *
 * The whole thing results in two values: One is the confidence, which is
 * simply the number of recorded events.  The other is a a rating for the
 * player's reliability, which is the average of all recorded bonusses and
 * malusses.
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
#include "gibbon-database.h"

typedef struct _GibbonArchivePrivate GibbonArchivePrivate;
struct _GibbonArchivePrivate {
        GibbonApp *app;

        gchar *servers_directory;
        gchar *session_directory;

        GibbonDatabase *db;

        gchar *archive_directory;

        GHashTable *droppers;
};

#define GIBBON_ARCHIVE_PRIVATE(obj) (G_TYPE_INSTANCE_GET_PRIVATE ((obj), \
        GIBBON_TYPE_ARCHIVE, GibbonArchivePrivate))

G_DEFINE_TYPE (GibbonArchive, gibbon_archive, G_TYPE_OBJECT)

static void gibbon_archive_remove_from_droppers (GibbonArchive *self,
                                                 const gchar *hostname,
                                                 guint port,
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
gibbon_archive_on_login (GibbonArchive *self, const gchar *hostname,
                         guint port, const gchar *login)
{
        gchar *session_directory;
        gchar *buf;
        mode_t mode;

        g_return_if_fail (GIBBON_IS_ARCHIVE (self));
        g_return_if_fail (hostname != NULL);
        g_return_if_fail (port > 0);
        g_return_if_fail (login != NULL);

        session_directory = g_build_filename (self->priv->servers_directory,
                                              hostname, NULL);
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

        if (gibbon_database_get_server_id (self->priv->db, hostname, port)) {
                (void) gibbon_database_get_user_id (self->priv->db,
                                                    hostname, port, login);
        }
}

void
gibbon_archive_update_user_full (GibbonArchive *self,
                                 const gchar *hostname, guint port,
                                 const gchar *login, gdouble rating,
                                 gint experience)
{
        g_return_if_fail (GIBBON_IS_ARCHIVE (self));
        g_return_if_fail (hostname != NULL);
        g_return_if_fail (login != NULL);

        gibbon_database_update_user_full (self->priv->db,
                                          hostname, port, login,
                                          rating, experience);
}

void
gibbon_archive_save_win (GibbonArchive *self,
                         const gchar *hostname, guint port,
                         const gchar *winner, const gchar *loser)
{
        g_return_if_fail (GIBBON_IS_ARCHIVE (self));
        g_return_if_fail (hostname != 0);
        g_return_if_fail (port != 0);
        g_return_if_fail (port <= 65536);
        g_return_if_fail (winner != NULL);
        g_return_if_fail (loser != NULL);

        gibbon_archive_remove_from_droppers (self, hostname, port, winner, loser);

        (void) gibbon_database_insert_activity (self->priv->db,
                                                hostname, port,
                                                winner, 1.0);
        (void) gibbon_database_insert_activity (self->priv->db,
                                                hostname, port,
                                                loser, 1.0);
}

void
gibbon_archive_save_drop (GibbonArchive *self,
                          const gchar *hostname, guint port,
                          const gchar *dropper, const gchar *victim)
{
        g_return_if_fail (GIBBON_IS_ARCHIVE (self));
        g_return_if_fail (hostname != 0);
        g_return_if_fail (port != 0);
        g_return_if_fail (port <= 65536);
        g_return_if_fail (dropper != NULL);
        g_return_if_fail (victim != NULL);

        g_hash_table_insert (self->priv->droppers,
                             g_strdup_printf ("%s:%u%s:%s",
                                              hostname, port, dropper, victim),
                             (gpointer) 1);

        (void) gibbon_database_insert_activity (self->priv->db,
                                                hostname, port,
                                                dropper, -1.0);
}

void
gibbon_archive_save_resume (GibbonArchive *self,
                            const gchar *hostname, guint port,
                            const gchar *player1, const gchar *player2)
{
        gchar *key;

        g_return_if_fail (GIBBON_IS_ARCHIVE (self));
        g_return_if_fail (hostname != 0);
        g_return_if_fail (port != 0);
        g_return_if_fail (port <= 65536);
        g_return_if_fail (player1 != NULL);
        g_return_if_fail (player2 != NULL);

        key = g_alloca (strlen (hostname) + 5
                        + strlen (player1) + strlen (player2) + 4);

        (void) sprintf (key, "%s:%u:%s:%s", hostname, port, player1, player2);
        if (g_hash_table_remove (self->priv->droppers, key)) {
                (void) gibbon_database_insert_activity (self->priv->db,
                                                        hostname, port,
                                                        player1, +0.75);
                return;
        }

        (void) sprintf (key, "%s:%u:%s:%s", hostname, port, player2, player1);
        if (g_hash_table_remove (self->priv->droppers, key)) {
                (void) gibbon_database_insert_activity (self->priv->db,
                                                        hostname, port,
                                                        player2, +0.75);
                return;
        }
}

static void
gibbon_archive_remove_from_droppers (GibbonArchive *self,
                                     const gchar *hostname, guint port,
                                     const gchar *player1, const gchar *player2)
{
        gchar *key = g_alloca (strlen (hostname) + 5
                               + strlen (player1) + strlen (player2) + 4);

        (void) sprintf (key, "%s:%u:%s:%s", hostname, port, player1, player2);
        (void) g_hash_table_remove (self->priv->droppers, key);
        (void) sprintf (key, "%s:%u:%s:%s", hostname, port, player2, player1);
        (void) g_hash_table_remove (self->priv->droppers, key);
}

gboolean
gibbon_archive_get_reliability (GibbonArchive *self,
                                const gchar *hostname, guint port,
                                const gchar *login,
                                gdouble *value, guint *confidence)
{
        g_return_val_if_fail (GIBBON_IS_ARCHIVE (self), FALSE);
        g_return_val_if_fail (hostname != NULL, FALSE);
        g_return_val_if_fail (port != 0, FALSE);
        g_return_val_if_fail (port <= 65536, FALSE);
        g_return_val_if_fail (login != NULL, FALSE);
        g_return_val_if_fail (value != NULL, FALSE);
        g_return_val_if_fail (confidence != NULL, FALSE);

        return gibbon_database_get_reliability (self->priv->db,
                                                hostname, port,
                                                login, value, confidence);
}
