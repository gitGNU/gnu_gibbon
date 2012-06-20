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

#include "gibbon-position.h"

#include "gibbon-game.h"
#include "gibbon-roll.h"
#include "gibbon-reject.h"
#include "gibbon-accept.h"
#include "gibbon-move.h"
#include "gibbon-double.h"
#include "gibbon-take.h"
#include "gibbon-drop.h"

typedef struct _GibbonMatchListPrivate GibbonMatchListPrivate;
struct _GibbonMatchListPrivate {
        GibbonMatch *match;

        GtkListStore *games;
        GtkListStore *moves;
};

#define GIBBON_MATCH_LIST_PRIVATE(obj) (G_TYPE_INSTANCE_GET_PRIVATE ((obj), \
        GIBBON_TYPE_MATCH_LIST, GibbonMatchListPrivate))

G_DEFINE_TYPE (GibbonMatchList, gibbon_match_list, G_TYPE_OBJECT)

static gboolean gibbon_match_list_add_action (GibbonMatchList *self,
                                              const GibbonGameAction *action,
                                              GibbonPositionSide side,
                                              const GibbonPosition *pos);
static gchar *gibbon_match_list_format_roll (GibbonMatchList *self,
                                             GibbonRoll *roll);
static gchar *gibbon_match_list_format_move (GibbonMatchList *self,
                                             const GibbonMove *move,
                                             GibbonPositionSide side,
                                             const GibbonPosition *pos);
static gchar *gibbon_match_list_format_double (GibbonMatchList *self,
                                              const GibbonPosition *pos);

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
                                    G_TYPE_STRING,
                                    G_TYPE_STRING,
                                    G_TYPE_STRING,
                                    G_TYPE_STRING,
                                    G_TYPE_STRING,
                                    G_TYPE_UINT);
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
        GibbonGame *game;
        gsize num_games;
        gsize i, num_actions;
        const GibbonGameAction *action;
        GibbonPositionSide side;
        const GibbonPosition *pos;

        g_return_if_fail (GIBBON_IS_MATCH_LIST (self));

        gtk_list_store_clear (self->priv->moves);

        if (active < 0)
                return;

        num_games = gibbon_match_get_number_of_games (self->priv->match);
        g_return_if_fail (num_games > active);

        game = gibbon_match_get_nth_game (self->priv->match, active);
        g_return_if_fail (game != NULL);

        num_actions = gibbon_game_get_num_actions (game);

        for (i = 0; i < num_actions; ++i) {
                action = gibbon_game_get_nth_action (game, i, &side);
                pos = gibbon_game_get_nth_position (game, i);
                if (!gibbon_match_list_add_action (self, action, side, pos))
                        break;
        }
}

