/*
 * This file is part of Gibbon, a graphical frontend to the First Internet 
 * Backgammon Server FIBS.
 * Copyright (C) 2009 Guido Flohr, http://guido-flohr.net/.
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
#include <netdb.h>
#include <errno.h>

#include "gibbon-connector.h"
#include "gui.h"

struct _GibbonConnectorPrivate {
        gchar *hostname;
        guint port;
        gchar *error;
        enum GibbonConnectorState state;
        void* mutex;
        int socket_fd;
        GThread *worker;
};

#define GIBBON_CONNECTOR_PRIVATE(o) (G_TYPE_INSTANCE_GET_PRIVATE ((o), \
                                      GIBBON_TYPE_CONNECTOR,           \
                                      GibbonConnectorPrivate))
G_DEFINE_TYPE (GibbonConnector, gibbon_connector, G_TYPE_OBJECT);

static gpointer *gibbon_connector_connect_worker (GibbonConnector *self);

static void
gibbon_connector_init (GibbonConnector *conn)
{
        conn->priv = G_TYPE_INSTANCE_GET_PRIVATE (conn, 
                                                        GIBBON_TYPE_CONNECTOR, 
                                                        GibbonConnectorPrivate);
        conn->priv->hostname = NULL;
        conn->priv->error = NULL;
        
        conn->priv->state = GIBBON_CONNECTOR_INITIAL;
        conn->priv->mutex = NULL;
        conn->priv->worker = NULL;
        
        conn->priv->socket_fd = -1;
}

static void
gibbon_connector_finalize (GObject *object)
{
        GibbonConnector *connector = GIBBON_CONNECTOR (object);

        if (connector->priv->hostname)
                g_free (connector->priv->hostname);
        connector->priv->hostname = NULL;

        if (connector->priv->error)
                g_free (connector->priv->error);        
        connector->priv->error = NULL;
        
        if (connector->priv->socket_fd >= 0)
                close (connector->priv->socket_fd);
        connector->priv->socket_fd = -1;
 
        connector->priv->worker = NULL;

        if (connector->priv->mutex)
                g_mutex_free (connector->priv->mutex);
        connector->priv->mutex = NULL;

        G_OBJECT_CLASS (gibbon_connector_parent_class)->finalize (object);
}

static void
gibbon_connector_class_init (GibbonConnectorClass *klass)
{
        GObjectClass* object_class = G_OBJECT_CLASS (klass);

        g_type_class_add_private (klass, sizeof (GibbonConnectorPrivate));

        object_class->finalize = gibbon_connector_finalize;
}

GibbonConnector *
gibbon_connector_new (const gchar *hostname, guint port)
{
         GibbonConnector *self = g_object_new (GIBBON_TYPE_CONNECTOR, NULL);

         self->priv->hostname = g_strdup (hostname);
         self->priv->port = port;
         self->priv->mutex = g_mutex_new ();
         
         return self;
}

static gpointer *
gibbon_connector_connect_worker (GibbonConnector *self)
{
        struct addrinfo hints;
        struct addrinfo *results;
        struct addrinfo *ai;
        int s;
        int socket_fd = -1;
        int last_errno = 0;
        gchar *service = NULL;
                
        g_mutex_lock (self->priv->mutex);
        if (self->priv->state == GIBBON_CONNECTOR_CANCELLED) {
                g_mutex_unlock (self->priv->mutex);
                gibbon_connector_cancel (self);
                return NULL;
        }

        memset (&hints, 0, sizeof hints);       
        hints.ai_family = AF_UNSPEC;
        hints.ai_socktype = SOCK_STREAM;
        
        self->priv->state = GIBBON_CONNECTOR_RESOLVING;        
        g_mutex_unlock (self->priv->mutex);
        
        service = g_strdup_printf ("%u", self->priv->port);
        s = getaddrinfo (self->priv->hostname, service, &hints, &results);
        g_free (service);
        
        g_mutex_lock (self->priv->mutex);
        if (self->priv->state == GIBBON_CONNECTOR_CANCELLED) {
                g_mutex_unlock (self->priv->mutex);
                gibbon_connector_cancel (self);
                return NULL;
        }
        if (s != 0) {
                self->priv->error = 
                        g_strdup_printf (_("Error resolving address for "
                                           "%s: %s.\n"),
                                         self->priv->hostname, 
                                         gai_strerror (s));
                self->priv->state = GIBBON_CONNECTOR_ERROR;
                g_mutex_unlock (self->priv->mutex);
                return NULL;
        }
        
        self->priv->state = GIBBON_CONNECTOR_CONNECTING;

        g_mutex_unlock (self->priv->mutex);

        for (ai = results; ai != NULL; ai = ai->ai_next) {
                socket_fd = socket (ai->ai_family, ai->ai_socktype,
                                    ai->ai_protocol);
                
                last_errno = errno;

                if (socket_fd < 0) {
                        last_errno = errno;
                        continue;
                }
                
                if (connect (socket_fd, ai->ai_addr, ai->ai_addrlen) < 0) {
                        last_errno = errno;
                        continue;
                }
                break; /* Success! */
        }

        freeaddrinfo (results);
        
        g_mutex_lock (self->priv->mutex);
        if (self->priv->state == GIBBON_CONNECTOR_CANCELLED) {
                g_mutex_unlock (self->priv->mutex);
                gibbon_connector_cancel (self);
                return NULL;
        }

        if (ai == NULL) {
                self->priv->state = GIBBON_CONNECTOR_ERROR;
                self->priv->error = g_strdup (strerror (last_errno));
                g_mutex_unlock (self->priv->mutex);
                return NULL;
        }

        self->priv->socket_fd = socket_fd;
        self->priv->state = GIBBON_CONNECTOR_CONNECTED;
        
        g_mutex_unlock (self->priv->mutex);
        
        return NULL;
}

