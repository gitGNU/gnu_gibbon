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

#include <stdlib.h>
#include <glib/gi18n.h>
#include <gio/gio.h>
#include <gtk/gtk.h>

#include <netdb.h>

#include "gibbon-connection.h"
#include "gibbon-connector.h"
#include "gui.h"

enum gibbon_connection_signals {
        RESOLVING,
        CONNECTING,
        DISCONNECTED,
        LAST_SIGNAL
};
static guint signals[LAST_SIGNAL] = { 0 };

struct _GibbonConnectionPrivate {
        gchar *hostname;
        guint port;
        gchar *login;
        gchar *password;
        
        enum GibbonConnectorState connector_state;
        
        GibbonConnector *connector;
        gchar *error;
};

#define GIBBON_CONNECTION_DEFAULT_PORT 4321
#define GIBBON_CONNECTION_DEFAULT_HOST "fibs.com"

#define GIBBON_CONNECTION_PRIVATE(o) (G_TYPE_INSTANCE_GET_PRIVATE ((o), \
                                      GIBBON_TYPE_CONNECTION,           \
                                      GibbonConnectionPrivate))
G_DEFINE_TYPE (GibbonConnection, gibbon_connection, G_TYPE_OBJECT);

static void
gibbon_connection_init (GibbonConnection *conn)
{
        conn->priv = G_TYPE_INSTANCE_GET_PRIVATE (conn, 
                                                        GIBBON_TYPE_CONNECTION, 
                                                        GibbonConnectionPrivate);

        conn->priv->hostname = NULL;
        conn->priv->port = 0;
        conn->priv->password = NULL;
        conn->priv->login = NULL;
        
        conn->priv->connector = NULL;
        conn->priv->connector_state = GIBBON_CONNECTOR_INITIAL;
        
        conn->priv->error = NULL;
}

static void
gibbon_connection_finalize (GObject *object)
{
        GibbonConnection *conn = GIBBON_CONNECTION (object);

        if (conn->priv->hostname)
                g_free (conn->priv->hostname);
        conn->priv->hostname = NULL;

        conn->priv->port = 0;

        if (conn->priv->password)
                g_free (conn->priv->password);
        conn->priv->password = NULL;

        if (conn->priv->login)
                g_free (conn->priv->login);
        conn->priv->login = NULL;

        if (conn->priv->connector)
                gibbon_connector_cancel (conn->priv->connector);
        conn->priv->connector = NULL;

        if (conn->priv->error)
                g_free (conn->priv->error);
        g_free (conn->priv->error);

        G_OBJECT_CLASS (gibbon_connection_parent_class)->finalize (object);
}

static void
gibbon_connection_class_init (GibbonConnectionClass *klass)
{
        GObjectClass* object_class = G_OBJECT_CLASS (klass);

        g_type_class_add_private (klass, sizeof (GibbonConnectionPrivate));

        object_class->finalize = gibbon_connection_finalize;

        signals[RESOLVING] =
                g_signal_new ("resolving",
                              G_TYPE_FROM_CLASS (klass),
                              G_SIGNAL_RUN_FIRST,
                              0,
                              NULL, NULL,
                              g_cclosure_marshal_VOID__STRING,
                              G_TYPE_NONE,
                              1,
                              G_TYPE_STRING);
        signals[CONNECTING] =
                g_signal_new ("connecting",
                              G_TYPE_FROM_CLASS (klass),
                              G_SIGNAL_RUN_FIRST,
                              0,
                              NULL, NULL,
                              g_cclosure_marshal_VOID__STRING,
                              G_TYPE_NONE,
                              1,
                              G_TYPE_STRING);
        signals[DISCONNECTED] =
                g_signal_new ("disconnected",
                              G_TYPE_FROM_CLASS (klass),
                              G_SIGNAL_RUN_FIRST,
                              0,
                              NULL, NULL,
                              g_cclosure_marshal_VOID__STRING,
                              G_TYPE_NONE,
                              1,
                              G_TYPE_STRING);
}

GibbonConnection *
gibbon_connection_new (void)
{
        return g_object_new (GIBBON_TYPE_CONNECTION, NULL);
}

void
gibbon_connection_set_hostname (GibbonConnection *self, const gchar *hostname)
{
        const gchar *new_hostname = NULL;
        
        g_return_if_fail (GIBBON_IS_CONNECTION (self));
        
        if (!(hostname && hostname[0])) {
                new_hostname = GIBBON_CONNECTION_DEFAULT_HOST;
        } else {
                new_hostname = hostname;
        }
        
        if (self->priv->hostname)
                g_free (self->priv->hostname);
        
        self->priv->hostname = g_strdup (new_hostname);
}

