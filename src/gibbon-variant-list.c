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
 * SECTION:gibbon-variant-list
 * @short_description: List store for move variants.
 *
 * Since: 0.2.0
 *
 * A GibbonVariantList stores all possible moves for a certain position
 * and the results of the analysis.
 */

#include <glib.h>
#include <glib/gi18n.h>

#include "gibbon-variant-list.h"

typedef struct _GibbonVariantListPrivate GibbonVariantListPrivate;
struct _GibbonVariantListPrivate {
        GtkListStore *store;
};

#define GIBBON_VARIANT_LIST_PRIVATE(obj) (G_TYPE_INSTANCE_GET_PRIVATE ((obj), \
        GIBBON_TYPE_VARIANT_LIST, GibbonVariantListPrivate))

G_DEFINE_TYPE (GibbonVariantList, gibbon_variant_list, G_TYPE_OBJECT)

static void 
gibbon_variant_list_init (GibbonVariantList *self)
{
        self->priv = G_TYPE_INSTANCE_GET_PRIVATE (self,
                GIBBON_TYPE_VARIANT_LIST, GibbonVariantListPrivate);

        self->priv->store = NULL;
}

static void
gibbon_variant_list_finalize (GObject *object)
{
        GibbonVariantList *self = GIBBON_VARIANT_LIST (object);

        if (self->priv->store)
                g_object_unref (self->priv->store);

        G_OBJECT_CLASS (gibbon_variant_list_parent_class)->finalize(object);
}

static void
gibbon_variant_list_class_init (GibbonVariantListClass *klass)
{
        GObjectClass *object_class = G_OBJECT_CLASS (klass);
        
        g_type_class_add_private (klass, sizeof (GibbonVariantListPrivate));

        object_class->finalize = gibbon_variant_list_finalize;
}

/**
 * gibbon_variant_list_new:
 *
 * Creates a new #GibbonVariantList.
 *
 * Returns: The newly created #GibbonVariantList or %NULL in case of failure.
 */
GibbonVariantList *
gibbon_variant_list_new ()
{
        GibbonVariantList *self = g_object_new (GIBBON_TYPE_VARIANT_LIST, NULL);
        GtkListStore *store;

        store = gtk_list_store_new (GIBBON_INVITER_LIST_N_COLUMNS,
                                    G_TYPE_UINT,
                                    G_TYPE_UINT,
                                    G_TYPE_STRING,
                                    G_TYPE_STRING);

        self->priv->store = store;

        return self;
}

GtkListStore *
gibbon_variant_list_get_store (GibbonVariantList *self)
{
        g_return_val_if_fail (self != NULL, NULL);
        g_return_val_if_fail (GIBBON_IS_VARIANT_LIST (self), NULL);

        return self->priv->store;
}
