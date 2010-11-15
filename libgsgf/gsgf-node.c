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
 * SECTION:gsgf-node
 * @short_description: An SGF node.
 *
 * A #GSGFNode is a list of#GSGFPropertyElements.
 */

#include <glib.h>
#include <glib/gi18n.h>

#include <libgsgf/gsgf.h>

#include "gsgf-internal.h"

struct _GSGFNodePrivate {
        GSGFNode *parent;

        GList *properties;
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

        self->priv->parent = NULL;
}

static void
gsgf_node_finalize(GObject *object)
{
        GSGFNode *self = GSGF_NODE(object);

        if (self->priv->properties) {
                g_list_foreach(self->priv->properties, (GFunc) g_object_unref, NULL);
                g_free(self->priv->properties);
        }
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

/**
 * gsgf_node_new:
 *
 * Build an empty #GSGFNode in memory.  The function cannot fail.
 *
 * Returns: An empty #GSGFNode.
 */
GSGFNode *
gsgf_node_new()
{
        GSGFNode *self = g_object_new(GSGF_TYPE_NODE, NULL);

        return self;
}

gboolean
_gsgf_node_write_stream(const GSGFNode *self, GOutputStream *out,
                        gsize *bytes_written, GCancellable *cancellable, GError **error)
{
        gsize written_here;

        *bytes_written = 0;

        if (!g_output_stream_write_all(out, ";", 1, &written_here,
                                       cancellable, error)) {
                *bytes_written += written_here;
                return FALSE;
        }

        *bytes_written += written_here;

/*
        iter = self->priv->children;
        while (iter) {
                if (!_gsgf_game_tree_write_stream(GSGF_GAME_TREE(iter->data), out,
                                                  &written_here, cancellable,
                                                  error)) {
                        *bytes_written += written_here;
                        return FALSE;
                }

                *bytes_written += written_here;

                iter = iter->next;
        }
*/

        return TRUE;
}
