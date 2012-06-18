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
 * SECTION:gibbon-match-list
 * @short_description: Match listing.
 *
 * Since: 0.2.0
 *
 * A #GibbonMatchList is the model for the match listing in the moves tab.
 */

#include <gtk/gtk.h>
#include <glib/gi18n.h>

#include "gibbon-match-list.h"
#include "gibbon-game.h"

typedef struct _GibbonMatchListPrivate GibbonMatchListPrivate;
struct _GibbonMatchListPrivate {
        GibbonMatch *match;

        GtkListStore *games;
        GtkListStore *moves;
};

#define GIBBON_MATCH_LIST_PRIVATE(obj) (G_TYPE_INSTANCE_GET_PRIVATE ((obj), \
        GIBBON_TYPE_MATCH_LIST, GibbonMatchListPrivate))

G_DEFINE_TYPE (GibbonMatchList, gibbon_match_list, G_TYPE_OBJECT)

static void 
gibbon_match_list_init (GibbonMatchList *self)
{
        self->priv = G_TYPE_INSTANCE_GET_PRIVATE (self,
                GIBBON_TYPE_MATCH_LIST, GibbonMatchListPrivate);

        self->priv->match = NULL;

        self->priv->games = NULL;
        self->priv->moves = NULL;
}

static void
gibbon_match_list_finalize (GObject *object)
{
        GibbonMatchList *self = GIBBON_MATCH_LIST (object);

        if (self->priv->match)
                g_object_unref (self->priv->match);

        G_OBJECT_CLASS (gibbon_match_list_parent_class)->finalize(object);
}

static void
gibbon_match_list_class_init (GibbonMatchListClass *klass)
{
        GObjectClass *object_class = G_OBJECT_CLASS (klass);
        
        g_type_class_add_private (klass, sizeof (GibbonMatchListPrivate));

        object_class->finalize = gibbon_match_list_finalize;
}

/**
 * gibbon_match_list_new:
 *
 * Creates a new #GibbonMatchList.
 *
 * Returns: The newly created #GibbonMatchList or %NULL in case of failure.
 */
GibbonMatchList *
gibbon_match_list_new (void)
{
        GibbonMatchList *self = g_object_new (GIBBON_TYPE_MATCH_LIST, NULL);
        GtkListStore *moves;

        self->priv->games = gtk_list_store_new (1, G_TYPE_STRING);

        moves = gtk_list_store_new (GIBBON_MATCH_LIST_N_COLUMNS,
                                    G_TYPE_UINT,
                                    G_TYPE_STRING,
                                    G_TYPE_STRING);
        self->priv->moves = moves;

        return self;
}

void
gibbon_match_list_set_match (GibbonMatchList *self, GibbonMatch *match)
{
        gsize i, num_games;
        GtkTreeIter iter;
        gchar *text;
        const GibbonGame *game;
        const GibbonPosition *pos;
        gchar *comment;

        g_return_if_fail (GIBBON_IS_MATCH_LIST (self));

        if (self->priv->match)
                g_object_unref (self->priv->match);
        self->priv->match = match;

        gtk_list_store_clear (self->priv->games);
        num_games = gibbon_match_get_number_of_games (match);

        for (i = 0; i < num_games; ++i) {
                game = gibbon_match_get_nth_game (match, i);
                pos = gibbon_game_get_initial_position (game);
                comment = gibbon_game_is_crawford (game) ?
                                _("(Crawford)") : "";
                text = g_strdup_printf (_("Game %u: %u-%u %s"),
                                        (unsigned int) i + 1,
                                        pos->scores[1], pos->scores[0],
                                        comment);
                gtk_list_store_append (self->priv->games, &iter);
                gtk_list_store_set (self->priv->games, &iter,
                                    0, text, -1);
                g_free (text);
        }
}

GtkListStore *
gibbon_match_list_get_games_store (const GibbonMatchList *self)
{
        g_return_val_if_fail (GIBBON_IS_MATCH_LIST (self), NULL);

        return self->priv->games;
}

GtkListStore *
gibbon_match_list_get_moves_store (const GibbonMatchList *self)
{
        g_return_val_if_fail (GIBBON_IS_MATCH_LIST (self), NULL);

        return self->priv->moves;
}

void
gibbon_match_list_set_active_game (GibbonMatchList *self, gint active)
{
        g_return_if_fail (GIBBON_IS_MATCH_LIST (self));


}
