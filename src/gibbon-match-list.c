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
#include "gibbon-game-actions.h"

#include "gibbon-analysis-roll.h"
#include "gibbon-analysis-move.h"
#include "gibbon-app.h"

enum gibbon_match_list_signal {
        GAME_UPDATING,
        GAME_SELECTED,
        LAST_SIGNAL
};
static guint gibbon_match_list_signals[LAST_SIGNAL] = { 0 };

typedef struct _GibbonMatchListPrivate GibbonMatchListPrivate;
struct _GibbonMatchListPrivate {
        GtkListStore *games;
        gint active;
        GtkListStore *moves;
};

#define GIBBON_MATCH_LIST_PRIVATE(obj) (G_TYPE_INSTANCE_GET_PRIVATE ((obj), \
        GIBBON_TYPE_MATCH_LIST, GibbonMatchListPrivate))

G_DEFINE_TYPE (GibbonMatchList, gibbon_match_list, G_TYPE_OBJECT)

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

        self->priv->games = NULL;
        self->priv->active = -1;
        self->priv->moves = NULL;
}

static void
gibbon_match_list_finalize (GObject *object)
{
        GibbonMatchList *self = GIBBON_MATCH_LIST (object);

        if (self->priv->games)
                g_object_unref (self->priv->games);
        if (self->priv->moves)
                g_object_unref (self->priv->moves);

        G_OBJECT_CLASS (gibbon_match_list_parent_class)->finalize(object);
}

