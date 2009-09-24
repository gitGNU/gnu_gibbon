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

#include <sys/types.h>
#include <netdb.h>

#include "gibbon-networker.h"
#include "gui.h"

enum gibbon_networker_signals {
        RESOLVED,
        LAST_SIGNAL
};
static guint signals[LAST_SIGNAL] = { 0 };

struct _GibbonNetworkerPrivate {
        gchar *hostname;
        GThread *worker;
};

#define GIBBON_NETWORKER_PRIVATE(o) (G_TYPE_INSTANCE_GET_PRIVATE ((o), \
                                      GIBBON_TYPE_NETWORKER,           \
                                      GibbonNetworkerPrivate))
G_DEFINE_TYPE (GibbonNetworker, gibbon_networker, G_TYPE_OBJECT);

static gpointer *gibbon_networker_resolve_worker (gpointer data);

static void
gibbon_networker_init (GibbonNetworker *conn)
{
        conn->priv = G_TYPE_INSTANCE_GET_PRIVATE (conn, 
                                                        GIBBON_TYPE_NETWORKER, 
                                                        GibbonNetworkerPrivate);
        conn->priv->hostname = NULL;
        conn->priv->worker = NULL;
}

static void
gibbon_networker_finalize (GObject *object)
{
        GibbonNetworker *networker = GIBBON_NETWORKER (object);

        if (networker->priv->hostname)
                g_free (networker->priv->hostname);
        networker->priv->hostname = NULL;
        
        /* There is no gthread_cancel().  We let the thread just vanish.  */
        networker->priv->worker = NULL;
        
        G_OBJECT_CLASS (gibbon_networker_parent_class)->finalize (object);
}

static void
gibbon_networker_class_init (GibbonNetworkerClass *klass)
{
        GObjectClass* object_class = G_OBJECT_CLASS (klass);

        g_type_class_add_private (klass, sizeof (GibbonNetworkerPrivate));

        object_class->finalize = gibbon_networker_finalize;
        
        signals[RESOLVED] =
                g_signal_new ("resolved",
                              G_TYPE_FROM_CLASS (klass),
                              G_SIGNAL_RUN_FIRST,
                              0,
                              NULL, NULL,
                              g_cclosure_marshal_VOID__POINTER,
                              G_TYPE_NONE,
                              1,
                              G_TYPE_POINTER);
        
}

GibbonNetworker *
gibbon_networker_new (const gchar *hostname)
{
         GibbonNetworker *self = g_object_new (GIBBON_TYPE_NETWORKER, NULL);

         self->priv->hostname = g_strdup (hostname);

         return self;
}

static gpointer *
gibbon_networker_resolve_worker (gpointer data)
{
        GibbonNetworker *self = GIBBON_NETWORKER (data);
        struct addrinfo hints;
        struct addrinfo *result;
        int s;
 
        memset (&hints, 0, sizeof hints);       
        hints.ai_family = AF_UNSPEC;
        hints.ai_socktype = SOCK_STREAM;
        
        s = getaddrinfo (self->priv->hostname, NULL, &hints, &result);
        if (s != 0) {
                display_error (_("Error resolving address for %s: %s\n"),
                               self->priv->hostname, gai_strerror (s));
                result = NULL;
        }
        
        g_signal_emit (G_OBJECT (self), signals[RESOLVED], 0, result);
        
        return NULL;
}

gint
gibbon_networker_resolve (GibbonNetworker *self)
{
        GError *error;
        
        g_return_val_if_fail (GIBBON_IS_NETWORKER (self), -1);
        
        self->priv->worker = 
                g_thread_create ((GThreadFunc) gibbon_networker_resolve_worker,
                                 (gpointer) self, FALSE, &error);
                                              
        if (!self->priv->worker) {
                display_error (_("Failed to create networker thread: %s."),
                               error->message);
                g_error_free (error);
                return -1;
        }
        
        return 0;
}

const gchar *
gibbon_networker_get_hostname (GibbonNetworker *self)
{
        g_return_val_if_fail (GIBBON_IS_NETWORKER (self), NULL);
        
        return self->priv->hostname;
}