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
 * SECTION:gibbon-match
 * @short_description: Representation of a backgammon match in Gibbon!
 *
 * Since: 0.1.0
 *
 * A GibbonMatch is the internal representation of a backgammon match in
 * Gibbon.  It is always linked to a #GSGFCollection that serves as the
 * storage backend.
 **/

#include <glib.h>
#include <glib/gi18n.h>

#include <libgsgf/gsgf.h>

#include "gibbon-match.h"
#include "gibbon-game.h"

typedef struct _GibbonMatchPrivate GibbonMatchPrivate;
struct _GibbonMatchPrivate {
        GSGFCollection *collection;
        GSGFFlavor *flavor;

        GList *games;

        gboolean crawford;
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

        self->priv->crawford = TRUE;
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
gibbon_match_new (const gchar *white, const gchar *black,
                  guint length, gboolean crawford)
{
        GibbonMatch *self = g_object_new (GIBBON_TYPE_MATCH, NULL);
        GSGFGameTree *game_tree;
        GibbonGame *game;
        GibbonPosition *position = gibbon_position_new ();

        self->priv->collection = gsgf_collection_new ();

        self->priv->flavor = gsgf_flavor_backgammon_new ();

        position->players[0] = g_strdup (white);
        position->players[1] = g_strdup (black);
        position->match_length = length;

        if (!length)
                crawford = FALSE;
        self->priv->crawford = crawford;

        game_tree = gsgf_collection_add_game_tree (self->priv->collection,
                                                   self->priv->flavor);

        /*
         * Note: The first game can never be the crawford game!
         */
        game = gibbon_game_new (self, game_tree, position,
                                0, crawford, FALSE);
        self->priv->games = g_list_prepend (self->priv->games, game);

        return self;
}

const GSGFCollection *
gibbon_match_get_collection (GibbonMatch *self)
{
        g_return_val_if_fail (GIBBON_IS_MATCH (self), NULL);

        return self->priv->collection;
}

GibbonGame *
gibbon_match_get_current_game (const GibbonMatch *self)
{
        g_return_val_if_fail (GIBBON_IS_MATCH (self), NULL);

        return self->priv->games->data;
}

GibbonGame *
gibbon_match_add_game (GibbonMatch *self)
{
        GibbonGame *game;
        GList *game_trees;
        GSGFGameTree *game_tree;
        guint game_number;
        GibbonPosition *position;

        g_return_val_if_fail (GIBBON_IS_MATCH (self), NULL);

        game_trees = gsgf_collection_get_game_trees (self->priv->collection);
        game_number = g_list_length (game_trees);
        game_tree = gsgf_collection_add_game_tree (self->priv->collection,
                                                   self->priv->flavor);

        game = self->priv->games->data;
        position = gibbon_position_copy (gibbon_game_get_position (game));
        gibbon_position_reset (position);

        /* FIXME! Check whether this is the crawford game! */
        game = gibbon_game_new (self, game_tree, position, game_number,
                                self->priv->crawford, FALSE);
        gibbon_position_free (position);
        self->priv->games = g_list_prepend (self->priv->games, game);

        return game;
}

guint
gibbon_match_score (const GibbonMatch *self, GibbonPositionSide side)
{
        g_return_val_if_fail (GIBBON_IS_MATCH (self), 0);

        return 0;
}

const GibbonPosition *
gibbon_match_get_current_position (const GibbonMatch *self)
{
        const GibbonGame *game;

        g_return_val_if_fail (GIBBON_IS_MATCH (self), NULL);

        game = self->priv->games->data;

        return gibbon_game_get_position (game);
}