const gchar *
gibbon_connection_get_hostname (GibbonConnection *self)
{
        g_return_val_if_fail (GIBBON_IS_CONNECTION (self), NULL);
        return self->priv->hostname;
}

guint
gibbon_connection_get_port (GibbonConnection *self)
{
        g_return_val_if_fail (GIBBON_IS_CONNECTION (self), 0);
        return self->priv->port;
}

const gchar *
gibbon_connection_get_login (GibbonConnection *self)
{
        g_return_val_if_fail (GIBBON_IS_CONNECTION (self), NULL);
        return self->priv->login;
}

const gchar *
gibbon_connection_get_password (GibbonConnection *self)
{
        g_return_val_if_fail (GIBBON_IS_CONNECTION (self), NULL);
        return self->priv->password;
}

void
gibbon_connection_set_port (GibbonConnection *self, guint port)
{
        g_return_if_fail (GIBBON_IS_CONNECTION (self));
        
        if (!port) {
                port = GIBBON_CONNECTION_DEFAULT_PORT;
        }
        
        self->priv->port = port;
}

void
gibbon_connection_set_login (GibbonConnection *self, const gchar *login)
{
        g_return_if_fail (GIBBON_IS_CONNECTION (self));
        
        g_assert (login && login[0]);
        
        if (self->priv->login)
                g_free (self->priv->login);
        
        self->priv->login = g_strdup (login);
}

void
gibbon_connection_set_password (GibbonConnection *self, const gchar *password)
{
        g_return_if_fail (GIBBON_IS_CONNECTION (self));
        
        g_assert (password && password[0]);
        
        if (self->priv->password)
                g_free (self->priv->password);
        
        self->priv->password = g_strdup (password);
}

static gboolean
gibbon_connection_wait_connect (GibbonConnection *self)
{
        enum GibbonConnectorState last_state;
        GibbonConnector *connector;
        
        g_return_val_if_fail (GIBBON_IS_CONNECTION (self), FALSE);
        
        connector = self->priv->connector;
        if (!connector)
                return FALSE;
                
        last_state = self->priv->connector_state;
        self->priv->connector_state = 
                gibbon_connector_get_state (connector);
        
        if (last_state == self->priv->connector_state)
                return TRUE;

        switch (self->priv->connector_state) {
                case GIBBON_CONNECTOR_INITIAL:
                        return FALSE;
                case GIBBON_CONNECTOR_RESOLVING:
                        g_signal_emit (self, signals[RESOLVING], 0, 
                                       self->priv->hostname);
                        break;
                case GIBBON_CONNECTOR_CONNECTING:
                        g_signal_emit (self, signals[CONNECTING], 0, 
                                       self->priv->hostname);
                        break;
                case GIBBON_CONNECTOR_CANCELLED:
                        return FALSE;
                case GIBBON_CONNECTOR_ERROR:
                        gibbon_connection_disconnect (self);
                        return FALSE;
                case GIBBON_CONNECTOR_CONNECTED:
                        gibbon_connection_disconnect (self);
                        return FALSE;
        }
        
        return TRUE;
}

void
gibbon_connection_connect (GibbonConnection *self)
{
        g_return_if_fail (GIBBON_IS_CONNECTION (self));

        if (self->priv->connector)
                g_object_unref (self->priv->connector);

        self->priv->connector = gibbon_connector_new (self->priv->hostname);  
        self->priv->connector_state = GIBBON_CONNECTOR_INITIAL;
        
        if (!gibbon_connector_connect (self->priv->connector)) {
                display_error (gibbon_connector_error (self->priv->connector));
                gibbon_connection_disconnect (self);
                return;
        }
        
        g_timeout_add (100, (GSourceFunc) gibbon_connection_wait_connect, self);
}

void
gibbon_connection_disconnect (GibbonConnection *self)
{
        gchar *error= NULL;
        
        g_return_if_fail (GIBBON_IS_CONNECTION (self));

        if (self->priv->connector) {
                error = g_strdup (gibbon_connector_error (self->priv->connector));
                gibbon_connector_cancel (self->priv->connector);
        }
        self->priv->connector = NULL;
        self->priv->connector_state = GIBBON_CONNECTOR_INITIAL;
        
        g_signal_emit (G_OBJECT (self), signals[DISCONNECTED], 0, 
                       error);
        
        if (error) {
                gdk_threads_enter ();
                display_error (error);
                gdk_threads_leave (); 
                g_free (error);
        }
}
