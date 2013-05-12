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
 * SECTION:gsgf-node
 * @short_description: An SGF node.
 * Since:0.1.0
 *
 * A #GSGFNode is a list of#GSGFProperty elements.
 */

#include <glib.h>
#include <glib/gi18n.h>

#include <libgsgf/gsgf.h>

#include "gsgf-private.h"

typedef struct _GSGFNodePrivate GSGFNodePrivate;
struct _GSGFNodePrivate {
        GHashTable *properties;
        GSGFNode *previous;
        GList *losers;
        GSGFGameTree *parent;
};

#define GSGF_NODE_PRIVATE(o) (G_TYPE_INSTANCE_GET_PRIVATE ((o), \
                                      GSGF_TYPE_NODE,           \
                                      GSGFNodePrivate))

static void gsgf_component_iface_init (GSGFComponentIface *iface);
G_DEFINE_TYPE_WITH_CODE (GSGFNode, gsgf_node, G_TYPE_OBJECT,
                         G_IMPLEMENT_INTERFACE (GSGF_TYPE_COMPONENT,
                                                gsgf_component_iface_init))

static gboolean gsgf_node_write_stream (const GSGFComponent *self,
                                        GOutputStream *out,
                                        gsize *bytes_written,
                                        GCancellable *cancellable,
                                        GError **error);
static gboolean gsgf_node_convert (GSGFComponent *self,
                                   const gchar *charset, GError **error);
static gboolean gsgf_node_cook (GSGFComponent *self, GSGFComponent **culprit,
                                GError **error);

static gint compare_property_ids (gconstpointer a, gconstpointer b);

static void
gsgf_node_init(GSGFNode *self)
{
        self->priv = G_TYPE_INSTANCE_GET_PRIVATE(self,  GSGF_TYPE_NODE,
                                                 GSGFNodePrivate);

        self->priv->properties = NULL;
        self->priv->previous = NULL;
        self->priv->losers = NULL;
        self->priv->parent = NULL;
}

static void
gsgf_node_finalize(GObject *object)
{
        GSGFNode *self = GSGF_NODE(object);

        if (self->priv->properties)
                g_hash_table_destroy(self->priv->properties);
        self->priv->properties = NULL;

        if (self->priv->losers) {
                g_list_foreach (self->priv->losers, (GFunc) g_free, NULL);
                g_list_free (self->priv->losers);
        }
        self->priv->losers = NULL;

        self->priv->parent = NULL;

        G_OBJECT_CLASS (gsgf_node_parent_class)->finalize(object);
}

static void
gsgf_node_class_init(GSGFNodeClass *klass)
{
        GObjectClass* object_class = G_OBJECT_CLASS (klass);

        g_type_class_add_private(klass, sizeof(GSGFNodePrivate));

        object_class->finalize = gsgf_node_finalize;
}

static void
gsgf_component_iface_init (GSGFComponentIface *iface)
{
        iface->write_stream = gsgf_node_write_stream;
        iface->cook = gsgf_node_cook;
        iface->_convert = gsgf_node_convert;
}


GSGFNode *
_gsgf_node_new (GSGFNode *previous, GSGFGameTree *parent)
{
        GSGFNode *self;

        g_return_val_if_fail(previous == NULL || GSGF_IS_NODE(previous), NULL);

        self = g_object_new(GSGF_TYPE_NODE, NULL);

        self->priv->properties = g_hash_table_new_full(g_str_hash, g_str_equal,
                                                       g_free, g_object_unref);
        self->priv->previous = previous;
        self->priv->parent = parent;

        return self;
}

