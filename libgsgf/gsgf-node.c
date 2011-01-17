/*
 * This file is part of Gibbon, a graphical frontend to the First Internet 
 * Backgammon Server FIBS.
 * Copyright (C) 2009-2011 Guido Flohr, http://guido-flohr.net/.
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
 * SECTION:gsgf-node
 * @short_description: An SGF node.
 *
 * A #GSGFNode is a list of#GSGFPropertyElements.
 */

#include <glib.h>
#include <glib/gi18n.h>

#include <libgsgf/gsgf.h>

#include "gsgf-private.h"

typedef struct _GSGFNodePrivate GSGFNodePrivate;
struct _GSGFNodePrivate {
        GHashTable *properties;
        GSGFNode *previous;
};

#define GSGF_NODE_PRIVATE(o) (G_TYPE_INSTANCE_GET_PRIVATE ((o), \
                                      GSGF_TYPE_NODE,           \
                                      GSGFNodePrivate))
G_DEFINE_TYPE (GSGFNode, gsgf_node, G_TYPE_OBJECT)

static void
gsgf_node_init(GSGFNode *self)
{
        self->priv = G_TYPE_INSTANCE_GET_PRIVATE(self,  GSGF_TYPE_NODE,
                                                 GSGFNodePrivate);

        self->priv->properties = g_hash_table_new_full(g_str_hash, g_str_equal,
                                                       g_free, g_object_unref);
        self->priv->previous = NULL;
}

static void
gsgf_node_finalize(GObject *object)
{
        GSGFNode *self = GSGF_NODE(object);

        if (self->priv->properties)
                g_hash_table_destroy(self->priv->properties);
        self->priv->properties = NULL;

        G_OBJECT_CLASS (gsgf_node_parent_class)->finalize(object);
}

static void
gsgf_node_class_init(GSGFNodeClass *klass)
{
        GObjectClass* object_class = G_OBJECT_CLASS (klass);

        g_type_class_add_private(klass, sizeof(GSGFNodePrivate));

        object_class->finalize = gsgf_node_finalize;
}

GSGFNode *
_gsgf_node_new(GSGFNode *previous)
{
        GSGFNode *self;

        g_return_val_if_fail(previous == NULL || GSGF_IS_NODE(previous), NULL);

        self = g_object_new(GSGF_TYPE_NODE, NULL);

        self->priv->previous = previous;

        return self;
}

gboolean
_gsgf_node_write_stream(const GSGFNode *self, GOutputStream *out,
                        gsize *bytes_written, GCancellable *cancellable, GError **error)
{
        gsize written_here;
        GList *keys;
        GList *iter;
        GList *property;

        g_return_val_if_fail(GSGF_IS_NODE(self), FALSE);
        g_return_val_if_fail(G_IS_OUTPUT_STREAM(out), FALSE);

        *bytes_written = 0;

        if (!g_output_stream_write_all(out, ";", 1, &written_here,
                                       cancellable, error)) {
                *bytes_written += written_here;
                return FALSE;
        }

        *bytes_written += written_here;

        /* Sorting is actually not needed.  We trade performance for easier
         * testing here.
         */
        if (self->priv->properties) {
                keys = g_hash_table_get_keys(self->priv->properties);

                iter = g_list_sort(keys, (GCompareFunc) g_strcmp0);

                while (iter) {
                        if (!g_output_stream_write_all(out, iter->data,
                                                       strlen(iter->data), &written_here,
                                                       cancellable, error)) {
                                *bytes_written += written_here;
                                g_list_free(keys);
                                return FALSE;
                        }

                        property = g_hash_table_lookup(self->priv->properties, iter->data);
                        if (!_gsgf_property_write_stream(GSGF_PROPERTY(property), out,
                                                         &written_here, cancellable,
                                                         error)) {
                                *bytes_written += written_here;
                                g_list_free(keys);
                                return FALSE;
                        }

                        *bytes_written += written_here;
                        iter = iter->next;
                }
                g_list_free(keys);
        }

        return TRUE;
}

/**
 * gsgf_node_add_property:
 * @self: a #GSGFNode to add the property to.
 * @id: identifier of the property.
 * @error: a #GError location to store the error occuring, or %NULL to ignore.
 *
 * Add an empty #GSGFProperty as a child. A copy of the %id is used internally;
 * you can safely free resources related to the %id.
 *
 * It is illegal to add a property with an already existing identifier to a 
 * node.
 *
 * Returns: The freshly added #GSGFProperty or %NULL in case of failure.
 */
