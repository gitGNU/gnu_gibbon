/*
 * This file is part of Gibbon, a graphical frontend to the First Internet 
 * Backgammon Server FIBS.
 * Copyright (C) 2009-2012 Guido Flohr, http://guido-flohr.net/.
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
 * SECTION:gsgf-list-of
 * @short_description: ListOf data in SGF files.
 *
 * A #GSGFListOf is a list of #GSGFCookedValue objects.
 *
 * The SGF specification has another type elist.  This is a list that can
 * possibly be empty.  In libgsgf, there is no corresponding type for this.
 * Whether a list can be empty or not is considered to be part of the
 * semantics.
 *
 * You can detect empty lists by checking the item type with
 * gsgf_list_of_get_item_type().  If it is  #GSGF_TYPE_EMPTY the list is
 * empty.
 */

#include <glib.h>
#include <glib/gi18n.h>

#include <libgsgf/gsgf.h>
#include "gsgf-private.h"

typedef struct _GSGFListOfPrivate GSGFListOfPrivate;
struct _GSGFListOfPrivate {
        const GSGFFlavor *flavor;
        GType type;

        GList *items;
};

#define GSGF_LIST_OF_PRIVATE(o) (G_TYPE_INSTANCE_GET_PRIVATE ((o), \
                                      GSGF_TYPE_LIST_OF,           \
                                      GSGFListOfPrivate))

G_DEFINE_TYPE(GSGFListOf, gsgf_list_of, GSGF_TYPE_COOKED_VALUE)

static gboolean gsgf_list_of_write_stream(const GSGFValue *self,
                                          GOutputStream *out,
                                          gsize *bytes_written,
                                          GCancellable *cancellable,
                                          GError **error);

static void
gsgf_list_of_init(GSGFListOf *self)
{
        self->priv = G_TYPE_INSTANCE_GET_PRIVATE (self,
                        GSGF_TYPE_LIST_OF,
                        GSGFListOfPrivate);

        self->priv->flavor = NULL;
        self->priv->type = G_TYPE_INVALID;
        self->priv->items = NULL;
}

static void
gsgf_list_of_finalize(GObject *object)
{
        GSGFListOf *self = GSGF_LIST_OF(object);

        if (self->priv->items) {
                g_list_foreach(self->priv->items, (GFunc) g_object_unref, NULL);
                g_list_free(self->priv->items);
        }
        self->priv->flavor = NULL;
        self->priv->type = G_TYPE_INVALID;

        G_OBJECT_CLASS (gsgf_list_of_parent_class)->finalize(object);
}

static void
gsgf_list_of_class_init(GSGFListOfClass *klass)
{
        GObjectClass* object_class = G_OBJECT_CLASS(klass);
        GSGFValueClass *gsgf_value_class = GSGF_VALUE_CLASS(klass);

        g_type_class_add_private(klass, sizeof(GSGFListOfPrivate));

        gsgf_value_class->write_stream = gsgf_list_of_write_stream;

        object_class->finalize = gsgf_list_of_finalize;
}

/**
 * gsgf_list_of_new:
 * @type: Type of items.
 * @flavor: #GSGFFlavor that this list belongs to.
 *
 * Creates a new #GSGFListOf that is prepared for storing items of type
 * @type.
 *
 * Returns: The new #GSGFListOf.
 */
GSGFListOf *
gsgf_list_of_new (GType type, const GSGFFlavor *flavor)
{
        GSGFListOf *self;

        self = g_object_new(GSGF_TYPE_LIST_OF, NULL);

        self->priv->type = type;
        self->priv->flavor = flavor;

        return self;
}

/**
 * gsgf_list_of_get_nth_item:
 * @self: The #GSGFListOf object.
 * @i: Position in the property list.
 *
 * Retrieve the value stored at position @i.
 *
 * Returns: The value stored at position @i or %NULL.
 */
GSGFCookedValue *
gsgf_list_of_get_nth_item(const GSGFListOf *self, gsize i)
{
        g_return_val_if_fail(GSGF_IS_LIST_OF(self), NULL);

        return GSGF_COOKED_VALUE(g_list_nth_data(self->priv->items, i));
}

static gboolean
gsgf_list_of_write_stream (const GSGFValue *_self,
                           GOutputStream *out, gsize *bytes_written,
                           GCancellable *cancellable, GError **error)
{
        gsize written_here;
        GList *iter;
        GSGFValue *value;
        GSGFListOf *self = GSGF_LIST_OF(_self);
        const GSGFFlavor *flavor = self->priv->flavor;

        *bytes_written = 0;

        iter = self->priv->items;

        if (!iter) {
                g_set_error(error, GSGF_ERROR, GSGF_ERROR_EMPTY_PROPERTY,
                            _("Attempt to write empty property"));
                return FALSE;
        }

        if (iter && GSGF_IS_POINT(iter->data)
            && GSGF_FLAVOR_GET_CLASS(flavor)->write_compressed_list) {
                return GSGF_FLAVOR_GET_CLASS(flavor)->write_compressed_list (
                                self->priv->flavor, self,
                                out, bytes_written, cancellable, error);
        }

        while (iter) {
                value = GSGF_VALUE (iter->data);
                if (!gsgf_value_write_stream(value, out, &written_here,
                                             cancellable, error)) {
                        *bytes_written += written_here;
                        return FALSE;
                }
                *bytes_written += written_here;

                iter = iter->next;

                if (iter) {
                        if (!g_output_stream_write_all(out, "][", 2,
                                                       &written_here,
                                                       cancellable, error)) {
                                *bytes_written += written_here;
                                return FALSE;
                        }
                        *bytes_written += written_here;
                }
        }

        return TRUE;
}

/**
 * gsgf_list_of_get_number_of_items:
 * @self: The #GSGFListOf object.
 *
 * Get the number of items.
 *
 * Returns: The number of values stored.
 */
gsize
gsgf_list_of_get_number_of_items(const GSGFListOf *self)
{
        g_return_val_if_fail(GSGF_IS_LIST_OF(self), 0);

        return g_list_length(self->priv->items);
}

/**
 * gsgf_list_of_get_item_type:
 * @self: The #GSGFListOf object.
 *
 * Get the type of items stored in the list.  This is the type that is expected
 * when you add items with gsgf_list_of_append().
 *
 * Returns: The #GType of the items stored.
 */
GType
gsgf_list_of_get_item_type(const GSGFListOf *self)
{
        g_return_val_if_fail(GSGF_IS_LIST_OF(self), G_TYPE_INVALID);

        return self->priv->type;
}

/**
 * gsgf_list_of_append:
 * @self: The #GSGFListOf object.
 * @item: The item to store.
 * @error: Optional #GError location or %NULL to ignore.
 *
 * Append a #GSGFCookedValue to a #GSGFListOf.
 *
 * Returns: %TRUE for success, %FALSE for failure.
 */
gboolean
gsgf_list_of_append (GSGFListOf *self, GSGFCookedValue *item,
                     GError **error)
{
        gsgf_return_val_if_fail (GSGF_IS_LIST_OF(self), FALSE, error);
        gsgf_return_val_if_fail (G_OBJECT_TYPE (G_OBJECT (item))
                                 == self->priv->type, FALSE, error);

        self->priv->items = g_list_append(self->priv->items, item);

        return TRUE;
}
