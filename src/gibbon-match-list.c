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
#include "gibbon-resign.h"
#include "gibbon-accept.h"
#include "gibbon-reject.h"
#include "gibbon-setup.h"

#include "gibbon-analysis-roll.h"
#include "gibbon-analysis-move.h"

enum gibbon_match_list_signal {
        NEW_MATCH,
        LAST_SIGNAL
};
static guint gibbon_match_list_signals[LAST_SIGNAL] = { 0 };

typedef struct _GibbonMatchListPrivate GibbonMatchListPrivate;
struct _GibbonMatchListPrivate {
        /*
         * This is only a copy of the global match owned by the GibbonApp.
         */
        GibbonMatch *match;

        GtkListStore *games;
        gint active;
        GtkListStore *moves;
};

#define GIBBON_MATCH_LIST_PRIVATE(obj) (G_TYPE_INSTANCE_GET_PRIVATE ((obj), \
        GIBBON_TYPE_MATCH_LIST, GibbonMatchListPrivate))

G_DEFINE_TYPE (GibbonMatchList, gibbon_match_list, G_TYPE_OBJECT)

static gboolean gibbon_match_list_add_action (GibbonMatchList *self,
                                              GibbonGame *game,
                                              gint action_no);
static gchar *gibbon_match_list_format_roll (GibbonMatchList *self,
                                             GibbonRoll *roll);
static gchar *gibbon_match_list_format_move (GibbonMatchList *self,
                                             const GibbonMove *move,
                                             GibbonPositionSide side,
                                             const GibbonPosition *pos);
static gchar *gibbon_match_list_format_double (GibbonMatchList *self,
                                               const GibbonPosition *pos);
static gchar *gibbon_match_list_format_resign (GibbonMatchList *self,
                                               const GibbonResign *resign,
                                               const GibbonPosition *pos);

static void 
gibbon_match_list_init (GibbonMatchList *self)
{
        self->priv = G_TYPE_INSTANCE_GET_PRIVATE (self,
                GIBBON_TYPE_MATCH_LIST, GibbonMatchListPrivate);

        self->priv->match = NULL;

        self->priv->games = NULL;
        self->priv->active = -1;
        self->priv->moves = NULL;
}

static void
gibbon_match_list_finalize (GObject *object)
{
        G_OBJECT_CLASS (gibbon_match_list_parent_class)->finalize(object);
}