GSGFProperty *
gsgf_node_add_property(GSGFNode *self, const gchar *id, GError **error)
{
        GSGFProperty *property;
        const gchar *ptr = id;

        g_return_val_if_fail(GSGF_IS_NODE(self), NULL);
        g_return_val_if_fail(id != NULL, NULL);

        if (error)
                *error = NULL;

        while (*ptr) {
                if (*ptr < 'A' || *ptr > 'Z') {
                        g_set_error(error, GSGF_ERROR, GSGF_ERROR_SYNTAX,
                                    _("Only upper case letters are allowed for property identifiers"));
                        return NULL;
                }
                ++ptr;
        }

        property = _gsgf_property_new(id, self);

        g_hash_table_insert(self->priv->properties, g_strdup(id), property);

        return property;
}

/**
 * gsgf_node_get_property:
 * @self: a #GSGFNode.
 * @id: identifier of the property.
 *
 * Get a #GSGFProperty identified by %id.
 *
 * Returns: The #GSGFProperty identified by %id or %NULL.
 */
GSGFProperty *
gsgf_node_get_property(const GSGFNode *self, const gchar *id)
{
        g_return_val_if_fail(GSGF_IS_NODE(self), NULL);
        g_return_val_if_fail(id != NULL, NULL);

        if (!self->priv->properties)
                return NULL;

        return g_hash_table_lookup(self->priv->properties, id);
}

/**
 * gsgf_node_get_property_cooked:
 * @self: a #GSGFNode.
 * @id: identifier of the property.
 *
 * Get a #GSGFCookedValue identified by %id.
 *
 * This is equivalent to retrieving the property first with
 * %gsgf_node_get_property(), and then calling %gsgf_property_get_value()
 * on this #GSGFProperty.
 *
 * Returns: The #GSGFCookedValue identified by %id or %NULL.
 */
GSGFCookedValue *
gsgf_node_get_property_cooked(const GSGFNode *self, const gchar *id)
{
        GSGFProperty *property;

        g_return_val_if_fail(GSGF_IS_NODE(self), NULL);
        g_return_val_if_fail(id != NULL, NULL);

        property = gsgf_node_get_property(self, id);

        if (!property)
                return NULL;

        return gsgf_property_get_value(property);
}

/**
 * gsgf_node_get_property_ids:
 * @self: a #GSGFNode.
 *
 * Return all #GSGFProperty ids stored in this node.  The returned list must be
 * freed with %g_list_free().  The returned list becomes invalid, when you add
 * or properties to the #GSGFNode or remove nodes from it.
 *
 * The data portion of each #GList item points to a #gchar.
 *
 * Returns: The list of #gchar ids.
 */
GList *
gsgf_node_get_property_ids(const GSGFNode *self)
{
        g_return_val_if_fail(GSGF_IS_NODE(self), NULL);

        if (!self->priv->properties)
                return NULL;

        return g_hash_table_get_keys(self->priv->properties);
}

/**
 * gsgf_node_remove_property:
 * @self: a #GSGFNode.
 * @id: identifier of the property.
 *
 * Remove a #GSGFProperty identified by %id and free all resources occupied by it.
 * If there is no such property the function silently returns without error.
 */
void
gsgf_node_remove_property(GSGFNode *self, const gchar *id)
{
        g_return_if_fail(GSGF_IS_NODE(self));
        g_return_if_fail(id != NULL);

        if (!self->priv->properties)
                return;

        (void) g_hash_table_remove(self->priv->properties, id);
}

gboolean
_gsgf_node_apply_flavor(GSGFNode *self, const GSGFFlavor *flavor, GError **error)
{
        GHashTableIter iter;
        gpointer key, value;

        g_return_val_if_fail(GSGF_IS_NODE(self), FALSE);
        g_return_val_if_fail(GSGF_IS_FLAVOR(flavor), FALSE);

        if (error && *error)
                return FALSE;

        g_hash_table_iter_init(&iter, self->priv->properties);
        while (g_hash_table_iter_next(&iter, &key, &value)) {
                if (!_gsgf_property_apply_flavor(GSGF_PROPERTY(value), flavor, error))
                        return FALSE;
        }

        return TRUE;
}

/**
 * gsgf_node_get_previous_node:
 * @self: a #GSGFNode.
 *
 * Get the previous #GSGFNode sibling of this #GSGFNode.
 *
 * Returns: The previous sibling as a #GSGFNode or %NULL for the root node.
 */
GSGFNode *
gsgf_node_get_previous_node(const GSGFNode *self)
{
        g_return_val_if_fail(GSGF_IS_NODE(self), NULL);

        return self->priv->previous;
}
