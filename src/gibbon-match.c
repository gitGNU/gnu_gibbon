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

        gchar *black_player;
        gchar *white_player;

        gint length;
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

        self->priv->black_player = NULL;
        self->priv->white_player = NULL;

        self->priv->length = -1;
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

        if (self->priv->black_player)
                g_free (self->priv->black_player);
        if (self->priv->white_player)
                g_free (self->priv->white_player);

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
                  guint length, gboolean crawford, GError **error)
{
        GibbonMatch *self = g_object_new (GIBBON_TYPE_MATCH, NULL);
        GSGFGameTree *game_tree;
        GibbonGame *game;

        self->priv->collection = gsgf_collection_new ();

        self->priv->flavor = gsgf_flavor_backgammon_new ();

        self->priv->black_player = g_strdup (black);
        self->priv->white_player = g_strdup (white);
        self->priv->length = length;

        if (!length)
                crawford = FALSE;
        self->priv->crawford = crawford;

        game_tree = gsgf_collection_add_game_tree (self->priv->collection,
                                                   self->priv->flavor);

        /*
         * Note: The first game can never be the crawford game!
         */
        game = gibbon_game_new (self, game_tree,
                                white, black, length, 0, 0, 0, TRUE, FALSE,
                                error);
        self->priv->games = g_list_prepend (self->priv->games, game);

        return self;
}

const GSGFCollection *
gibbon_match_get_collection (GibbonMatch *self)
{
        g_return_val_if_fail (GIBBON_IS_MATCH (self), NULL);

        return self->priv->collection;
}

const gchar *
gibbon_match_get_white_player (const GibbonMatch *self)
{
        g_return_val_if_fail (GIBBON_IS_MATCH (self), NULL);

        return self->priv->white_player;
}

const gchar *
gibbon_match_get_black_player (const GibbonMatch *self)
{
        g_return_val_if_fail (GIBBON_IS_MATCH (self), NULL);

        return self->priv->black_player;
}

gint
gibbon_match_get_length (const GibbonMatch *self)
{
        g_return_val_if_fail (GIBBON_IS_MATCH (self), -1);

        return self->priv->length;
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
        GSGFGameTree *game_tree;

        g_return_val_if_fail (GIBBON_IS_MATCH (self), NULL);

        game_tree = gsgf_collection_add_game_tree (self->priv->collection,
                                                   self->priv->flavor);

        /*
         * FIXME! Match scores are wrong! Game number is wrong!
         */
        game = gibbon_game_new (self, game_tree,
                                self->priv->black_player,
                                self->priv->white_player,
                                self->priv->length, 0, 0, 0, TRUE, FALSE,
                                NULL);
        self->priv->games = g_list_append (self->priv->games, game);

        return game;
}
