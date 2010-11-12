/*
 * This file is part of Gibbon, a graphical frontend to the First Internet 
 * Backgammon Server FIBS.
 * Copyright (C) 2009 Guido Flohr, http://guido-flohr.net/.
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
 * SECTION:gsgf-node-list
 * @short_description: An SGF node list (or sequence).
 *
 * A #GSGFNodeList is a list of#GSGFPropertyElements.
 */

#include <glib.h>
#include <glib/gi18n.h>

#include <libgsgf/gsgf.h>

struct _GSGFNodeListPrivate {
        GSGFNodeList *parent;
        GList *children;
};

#define GSGF_NODE_LIST_PRIVATE(o) (G_TYPE_INSTANCE_GET_PRIVATE ((o), \
                                      GSGF_TYPE_NODE_LIST,           \
                                      GSGFNodeListPrivate))
G_DEFINE_TYPE (GSGFNodeList, gsgf_node_list, G_TYPE_OBJECT)

static void
gsgf_node_list_init(GSGFNodeList *self)
{
        self->priv = G_TYPE_INSTANCE_GET_PRIVATE(self,  GSGF_TYPE_NODE_LIST,
                                                 GSGFNodeListPrivate);

        self->priv->parent = NULL;
        self->priv->children = NULL;
}

static void
gsgf_node_list_finalize(GObject *object)
{
        GSGFNodeList *self = GSGF_NODE_LIST(object);

        if (self->priv->children) {
                g_list_foreach(self->priv->children, (GFunc) g_object_unref, NULL);
                g_list_free(self->priv->children);
        }
        self->priv->children = NULL;

        G_OBJECT_CLASS (gsgf_node_list_parent_class)->finalize(object);
}

static void
gsgf_node_list_class_init(GSGFNodeListClass *klass)
{
        GObjectClass* object_class = G_OBJECT_CLASS (klass);

        g_type_class_add_private(klass, sizeof(GSGFNodeListPrivate));

        object_class->finalize = gsgf_node_list_finalize;
}

/**
 * gsgf_node_list_new:
 *
 * Build an empty #GSGFNodeList in memory.  The function cannot fail.
 *
 * Returns: An empty #GSGFNodeList.
 */
GSGFNodeList *
gsgf_node_list_new()
{
        GSGFNodeList *self = g_object_new(GSGF_TYPE_NODE_LIST, NULL);

        return self;
}

/**
 * gsgf_node_list_add_child:
 *
 * Add an empty #GSGFNodeList as a child.  The function cannot fail.
 *
 * Returns the freshly added child.
 */
GSGFNodeList *
gsgf_node_list_add_child(GSGFNodeList *self)
{
        GSGFNodeList *child = gsgf_node_list_new();

        g_list_append(self->priv->children, child);

        child->priv->parent = self;

        return child;
}
