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
 * SECTION:gibbon-shouts
 * @short_description: Abstraction for the shout area.
 *
 * Since: 0.1.1
 *
 * Handling of FIBS shouts.
 **/

#include <glib.h>
#include <glib/gi18n.h>

#include "gibbon-shouts.h"

typedef struct _GibbonShoutsPrivate GibbonShoutsPrivate;
struct _GibbonShoutsPrivate {
        GibbonApp *app;
};

#define GIBBON_SHOUTS_PRIVATE(obj) (G_TYPE_INSTANCE_GET_PRIVATE ((obj), \
        GIBBON_TYPE_SHOUTS, GibbonShoutsPrivate))

G_DEFINE_TYPE (GibbonShouts, gibbon_shouts, G_TYPE_OBJECT)

static void 
gibbon_shouts_init (GibbonShouts *self)
{
        self->priv = G_TYPE_INSTANCE_GET_PRIVATE (self,
                GIBBON_TYPE_SHOUTS, GibbonShoutsPrivate);

        self->priv->app = NULL;
}

static void
gibbon_shouts_finalize (GObject *object)
{
        GibbonShouts *self = GIBBON_SHOUTS (object);

        self->priv->app = NULL;

        G_OBJECT_CLASS (gibbon_shouts_parent_class)->finalize(object);
}

static void
gibbon_shouts_class_init (GibbonShoutsClass *klass)
{
        GObjectClass *object_class = G_OBJECT_CLASS (klass);
        
        g_type_class_add_private (klass, sizeof (GibbonShoutsPrivate));

        object_class->finalize = gibbon_shouts_finalize;
}

/**
 * gibbon_shouts_new:
 * @app: The #GibbonApp.
 *
 * Creates a new #GibbonShouts.
 *
 * Returns: The newly created #GibbonShouts or %NULL in case of failure.
 */
GibbonShouts *
gibbon_shouts_new (/* FIXME! Argument list! */ const gchar *dummy)
{
        GibbonShouts *self = g_object_new (GIBBON_TYPE_SHOUTS, NULL);

        self->priv->app = NULL;

        return self;
}
