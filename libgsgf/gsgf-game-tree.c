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
 * SECTION:gsgf-game-tree
 * @short_description: An SGF game tree.
 *
 * A #GSGFGameTree represents a single game in a #GSGFCollection.
 */

#include <glib.h>
#include <glib/gi18n.h>

#include <libgsgf/gsgf.h>

#include "gsgf-private.h"

struct _GSGFGameTreePrivate {
        GSGFGameTree *parent;

        GList *nodes;
        GList *children;
};

#define GSGF_GAME_TREE_PRIVATE(o) (G_TYPE_INSTANCE_GET_PRIVATE ((o), \
                                      GSGF_TYPE_GAME_TREE,           \
                                      GSGFGameTreePrivate))
G_DEFINE_TYPE (GSGFGameTree, gsgf_game_tree, G_TYPE_OBJECT)

static void
gsgf_game_tree_init(GSGFGameTree *self)
{
        self->priv = G_TYPE_INSTANCE_GET_PRIVATE(self,  GSGF_TYPE_GAME_TREE,
                                                 GSGFGameTreePrivate);

        self->priv->parent = NULL;
        self->priv->nodes = NULL;
        self->priv->children = NULL;
}

static void
gsgf_game_tree_finalize(GObject *object)
{
        GSGFGameTree *self = GSGF_GAME_TREE(object);

        if (self->priv->nodes) {
                g_list_foreach(self->priv->nodes, (GFunc) g_object_unref, NULL);
                g_list_free(self->priv->nodes);
        }
        self->priv->nodes = NULL;

        if (self->priv->children) {
                g_list_foreach(self->priv->children, (GFunc) g_object_unref, NULL);
                g_list_free(self->priv->children);
        }
        self->priv->children = NULL;

        G_OBJECT_CLASS (gsgf_game_tree_parent_class)->finalize(object);
}

static void
gsgf_game_tree_class_init(GSGFGameTreeClass *klass)
{
        GObjectClass* object_class = G_OBJECT_CLASS (klass);

        g_type_class_add_private(klass, sizeof(GSGFGameTreePrivate));

        object_class->finalize = gsgf_game_tree_finalize;
}

GSGFGameTree *
_gsgf_game_tree_new()
{
        GSGFGameTree *self = g_object_new(GSGF_TYPE_GAME_TREE, NULL);

        return self;
}

/**
 * gsgf_game_tree_add_child:
 *
 * Add an empty #GSGFGameTree as a child.  The function cannot fail.
 *
 * Returns: the freshly added child #GSGFGameTree.
 */
GSGFGameTree *
gsgf_game_tree_add_child(GSGFGameTree *self)
{
        GSGFGameTree *child = _gsgf_game_tree_new();

        self->priv->children = g_list_append(self->priv->children, child);

        child->priv->parent = self;

        return child;
}

/**
 * gsgf_game_tree_add_node:
 *
 * Add an empty #GSGFNode as a child.  The function cannot fail.
 *
 * Returns: the freshly added child #GSGFNode.
 */
GSGFNode *
gsgf_game_tree_add_node(GSGFGameTree *self)
{
        GSGFNode *node = _gsgf_node_new();

        self->priv->nodes = g_list_append(self->priv->nodes, node);

        return node;
}

/**
 * gsgf_game_tree_get_parent:
 *
 * Return the parent #GSGFGameTree of this #GSGFGameTree or NULL if it
 * is the root of the collection.
 */
GSGFGameTree *
gsgf_game_tree_get_parent(const GSGFGameTree *self)
{
        return self->priv->parent;
}

gboolean
_gsgf_game_tree_write_stream(const GSGFGameTree *self,
                             GOutputStream *out, gsize *bytes_written,
                             GCancellable *cancellable, GError **error)
{
        gsize written_here;
        GList *iter;
        GSGFNode *root;
        GSGFProperty *ap_property;
        gchar *version_string;

        /* Force our version.  */
        version_string = g_strdup_printf("libgsgf:%s", VERSION);
        root = GSGF_NODE(self->priv->nodes->data);
        gsgf_node_remove_property(root, "AP");
        ap_property = gsgf_node_add_property(root, "AP", NULL);
        _gsgf_property_add_value(ap_property, version_string);
        g_free(version_string);

        *bytes_written = 0;

        if (!g_output_stream_write_all(out, "(", 1, &written_here,
                                       cancellable, error)) {
                *bytes_written += written_here;
                return FALSE;
        }

        *bytes_written += written_here;

        iter = self->priv->nodes;
        while (iter) {
                if (!_gsgf_node_write_stream(GSGF_NODE(iter->data), out, &written_here,
                                             cancellable, error)) {
                        *bytes_written += written_here;
                        return FALSE;
                }

                *bytes_written += written_here;

                iter = iter->next;
        }

        iter = self->priv->children;
        while (iter) {
                if (!_gsgf_game_tree_write_stream(GSGF_GAME_TREE(iter->data), out,
                                                  &written_here, cancellable, error)) {
                        *bytes_written += written_here;
                        return FALSE;
                }

                *bytes_written += written_here;

                iter = iter->next;
        }

        if (!g_output_stream_write_all(out, ")", 1, &written_here,
                                       cancellable, error)) {
                *bytes_written += written_here;
                return FALSE;
        }

        *bytes_written += written_here;

        return TRUE;
}

