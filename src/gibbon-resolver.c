/* This file is part of Gibbon
 * Copyright (C) 2009 Guido Flohr
 * 
 * Gibbon is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 3 of the License, or (at your option) any later version.
 *
 * Gibbon is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with Gibbon; if not, see <http://www.gnu.org/licenses/>.
 */

#include <glib/gi18n.h>
#include <gtk/gtk.h>

#include "gibbon-resolver.h"
#include "gui.h"

enum gibbon_resolver_signals {
        RESOLVED,
        LAST_SIGNAL
};
static guint signals[LAST_SIGNAL] = { 0 };

struct _GibbonResolverPrivate {
        gchar *hostname;
        GThread *worker;
};

#define GIBBON_RESOLVER_PRIVATE(o) (G_TYPE_INSTANCE_GET_PRIVATE ((o), \
                                      GIBBON_TYPE_RESOLVER,           \
                                      GibbonResolverPrivate))
G_DEFINE_TYPE (GibbonResolver, gibbon_resolver, G_TYPE_OBJECT);

static gpointer *gibbon_resolver_resolve_worker (gpointer data);

static void
gibbon_resolver_init (GibbonResolver *conn)
{
        conn->priv = G_TYPE_INSTANCE_GET_PRIVATE (conn, 
                                                        GIBBON_TYPE_RESOLVER, 
                                                        GibbonResolverPrivate);
        conn->priv->hostname = NULL;
        conn->priv->worker = NULL;
}

static void
gibbon_resolver_finalize (GObject *object)
{
        GibbonResolver *resolver = GIBBON_RESOLVER (object);

        if (resolver->priv->hostname)
                g_free (resolver->priv->hostname);
        resolver->priv->hostname = NULL;
        
        /* There is no gthread_cancel().  We let the thread just vanish.  */
        resolver->priv->worker = NULL;
        
        G_OBJECT_CLASS (gibbon_resolver_parent_class)->finalize (object);
}

static void
gibbon_resolver_class_init (GibbonResolverClass *klass)
{
        GObjectClass* object_class = G_OBJECT_CLASS (klass);

        g_type_class_add_private (klass, sizeof (GibbonResolverPrivate));

        object_class->finalize = gibbon_resolver_finalize;
        
        signals[RESOLVED] =
                g_signal_new ("resolved",
                              G_TYPE_FROM_CLASS (klass),
                              G_SIGNAL_RUN_FIRST,
                              0,
                              NULL, NULL,
                              g_cclosure_marshal_VOID__STRING,
                              G_TYPE_NONE,
                              1,
                              G_TYPE_STRING);
        
}

GibbonResolver *
gibbon_resolver_new (const gchar *hostname)
{
         GibbonResolver *self = g_object_new (GIBBON_TYPE_RESOLVER, NULL);

         self->priv->hostname = g_strdup (hostname);

         return self;
}

static gpointer *
gibbon_resolver_resolve_worker (gpointer data)
{
        /* GibbonResolver *self = GIBBON_RESOLVER (data); */

        g_print ("Start resolving ...\n");
        sleep (500);

        /* g_signal_emit (G_OBJECT (self), signals[RESOLVED], 0, hostname, 0); */
        
        g_print ("Resolver thread exiting ...\n");
        return NULL;
}

gint
gibbon_resolver_resolve (GibbonResolver *self)
{
        GError *error;
        
        g_return_val_if_fail (GIBBON_IS_RESOLVER (self), -1);
        
        self->priv->worker = 
                g_thread_create ((GThreadFunc) gibbon_resolver_resolve_worker,
                                 (gpointer) self, FALSE, &error);
                                              
        if (!self->priv->worker) {
                display_error (_("Failed to create resolver thread: %s."),
                               error->message);
                g_error_free (error);
                return -1;
        }
        
        return 0;
}
