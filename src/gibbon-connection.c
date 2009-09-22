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
#include "gibbon-resolver.h"

enum gibbon_connection_state {
        GIBBON_CONNECTION_DISCONNECTED,
        GIBBON_CONNECTION_RESOLVING,
        GIBBON_CONNECTION_CONNECTING,
        GIBBON_CONNECTION_CONNECTED,
};

enum gibbon_connection_signals {
        RESOLVING,
        DISCONNECTED,
        LAST_SIGNAL
};
static guint signals[LAST_SIGNAL] = { 0 };

struct _GibbonConnectionPrivate {
        gchar *host;
        guint port;
        gchar *login;
        gchar *password;
        
        enum gibbon_connection_state state;
};

#define GIBBON_CONNECTION_DEFAULT_PORT 4321
#define GIBBON_CONNECTION_DEFAULT_HOST "fibs.com"

#define GIBBON_CONNECTION_PRIVATE(o) (G_TYPE_INSTANCE_GET_PRIVATE ((o), \
                                      GIBBON_TYPE_CONNECTION,           \
                                      GibbonConnectionPrivate))
G_DEFINE_TYPE (GibbonConnection, gibbon_connection, G_TYPE_OBJECT);

static void gibbon_connection_connect_addr (GibbonConnection *self,
                                            const gpointer addrinfo,
                                            GObject *resolver);

static void
gibbon_connection_init (GibbonConnection *conn)
{
        conn->priv = G_TYPE_INSTANCE_GET_PRIVATE (conn, 
                                                        GIBBON_TYPE_CONNECTION, 
                                                        GibbonConnectionPrivate);

        conn->priv->host = NULL;
        conn->priv->port = 0;
        conn->priv->password = NULL;
        conn->priv->login = NULL;
        
        conn->priv->state = GIBBON_CONNECTION_DISCONNECTED;
}

static void
gibbon_connection_finalize (GObject *object)
{
        GibbonConnection *conn = GIBBON_CONNECTION (object);

        if (conn->priv->host)
                g_free (conn->priv->host);
        conn->priv->host = NULL;

        conn->priv->port = 0;

        if (conn->priv->password)
                g_free (conn->priv->password);
        conn->priv->password = NULL;

        if (conn->priv->login)
                g_free (conn->priv->login);
        conn->priv->login = NULL;

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
gibbon_connection_set_host (GibbonConnection *self, const gchar *host)
{
        const gchar *new_host = NULL;
        
        g_return_if_fail (GIBBON_IS_CONNECTION (self));
        
        if (!(host && host[0])) {
                new_host = GIBBON_CONNECTION_DEFAULT_HOST;
        } else {
                new_host = host;
        }
        
        if (self->priv->host)
                g_free (self->priv->host);
        
        self->priv->host = g_strdup (new_host);
}

const gchar *
gibbon_connection_get_host (GibbonConnection *self)
{
        g_return_val_if_fail (GIBBON_IS_CONNECTION (self), NULL);
        return self->priv->host;
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

void
gibbon_connection_connect (GibbonConnection *self)
{
        GibbonResolver *resolver;
        
        g_return_if_fail (GIBBON_IS_CONNECTION (self));

        g_return_if_fail (gibbon_connection_disconnected (self));
 
        self->priv->state = GIBBON_CONNECTION_CONNECTING; 

        /* FIXME: Rather let the signal be emitted by the resolver, when it
         * actually starts resolving.  That will give it a chance omit the
         * signal if we have a plain IP address, and no resolution is
         * needed.
         */
        g_signal_emit (G_OBJECT (self), signals[RESOLVING], 0, 
                       self->priv->host);
        
        self->priv->state = GIBBON_CONNECTION_RESOLVING;        
        
        resolver = gibbon_resolver_new (self->priv->host);
        g_signal_connect_swapped (G_OBJECT (resolver), "resolved",
                                  G_CALLBACK (gibbon_connection_connect_addr), 
                                  self);

        if (gibbon_resolver_resolve (resolver)) {
                /* The resolver has displayed the error message.  */
                gibbon_connection_disconnect (self);
        }
}

void
gibbon_connection_connect_addr (GibbonConnection *self, 
                                const gpointer addrinfo,
                                GObject *resolver)
{
        const gchar *hostname;
        
        g_return_if_fail (GIBBON_IS_RESOLVER (resolver));

        hostname = gibbon_resolver_get_hostname (GIBBON_RESOLVER (resolver));        
        g_object_unref (resolver);
                
        g_return_if_fail (GIBBON_IS_CONNECTION (self));
        if (self->priv->state != GIBBON_CONNECTION_RESOLVING)
                return;
        if (g_strcmp0 (self->priv->host, hostname) != 0)
                return;
                
        g_print ("%s resolved, connecting not yet implemented :-(\n",
                 hostname);
}
        
gboolean
gibbon_connection_disconnected (GibbonConnection *self)
{
        if (self->priv->state == GIBBON_CONNECTION_DISCONNECTED)
                return TRUE;

        return FALSE;
}

void
gibbon_connection_disconnect (GibbonConnection *self)
{
        g_return_if_fail (GIBBON_IS_CONNECTION (self));

        g_return_if_fail (!gibbon_connection_disconnected (self));

        self->priv->state = GIBBON_CONNECTION_DISCONNECTED;
        g_signal_emit (G_OBJECT (self), signals[DISCONNECTED], 0, 
                       self->priv->host);
}