gboolean
gibbon_connector_connect (GibbonConnector *self)
{
        GError *error;
        
        g_return_val_if_fail (GIBBON_IS_CONNECTOR (self), FALSE);

        self->priv->worker = 
                g_thread_create ((GThreadFunc) gibbon_connector_connect_worker,
                                 (gpointer) self, FALSE, &error);
        if (!self->priv->worker) {
                self->priv->error = 
                        g_strdup_printf (_("Failed to create connector " 
                                           "thread: %s."),
                                         error->message);
                g_error_free (error);
                return FALSE;
        }
        
        return TRUE;
}

void
gibbon_connector_cancel (GibbonConnector *self)
{
        g_return_if_fail (GIBBON_IS_CONNECTOR (self));
 
        if (self->priv->state == GIBBON_CONNECTOR_CONNECTED
            || self->priv->state == GIBBON_CONNECTOR_ERROR
            || self->priv->state == GIBBON_CONNECTOR_CANCELLED) {
                g_object_unref (self);
        } else {
                g_mutex_lock (self->priv->mutex);
                self->priv->state = GIBBON_CONNECTOR_CANCELLED;
                g_mutex_unlock (self->priv->mutex);
        }
}

enum GibbonConnectorState 
gibbon_connector_get_state (GibbonConnector *self)
{
        g_return_val_if_fail (GIBBON_IS_CONNECTOR (self),
                              GIBBON_CONNECTOR_ERROR);
        return self->priv->state;
}

const gchar *
gibbon_connector_error (GibbonConnector *self)
{
        g_return_val_if_fail (GIBBON_IS_CONNECTOR (self), NULL);
        return self->priv->error;
}

gint
gibbon_connector_steal_socket (GibbonConnector *self)
{
        int socket_fd;
        
        g_return_val_if_fail (GIBBON_IS_CONNECTOR (self), -1);
        g_return_val_if_fail (self->priv->socket_fd >= 0, -1);
        
        socket_fd = self->priv->socket_fd;
        self->priv->socket_fd = -1;
        
        return socket_fd;
}