static gboolean
gsgf_node_write_stream (const GSGFComponent *_self, GOutputStream *out,
                        gsize *bytes_written, GCancellable *cancellable,
                        GError **error)
{
        GSGFNode *self;
        gsize written_here;
        GList *keys;
        GList *iter;
        GList *property;
        GList *siblings;
        GSGFNode *root;
        const gchar *intro;
        gsize intro_length;

        *bytes_written = 0;

        gsgf_return_val_if_fail (GSGF_IS_NODE(_self), FALSE, error);
        gsgf_return_val_if_fail (G_IS_OUTPUT_STREAM(out), FALSE, error);

        self = GSGF_NODE (_self);

        siblings = gsgf_game_tree_get_nodes (self->priv->parent);
        root = GSGF_NODE (g_list_nth_data (siblings, 0));
        if (root == self) {
                intro = ";";
                intro_length = 1;
        } else {
                intro = "\n;";
                intro_length = 2;
        }

        if (!g_output_stream_write_all(out, intro, intro_length, &written_here,
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

                iter = g_list_sort(keys, compare_property_ids);

                while (iter) {
                        if (!g_output_stream_write_all(out, iter->data,
                                                       strlen(iter->data), &written_here,
                                                       cancellable, error)) {
                                *bytes_written += written_here;
                                g_list_free(keys);
                                return FALSE;
                        }

                        property = g_hash_table_lookup(self->priv->properties, iter->data);
                        if (!gsgf_component_write_stream (GSGF_COMPONENT (property),
                                                          out, &written_here,
                                                          cancellable,
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
 * Add an empty #GSGFProperty as a child. A copy of the @id is used internally;
 * you can safely free resources related to the @id.
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

        if (error)
                *error = NULL;

        gsgf_return_val_if_fail (GSGF_IS_NODE(self), NULL, error);
        gsgf_return_val_if_fail (id != NULL, NULL, error);

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
 * Get a #GSGFProperty identified by @id.
 *
 * Returns: The #GSGFProperty identified by @id or %NULL.
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
 * gsgf_node_get_property_value:
 * @self: a #GSGFNode.
 * @id: identifier of the property.
 *
 * Get a #GSGFValue identified by @id.
 *
 * This is equivalent to retrieving the property first with
 * gsgf_node_get_property(), and then calling gsgf_property_get_value()
 * on this #GSGFProperty.
 *
 * Returns: The #GSGFValue identified by @id or %NULL.
 */
GSGFValue *
gsgf_node_get_property_value (const GSGFNode *self, const gchar *id)
{
        GSGFProperty *property;

        g_return_val_if_fail(GSGF_IS_NODE(self), NULL);
        g_return_val_if_fail(id != NULL, NULL);

        property = gsgf_node_get_property(self, id);

        if (!property)
                return NULL;

        return gsgf_property_get_value (property);
}

/**
 * gsgf_node_get_property_ids:
 * @self: a #GSGFNode.
 *
 * Return all #GSGFProperty ids stored in this node.  The returned list must be
 * freed with g_list_free().  The returned list becomes invalid, when you add
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
 * Remove a #GSGFProperty identified by @id and free all resources occupied by
 * it.  If there is no such property the function silently returns without
 * error.
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

static gboolean
gsgf_node_cook (GSGFComponent *_self, GSGFComponent **culprit, GError **error)
{
        GHashTableIter iter;
        gpointer key, value;
        GList *loser;
        GSGFNode *self;
        GSGFComponentIface *iface;

        if (error && *error)
                return FALSE;

        gsgf_return_val_if_fail (GSGF_IS_NODE (_self), FALSE, error);

        self = GSGF_NODE (_self);

        g_hash_table_iter_init (&iter, self->priv->properties);
        while (g_hash_table_iter_next(&iter, &key, &value)) {
                iface = GSGF_COMPONENT_GET_IFACE (value);
                if (!iface->cook (GSGF_COMPONENT (value), culprit, error))
                        return FALSE;
        }

        /* Properties cannot be removed while iterating over the hash
         * table since this would invalidate the iterator.
         */
        loser = self->priv->losers;
        while (loser) {
                gsgf_node_remove_property (self, loser->data);
                loser = loser->next;
        }

        if (self->priv->losers) {
                g_list_foreach (self->priv->losers, (GFunc) g_free, NULL);
                g_list_free (self->priv->losers);
        }
        self->priv->losers = NULL;

        return TRUE;
}

/*< private >*/
void
_gsgf_node_mark_loser_property (GSGFNode *self, const gchar *id)
{
        g_return_if_fail (GSGF_IS_NODE (self));

        self->priv->losers = g_list_append (self->priv->losers, g_strdup (id));
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

/**
 * gsgf_node_get_flavor:
 * @self: a #GSGFNode.
 *
 * Get the #GSGFFlavor of this #GSGFNode.
 *
 * Returns: The #GSGFFlavor of @self or %NULL if not yet cooked.
 */
const GSGFFlavor *
gsgf_node_get_flavor (const GSGFNode *self)
{
        g_return_val_if_fail (GSGF_IS_NODE (self), NULL);

        return gsgf_game_tree_get_flavor (self->priv->parent);
}

/**
 * gsgf_node_get_game_tree:
 * @self: a #GSGFNode.
 *
 * Get the #GSGFGameTree that this #GSGFNode belongs to.
 *
 * Returns: The parent #GSGFGameTree of @self.
 */
GSGFGameTree *
gsgf_node_get_game_tree (const GSGFNode *self)
{
        g_return_val_if_fail (GSGF_IS_NODE (self), NULL);

        return self->priv->parent;
}

static gboolean
gsgf_node_convert (GSGFComponent *_self, const gchar *charset, GError **error)
{
        GSGFNode *self;
        GList *ids;
        GList *id_item;
        gchar *id;
        GSGFProperty *property;
        GSGFComponentIface *iface;

        if (error)
                *error = NULL;

        self = GSGF_NODE (_self);

        ids = g_hash_table_get_keys (self->priv->properties);
        for (id_item = ids; id_item; id_item = id_item->next) {
                id = (gchar *) id_item->data;
                property = g_hash_table_lookup (self->priv->properties, id);
                iface = GSGF_COMPONENT_GET_IFACE (property);
                if (!iface->_convert (GSGF_COMPONENT (property),
                                      charset, error)) {
                        g_list_free (ids);
                        return FALSE;
                }
        }
        g_list_free (ids);

        return TRUE;
}

/**
 * gsgf_node_set_property:
 * @self: The #GSGFProperty.
 * @id: Name of the property.
 * @value: The value to set.
 * @error: a #GError location to store the error occuring, or %NULL to ignore.
 *
 * Set a property.  The property is created if it does not exist.  The
 * value is a #GSGFRaw.
 *
 * Returns: %TRUE for success, %FALSE for failure.
 */
gboolean
gsgf_node_set_property (GSGFNode *self,
                        const gchar *id, GSGFValue *value,
                        GError **error)
{
        GSGFProperty *property;

        gsgf_return_val_if_fail (GSGF_IS_NODE (self), FALSE, error);
        gsgf_return_val_if_fail (GSGF_IS_VALUE (value), FALSE, error);

        property = gsgf_node_get_property (self, id);
        if (!property) {
                property = gsgf_node_add_property (self, id, error);
                if (!property)
                        return FALSE;
        }

        if (!gsgf_property_set_value (property, value, error))
                return FALSE;

        return TRUE;
}

/*
 * GNU Backgammon expects the FF and GM attributes at the head of the list. :-(
 * We also write the CA and AP properties in the order that gnubg expects it.
 *
 * TODO! Write a lookup table and use that to compare values.
 */
static gint
compare_property_ids (gconstpointer _a, gconstpointer _b)
{
        const gchar *a = (const gchar *) _a;
        const gchar *b = (const gchar *) _b;
        gint retval = g_strcmp0 (a, b);

        if (!retval)
                return 0;

        if (0 == g_strcmp0 (a, "FF"))
                return -1;
        if (0 == g_strcmp0 (b, "FF"))
                return 1;
        if (0 == g_strcmp0 (a, "GM"))
                return -1;
        if (0 == g_strcmp0 (b, "GM"))
                return 1;
        if (0 == g_strcmp0 (a, "CA"))
                return -1;
        if (0 == g_strcmp0 (b, "CA"))
                return 1;
        if (0 == g_strcmp0 (a, "AP"))
                return -1;
        if (0 == g_strcmp0 (b, "AP"))
                return 1;

        /*
         * Gnubg needs those as well.
         */
        if (0 == g_strcmp0 (a, "PL"))
                return -1;
        if (0 == g_strcmp0 (b, "PL"))
                return 1;
        if (0 == g_strcmp0 (a, "AE"))
                return -1;
        if (0 == g_strcmp0 (b, "AE"))
                return 1;

        return retval;
}
