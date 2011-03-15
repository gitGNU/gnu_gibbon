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

#include <errno.h>
#include <string.h>

#include <glib.h>
#include <glib/gi18n.h>
#include <glib/gstdio.h>
#include <glib/gprintf.h>

#include "gui.h"
#include "gibbon.h"
#include "gibbon-archive.h"
#include "gibbon-connection.h"

typedef struct _GibbonArchivePrivate GibbonArchivePrivate;
struct _GibbonArchivePrivate {
        gchar *servers_directory;
        gchar *session_directory;
};

#define GIBBON_ARCHIVE_PRIVATE(obj) (G_TYPE_INSTANCE_GET_PRIVATE ((obj), \
        GIBBON_TYPE_ARCHIVE, GibbonArchivePrivate))

G_DEFINE_TYPE (GibbonArchive, gibbon_archive, G_TYPE_OBJECT)

static void gibbon_archive_on_login (GibbonArchive *archive,
                                     GibbonConnection *connection);

static void 
gibbon_archive_init (GibbonArchive *self)
{
        self->priv = G_TYPE_INSTANCE_GET_PRIVATE (self,
                GIBBON_TYPE_ARCHIVE, GibbonArchivePrivate);

        self->priv->servers_directory = NULL;
}

static void
gibbon_archive_finalize (GObject *object)
{
        GibbonArchive *self = GIBBON_ARCHIVE (object);

        if (self->priv->servers_directory)
                g_free (self->priv->servers_directory);
        self->priv->servers_directory = NULL;

        if (self->priv->session_directory)
                g_free (self->priv->session_directory);
        self->priv->session_directory = NULL;

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
gibbon_archive_new (GibbonConnection *connection)
{
        GibbonArchive *self;
        const gchar *documents_servers_directory;
        gboolean first_run = FALSE;

        self = g_object_new (GIBBON_TYPE_ARCHIVE, NULL);
        documents_servers_directory =
                g_get_user_special_dir (G_USER_DIRECTORY_DOCUMENTS);

        if (!documents_servers_directory)
                documents_servers_directory = g_get_home_dir ();

        if (!documents_servers_directory) {
                display_error (_("Cannot determine documents servers_directory!"));
                g_object_unref (self);
                return NULL;
        }

        self->priv->servers_directory = g_build_filename (documents_servers_directory,
                                                  PACKAGE, "servers", NULL);

        if (!g_file_test (self->priv->servers_directory, G_FILE_TEST_EXISTS))
                first_run = TRUE;

        /* FIXME! Use constants from sys/stat.h!  */
        if (0 != g_mkdir_with_parents (self->priv->servers_directory, 0755)) {
                display_error (_("Failed to created servers_directory"
                               " `%s': %s!"),
                               self->priv->servers_directory,
                               strerror (errno));
                g_object_unref (self);
                return NULL;
        }

        g_signal_connect_swapped (connection, "logged_in",
                                  G_CALLBACK (gibbon_archive_on_login),
                                  self);

        if (first_run)
                display_info (_("You can import settings and saved"
                                " games from your old client."
                                " Check the menu `Extras' to see if"
                                " your old client software is supported!"));

        return self;
}

GibbonArchive *
gibbon_archive_new_from_session_info (const gchar *host, guint port,
                                      const gchar *login)
{
        GibbonArchive *self = gibbon_archive_new (NULL);
        gchar *session_directory;
        gchar *buf;

        if (!self)
                return NULL;

        session_directory = g_build_filename (self->priv->servers_directory,
                                              host, NULL);
        if (port != 4321) {
                buf = g_strdup_printf ("%s_%u", session_directory, port);
                g_free (session_directory);
                session_directory = buf;
        }
        buf = g_build_filename (session_directory, login, NULL);
        g_free (session_directory);
        self->priv->session_directory = buf;

        return self;
}

static void
gibbon_archive_on_login (GibbonArchive *self, GibbonConnection *connection)
{
        guint port;
        const gchar *login;
        const gchar *host;
        gchar *session_directory;
        gchar *buf;

        g_return_if_fail (GIBBON_IS_ARCHIVE (self));

        login = gibbon_connection_get_login (connection);
        host = gibbon_connection_get_hostname (connection);
        port = gibbon_connection_get_port (connection);

        if (self->priv->session_directory)
                g_free (self->priv->session_directory);

        session_directory = g_build_filename (self->priv->servers_directory,
                                              host, NULL);
        if (port != 4321) {
                buf = g_strdup_printf ("%s_%u", session_directory, port);
                g_free (session_directory);
                session_directory = buf;
        }
        buf = g_build_filename (session_directory, login, NULL);
        g_free (session_directory);
        self->priv->session_directory = buf;
}