static gboolean
gibbon_match_list_add_action (GibbonMatchList *self,
                              const GibbonGameAction *action,
                              GibbonPositionSide side,
                              const GibbonPosition *pos)
{
        GtkTreeIter last_row;
        GtkTreeIter iter;
        gint rows;
        guint moveno, last_moveno;
        guint colno, colno2;
        gchar *buf;

        /* Get an iter to the last row.  */
        rows = gtk_tree_model_iter_n_children (
                        GTK_TREE_MODEL (self->priv->moves), NULL);

        /* Do we need a new row? */
        if (!rows) {
                gtk_list_store_append (self->priv->moves, &iter);
                if (!side && GIBBON_IS_ROLL (action)) {
                        moveno = 0;
                } else {
                        moveno = 1;
                }
                buf = g_strdup_printf ("%u", moveno);
                gtk_list_store_set (self->priv->moves, &iter,
                                    GIBBON_MATCH_LIST_COL_MOVENO, buf,
                                    GIBBON_MATCH_LIST_COL_LOGICAL_MOVENO, moveno,
                                    -1);
                g_free (buf);
                last_row = iter;

                if (side > 0) {
                        /*
                         * If the very first action will be listed on white's
                         * side we add a placeholder for black's move.
                         */
                        gtk_list_store_set (self->priv->moves, &iter,
                                            GIBBON_MATCH_LIST_COL_BLACK_MOVE,
                                            "",
                                            -1);
                }
        } else {
                g_return_val_if_fail (gtk_tree_model_iter_nth_child (
                                GTK_TREE_MODEL (self->priv->moves),
                                &last_row, NULL, rows - 1), FALSE);
        }

        /* Determine the column that has to be manipulated.  */
        if (GIBBON_IS_ROLL (action)) {
                if (side > 0)
                        colno = GIBBON_MATCH_LIST_COL_WHITE_ROLL;
                else
                        colno = GIBBON_MATCH_LIST_COL_BLACK_ROLL;
        } else {
                if (side > 0)
                        colno = GIBBON_MATCH_LIST_COL_WHITE_MOVE;
                else
                        colno = GIBBON_MATCH_LIST_COL_BLACK_MOVE;
        }

        /*
         * Check whether the column that has to be manipulated is already
         * filled.  If it is we definitely have to add a new row.
         */
        gtk_tree_model_get (GTK_TREE_MODEL (self->priv->moves), &last_row,
                                            colno, &buf,
                                            GIBBON_MATCH_LIST_COL_LOGICAL_MOVENO,
                                            &last_moveno,
                                            -1);
        if (!buf && GIBBON_IS_ROLL (action)) {
                 /*
                  * If this is a roll make sure that we do not write overwrite
                  * a possible resignation or rejected resignation.
                  */
                if (side > 0)
                        colno2 = GIBBON_MATCH_LIST_COL_WHITE_MOVE;
                else
                        colno2 = GIBBON_MATCH_LIST_COL_BLACK_MOVE;
                gtk_tree_model_get (GTK_TREE_MODEL (self->priv->moves),
                                                    &last_row,
                                                    colno2, &buf,
                                                    -1);
        }
        if (buf) {
                /* We must add a new row.  */
                gtk_list_store_append (self->priv->moves, &iter);

                if (!(!side || GIBBON_IS_REJECT (action)
                      || GIBBON_IS_ACCEPT (action))) {
                        g_free (buf);
                        buf = g_strdup_printf ("%u", last_moveno + 1);
                        gtk_list_store_set (self->priv->moves, &iter,
                                            GIBBON_MATCH_LIST_COL_MOVENO, buf,
                                            GIBBON_MATCH_LIST_COL_LOGICAL_MOVENO,
                                            last_moveno + 1,
                                            -1);
                } else {
                        gtk_list_store_set (self->priv->moves, &iter,
                                            GIBBON_MATCH_LIST_COL_LOGICAL_MOVENO,
                                            last_moveno,
                                            -1);
                }
        } else {
                iter = last_row;
        }
        g_free (buf);
        buf = NULL;

        if (GIBBON_IS_ROLL (action)) {
                buf = gibbon_match_list_format_roll (self,
                                                     GIBBON_ROLL (action));
        } else if (GIBBON_IS_MOVE (action)) {
                buf = gibbon_match_list_format_move (self,
                                                     GIBBON_MOVE (action),
                                                     side, pos);
        } else if (GIBBON_IS_DOUBLE (action)) {
                buf = gibbon_match_list_format_double (self, pos);
        } else if (GIBBON_IS_TAKE (action)) {
                buf = g_strdup (_("Take"));
        } else if (GIBBON_IS_DROP (action)) {
                buf = g_strdup (_("Drop"));
        }

        if (buf) {
                gtk_list_store_set (self->priv->moves, &iter,
                                    colno, buf, -1);
                g_free (buf);
        }

        return TRUE;
}

static gchar *
gibbon_match_list_format_roll (GibbonMatchList *self, GibbonRoll *roll)
{
        /*
         * TRANSLATORS: This is how a roll is formatted in the move list.
         * Normally this string can be copied verbatim unless another format
         * is usual for your language.
         */
        return g_strdup_printf (_("%u%u:"), roll->die1, roll->die2);
}

static gchar *
gibbon_match_list_format_move (GibbonMatchList *self,
                               const GibbonMove *move,
                               GibbonPositionSide side,
                               const GibbonPosition *pos)
{
        return gibbon_position_format_move (pos, move, side, FALSE);
}

static gchar *
gibbon_match_list_format_double (GibbonMatchList *self,
                                 const GibbonPosition *pos)
{
        if (pos->cube > 1)
                return g_strdup_printf (_("Redouble to %u"), pos->cube << 1);
        else
                return g_strdup_printf (_("Double to %u"), pos->cube << 1);
}
