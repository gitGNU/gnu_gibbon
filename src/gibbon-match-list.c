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
        SELECT_GAME,
        GAME_SELECTED,
        LOAD_MATCH,
        MATCH_LOADED,
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

        gibbon_match_list_signals[LOAD_MATCH] =
                g_signal_new ("load-match",
                              G_TYPE_FROM_CLASS (klass),
                              G_SIGNAL_RUN_FIRST,
                              0,
                              NULL, NULL,
                              g_cclosure_marshal_VOID__OBJECT,
                              G_TYPE_NONE,
                              1,
                              G_TYPE_OBJECT);
        gibbon_match_list_signals[MATCH_LOADED] =
                g_signal_new ("match-loaded",
                              G_TYPE_FROM_CLASS (klass),
                              G_SIGNAL_RUN_FIRST,
                              0,
                              NULL, NULL,
                              g_cclosure_marshal_VOID__OBJECT,
                              G_TYPE_NONE,
                              1,
                              G_TYPE_OBJECT);
        gibbon_match_list_signals[SELECT_GAME] =
                g_signal_new ("select-game",
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
gibbon_match_list_set_match (GibbonMatchList *self, GibbonMatch *match)
{
        gsize i, num_games;
        GtkTreeIter iter;
        gchar *text;
        const GibbonGame *game;
        const GibbonPosition *pos;
        gchar *comment;

        g_return_if_fail (GIBBON_IS_MATCH_LIST (self));

        g_signal_emit (self, gibbon_match_list_signals[LOAD_MATCH], 0, self);

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

        g_signal_emit (self, gibbon_match_list_signals[MATCH_LOADED], 0, self);
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

        g_signal_emit (self, gibbon_match_list_signals[SELECT_GAME], 0, self);
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

static gboolean
gibbon_match_list_add_action (GibbonMatchList *self, GibbonGame *game,
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
        gint colno;
        gchar *text;
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
        const gchar *sup;

        action = gibbon_game_get_nth_action (game, action_no, &side);
        if (action_no)
                last_action = gibbon_game_get_nth_action (game, action_no - 1,
                                                          NULL);
        pos = gibbon_game_get_nth_position (game, action_no);
        analysis = gibbon_game_get_nth_analysis (game, action_no);

        moveno = gtk_tree_model_iter_n_children (
                        GTK_TREE_MODEL (self->priv->moves), NULL);

        /*
         * Always insert a dummy row if the first action of a game is not a
         * position setup.
         */
        if (!action_no && !GIBBON_IS_SETUP (action)) {
                gtk_list_store_append (self->priv->moves, &iter);
                gtk_list_store_set (self->priv->moves, &iter,
                                    GIBBON_MATCH_LIST_COL_MOVENO, 0,
                                    GIBBON_MATCH_LIST_COL_MOVE,
                                    _("Initial position"),
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
                        player = gibbon_match_get_black (self->priv->match);
                else if (side > 0)
                        player = gibbon_match_get_white (self->priv->match);
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
        } else if (GIBBON_IS_SETUP (action)) {
                text = g_strdup (_("Position set up"));
                colno = GIBBON_MATCH_LIST_COL_MOVE;
        }

        if (analysis && GIBBON_IS_ANALYSIS_MOVE (analysis)) {
                ma = GIBBON_ANALYSIS_MOVE (analysis);
                if (ma->da) {
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
                }

                if (ma->ma) {
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

        formatted = g_string_new_len ("", 80);
        g_string_printf (formatted, "%s%s%s", open_tag, text, close_tag);
        g_free (text);

        if (dbl_mark) {
                if (GIBBON_IS_MOVE (action))
                        sup = "\xc2\xb2"; /* Superscript two.  */
                else
                        sup = "";
                g_string_append_printf (formatted, " %s%s%s%s",
                                        open_tag, dbl_mark, close_tag, sup);

        }

        if (chk_mark) {
                if (GIBBON_IS_MOVE (action))
                        sup = "\xc2\xb2"; /* Superscript two.  */
                else
                        sup = "";
                g_string_append_printf (formatted, " %s%s%s%s",
                                        open_tag, chk_mark, close_tag, sup);

        }

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
                return g_strdup_printf (_("redoubles to %u"), pos->cube << 1);
        else
                return g_strdup_printf (_("doubles to %u"), pos->cube << 1);
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
                return g_strdup_printf (_("resigns with %u points"),
                                        resign->value);
}

const GibbonMatch *
gibbon_match_list_get_match (const GibbonMatchList *self)
{
        g_return_val_if_fail (GIBBON_IS_MATCH_LIST (self), NULL);

        return self->priv->match;
}
