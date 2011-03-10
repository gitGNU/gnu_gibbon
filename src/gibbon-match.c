/*
 * This file is part of gibbon.
 * Gibbon is a Gtk+ frontend for the First Internet Backgammon Server FIBS.
 * Copyright (C) 2009-2011 Guido Flohr, http://guido-flohr.net/.
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
 * SECTION:gibbon-match
 * @short_description: Representation of a backgammon match in Gibbon!
 *
 * Since: 0.1.1
 *
 * A GibbonMatch is the internal representation of a backgammon match in
 * Gibbon.  It is always linked to a #GSGFCollection that serves as the
 * storage backend.
 **/

#include <glib.h>
#include <glib/gi18n.h>

#include "gibbon-match.h"
#include "gibbon-game.h"

typedef struct _GibbonMatchPrivate GibbonMatchPrivate;
struct _GibbonMatchPrivate {
        GSGFCollection *collection;
        GSGFFlavor *flavor;

        GList *games;
};

#define GIBBON_MATCH_PRIVATE(obj) (G_TYPE_INSTANCE_GET_PRIVATE ((obj), \
        GIBBON_TYPE_MATCH, GibbonMatchPrivate))

G_DEFINE_TYPE (GibbonMatch, gibbon_match, G_TYPE_OBJECT)

static void 
gibbon_match_init (GibbonMatch *self)
{
        self->priv = G_TYPE_INSTANCE_GET_PRIVATE (self,
                GIBBON_TYPE_MATCH, GibbonMatchPrivate);

        self->priv->collection = NULL;
        self->priv->flavor = NULL;
        self->priv->games = NULL;
}

static void
gibbon_match_finalize (GObject *object)
{
        GibbonMatch *self = GIBBON_MATCH (object);

        if (self->priv->collection)
                g_object_unref (self->priv->collection);
        self->priv->collection = NULL;

        if (self->priv->games) {
                g_list_foreach (self->priv->games,
                                (GFunc) g_object_unref, NULL);
                g_list_free (self->priv->games);
        }
        self->priv->games = NULL;

        if (self->priv->flavor)
                g_object_unref (self->priv->flavor);
        self->priv->flavor = NULL;

        G_OBJECT_CLASS (gibbon_match_parent_class)->finalize(object);
}

static void
gibbon_match_class_init (GibbonMatchClass *klass)
{
        GObjectClass *object_class = G_OBJECT_CLASS (klass);
        
        g_type_class_add_private (klass, sizeof (GibbonMatchPrivate));

        object_class->finalize = gibbon_match_finalize;
}

/**
 * gibbon_match_new:
 *
 * Creates a new, empty #GibbonMatch.
 *
 * Returns: The newly created #GibbonMatch or %NULL.
 */
GibbonMatch *
gibbon_match_new ()
{
        GibbonMatch *self = g_object_new (GIBBON_TYPE_MATCH, NULL);
        GSGFGameTree *game_tree;
        GibbonGame *game;

        self->priv->collection = gsgf_collection_new ();

        self->priv->flavor = gsgf_flavor_backgammon_new ();

        game_tree = gsgf_collection_add_game_tree (self->priv->collection,
                                                   self->priv->flavor);

        game = gibbon_game_new (self, game_tree);
        self->priv->games = g_list_append (self->priv->games, game);

        return self;
}

const GSGFCollection *
gibbon_match_get_collection (GibbonMatch *self)
{
        g_return_val_if_fail (GIBBON_IS_MATCH (self), NULL);

        return self->priv->collection;
}

gboolean
gibbon_match_set_white_player (GibbonMatch *self, const gchar *name,
                               GError **error)
{
        GList *iter;
        GSGFGameTree *game_tree;
        GList* nodes;
        GSGFNode* root;
        GSGFValue *simple_text;

        g_return_val_if_fail (GIBBON_IS_MATCH (self), FALSE);
        g_return_val_if_fail (name, FALSE);

        for (iter = self->priv->games; iter; iter = iter->next) {
g_printerr ("Setting PW in root node ...\n");
                game_tree = GSGF_GAME_TREE (iter->data);
                nodes = gsgf_game_tree_get_nodes (game_tree);
                root = nodes->data;
                simple_text = GSGF_VALUE (gsgf_simple_text_new (name));
                if (!gsgf_node_set_property (root, "PW", simple_text, error)) {
                        g_object_unref (simple_text);
                        return FALSE;
                }
        }

        return TRUE;
}

gboolean
gibbon_match_set_black_player (GibbonMatch *self, const gchar *name,
                               GError **error)
{
        GList *iter;
        GSGFGameTree *game_tree;
        GList* nodes;
        GSGFNode* root;
        GSGFValue *simple_text;

        g_return_val_if_fail (GIBBON_IS_MATCH (self), FALSE);
        g_return_val_if_fail (name, FALSE);

        for (iter = self->priv->games; iter; iter = iter->next) {
                game_tree = GSGF_GAME_TREE (iter->data);
                nodes = gsgf_game_tree_get_nodes (game_tree);
                root = nodes->data;
                simple_text = GSGF_VALUE (gsgf_simple_text_new (name));
                if (!gsgf_node_set_property (root, "PB", simple_text, error)) {
                        g_object_unref (simple_text);
                        return FALSE;
                }
        }

        return TRUE;
}
