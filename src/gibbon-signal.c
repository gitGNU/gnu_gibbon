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
 * SECTION:gibbon-signal
 * @short_description: Wrapper around a glib signal bound to an object.
 *
 * Since: 0.1.0
 *
 * Simplifies the handling of temporary signals.
 **/

#include <glib.h>
#include <glib-object.h>

#include "gibbon-signal.h"

typedef struct _GibbonSignalPrivate GibbonSignalPrivate;
struct _GibbonSignalPrivate {
        gulong handler_id;
        GObject *emitter;
};

#define GIBBON_SIGNAL_PRIVATE(obj) (G_TYPE_INSTANCE_GET_PRIVATE ((obj), \
        GIBBON_TYPE_SIGNAL, GibbonSignalPrivate))

G_DEFINE_TYPE (GibbonSignal, gibbon_signal, G_TYPE_OBJECT)

static void 
gibbon_signal_init (GibbonSignal *self)
{
        self->priv = G_TYPE_INSTANCE_GET_PRIVATE (self,
                GIBBON_TYPE_SIGNAL, GibbonSignalPrivate);

        self->priv->emitter = NULL;
}

static void
gibbon_signal_finalize (GObject *object)
{
        GibbonSignal *self = GIBBON_SIGNAL (object);

        if (self->priv->emitter
            && g_signal_handler_is_connected (self->priv->emitter,
                                              self->priv->handler_id))
                g_signal_handler_disconnect (self->priv->emitter,
                                             self->priv->handler_id);
        self->priv->emitter = NULL;

        G_OBJECT_CLASS (gibbon_signal_parent_class)->finalize(object);
}

static void
gibbon_signal_class_init (GibbonSignalClass *klass)
{
        GObjectClass *object_class = G_OBJECT_CLASS (klass);
        
        g_type_class_add_private (klass, sizeof (GibbonSignalPrivate));

        object_class->finalize = gibbon_signal_finalize;
}

GibbonSignal *
gibbon_signal_new (GObject *emitter, const gchar *name,
                   GCallback callback, GObject *receiver)
{
        GibbonSignal *self = g_object_new (GIBBON_TYPE_SIGNAL, NULL);

        self->priv->emitter = emitter;
        self->priv->handler_id = g_signal_connect_swapped ((gpointer) emitter,
                                                           name,
                                                           callback,
                                                           (gpointer) receiver);

        return self;
}
