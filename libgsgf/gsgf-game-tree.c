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
 * SECTION:gsgf-game-tree
 * @short_description: An SGF game tree.
 *
 * A #GSGFGameTree represents a single game in a #GSGFCollection.
 */

#include <glib.h>
#include <glib/gi18n.h>

#include <libgsgf/gsgf.h>

struct _GSGFGameTreePrivate {
        GSGFGameTree *parent;
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
        self->priv->children = NULL;
}

static void
gsgf_game_tree_finalize(GObject *object)
{
        GSGFGameTree *self = GSGF_GAME_TREE(object);

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

/**
 * gsgf_game_tree_new:
 *
 * Build an empty #GSGFGameTree in memory.  The function cannot fail.
 *
 * Returns: An empty #GSGFGameTree.
 */
GSGFGameTree *
gsgf_game_tree_new()
{
        GSGFGameTree *self = g_object_new(GSGF_TYPE_GAME_TREE, NULL);

        return self;
}

/**
 * gsgf_game_tree_add_child:
 *
 * Add an empty #GSGFGameTree as a child.  The function cannot fail.
 *
 * Returns the freshly added child.
 */
GSGFGameTree *
gsgf_game_tree_add_child(GSGFGameTree *self)
{
        GSGFGameTree *child = gsgf_game_tree_new();

        self->priv->children = g_list_append(self->priv->children, child);

        child->priv->parent = self;

        return child;
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

gssize
_gsgf_game_tree_write_stream(const GSGFGameTree *self,
                             GOutputStream *out, GCancellable *cancellable,
                             GError **error)
{
        gssize written = 0;
        gssize written_here;
        GList *iter;

        /* FIXME! The warning about the sign of the pointer &written_here is
         * important! We have to change our interface to match that of
         * g_output_stream_write_all().
         */
        if (!g_output_stream_write_all(out, "(", 1, &written_here,
                                       cancellable, error))
                return -1;
        written += written_here;

        iter = self->priv->children;
        while (iter) {
                written_here = _gsgf_game_tree_write_stream(GSGF_GAME_TREE(iter->data),
                                                            out, cancellable,
                                                            error);
                if (written_here < 0)
                        return -1;

                written += written_here;

                iter = iter->next;
        }

        if (!g_output_stream_write_all(out, ")", 1, &written_here,
                                       cancellable, error))
                return -1;
        written += written_here;

        return written;
}
