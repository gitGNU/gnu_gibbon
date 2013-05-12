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

#include <stdlib.h>

#include <glib.h>
#include <glib/gi18n.h>

#include "gibbon-match.h"
#include "gibbon-game.h"
#include "gibbon-position.h"
#include "gibbon-game-actions.h"
#include "gibbon-match-play.h"
#include "gibbon-util.h"

typedef struct _GibbonMatchPrivate GibbonMatchPrivate;
struct _GibbonMatchPrivate {
        GList *games;

        gchar *white;
        gchar *black;
        gchar *wrank;
        gchar *brank;
        gboolean crawford;
        guint64 length;
        gchar *location;

        gboolean debug;
};

#define GIBBON_MATCH_PRIVATE(obj) (G_TYPE_INSTANCE_GET_PRIVATE ((obj), \
        GIBBON_TYPE_MATCH, GibbonMatchPrivate))

G_DEFINE_TYPE (GibbonMatch, gibbon_match, G_TYPE_OBJECT)

static GSList *_gibbon_match_get_missing_actions (const GibbonMatch *self,
                                                  GibbonPosition *current,
                                                  const GibbonPosition *target,
                                                  gboolean try_move);
static GSList *gibbon_match_try_roll (const GibbonMatch *self,
                                      GibbonPosition *current,
                                      const GibbonPosition *target,
                                      gboolean try_move);
static GSList *gibbon_match_try_accept (const GibbonMatch *self,
                                        GibbonPosition *current,
                                        const GibbonPosition *target);
static GSList *gibbon_match_try_double (const GibbonMatch *self,
                                        GibbonPosition *current,
                                        const GibbonPosition *target);
static GSList *gibbon_match_try_take (const GibbonMatch *self,
                                      GibbonPosition *current,
                                      const GibbonPosition *target);
static GSList *gibbon_match_try_drop (const GibbonMatch *self,
                                      GibbonPosition *current,
                                      const GibbonPosition *target);
static GSList *gibbon_match_try_move (const GibbonMatch *self,
                                      GibbonPosition *current,
                                      const GibbonPosition *target);

static void 
gibbon_match_init (GibbonMatch *self)
{
        self->priv = G_TYPE_INSTANCE_GET_PRIVATE (self,
                GIBBON_TYPE_MATCH, GibbonMatchPrivate);

        self->priv->games = NULL;

        self->priv->white = NULL;
        self->priv->black = NULL;
        self->priv->wrank = NULL;
        self->priv->brank = NULL;
        self->priv->crawford = TRUE;
        self->priv->length = 0;
        self->priv->location = NULL;

        self->priv->debug = FALSE;
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
        g_free (self->priv->wrank);
        g_free (self->priv->brank);
        g_free (self->priv->location);

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
                  gsize length, gboolean crawford)
{
        GibbonMatch *self = g_object_new (GIBBON_TYPE_MATCH, NULL);

        if (!length)
                crawford = FALSE;
        self->priv->crawford = crawford;

        gibbon_match_set_white (self, white);
        gibbon_match_set_black (self, black);
        gibbon_match_set_length (self, length);

        self->priv->debug = gibbon_debug ("match-tracking");

        return self;
}

GibbonGame *
gibbon_match_get_current_game (const GibbonMatch *self)
{
        GList *iter;

        g_return_val_if_fail (GIBBON_IS_MATCH (self), NULL);

        iter = g_list_last (self->priv->games);
        if (!iter)
                return NULL;

        return iter->data;
}