static void
gibbon_match_list_class_init (GibbonMatchListClass *klass)
{
        GObjectClass *object_class = G_OBJECT_CLASS (klass);
        
        g_type_class_add_private (klass, sizeof (GibbonMatchListPrivate));

        gibbon_match_list_signals[GAME_UPDATING] =
                g_signal_new ("game-updating",
                              G_TYPE_FROM_CLASS (klass),
                              G_SIGNAL_RUN_FIRST,
                              0,
                              NULL, NULL,
                              g_cclosure_marshal_VOID__OBJECT,
                              G_TYPE_NONE,
                              1,
                              G_TYPE_OBJECT);
        gibbon_match_list_signals[GAME_SELECTED] =
                g_signal_new ("game-selected",
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
                        /* GIBBON_MATCH_LIST_COL_PLAYER */
                        G_TYPE_STRING,
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
gibbon_match_list_on_new_match (GibbonMatchList *self,
                                const GibbonMatch *match)
{
        gsize i, num_games;
        const GibbonGame *game;

        g_return_if_fail (GIBBON_IS_MATCH_LIST (self));
        g_return_if_fail (GIBBON_IS_MATCH (match));

        self->priv->active = -1;

        gtk_list_store_clear (self->priv->games);
        num_games = gibbon_match_get_number_of_games (match);

        for (i = 0; i < num_games; ++i) {
                self->priv->active = i;
                game = gibbon_match_get_nth_game (match, i);
                gibbon_match_list_add_game (self, game);
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
        GibbonMatch *match;

        g_return_if_fail (GIBBON_IS_MATCH_LIST (self));

        gtk_list_store_clear (self->priv->moves);

        if (active < 0)
                return;

        match = gibbon_app_get_match (app);
        g_return_if_fail (GIBBON_IS_MATCH (match));

        num_games = gibbon_match_get_number_of_games (match);
        g_return_if_fail (num_games > active);

        game = gibbon_match_get_nth_game (match, active);
        g_return_if_fail (game != NULL);

        self->priv->active = active;

        num_actions = gibbon_game_get_num_actions (game);

        g_signal_emit (self, gibbon_match_list_signals[GAME_UPDATING], 0, self);
        for (i = 0; i < num_actions; ++i) {
                if (!gibbon_match_list_add_action (self, game, i))
                        break;
        }
        g_signal_emit (self, gibbon_match_list_signals[GAME_SELECTED], 0, self);
}

gint
gibbon_match_list_get_active_game (const GibbonMatchList *self)
{
        g_return_val_if_fail (GIBBON_IS_MATCH_LIST (self), -1);

        return self->priv->active;
}

gboolean
gibbon_match_list_add_action (GibbonMatchList *self, const GibbonGame *game,
                              gint action_no)
{
        GibbonPositionSide side;
        const GibbonGameAction *action;
        const GibbonGameAction *last_action = NULL;
        const GibbonPosition *pos, *last_pos;
        const GibbonAnalysis *analysis;
        GtkTreeIter iter;
        gint moveno;
        const gchar *player;
        gint colno = 0;
        gchar *text = NULL;
        const gchar *dbl_mark = NULL;
        const gchar *chk_mark = NULL;
        GString *formatted;
        GibbonAnalysisRoll *ra;
        GibbonAnalysisRollLuck luck_type;
        gdouble luck_value;
        GibbonAnalysisMove *ma;
        guint badness = 0;
        const gchar *open_tag;
        const gchar *close_tag;
        GibbonMatch *match;
        const gchar *initial_label;

        g_return_val_if_fail (GIBBON_IS_MATCH_LIST (self), FALSE);
        g_return_val_if_fail (GIBBON_IS_GAME (game), FALSE);
        match = gibbon_app_get_match (app);
        g_return_val_if_fail (GIBBON_IS_MATCH (match), FALSE);

        action = gibbon_game_get_nth_action (game, action_no, &side);
        if (action_no)
                last_action = gibbon_game_get_nth_action (game, action_no - 1,
                                                          NULL);
        pos = gibbon_game_get_nth_position (game, action_no);
        analysis = gibbon_game_get_nth_analysis (game, action_no);

        moveno = gtk_tree_model_iter_n_children (
                        GTK_TREE_MODEL (self->priv->moves), NULL);

        if (!action_no) {
                if (gibbon_game_get_edited (game))
                        initial_label = _("Setup position");
                else
                        initial_label = _("Initial position");
                gtk_list_store_append (self->priv->moves, &iter);
                gtk_list_store_set (self->priv->moves, &iter,
                                    GIBBON_MATCH_LIST_COL_MOVENO, 0,
                                    GIBBON_MATCH_LIST_COL_MOVE, initial_label,
                                    GIBBON_MATCH_LIST_COL_ROLL_ACTION, -1,
                                    GIBBON_MATCH_LIST_COL_MOVE_ACTION, -1,
                                    -1);
                ++moveno;
        }

        /*
         * Normally, we do not have to add a row for a move because there
         * is already one for the roll.  However, this could be intercepted
         * by a resignation or a position setup.
         */
        if (action_no && GIBBON_IS_MOVE (action) && last_action
            && GIBBON_IS_ROLL (last_action)) {
                g_return_val_if_fail (gtk_tree_model_iter_nth_child (
                                GTK_TREE_MODEL (self->priv->moves),
                                &iter, NULL, moveno - 1), FALSE);
        } else {
                gtk_list_store_append (self->priv->moves, &iter);
                if (side <  0)
                        player = gibbon_match_get_black (match);
                else if (side > 0)
                        player = gibbon_match_get_white (match);
                else
                        player = NULL;
                gtk_list_store_set (self->priv->moves, &iter,
                                    GIBBON_MATCH_LIST_COL_MOVENO, moveno,
                                    GIBBON_MATCH_LIST_COL_PLAYER, player,
                                    GIBBON_MATCH_LIST_COL_SIDE, side,
                                    GIBBON_MATCH_LIST_COL_ROLL_ACTION, -1,
                                    GIBBON_MATCH_LIST_COL_MOVE_ACTION, -1,
                                    -1);
        }

        if (GIBBON_IS_ROLL (action)) {
                text = gibbon_match_list_format_roll (self,
                                                      GIBBON_ROLL (action));
                colno = GIBBON_MATCH_LIST_COL_ROLL;
                if (text && GIBBON_IS_ANALYSIS_ROLL (analysis)) {
                        ra = GIBBON_ANALYSIS_ROLL (analysis);
                        luck_type = gibbon_analysis_roll_get_luck_type (ra);
                        luck_value = gibbon_analysis_roll_get_luck_value (ra);
                        gtk_list_store_set (self->priv->moves, &iter,
                                            GIBBON_MATCH_LIST_COL_LUCK,
                                            luck_value,
                                            GIBBON_MATCH_LIST_COL_LUCK_TYPE,
                                            luck_type,
                                            -1);
                }
        } else if (GIBBON_IS_MOVE (action)) {
                if (action_no)
                        last_pos = gibbon_game_get_nth_position (game,
                                                                 action_no - 1);
                else
                        last_pos = gibbon_game_get_initial_position (game);
                text = gibbon_match_list_format_move (self,
                                                      GIBBON_MOVE (action),
                                                      side, last_pos);
                colno = GIBBON_MATCH_LIST_COL_MOVE;
        } else if (GIBBON_IS_DOUBLE (action)) {
                text = gibbon_match_list_format_double (self, pos);
                colno = GIBBON_MATCH_LIST_COL_MOVE;
        } else if (GIBBON_IS_TAKE (action)) {
                text = g_strdup (_("takes"));
                colno = GIBBON_MATCH_LIST_COL_MOVE;
        } else if (GIBBON_IS_DROP (action)) {
                text = g_strdup (_("drops"));
                colno = GIBBON_MATCH_LIST_COL_MOVE;
        } else if (GIBBON_IS_RESIGN (action)) {
                text = gibbon_match_list_format_resign (self,
                                                        GIBBON_RESIGN (action),
                                                        pos);
                colno = GIBBON_MATCH_LIST_COL_MOVE;
        } else if (GIBBON_IS_ACCEPT (action)) {
                text = g_strdup (_("accepts"));
                colno = GIBBON_MATCH_LIST_COL_MOVE;
        } else if (GIBBON_IS_REJECT (action)) {
                text = g_strdup (_("rejects"));
                colno = GIBBON_MATCH_LIST_COL_MOVE;
        }

        if (analysis && GIBBON_IS_ANALYSIS_MOVE (analysis)) {
                ma = GIBBON_ANALYSIS_MOVE (analysis);
                badness += ma->da_bad;
                switch (ma->da_bad) {
                case 0:
                        break;
                case 1:
                        dbl_mark = _("?!");
                        break;
                case 2:
                        /* TRANSLATORS: Mark for bad move!  */
                        dbl_mark = _("?");
                        break;
                default:
                        dbl_mark = _("??");
                        break;
                }

                badness += ma->ma_bad;
                chk_mark = NULL;
                switch (ma->ma_bad) {
                case 0:
                        break;
                case 1:
                        chk_mark = _("?!");
                        break;
                case 2:
                        chk_mark = _("?");
                        break;
                default:
                        chk_mark = _("??");
                        break;
                }

                gtk_list_store_set (self->priv->moves, &iter,
                                    GIBBON_MATCH_LIST_COL_MOVE_BADNESS,
                                    badness, -1);
        }

        switch (badness) {
        case 0:
                open_tag = "";
                close_tag = "";
                break;
        case 1:
                open_tag = "<i>";
                close_tag = "</i>";
                break;
        case 2:
                open_tag = "<b>";
                close_tag = "</b>";
                break;
        default:
                open_tag = "<b><i>";
                close_tag = "</i></b>";
                break;
        }

        if (dbl_mark && !GIBBON_IS_MOVE (action)) {
                chk_mark = dbl_mark;
                dbl_mark = NULL;
        }

        formatted = g_string_new (open_tag);
        if (dbl_mark) {
                g_string_append (formatted, dbl_mark);
                g_string_append_c (formatted, ' ');
        }
        g_string_append (formatted, text);
        g_free (text);

        if (chk_mark)
                g_string_append (formatted, chk_mark);

        if (*close_tag)
                g_string_append (formatted, close_tag);

        gtk_list_store_set (self->priv->moves, &iter,
                            colno, formatted->str,
                            colno + 1, action_no,
                            -1);
        g_string_free (formatted, TRUE);

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
                return g_strdup_printf (_("redoubles to %llu"),
                                        (unsigned long long) pos->cube << 1);
        else
                return g_strdup_printf (_("doubles to %llu"),
                                        (unsigned long long) pos->cube << 1);
}

static gchar *
gibbon_match_list_format_resign (GibbonMatchList *self,
                                 const GibbonResign *resign,
                                 const GibbonPosition *pos)
{
        if (resign->value == pos->cube)
                return g_strdup (_("resigns"));
        else if (resign->value == (pos->cube << 1))
                return g_strdup (_("resigns gammon"));
        else if (resign->value == (pos->cube + (pos->cube << 1)))
                return g_strdup (_("resigns backgammon"));
        else
	  /*
	   * Yes, the case for one point is already handled.  But other
	   * languages than English may require the plural handling here.
	   */
	  return g_strdup_printf (g_dngettext (GETTEXT_PACKAGE,
					       "resigns with one point",
                                               "resigns with %u points",
					       resign->value), 
				  resign->value);
}

void
gibbon_match_list_add_game (GibbonMatchList *self, const GibbonGame *game)
{
        const GibbonPosition *pos;
        const gchar *comment;
        gchar *text;
        GtkTreeIter iter;
        gint gameno;

        g_return_if_fail (GIBBON_IS_MATCH_LIST (self));
        g_return_if_fail (GIBBON_IS_GAME (game));

        pos = gibbon_game_get_initial_position (game);
        comment = gibbon_game_is_crawford (game) ?
                        _("(Crawford)") : "";
        gtk_list_store_append (self->priv->games, &iter);
        gameno = gtk_tree_model_iter_n_children (
                        GTK_TREE_MODEL (self->priv->games), NULL);
        text = g_strdup_printf (_("Game %d: %llu-%llu %s"),
                                gameno,
                                (unsigned long long) pos->scores[1],
                                (unsigned long long) pos->scores[0],
                                comment);
        gtk_list_store_set (self->priv->games, &iter,
                            0, text, -1);
        g_free (text);
}