gboolean
_gsgf_game_tree_convert(GSGFGameTree *self, GError **error)
{
        GSGFNode *root;
        GSGFProperty *ca_property;
        GSGFRaw *value;
        gchar *charset = "ISO-8859-1";
        GList *ids;
        GList *id_item;
        gchar *id;
        GList *nodes;
        GSGFNode *node;
        GSGFProperty *property;
        gboolean free_charset = FALSE;
        GList *child;

        if (error)
                *error = NULL;

        /* It is debatable, whether this should be an error.  But when
         * there are no nodes, there's nothing to convert.  We leave
         * complaints about that to the serializer.
         */
        if (!self->priv->nodes)
                return TRUE;

        root = GSGF_NODE(self->priv->nodes->data);
        ca_property = gsgf_node_get_property(root, "CA");
        if (ca_property) {
                value = GSGF_RAW(gsgf_property_get_value(ca_property));
                charset = gsgf_util_read_simpletext(gsgf_raw_get_value(value, 0),
                                                    NULL, 0);
                free_charset = TRUE;
        }

        if (g_ascii_strcasecmp(charset, "UTF-8")) {
                if (ca_property)
                        gsgf_node_remove_property(root, "CA");

                ca_property = gsgf_node_add_property(root, "CA", NULL);
                _gsgf_property_add_value(ca_property, "UTF-8");

                for (nodes = self->priv->nodes; nodes; nodes = nodes->next) {
                        node = GSGF_NODE(nodes->data);
                        ids = gsgf_node_get_property_ids(node);
                        for (id_item = ids; id_item; id_item = id_item->next) {
                                id = (gchar *) id_item->data;
                                property = gsgf_node_get_property(node, id);
                                if (!_gsgf_property_convert(property, charset, error)) {
                                        if (free_charset)
                                                g_free(charset);
                                        return FALSE;
                                }
                        }
                        g_list_free(ids);
                }
                /* The SGF specification is a little ambiguous here? Do child
                 * game trees inherit the CA property?  Short of any hint in the
                 * specs we assume they do not.
                 */
                for (child = self->priv->children; child; child = child->next) {
                        if (!_gsgf_game_tree_convert(GSGF_GAME_TREE(child->data),
                                                     error)) {
                                if (free_charset)
                                        g_free(charset);
                        }
                }
        }

        if (free_charset)
                g_free(charset);

        return TRUE;
}

gboolean
_gsgf_game_tree_apply_flavor(GSGFGameTree *self, GError **error)
{
        GSGFNode *node;
        GSGFProperty *gm_property;
        GSGFRaw *raw;
        const gchar *flavor_id = "1";
        GSGFFlavor *flavor;
        GList *iter;

        if (error)
                error = NULL;

        node = GSGF_NODE(self->priv->nodes->data);
        gm_property = gsgf_node_get_property(node, "GM");
        if (gm_property) {
                raw = GSGF_RAW(gsgf_property_get_value(gm_property));
                flavor_id = gsgf_raw_get_value(raw, 0);
        }

        flavor = _libgsgf_get_flavor(flavor_id);

        for (iter = self->priv->nodes; iter; iter = iter->next)
                if (!_gsgf_node_apply_flavor(GSGF_NODE(iter->data), flavor, error))
                        return FALSE;

        return TRUE;
}

/**
 * gsgf_game_tree_get_nodes
 * @self: the #GSGFGameTree.
 *
 * Get the list of #GSGFNode objects stored in a #GSGFGameTree.
 *
 * This list is not a copy.  You should not free it.  The list becomes invalid,
 * when you add or remove nodes.
 *
 * Returns: Returns a #GList of #GSGFNode objects..
 **/
GList *
gsgf_game_tree_get_nodes(const GSGFGameTree *self)
{
        return self->priv->nodes;
}
