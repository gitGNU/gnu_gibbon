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
 * SECTION:gsgf-game-tree
 * @short_description: An SGF game tree.
 *
 * A #GSGFGameTree represents a single game in a #GSGFCollection.
 */

#include <glib.h>
#include <glib/gi18n.h>

#include <libgsgf/gsgf.h>

#include "gsgf-private.h"

typedef struct _GSGFGameTreePrivate GSGFGameTreePrivate;
struct _GSGFGameTreePrivate {
        GSGFGameTree *parent;

        const GSGFFlavor *flavor;
        GList *nodes;
        GList *last_node;
        GList *children;

        gchar *app;
        gchar *version;
};

#define GSGF_GAME_TREE_PRIVATE(o) (G_TYPE_INSTANCE_GET_PRIVATE ((o), \
                                      GSGF_TYPE_GAME_TREE,           \
                                      GSGFGameTreePrivate))

static void gsgf_component_iface_init (GSGFComponentIface *iface);
G_DEFINE_TYPE_WITH_CODE (GSGFGameTree, gsgf_game_tree, G_TYPE_OBJECT,
                         G_IMPLEMENT_INTERFACE (GSGF_TYPE_COMPONENT,
                                                gsgf_component_iface_init))

static gboolean gsgf_game_tree_write_stream (const GSGFComponent *self,
                                             GOutputStream *out,
                                             gsize *bytes_written,
                                             GCancellable *cancellable,
                                             GError **error);
static gboolean gsgf_game_tree_convert (GSGFComponent *self,
                                        const gchar *charset, GError **error);
static gboolean gsgf_game_tree_cook (GSGFComponent *self,
                                     GSGFComponent **culprit,
                                     GError **error);

static void
gsgf_game_tree_init(GSGFGameTree *self)
{
        self->priv = G_TYPE_INSTANCE_GET_PRIVATE(self,  GSGF_TYPE_GAME_TREE,
                                                 GSGFGameTreePrivate);

        self->priv->flavor = NULL;
        self->priv->parent = NULL;
        self->priv->nodes = NULL;
        self->priv->last_node = NULL;
        self->priv->children = NULL;

        self->priv->app = NULL;
        self->priv->version = NULL;
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

        self->priv->flavor = NULL;

        if (self->priv->app)
                g_free (self->priv->app);
        self->priv->app = NULL;

        if (self->priv->version)
                g_free (self->priv->version);

        G_OBJECT_CLASS (gsgf_game_tree_parent_class)->finalize(object);
}

static void
gsgf_game_tree_class_init(GSGFGameTreeClass *klass)
{
        GObjectClass* object_class = G_OBJECT_CLASS (klass);

        g_type_class_add_private(klass, sizeof(GSGFGameTreePrivate));

        object_class->finalize = gsgf_game_tree_finalize;
}

static void
gsgf_component_iface_init (GSGFComponentIface *iface)
{
        iface->write_stream = gsgf_game_tree_write_stream;
        iface->cook = gsgf_game_tree_cook;
        iface->_convert = gsgf_game_tree_convert;
}

GSGFGameTree *
_gsgf_game_tree_new (const GSGFFlavor *flavor)
{
        GSGFGameTree *self = g_object_new(GSGF_TYPE_GAME_TREE, NULL);

        self->priv->flavor = flavor;

        return self;
}

/**
 * gsgf_game_tree_add_child:
 * @self: The #GSGFGameTree to add.
 *
 * Add an empty #GSGFGameTree as a child.  The function cannot fail.
 *
 * Returns: the freshly added child #GSGFGameTree.
 */
GSGFGameTree *
gsgf_game_tree_add_child (GSGFGameTree *self)
{
        GSGFGameTree *child;

        g_return_val_if_fail (GSGF_IS_GAME_TREE (self), NULL);

        child = _gsgf_game_tree_new (self->priv->flavor);

        self->priv->children = g_list_append (self->priv->children, child);

        child->priv->parent = self;

        return child;
}

/**
 * gsgf_game_tree_add_node:
 * @self: The #GSGFNode.
 *
 * Add an empty #GSGFNode as a child.  The function cannot fail.
 *
 * Returns: the freshly added child #GSGFNode.
 */
