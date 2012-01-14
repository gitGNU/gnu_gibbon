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
        GList *games;

        gchar *white;
        gchar *black;
        gboolean crawford;
        gsize length;
};

#define GIBBON_MATCH_PRIVATE(obj) (G_TYPE_INSTANCE_GET_PRIVATE ((obj), \
        GIBBON_TYPE_MATCH, GibbonMatchPrivate))

G_DEFINE_TYPE (GibbonMatch, gibbon_match, G_TYPE_OBJECT)

static void 
gibbon_match_init (GibbonMatch *self)
{
        self->priv = G_TYPE_INSTANCE_GET_PRIVATE (self,
                GIBBON_TYPE_MATCH, GibbonMatchPrivate);

        self->priv->games = NULL;

        self->priv->white = NULL;
        self->priv->black = NULL;
        self->priv->crawford = TRUE;
        self->priv->length = 0;
}

static void
gibbon_match_finalize (GObject *object)
{
        GibbonMatch *self = GIBBON_MATCH (object);

        if (self->priv->games) {
                g_list_foreach (self->priv->games,
                                (GFunc) g_object_unref, NULL);
                g_list_free (self->priv->games);
        }
        self->priv->games = NULL;

        g_free (self->priv->white);
        g_free (self->priv->black);

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

        if (!length)
                crawford = FALSE;
        self->priv->crawford = crawford;

        gibbon_match_set_white (self, white);
        gibbon_match_set_black (self, black);
        gibbon_match_set_length (self, length);

        return self;
}

GibbonGame *
gibbon_match_get_current_game (const GibbonMatch *self)
{
        g_return_val_if_fail (GIBBON_IS_MATCH (self), NULL);

        if (!self->priv->games)
                return NULL;

        return self->priv->games->data;
}

GibbonGame *
gibbon_match_add_game (GibbonMatch *self)
{
        GibbonGame *game;
        guint game_number;
        GibbonPosition *position;
        gboolean is_crawford = FALSE;
        gint white_away, black_away;
        const GibbonPosition *last_position;

        g_return_val_if_fail (GIBBON_IS_MATCH (self), NULL);

        if (self->priv->games) {
                game = self->priv->games->data;
                position = gibbon_position_copy (gibbon_game_get_position (game));
                gibbon_position_reset (position);
        } else {
                position = gibbon_position_new ();
                position->players[0] = g_strdup (self->priv->white);
                position->players[1] = g_strdup (self->priv->black);
                position->match_length = self->priv->length;
        }

        /*
         * Check whether this is the crawford game.  This is less trivial than
         * it seems at first glance.
         *
         * A necessary but not sufficient condition is that we have a fixed
         * match length, and that exactly one of the opponents is 1-away.
         */
        if (!self->priv->length)
                goto no_crawford;

        white_away = position->match_length - position->scores[0];
        black_away = position->match_length - position->scores[1];

        if (white_away != 1 && black_away != 1)
                goto no_crawford;

        if (white_away ==  1 && black_away == 1)
                goto no_crawford;

        if (white_away == position->match_length
            || black_away == position->match_length) {
                is_crawford = TRUE;
                goto no_crawford;
        }

        /*
         * Now check for a transition in the first game action.
         */
        game = self->priv->games->data;
        if (gibbon_game_is_crawford (game))
                goto no_crawford;

        last_position = gibbon_game_get_nth_position (game, -2);
        if (!last_position)
                goto no_crawford;

        white_away = last_position->match_length - last_position->scores[0];
        black_away = last_position->match_length - last_position->scores[1];
        if (white_away == 1 || black_away == 1)
                goto no_crawford;

        is_crawford = TRUE;

        /*
         * So far for the regular cases.  But we also have to bear in mind
         * that the last match
         */
    no_crawford:
        game_number = g_list_length (self->priv->games);
        game = gibbon_game_new (self, position, game_number,
                                self->priv->crawford, is_crawford);
        gibbon_position_free (position);
        self->priv->games = g_list_append (self->priv->games, game);

        return game;
}

const GibbonPosition *
gibbon_match_get_current_position (const GibbonMatch *self)
{
        const GibbonGame *game;

        g_return_val_if_fail (GIBBON_IS_MATCH (self), NULL);

        game = self->priv->games->data;

        return gibbon_game_get_position (game);
}

void
gibbon_match_set_white (GibbonMatch *self, const gchar *white)
{
        GList *iter;
        GibbonGame *game;

        g_return_if_fail (GIBBON_IS_MATCH (self));

        g_free (self->priv->white);
        if (white)
                self->priv->white = g_strdup (white);
        else
                self->priv->white = g_strdup ("white");

        iter = self->priv->games;
        while (iter) {
                game = GIBBON_GAME (iter->data);
                gibbon_game_set_white (game, self->priv->white);
                iter = iter->next;
        }

        return;
}

void
gibbon_match_set_black (GibbonMatch *self, const gchar *black)
{
        GList *iter;
        GibbonGame *game;

        g_return_if_fail (GIBBON_IS_MATCH (self));

        g_free (self->priv->black);
        if (black)
                self->priv->black = g_strdup (black);
        else
                self->priv->black = g_strdup ("black");

        iter = self->priv->games;
        while (iter) {
                game = GIBBON_GAME (iter->data);
                gibbon_game_set_black (game, self->priv->black);
                iter = iter->next;
        }

        return;
}

void
gibbon_match_set_length (GibbonMatch *self, gsize length)
{
        GList *iter;
        GibbonGame *game;

        g_return_if_fail (GIBBON_IS_MATCH (self));

        if (length)
                self->priv->crawford = TRUE;
        else
                self->priv->crawford = FALSE;

        self->priv->length = length;

        iter = self->priv->games;
        while (iter) {
                game = GIBBON_GAME (iter->data);
                gibbon_game_set_match_length (game, length);
                iter = iter->next;
        }

}

gsize
gibbon_match_get_length (const GibbonMatch *self)
{
        g_return_val_if_fail (GIBBON_IS_MATCH (self), 0);

        return self->priv->length;
}

gsize
gibbon_match_get_number_of_games (const GibbonMatch *self)
{
        g_return_val_if_fail (GIBBON_IS_MATCH (self), 0);

        return g_list_length (self->priv->games);
}

GibbonGame *
gibbon_match_get_nth_game (const GibbonMatch *self, gsize i)
{
        GList *iter;

        g_return_val_if_fail (GIBBON_IS_MATCH (self), NULL);

        iter = g_list_nth (self->priv->games, i);
        if (!iter)
                return NULL;

        return iter->data;
}
