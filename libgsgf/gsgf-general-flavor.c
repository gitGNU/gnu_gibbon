/*
 * This file is part of Gibbon, a graphical frontend to the First Internet 
 * Backgammon Server FIBS.
 * Copyright (C) 2009-2010 Guido Flohr, http://guido-flohr.net/.
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

/**
 * SECTION:gsgf-general_flavor
 * @short_description: Handlers for common SGF properties.
 *
 * A number of SGF properties are common across all different flavors.
 * These properties can be handled by this fallback #GSGFFlavor class.
 */

#include <glib.h>
#include <glib/gi18n.h>

#include <libgsgf/gsgf.h>

struct _GSGFGeneralFlavorPrivate {
        const gchar *id;
        const gchar *name;
        const GSGFGeneralFlavor *parent;

        GHashTable *handlers;
};

#define GSGF_GENERAL_FLAVOR_PRIVATE(o) (G_TYPE_INSTANCE_GET_PRIVATE ((o), \
                                      GSGF_TYPE_GENERAL_FLAVOR,           \
                                      GSGFGeneralFlavorPrivate))

G_DEFINE_TYPE (GSGFGeneralFlavor, gsgf_general_flavor, GSGF_TYPE_FLAVOR)

static void
gsgf_general_flavor_init(GSGFGeneralFlavor *self)
{
        self->priv = G_TYPE_INSTANCE_GET_PRIVATE (self,
                        GSGF_TYPE_GENERAL_FLAVOR,
                        GSGFGeneralFlavorPrivate);

        self->priv->id = NULL;
        self->priv->name = NULL;
        self->priv->handlers = NULL;
}

static void
gsgf_general_flavor_finalize(GObject *object)
{
        GSGFGeneralFlavor *self = GSGF_GENERAL_FLAVOR (object);

        self->priv->id = NULL;

        self->priv->name = NULL;

        if (self->priv->handlers)
                g_hash_table_destroy(self->priv->handlers);
        self->priv->handlers = NULL;

        G_OBJECT_CLASS (gsgf_general_flavor_parent_class)->finalize(object);
}

static void
gsgf_general_flavor_class_init(GSGFGeneralFlavorClass *klass)
{
        GObjectClass* object_class = G_OBJECT_CLASS (klass);

        g_type_class_add_private(klass, sizeof(GSGFGeneralFlavorPrivate));

        object_class->finalize = gsgf_general_flavor_finalize;
}

/**
 * gsgf_general_flavor_new:
 * @id: ID for the general_flavor.
 * @name: Human-readable name.
 *
 * Creates an empty #GSGFGeneralFlavor.
 *
 * Returns: The new #GSGFGeneralFlavor.
 */
GSGFGeneralFlavor *
gsgf_general_flavor_new (const gchar *id, const gchar *name, const GSGFGeneralFlavor *parent)
{
        GSGFGeneralFlavor *self = g_object_new(GSGF_TYPE_GENERAL_FLAVOR, NULL);

        self->priv->id = id;
        self->priv->name = name;
        self->priv->handlers = g_hash_table_new_full(g_str_hash, g_str_equal,
                                                     g_free, g_object_unref);
        self->priv->parent = parent;

        return self;
}