static void
gibbon_match_list_class_init (GibbonMatchListClass *klass)
{
        GObjectClass *object_class = G_OBJECT_CLASS (klass);
        
        g_type_class_add_private (klass, sizeof (GibbonMatchListPrivate));

        gibbon_match_list_signals[NEW_MATCH] =
                g_signal_new ("new-match",
                              G_TYPE_FROM_CLASS (klass),
                              G_SIGNAL_RUN_FIRST,
                              0,
                              NULL, NULL,
                              g_cclosure_marshal_VOID__OBJECT,
                              G_TYPE_NONE,
                              1,
                              G_TYPE_OBJECT);
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

        moves = gtk_list_store_new (
                        GIBBON_MATCH_LIST_N_COLUMNS,
                        /* GIBBON_MATCH_LIST_COL_SIDE */
                        G_TYPE_INT,
                        /* GIBBON_MATCH_LIST_COL_MOVENO */
                        G_TYPE_UINT,
                        /* GIBBON_MATCH_LIST_COL_ROLL */
                        G_TYPE_STRING,
                        /* GIBBON_MATCH_LIST_COL_ROLL_ACTION */
                        G_TYPE_INT,
                        /* GIBBON_MATCH_LIST_COL_LUCK */
                        G_TYPE_DOUBLE,
                        /* GIBBON_MATCH_LIST_COL_LUCK_TYPE */
                        G_TYPE_INT,
                        /* GIBBON_MATCH_LIST_COL_MOVE */
                        G_TYPE_STRING,
                        /* GIBBON_MATCH_LIST_COL_MOVE_ACTION */
                        G_TYPE_INT,
                        /* GIBBON_MATCH_LIST_COL_MOVE_BADNESS */
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

        self->priv->match = match;
        self->priv->active = -1;

        gtk_list_store_clear (self->priv->games);
        num_games = gibbon_match_get_number_of_games (match);

        for (i = 0; i < num_games; ++i) {
                self->priv->active = i;
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

        g_signal_emit (self, gibbon_match_list_signals[NEW_MATCH], 0, self);
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
        const GibbonAnalysis *analysis;

        g_return_if_fail (GIBBON_IS_MATCH_LIST (self));

        gtk_list_store_clear (self->priv->moves);

        if (active < 0)
                return;

        num_games = gibbon_match_get_number_of_games (self->priv->match);
        g_return_if_fail (num_games > active);

        game = gibbon_match_get_nth_game (self->priv->match, active);
        g_return_if_fail (game != NULL);

        self->priv->active = active;

        num_actions = gibbon_game_get_num_actions (game);

        for (i = 0; i < num_actions; ++i) {
                if (!gibbon_match_list_add_action (self, game, i))
                        break;
        }
}

gint
gibbon_match_list_get_active_game (const GibbonMatchList *self)
{
        g_return_val_if_fail (GIBBON_IS_MATCH_LIST (self), -1);

        return self->priv->active;
}

static gboolean
gibbon_match_list_add_action (GibbonMatchList *self, GibbonGame *game,
                              gint action_no)
{
        GibbonPositionSide side;
        const GibbonGameAction *action;
        const GibbonPosition *pos;
        const GibbonAnalysis *analysis;
        GtkTreeIter iter;

        action = gibbon_game_get_nth_action (game, action_no, &side);
        pos = gibbon_game_get_nth_position (game, action_no);
        analysis = gibbon_game_get_nth_analysis (game, action_no);

        /*
         * Always insert a dummy row if the first action of a game is not a
         * position setup.
         */
        if (!action_no && !GIBBON_IS_SETUP (action)) {
                g_printerr ("Append a row\n");
                gtk_list_store_append (self->priv->moves, &iter);
                gtk_list_store_set (self->priv->moves, &iter,
                                    GIBBON_MATCH_LIST_COL_MOVENO, 0,
                                    GIBBON_MATCH_LIST_COL_MOVE,
                                    _("Initial position"),
                                    -1);

        }

        return TRUE;

#if 0
        GtkTreeIter last_row;
        GtkTreeIter iter;
        gint rows;
        guint moveno, last_moveno;
        guint colno, colno2, colno3;
        gint roll_col = -1;
        gchar *buf;
        GibbonAnalysisRoll *ra;
        GibbonAnalysisRollLuck luck_type;
        gdouble luck_value;
        gchar *tmp;
        GibbonAnalysisMove *ma;
        const gchar *dbl_mark = NULL;
        const gchar *move_mark = NULL;
        guint badness = 0;

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
                         * side we add a placeholder for black's move.  That
                         * is more efficent than wasting an entire column
                         * for that purpose.
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
        } else if (GIBBON_IS_MOVE (action)) {
                if (side > 0)
                        colno = GIBBON_MATCH_LIST_COL_WHITE_MOVE;
                else
                        colno = GIBBON_MATCH_LIST_COL_BLACK_MOVE;
        } else {
                if (side > 0) {
                        colno = GIBBON_MATCH_LIST_COL_WHITE_MOVE;
                        roll_col = GIBBON_MATCH_LIST_COL_WHITE_ROLL_ACTION;
                } else {
                        colno = GIBBON_MATCH_LIST_COL_BLACK_MOVE;
                        roll_col = GIBBON_MATCH_LIST_COL_BLACK_ROLL_ACTION;
                }
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
                colno2 = side < 0
                    ? GIBBON_MATCH_LIST_COL_BLACK_LUCK
                    : GIBBON_MATCH_LIST_COL_WHITE_LUCK;
                colno3 = side < 0
                    ? GIBBON_MATCH_LIST_COL_BLACK_LUCK_TYPE
                    : GIBBON_MATCH_LIST_COL_WHITE_LUCK_TYPE;
                if (buf && GIBBON_IS_ANALYSIS_ROLL (analysis)) {
                        ra = GIBBON_ANALYSIS_ROLL (analysis);
                        luck_type = gibbon_analysis_roll_get_luck_type (ra);
                        luck_value = gibbon_analysis_roll_get_luck_value (ra);
                        gtk_list_store_set (self->priv->moves, &iter,
                                            colno2, luck_value,
                                            colno3, luck_type,
                                            -1);
                }
        } else if (GIBBON_IS_MOVE (action)) {
                buf = gibbon_match_list_format_move (self,
                                                     GIBBON_MOVE (action),
                                                     side, pos);
        } else if (GIBBON_IS_DOUBLE (action)) {
                buf = gibbon_match_list_format_double (self, pos);
        } else if (GIBBON_IS_TAKE (action)) {
                buf = g_strdup (_("Takes"));
        } else if (GIBBON_IS_DROP (action)) {
                buf = g_strdup (_("Drops"));
        } else if (GIBBON_IS_RESIGN (action)) {
                buf = gibbon_match_list_format_resign (self,
                                                       GIBBON_RESIGN (action),
                                                       pos);
        } else if (GIBBON_IS_ACCEPT (action)) {
                buf = g_strdup (_("Accepts"));
        } else if (GIBBON_IS_REJECT (action)) {
                buf = g_strdup (_("Rejects"));
        } else if (GIBBON_IS_SETUP (action)) {
                buf = g_strdup (_("Position set up"));
        }

        if (buf && analysis && GIBBON_IS_ANALYSIS_MOVE (analysis)) {
                ma = GIBBON_ANALYSIS_MOVE (analysis);
                if (ma->da) {
                        badness += ma->da_bad;
                        switch (ma->da_bad) {
                        case 0:
                                break;
                        case 1:
                                /* This is an inverted (Spanish) "?!".  */
                                dbl_mark = "\xc2\xbf\xc2\xa1";
                                break;
                        case 2:
                                /* This is an inverted (Spanish) "?".  */
                                dbl_mark = "\xc2\xbf";
                                break;
                        default:
                                /* This is an inverted (Spanish) "??".  */
                                dbl_mark = "\xc2\xbf\xc2\xbf";
                                break;
                        }
                }
                if (dbl_mark) {
                        tmp = g_strdup_printf ("%s %s", buf, dbl_mark);
                        g_free (buf);
                        buf = tmp;
                }

                if (1 || ma->ma) {
                        badness += ma->ma_bad;
                        switch (ma->ma_bad) {
                        case 0:
                                break;
                        case 1:
                                move_mark = "?!";
                                break;
                        case 2:
                                move_mark = "?f";
                                break;
                        default:
                                move_mark = "??";
                                break;
                        }
                }
                if (move_mark) {
                        tmp = g_strdup_printf ("%s %s", buf, move_mark);
                        g_free (buf);
                        buf = tmp;
                }
                if (side > 0) {
                        gtk_list_store_set (
                                self->priv->moves, &iter,
                                GIBBON_MATCH_LIST_COL_WHITE_MOVE_BADNESS,
                                badness, -1);
                } else {
                        gtk_list_store_set (
                                self->priv->moves, &iter,
                                GIBBON_MATCH_LIST_COL_BLACK_MOVE_BADNESS,
                                badness, -1);
                }
        }

        if (buf) {
                /*
                 * If this is neither a move nor a roll, the roll column
                 * will be empty.  Make it point to the following action
                 * in these cases.  And since roll_col is initialized to
                 * the function guard -1, this happens only in these cases.
                 */
                gtk_list_store_set (self->priv->moves, &iter,
                                    colno, buf, colno + 1, action_no,
                                    roll_col, action_no,
                                    -1);
                g_free (buf);
        }
#endif
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
                return g_strdup_printf (_("Redoubles to %u"), pos->cube << 1);
        else
                return g_strdup_printf (_("Doubles to %u"), pos->cube << 1);
}

static gchar *
gibbon_match_list_format_resign (GibbonMatchList *self,
                                 const GibbonResign *resign,
                                 const GibbonPosition *pos)
{
        if (resign->value == pos->cube)
                return g_strdup (_("Resigns"));
        else if (resign->value == (pos->cube << 1))
                return g_strdup (_("Resigns gammon"));
        else if (resign->value == (pos->cube + (pos->cube << 1)))
                return g_strdup (_("Resigns backgammon"));
        else
                return g_strdup_printf (_("Resigns with %u points"),
                                        resign->value);
}

const GibbonMatch *
gibbon_match_list_get_match (const GibbonMatchList *self)
{
        g_return_val_if_fail (GIBBON_IS_MATCH_LIST (self), NULL);

        return self->priv->match;
}