GSGFNode *
gsgf_game_tree_add_node(GSGFGameTree *self)
{
        GSGFNode *previous_node;
        GSGFNode *node;

        g_return_val_if_fail(GSGF_IS_GAME_TREE(self), NULL);

        previous_node = self->priv->last_node
                        ? self->priv->last_node->data : NULL;

        node = _gsgf_node_new (previous_node, self);

        self->priv->nodes = g_list_append (self->priv->nodes, node);
        self->priv->last_node = g_list_last (self->priv->nodes);

        return node;
}

/**
 * gsgf_game_tree_get_parent:
 * @self: The #GSGFGameTree.
 *
 * Get the parent #GSGFGameTree of this #GSGFGameTree or NULL if it
 * is the root of the collection.
 *
 * Returns: The parent #GSGFGameTree or %NULL if there is none.
 */
GSGFGameTree *
gsgf_game_tree_get_parent(const GSGFGameTree *self)
{
        g_return_val_if_fail(GSGF_IS_GAME_TREE(self), NULL);

        return self->priv->parent;
}

static gboolean
gsgf_game_tree_write_stream (const GSGFComponent *_self,
                             GOutputStream *out, gsize *bytes_written,
                             GCancellable *cancellable, GError **error)
{
        GSGFGameTree *self;
        gsize written_here;
        GList *iter;
        GSGFNode *root;
        GSGFProperty *my_property;
        GSGFValue *my_value;
        gchar *app;
        gchar *version;
        guint game_id;

        gsgf_return_val_if_fail (bytes_written, FALSE, error);

        *bytes_written = 0;

        gsgf_return_val_if_fail (GSGF_IS_GAME_TREE (_self), FALSE, error);
        gsgf_return_val_if_fail (G_IS_OUTPUT_STREAM (out), FALSE, error);

        self = GSGF_GAME_TREE (_self);

        if (!self->priv->nodes) {
                g_set_error(error, GSGF_ERROR, GSGF_ERROR_EMPTY_COLLECTION,
                            _("Attempt to write an empty collection"));
                return FALSE;
        }

        if (!self->priv->parent) {
                if (self->priv->app && self->priv->version) {
                        app = g_strdup_printf ("%s (libgsgf)", self->priv->app);
                        version = g_strdup_printf ("%s (%s)",
                                self->priv->version, VERSION);
                } else {
                        app = g_strdup ("libgsgf");
                        version = g_strdup (VERSION);
                }

                my_value = GSGF_VALUE (gsgf_compose_new (
                        GSGF_COOKED_VALUE (gsgf_simple_text_new (app)),
                        GSGF_COOKED_VALUE (gsgf_simple_text_new (version)),
                        NULL));
                g_free (app);
                g_free (version);
                root = GSGF_NODE (self->priv->nodes->data);
                gsgf_node_remove_property (root, "AP");
                my_property = gsgf_node_add_property (root, "AP", error);
                if (!my_property) {
                        g_object_unref (my_value);
                        return FALSE;
                }

                if (!gsgf_property_set_value (my_property, my_value, error)) {
                        g_object_unref (my_value);
                        return FALSE;
                }

                my_value = GSGF_VALUE (gsgf_simple_text_new ("UTF-8"));
                gsgf_node_remove_property (root, "CA");
                my_property = gsgf_node_add_property (root, "CA", error);
                if (!my_property) {
                        g_object_unref (my_value);
                        return FALSE;
                }

                if (!gsgf_property_set_value (my_property, my_value, error)) {
                        g_object_unref (my_value);
                        return FALSE;
                }

                my_value = GSGF_VALUE (gsgf_number_new (4));
                gsgf_node_remove_property (root, "FF");
                my_property = gsgf_node_add_property (root, "FF", error);
                if (!my_property) {
                        g_object_unref (my_value);
                        return FALSE;
                }

                if (!gsgf_property_set_value (my_property, my_value, error)) {
                        g_object_unref (my_value);
                        return FALSE;
                }

                if (self->priv->flavor) {
                        game_id = gsgf_flavor_get_game_id (self->priv->flavor,
                                                           error);
                        if (!game_id)
                                return FALSE;
                        my_value = GSGF_VALUE (gsgf_number_new (game_id));
                        gsgf_node_remove_property (root, "GM");
                        my_property = gsgf_node_add_property (root, "GM", error);
                        if (!my_property) {
                                g_object_unref (my_value);
                                return FALSE;
                        }

                        if (!gsgf_property_set_value (my_property, my_value,
                                                      error)) {
                                g_object_unref (my_value);
                                return FALSE;
                        }
                }
        }

        if (!g_output_stream_write_all(out, "(", 1, &written_here,
                                       cancellable, error)) {
                *bytes_written += written_here;
                return FALSE;
        }

        *bytes_written += written_here;

        iter = self->priv->nodes;
        while (iter) {
                if (!gsgf_component_write_stream (GSGF_COMPONENT (iter->data),
                                                  out, &written_here,
                                                  cancellable, error)) {
                        *bytes_written += written_here;
                        return FALSE;
                }

                *bytes_written += written_here;

                iter = iter->next;
        }

        iter = self->priv->children;
        while (iter) {
                if (!gsgf_game_tree_write_stream (GSGF_COMPONENT (iter->data),
                                                  out, &written_here,
                                                  cancellable, error)) {
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

static gboolean
gsgf_game_tree_convert (GSGFComponent *_self, const gchar *_charset,
                        GError **error)
{
        GSGFNode *root;
        GSGFProperty *ca_property;
        GSGFRaw *value;
        GList *nodes;
        GSGFNode *node;
        GList *child;
        GSGFComponentIface *iface;
        GSGFGameTree *self;
        gchar *charset;

        if (error)
                *error = NULL;

        self = GSGF_GAME_TREE (_self);

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
                charset = gsgf_util_read_simple_text(gsgf_raw_get_value(value, 0),
                                                    NULL, 0);
        } else {
                charset = g_strdup (_charset);
        }

        if (g_ascii_strcasecmp(charset, "UTF-8")) {
                if (ca_property)
                        gsgf_node_remove_property(root, "CA");

                if (!self->priv->parent) {
                        ca_property = gsgf_node_add_property(root, "CA", NULL);
                        _gsgf_property_add_value(ca_property, "UTF-8");
                }
                for (nodes = self->priv->nodes; nodes; nodes = nodes->next) {
                        node = GSGF_NODE (nodes->data);
                        iface = GSGF_COMPONENT_GET_IFACE (node);
                        if (!iface->_convert (GSGF_COMPONENT (node),
                                              charset, error)) {
                                g_free (charset);
                                return FALSE;
                        }
                }

                /* The SGF specification is a little ambiguous here? Do child
                 * game trees inherit the CA property?  Short of any hint in
                 * the specs we assume they do not.
                 */
                for (child = self->priv->children; child; child = child->next) {
                        if (!gsgf_game_tree_convert (GSGF_COMPONENT (child
                                                                     ->data),
                                                     charset, error)) {
                                g_free (charset);
                                return FALSE;
                        }
                }
        }

        g_free (charset);

        return TRUE;
}

static gboolean
gsgf_game_tree_cook (GSGFComponent *_self, GSGFComponent **culprit,
                     GError **error)
{
        GSGFNode *node;
        GSGFProperty *gm_property;
        GSGFRaw *raw;
        const gchar *flavor_id = "1";
        GList *iter;
        GSGFGameTree *self = GSGF_GAME_TREE (_self);
        GSGFComponentIface *iface;

        if (error && *error)
                return FALSE;

        if (!GSGF_IS_GAME_TREE (_self)) {
                g_set_error (error, GSGF_ERROR, GSGF_ERROR_USAGE_ERROR,
                             _("Method cook() called on something that is"
                               " not a GSGFComponent."));
                if (culprit)
                        *culprit = _self;
                g_return_val_if_fail (GSGF_IS_GAME_TREE (_self), FALSE);
        }

        node = GSGF_NODE(self->priv->nodes->data);
        gm_property = gsgf_node_get_property(node, "GM");
        if (gm_property) {
                raw = GSGF_RAW(gsgf_property_get_value(gm_property));
                flavor_id = gsgf_raw_get_value(raw, 0);
        }

        self->priv->flavor = _libgsgf_get_flavor (flavor_id);

        for (iter = self->priv->nodes; iter; iter = iter->next) {
                iface = GSGF_COMPONENT_GET_IFACE (iter->data);
                if (!iface->cook (GSGF_COMPONENT (iter->data), culprit,
                                  error))
                        return FALSE;
        }

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
        g_return_val_if_fail(GSGF_IS_GAME_TREE(self), NULL);

        return self->priv->nodes;
}


/**
 * gsgf_game_tree_get_last_node
 * @self: the #GSGFGameTree.
 *
 * Get the last element of the list of #GSGFNode objects stored in a
 * #GSGFGameTree.
 *
 * This list is not a copy.  You should not free it.  The list becomes invalid,
 * when you add or remove nodes.
 *
 * Returns: Returns a #GList of #GSGFGameTree objects.
 *
 * Since: 0.2.0
 **/
GList *
gsgf_game_tree_get_last_node (const GSGFGameTree *self)
{
        g_return_val_if_fail (GSGF_IS_GAME_TREE(self), NULL);

        return self->priv->last_node;
}

/**
 * gsgf_game_tree_get_children
 * @self: the #GSGFGameTree.
 *
 * Get the list of #GSGFGameTree objects stored in a #GSGFGameTree.
 *
 * A #GSGFGameTree can have other #GSGFGameTree instances as children.
 * This is seldom useful.
 *
 * This list is not a copy.  You should not free it.  The list becomes invalid,
 * when you add or remove nodes.
 *
 * Returns: Returns a #GList of #GSGFGameTree objects..
 **/
GList *
gsgf_game_tree_get_children(const GSGFGameTree *self)
{
        g_return_val_if_fail(GSGF_IS_GAME_TREE(self), NULL);

        return self->priv->children;
}

/**
 * gsgf_game_tree_get_flavor
 * @self: The #GSGFGameTree:
 *
 * Get the flavor of the #GSGFGameTree.
 *
 * Returns: The #GSGFFlavor or %NULL for unknown flavors.
 */
const GSGFFlavor *
gsgf_game_tree_get_flavor (const GSGFGameTree *self)
{
        g_return_val_if_fail (GSGF_IS_GAME_TREE (self), NULL);

        return self->priv->flavor;
}

/**
 * gsgf_game_tree_set_application
 * @self: The #GSGFGameTree.
 * @app: The name of your application.
 * @version: The version of your application.
 * @error: a #GError location to store the error occurring, or %NULL to ignore.
 *
 * Set the application name and version in the @self.  By default, libgsgf
 * will use only its own name and version information when writing out SGF
 * files.  You can override the default values with this method.
 *
 * Returns: %TRUE for success, %FALSE for failure.
 */
gboolean
gsgf_game_tree_set_application (GSGFGameTree *self,
                                const gchar *app, const gchar *version,
                                GError **error)
{
        gsgf_return_val_if_fail (GSGF_IS_GAME_TREE (self), FALSE, error);

        if (!app || !version) {
                g_set_error (error, GSGF_ERROR, GSGF_ERROR_USAGE_ERROR,
                             _("Application name or version info missing."));
                return FALSE;
        }

#ifndef HAVE_INDEX
#define index(str, c) memchr (str, c, strlen (str))
#endif
        if (index (app, '\n') || index (version, '\n')
            || index (app, '\r') || index (version, '\r')) {
                g_set_error (error, GSGF_ERROR, GSGF_ERROR_USAGE_ERROR,
                             _("Application name or version info must not"
                               " contain linefeeds or carriage returns."));
                return FALSE;
        }

        if (self->priv->app)
                g_free (self->priv->app);
        self->priv->app = g_strdup (app);

        if (self->priv->version)
                g_free (self->priv->version);
        self->priv->version = g_strdup (version);

        return TRUE;
}