GibbonGame *
gibbon_match_add_game (GibbonMatch *self, GError **error)
{
        GibbonGame *game;
        GibbonPosition *position;
        gboolean is_crawford = FALSE;
        gint white_away, black_away;
        const GibbonPosition *last_position;
        GList *iter;

        gibbon_return_val_if_fail (GIBBON_IS_MATCH (self), NULL, error);

        iter = g_list_last (self->priv->games);

        if (iter) {
                game = iter->data;
                position = gibbon_position_copy (gibbon_game_get_position (game));
                gibbon_position_reset (position);
        } else {
                position = gibbon_position_new ();
                position->players[0] = g_strdup (self->priv->white);
                position->players[1] = g_strdup (self->priv->black);
                position->match_length = self->priv->length;
        }

        if (position->match_length) {
                if (position->scores[0] >= position->match_length
                    || position->scores[1] >= position->match_length) {
                        g_set_error_literal (error, GIBBON_ERROR,
                                             GIBBON_MATCH_ERROR_END_OF_MATCH,
                                             _("Match is already over!"));
                        gibbon_position_free (position);
                        return NULL;
                }
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
        game = gibbon_match_get_current_game (self);
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
        game = gibbon_game_new (self, position,
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

        game = gibbon_match_get_current_game (self);
        if (!game) return NULL;

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
}

const gchar *
gibbon_match_get_white (const GibbonMatch *self)
{
        g_return_val_if_fail (GIBBON_IS_MATCH (self), NULL);

        return self->priv->white;
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
}
const gchar *
gibbon_match_get_black (const GibbonMatch *self)
{
        g_return_val_if_fail (GIBBON_IS_MATCH (self), NULL);

        return self->priv->black;
}

void
gibbon_match_set_rank (GibbonMatch *self, GibbonPositionSide side,
                       const gchar *rank)
{
        g_return_if_fail (GIBBON_IS_MATCH (self));
        g_return_if_fail (side != GIBBON_POSITION_SIDE_NONE);

        if (side < 0) {
                g_free (self->priv->brank);
                self->priv->brank = g_strdup (rank);
        } else {
                g_free (self->priv->wrank);
                self->priv->wrank = g_strdup (rank);
        }
}

const gchar *
gibbon_match_get_rank (const GibbonMatch *self, GibbonPositionSide side)
{
        g_return_val_if_fail (GIBBON_IS_MATCH (self), NULL);
        g_return_val_if_fail (side != GIBBON_POSITION_SIDE_NONE, NULL);

        if (side < 0)
                return self->priv->brank;
        else
                return self->priv->wrank;
}

void
gibbon_match_set_length (GibbonMatch *self, gsize length)
{
        GList *iter;
        GibbonGame *game;

        g_return_if_fail (GIBBON_IS_MATCH (self));

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

void
gibbon_match_set_location (GibbonMatch *self, const gchar *location)
{
        g_return_if_fail (GIBBON_IS_MATCH (self));

        g_free (self->priv->location);

        /* g_strdup () returns NULL if the argument is NULL.  */
        self->priv->location = g_strdup (location);
}

const gchar *
gibbon_match_get_location (const GibbonMatch *self)
{
        g_return_val_if_fail (GIBBON_IS_MATCH (self), 0);

        return self->priv->location;
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

gboolean
gibbon_match_get_crawford (const GibbonMatch *self)
{
        g_return_val_if_fail (GIBBON_IS_MATCH (self), FALSE);

        return self->priv->crawford;
}

void
gibbon_match_set_crawford (GibbonMatch *self, gboolean crawford)
{
        g_return_if_fail (GIBBON_IS_MATCH (self));

        if (self->priv->length && crawford)
                self->priv->crawford = crawford;
        else
                self->priv->crawford = FALSE;
}

gboolean
gibbon_match_add_action (GibbonMatch *self, GibbonPositionSide side,
                         GibbonGameAction *action, gint64 timestamp,
                         GError **error)
{
        GibbonGame *game;
        const GibbonPosition *current;

        gibbon_return_val_if_fail (GIBBON_IS_MATCH (self), FALSE, error);
        gibbon_return_val_if_fail (GIBBON_IS_GAME_ACTION (action), FALSE,
                                         error);

        game = gibbon_match_get_current_game (self);
        if (!game) {
                game = gibbon_match_add_game (self, error);
                if (!game)
                        return FALSE;
        }

        if (gibbon_game_over (game)) {
                current = gibbon_game_get_position (game);
                if (gibbon_position_match_over (current)) {
                        g_set_error_literal (error, GIBBON_ERROR,
                                             GIBBON_MATCH_ERROR_END_OF_MATCH,
                                             "The match is already over.");
                        return FALSE;
                }

                game = gibbon_match_add_game (self, error);
                if (!game)
                        return FALSE;
        }

        if (!gibbon_game_add_action (game, side, action, timestamp, error))
                return FALSE;

        return TRUE;
}

/**
 * gibbon_match_get_missing_actions:
 * @self: The incomplete #GibbonMatch.
 * @target: The position to achieve.
 * @result: A #GSList that holds the missing actions.
 *
 * Recording online matches can lead to gaps if the connection to the server
 * gets lost after the server has recorded a new action in the match but
 * before the client got the notification.  This function tries to guess
 * missing actions by comparing the last recorded position with the position
 * saved on the server.
 *
 * If both a roll and a move is missing in the sequence, the function has
 * a lot to do.  For every possible roll it has to calculate all possible
 * moves and compare them to the target position.  It will do that only
 * once, and give up if the target position cannot be achieved after
 * one try.
 *
 * It lies in the nature of the problem that certain sequences of game actions
 * cannot be recovered, for example a rejected resignation or a roll, move,
 * roll, move sequence, when both opponents dance.  But this is only
 * significant if you want to measure the luck factor.
 *
 * You can safely call this function if the target position is the current
 * position in the match.  In that case, %TRUE is returned, and the output
 * list @result will be empty.
 *
 * Returns: %TRUE for success, %FALSE for failure.
 */
gboolean
gibbon_match_get_missing_actions (const GibbonMatch *self,
                                  const GibbonPosition *target,
                                  GSList **_result)
{
        GSList *result;
        const GibbonPosition *last_pos;
        GibbonPosition *current;

        g_return_val_if_fail (GIBBON_IS_MATCH (self), FALSE);
        g_return_val_if_fail (target != NULL, FALSE);

        last_pos = gibbon_match_get_current_position (self);
        if (last_pos) {
                if (gibbon_position_equals_technically (last_pos, target)) {
                        *_result = NULL;
                        return TRUE;
                }

                /*
                 * Pre-flight check.  We sort out match pairs that obviously do
                 * not fit as well as hopeless cases.
                 */
                if (g_strcmp0 (target->players[0], last_pos->players[0]))
                        return FALSE;
                if (g_strcmp0 (target->players[1], last_pos->players[1]))
                        return FALSE;
                if (target->match_length != last_pos->match_length)
                        return FALSE;
                if (target->scores[0] != last_pos->scores[0]
                    && target->scores[1] != last_pos->scores[1])
                        return FALSE;
                if (target->scores[0] < last_pos->scores[0])
                        return FALSE;
                if (target->scores[1] < last_pos->scores[1])
                        return FALSE;
                if (target->scores[0] - last_pos->scores[0]
                    > 6 * last_pos->cube)
                        return FALSE;
                if (target->scores[1] - last_pos->scores[1]
                    > 6 * last_pos->cube)
                        return FALSE;

                current = gibbon_position_copy (last_pos);
        } else {
                /* No last position.  That means that we are at the beginning
                 * of the match.
                 */
                last_pos = gibbon_position_initial ();
                if (gibbon_position_equals_technically (last_pos, target)) {
                        *_result = NULL;
                        return TRUE;
                }
                current = gibbon_position_copy (last_pos);
                current->match_length = target->match_length;
        }

        result = _gibbon_match_get_missing_actions (self, current,
                                                    target, TRUE);
        gibbon_position_free (current);

        if (result) {
                if (_result)
                        *_result = g_slist_reverse (result);
                return TRUE;
        }

        return FALSE;
}

static GSList *
_gibbon_match_get_missing_actions (const GibbonMatch *self,
                                   GibbonPosition *current,
                                   const GibbonPosition *target,
                                   gboolean try_move)
{
        GSList *retval = NULL;
        GSList *head;
        GTimeVal timeval;
        struct tm *now;

        g_return_val_if_fail (GIBBON_IS_MATCH (self), FALSE);
        g_return_val_if_fail (target != NULL, FALSE);

        if (gibbon_position_game_over (current)) {
                gibbon_position_reset (current);
                /*
                 * FIXME! We must check for the crawford game here, and see
                 * whether we have to adjust the may_double flags.
                 */
        }

        if (current->resigned) {
                retval = gibbon_match_try_accept (self, current, target);
        } else if (current->cube_turned) {
                retval = gibbon_match_try_take (self, current, target);
                if (!retval)
                        retval = gibbon_match_try_drop (self, current, target);
        } else if (!current->dice[0]) {
                retval = gibbon_match_try_double (self, current, target);
                if (!retval) {
                        retval = gibbon_match_try_roll (self, current, target,
                                                        try_move);
                        try_move = FALSE;
                }
        } else {
                /*
                 * Handle the case after an initial opening double.
                 */
                if (!current->turn) {
                        retval = gibbon_match_try_roll (self, current, target,
                                                        try_move);
                } else if (!retval && try_move) {
                        retval = gibbon_match_try_move (self, current, target);
                        try_move = FALSE;
                }
        }

        if (!retval && self->priv->debug) {
                g_get_current_time (&timeval);
                now = localtime ((time_t *) &timeval.tv_sec);
                g_printerr ("[%02d:%02d:%02d.%06ld] Got stuck here:\n",
                           now->tm_hour, now->tm_min, now->tm_sec,
                           timeval.tv_usec);
                gibbon_position_dump (current);
        }

        if (!retval)
                return NULL;

        if (gibbon_position_equals_technically (current, target))
                return retval;

        head = _gibbon_match_get_missing_actions (self, current, target,
                                                  try_move);

        if (!head) {
                g_slist_free_full (retval,
                                   (GDestroyNotify) gibbon_match_play_free);
                return NULL;
        }

        return g_slist_concat (head, retval);
}

static GSList *
gibbon_match_try_roll (const GibbonMatch *self,
                       GibbonPosition *current,
                       const GibbonPosition *target,
                       gboolean try_move)
{
        GibbonGameAction *action;
        gboolean must_move;
        gboolean white_moved, black_moved;
        guint die1, die2;
        GibbonMove *move;
        GibbonMatchPlay *play;
        GSList *retval = NULL;
        gboolean reverse;

        /*
         * When the recorded match was completely empty, it is a little bit
         * hard to deduce who must have won the first roll.  We rely on
         * the caller to check before whether the very first action was an
         * unanswered resignation offer.
         *
         * If both of the opponents have moved already, we cannot fill the
         * missing actions anyway.  If exactly one opponent has moved at
         * least one checker we can safely assume that this opponent won
         * the opening roll.
         *
         * If none of the opponents have moved, the first roll can be
         * read from the target position.
         */
        if (!current->turn) {
                if (target->points[5] == 5 && target->points[7] == 3
                    && target->points[12] == 5 && target->points[23] == 2)
                        white_moved = FALSE;
                else
                        white_moved = TRUE;
                if (target->points[0] == -2 && target->points[11] == -5
                    && target->points[16] == -3 && target->points[18] == -5)
                        black_moved = FALSE;
                else
                        black_moved = TRUE;

                if (white_moved && black_moved)
                        return NULL;

                if (white_moved)
                        current->turn = GIBBON_POSITION_SIDE_WHITE;
                else if (black_moved)
                        current->turn = GIBBON_POSITION_SIDE_BLACK;
                else {
                        /*
                         * The roll will be copied from the target position.
                         */
                        if (target->dice[0] > target->dice[1])
                                current->turn = GIBBON_POSITION_SIDE_WHITE;
                        else if (target->dice[0] < target->dice[1])
                                current->turn = GIBBON_POSITION_SIDE_BLACK;
                        else
                                current->turn = GIBBON_POSITION_SIDE_NONE;

                        /*
                         * And ignore the may double flags in the comparison.
                         */
                        current->may_double[0] = target->may_double[0];
                        current->may_double[1] = target->may_double[1];
                }
        }

        if (memcmp (current->points, target->points, sizeof current->points))
                must_move = TRUE;
        else
                must_move = FALSE;

        /*
         * Can we just copy the dice from the target position? We have to make
         * sure that this is not really a case of "cannot move".
         */
        if (!must_move && target->dice[0] && target->dice[1]
            && target->turn == current->turn) {
                current->dice[0] = target->dice[0];
                current->dice[1] = target->dice[1];

                action = GIBBON_GAME_ACTION (gibbon_roll_new (target->dice[0],
                                                              target->dice[1]));
                return g_slist_prepend (NULL,
                                        gibbon_match_play_new (action,
                                                               current->turn));
        }

        if (!try_move)
                return NULL;

        /*
         * First try non-doubles.  They are more likely and easier to
         * process.
         */
        reverse = current->turn < 0 ? TRUE : FALSE;
        for (die1 = 6; die1 > 1; --die1) {
                for (die2 = die1 - 1; die2 > 0; --die2) {
                        current->dice[0] = die1;
                        current->dice[1] = die2;
                        move = gibbon_position_check_move (current, target,
                                                           current->turn);
                        if (move->status != GIBBON_MOVE_LEGAL) {
                                g_object_unref (move);
                                continue;
                        }
                        if (!gibbon_position_apply_move (current, move,
                                                         current->turn,
                                                         reverse))
                                return NULL;
                        /*
                         * gibbon_position_apply_move() has already swapped
                         * the sides! */
                        play = gibbon_match_play_new (GIBBON_GAME_ACTION (
                                        gibbon_roll_new (die1, die2)),
                                        -current->turn);
                        retval = g_slist_prepend (NULL, play);
                        play = gibbon_match_play_new (GIBBON_GAME_ACTION (move),
                                                      -current->turn);
                        retval = g_slist_prepend (retval, play);
                        break;
                }
                if (retval)
                        break;
        }
        for (die1 = 6; !retval && die1 > 0; --die1) {
                current->dice[0] = die1;
                current->dice[1] = die1;
                move = gibbon_position_check_move (current, target, current->turn);
                if (move->status != GIBBON_MOVE_LEGAL) {
                        g_object_unref (move);
                        continue;
                }
                if (!gibbon_position_apply_move (current, move,
                                                 current->turn, reverse))
                        return FALSE;
                /*
                 * gibbon_position_apply_move() has already swapped
                 * the sides! */
                play = gibbon_match_play_new (GIBBON_GAME_ACTION (
                                gibbon_roll_new (die1, die1)),
                                -current->turn);
                retval = g_slist_prepend (NULL, play);
                play = gibbon_match_play_new (GIBBON_GAME_ACTION (move),
                                              -current->turn);
                retval = g_slist_prepend (retval, play);
                break;
        }

        return retval;
}

static GSList *
gibbon_match_try_move (const GibbonMatch *self,
                       GibbonPosition *current,
                       const GibbonPosition *target)
{
        GibbonMove *move;
        GibbonMatchPlay *play;
        GSList *retval = NULL;
        gboolean reverse;

        if (!current->dice[0] || !current->dice[1])
                return NULL;

        move = gibbon_position_check_move (current, target, current->turn);
        if (move->status != GIBBON_MOVE_LEGAL) {
                g_object_unref (move);
                return NULL;
        }
        reverse = current->turn < 0 ? TRUE : FALSE;
        if (!gibbon_position_apply_move (current, move, current->turn,
                                         reverse))
                return NULL;

        /* Gibbon_position_apply_move() has already swapped the sides! */
        play = gibbon_match_play_new (GIBBON_GAME_ACTION (move),
                                      -current->turn);
        retval = g_slist_prepend (NULL, play);

        return retval;
}

static GSList *
gibbon_match_try_accept (const GibbonMatch *self,
                         GibbonPosition *current,
                         const GibbonPosition *target)
{
        GibbonGameAction *action;
        GibbonPositionSide side;

        if (current->resigned > 0) {
                if (current->scores[0] != target->scores[0]
                    || (current->scores[1] + current->resigned
                        != target->scores[1]))
                        return NULL;
                side = GIBBON_POSITION_SIDE_WHITE;
        } else if (current->resigned < 0) {
                if (current->scores[1] != target->scores[1]
                    || (current->scores[0] - current->resigned
                        != target->scores[0]))
                        return NULL;
                side = GIBBON_POSITION_SIDE_BLACK;
        } else {
                return NULL;
        }

        current->turn = GIBBON_POSITION_SIDE_NONE;
        current->scores[0] = target->scores[0];
        current->scores[1] = target->scores[1];
        current->score = current->resigned;

        action = GIBBON_GAME_ACTION (gibbon_accept_new ());

        return g_slist_prepend (NULL, gibbon_match_play_new (action, -side));
}

static GSList *
gibbon_match_try_double (const GibbonMatch *self,
                         GibbonPosition *current,
                         const GibbonPosition *target)
{
        GibbonGameAction *action;
        GibbonPositionSide side;

        if (current->dice[0] || current->dice[1])
                return NULL;

        if (current->turn < 0) {
                if (!current->may_double[1])
                        return NULL;
                if (target->cube_turned) {
                        if (target->cube_turned != GIBBON_POSITION_SIDE_BLACK)
                                return NULL;
                } else if (target->cube == 1) {
                        if (current->scores[1] + current->cube
                                        != target->scores[1]
                            || current->scores[0] != target->scores[0])
                                return NULL;
                } else if (current->cube << 1 != target->cube) {
                        return NULL;
                }
                side = GIBBON_POSITION_SIDE_BLACK;
        } else if (current->turn > 0) {
                if (!current->may_double[0])
                        return NULL;
                if (target->cube_turned) {
                        if (target->cube_turned != GIBBON_POSITION_SIDE_WHITE)
                                return NULL;
                } else if (target->cube == 1) {
                        if (current->scores[0] + current->cube
                                        != target->scores[0]
                            || current->scores[1] != target->scores[1])
                                return NULL;
                } else if ((current->cube << 1) != target->cube) {
                        return NULL;
                }
                side = GIBBON_POSITION_SIDE_WHITE;
        } else {
                return NULL;
        }

        current->cube_turned = side;

        action = GIBBON_GAME_ACTION (gibbon_double_new ());

        return g_slist_prepend (NULL, gibbon_match_play_new (action, side));
}

static GSList *
gibbon_match_try_take (const GibbonMatch *self,
                       GibbonPosition *current,
                       const GibbonPosition *target)
{
        GibbonGameAction *action;
        GibbonPositionSide side;

        if (current->cube << 1 != target->cube)
                return NULL;

        if (current->cube_turned < 0) {
                current->may_double[0] = TRUE;
                current->may_double[1] = FALSE;
                side = GIBBON_POSITION_SIDE_WHITE;
        } else if (current->cube_turned > 0) {
                current->may_double[0] = FALSE;
                current->may_double[1] = TRUE;
                side = GIBBON_POSITION_SIDE_BLACK;
        } else {
                return NULL;
        }

        current->cube_turned = GIBBON_POSITION_SIDE_NONE;
        current->cube <<= 1;

        action = GIBBON_GAME_ACTION (gibbon_take_new ());

        return g_slist_prepend (NULL, gibbon_match_play_new (action, side));
}

static GSList *
gibbon_match_try_drop (const GibbonMatch *self,
                       GibbonPosition *current,
                       const GibbonPosition *target)
{
        GibbonGameAction *action;
        GibbonPositionSide side;

        if (current->cube_turned < 0) {
                side = GIBBON_POSITION_SIDE_WHITE;
                current->scores[1] += current->cube;
                current->score = -current->cube;
        } else if (current->cube_turned > 0) {
                side = GIBBON_POSITION_SIDE_BLACK;
                current->scores[0] += current->cube;
                current->score = +current->cube;
        } else {
                return NULL;
        }

        action = GIBBON_GAME_ACTION (gibbon_drop_new ());

        return g_slist_prepend (NULL, gibbon_match_play_new (action, side));
}

gint64
gibbon_match_get_start_time (const GibbonMatch *self)
{
        const GibbonGame *game;

        g_return_val_if_fail (GIBBON_IS_MATCH (self), G_MININT64);

        game = gibbon_match_get_nth_game (self, 0);
        if (!game)
                return G_MININT64;

        return gibbon_game_get_nth_timestamp (game, 0);
}

guint
gibbon_match_get_white_score (const GibbonMatch *self)
{
        const GibbonPosition *position;

        g_return_val_if_fail (GIBBON_IS_MATCH (self), 0);

        position = gibbon_match_get_current_position (self);
        g_return_val_if_fail (position != NULL, 0);

        return position->scores[0];
}

guint
gibbon_match_get_black_score (const GibbonMatch *self)
{
        const GibbonPosition *position;

        g_return_val_if_fail (GIBBON_IS_MATCH (self), 0);

        position = gibbon_match_get_current_position (self);
        g_return_val_if_fail (position != NULL, 0);

        return position->scores[1];
}

void
gibbon_match_set_start_time (GibbonMatch *self, gint64 timestamp)
{
        GibbonGame *game;

        g_return_if_fail (GIBBON_IS_MATCH (self));

        game = gibbon_match_get_nth_game (self, 0);
        if (!game)
                return;

        gibbon_game_set_start_time (game, timestamp);
}

GibbonPositionSide
gibbon_match_over (const GibbonMatch *self)
{
        const GibbonPosition *position;

        g_return_val_if_fail (GIBBON_IS_MATCH (self),
                              GIBBON_POSITION_SIDE_NONE);
        position = gibbon_match_get_current_position (self);
        if (!position)
                return GIBBON_POSITION_SIDE_NONE;

        return gibbon_position_match_over (position);
}
