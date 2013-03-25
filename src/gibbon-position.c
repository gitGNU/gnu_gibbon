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
 * SECTION:gibbon-position
 * @short_description: Boxed type for a position in backgammon.
 *
 * Since: 0.1.0
 *
 * This structure holds all information needed to display a
 * normalized backgammon position.
 *
 * If X is black, and O is white, the normalized starting position of a
 * game of backgammon looks like this:
 *
 * <programlisting>
 *  +13-14-15-16-17-18-------19-20-21-22-23-24-+ X: black
 *  | O           X    |   |  X              O |
 *  | O           X    |   |  X              O |
 *  | O           X    |   |  X                |
 *  | O                |   |  X                |
 *  | O                |   |  X                |
 * v|                  |BAR|                   |
 *  | X                |   |  O                |
 *  | X                |   |  O                |
 *  | X           O    |   |  O                |
 *  | X           O    |   |  O              X |
 *  | X           O    |   |  O              X |
 *  +12-11-10--9--8--7--------6--5--4--3--2--1-+ O: white
 * </programlisting>
 *
 * Translated into the checkers array of a #GibbonPosition the image
 * looks like this:
 *
 * <programlisting>
 *  +12-13-14-15-16-17-------18-19-20-21-22-23-+ negative: black or X
 *  |+5          -3    |   | -5             +2 |
 * v|                  |BAR|                   |
 *  |-5          +3    |   | +5             -2 |
 *  +11-10--9--8--7--6--------5--4--3--2--1--0-+ positive: white or O
 * </programlisting>
 */

#include <stdlib.h>

#include <glib.h>
#include <glib/gi18n.h>

#include "gibbon-position.h"
#include "gibbon-util.h"
#include "gibbon-move.h"

G_DEFINE_BOXED_TYPE (GibbonPosition, gibbon_position,            \
                     gibbon_position_copy, gibbon_position_free)

static GibbonPosition initial = {
                /* match_length */
                0,
                /* points */
                { -2,  0,  0,  0,  0,  5,  0,  3,  0,  0,  0, -5,
                   5,  0,  0,  0, -3,  0, -5,  0,  0,  0,  0,  2 },
                /* bar */
                { 0, 0 },
                /* dice */
                { 0, 0 },
                /* cube */
                1,
                /* may double, cube_turned */
                { TRUE, TRUE }, GIBBON_POSITION_SIDE_NONE,
                /* scores, turn */
                { 0, 0 }, GIBBON_POSITION_SIDE_NONE,
                /* resigned, resignation_accepted */
                0, 0,
                /* dice_swapped */
                FALSE,
                /* players */
                { NULL, NULL },
                /* unused dice */
                { 0, 0, 0, 0 },
                /* game_info */
                NULL,
                /* status */
                NULL
};

/* Move patterns describe how a double roll was used, when we know the set
 * of starting points.
 *
 * For decoding, they have to be split into bytes first.  Each byte is then
 * split into nibbles.  The left nibble gives the index into the set of
 * starting points, and the right number gives the number of dice values
 * to use.
 *
 * For example, if we know that the user has moved checkers from points
 * 6, 9, and 20, and has rolled a double 2, a 0x12 means that he has
 * moved two checkers from the 9-point (9 is point number 1 in the set
 * 6, 9, and 20).
 */
guint move_patterns1[] = {
                /* Using all four numbers.  */
                0x04,
                0x0103,
                0x0202,
                0x010102,
                0x01010101,

                /* Using three out of four numbers.  The pattern 0x0201
                 * seems to be missing here but it is not.  There is no
                 * such legal move.
                 */
                0x03,
                0x010101,

                /* Using two out of four numbers.  */
                0x02,
                0x0101,

                /* Using one number.  */
                0x01
};

guint move_patterns2[] = {
                /* Using all four numbers.  */
                0x1103,
                0x1202,
                0x1301,
                0x110201,
                0x111201,
                0x111102,
                0x120101,
                0x11010101,
                0x11110101,
                0x11111101,

                /* Using three out of four numbers.  */
                0x110101,
                0x111101,
                0x1102,
                0x1201,

                /* Using two out of four numbers.  */
                0x1101
};

guint move_patterns3[] = {
                /* Using all four numbers.  */
                0x211102,
                0x211201,
                0x221101,
                0x21110101,
                0x21111101,
                0x21211101,

                /* Using three out of four numbers.  */
                0x211101
};

guint move_patterns4[] = {
                0x31211101
};

#if (0)
static void dump_move (const GibbonMove *move);
#endif

static void gibbon_position_fill_movement (GibbonMove *move,
                                           guint point, guint die);
static GList *gibbon_position_find_double (const gint *before,
                                           const gint *after,
                                           guint die,
                                           gsize num_froms,
                                           const guint *froms);
static GList *gibbon_position_find_non_double (const gint *before,
                                               const gint *after,
                                               guint die1, guint die2,
                                               gsize num_froms,
                                               const guint *froms);
static gboolean gibbon_position_is_diff (const gint before[26],
                                         const gint after[26],
                                         GibbonMove *move);
static gboolean gibbon_position_can_move (const gint board[26], gint die);
static gboolean gibbon_position_can_move_checker (const gint board[26],
                                                  gint point,
                                                  gint die, gint backmost);
static gboolean gibbon_position_can_move2 (gint board[26],
                                           gint die1, gint die2);
static gint find_backmost_checker (const gint board[26]);
static void swap_movements (GibbonMovement *m1, GibbonMovement *m2);
static void order_movements (GibbonMove *move);

/**
 * gibbon_position_new:
 *
 * Creates a new #GibbonPosition, set to the backgammon starting position.
 * Both player names are %NULL, the cube is at 0, and both dice are set
 * to 0.  The @may_double flag is %FALSE.
 *
 * Returns: The newly created #GibbonPosition or %NULL in case of failure.
 */
GibbonPosition *
gibbon_position_new (void)
{
        GibbonPosition *self = g_malloc (sizeof initial);

        *self = initial;

        return self;
}

/**
 * gibbon_position_free:
 *
 * Free all resources associated with the #GibbonPosition.  Note that this
 * function calls g_free() with the player names if not %NULL.
 */
void
gibbon_position_free (GibbonPosition *self)
{
        if (self) {
                g_free (self->players[0]);
                g_free (self->players[1]);
                g_free (self->game_info);
                g_free (self->status);
                g_free (self);
        }
}

/**
 * gibbon_position_copy:
 * @self: the original #GibbonPosition.
 *
 * Creates an exact copy of @self.  If player names were set, they are
 * identical in the copy but will use new buffers that can be freed with
 * g_free().
 *
 * Returns: The copied #GibbonPosition or %NULL if @self was %NULL;
 */
GibbonPosition *
gibbon_position_copy (const GibbonPosition *self)
{
        GibbonPosition *copy;

        g_return_val_if_fail (self != NULL, NULL);

        copy = g_malloc (sizeof *self);
        *copy = *self;
        copy->players[0] = g_strdup (copy->players[0]);
        copy->players[1] = g_strdup (copy->players[1]);
        copy->game_info = g_strdup (copy->game_info);
        copy->status = g_strdup (copy->status);

        return copy;
}

guint
gibbon_position_get_borne_off (const GibbonPosition *self,
                               GibbonPositionSide side)
{
        gint checkers = 15;
        guint i;

        if (!side)
                return 0;

        for (i = 0; i < 24; ++i) {
                if (side < 0 && self->points[i] < 0)
                        checkers += self->points[i];
                else if (side > 0 && self->points[i] > 0)
                        checkers -= self->points[i];
        }

        if (side < 0)
                checkers -= self->bar[1];
        else if (side > 0)
                checkers -= self->bar[0];

        if (checkers < 0)
                checkers = 0;

        return checkers;
}

guint
gibbon_position_get_pip_count (const GibbonPosition *self,
                               GibbonPositionSide side)
{
        guint pips = 0;
        guint i;

        if (!side)
                return 0;

        if (side < 0) {
                for (i = 0; i < 24; ++i) {
                        if (self->points[i] < 0)
                                pips -= (24 - i) * self->points[i];
                }
                pips += 25 * self->bar[1];
        } else {
                for (i = 0; i < 24; ++i) {
                        if (self->points[i] > 0)
                                pips += (i + 1) * self->points[i];
                }
                pips += 25 * self->bar[0];
        }

        return pips;
}

GibbonMove *
gibbon_position_check_move (const GibbonPosition *_before,
                            const GibbonPosition *_after,
                            GibbonPositionSide side)
{
        GibbonMove *move, *this_move;
        GList *found;
        gint before[26];
        gint after[26];
        gint i, tmp;
        guint num_froms = 0;
        guint froms[4];
        guint die1, die2, this_die, other_die;
        GList *iter;

        die1 = _before->dice[0];
        die2 = _before->dice[1];
        move = gibbon_move_new (die1, die2, 0);
        move->status = GIBBON_MOVE_ILLEGAL;

        g_return_val_if_fail (die1 != 0, move);
        g_return_val_if_fail (die2 != 0, move);
        g_return_val_if_fail (side != GIBBON_POSITION_SIDE_NONE, move);

        /* This structure is handier for us.  It would probably be easier
         * if we also change GibbonPosition accordingly.
         */
        memcpy (before + 1, _before->points, 24 * sizeof *before);
        memcpy (after + 1, _after->points, 24 * sizeof *after);
        if (side > 0) {
                before[0] = _before->bar[1];
                after[0] = _after->bar[1];
                before[25] = _before->bar[0];
                after[25] = _after->bar[0];
        } else {
                /* "Normalize" the board representation.  Negative
                 * checker counts are ours, positive ones are hers.
                 */
                before[0] = _before->bar[0];
                after[0] = _after->bar[0];
                for (i = 1; i <= 24; ++i) {
                        before[i] = -before[i];
                        after[i] = -after[i];
                }

                /* And swap the direction.  */
                for (i = 1; i <= 12; ++i) {
                        tmp = before[25 - i];
                        before[25 - i] = before[i];
                        before[i] = tmp;
                        tmp = after[25 - i];
                        after[25 - i] = after[i];
                        after[i] = tmp;
                }
                before[25] = _before->bar[1];
                after[25] = _after->bar[1];
        }

        /* Find the number of possible starting points.  */
        for (i = 25; i >= 1; --i) {
                if (after[i] < before[i]) {
                        froms[num_froms++] = i;
                        /* More than four are always illegal.  */
                        if (num_froms > 4) {
                                move->status = GIBBON_MOVE_TOO_MANY_MOVES;
                                return move;
                        }
                }
        }

        /* Find candidate moves.  */
        if (die1 == die2) {
                found = gibbon_position_find_double (before, after,
                                                     die1,
                                                     num_froms, froms);
        } else {
                found = gibbon_position_find_non_double (before, after,
                                                         die1, die2,
                                                         num_froms, froms);
        }

        iter = found;
        while (iter) {
                if (gibbon_position_is_diff (before, after,
                                             (GibbonMove *) iter->data)) {
                        this_move = (GibbonMove *) iter->data;

                        if (this_move->status == GIBBON_MOVE_LEGAL
                            || move->status == GIBBON_MOVE_ILLEGAL) {
                                g_object_unref (move);
                                move = this_move;
                                iter->data = NULL;
                        }

                        /* If this is a bear-off error, we keep on
                         * searching for a possibly legal alternative.
                         */
                        if (this_move->status == GIBBON_MOVE_LEGAL
                            || (move->status != GIBBON_MOVE_PREMATURE_BEAR_OFF
                                && move->status != GIBBON_MOVE_ILLEGAL_WASTE))
                                break;
                }
                iter = iter->next;
        }
        /*
         * We cannot use g_object_unref here because we may have set the
         * data pointer to NULL for certain items.
         */
        g_list_foreach (found, (GFunc) gibbon_safe_object_unref, NULL);
        g_list_free (found);

        if (move->status != GIBBON_MOVE_LEGAL)
                return move;

        /* Dancing? */
        if (after[25]) {
                for (i = 0; i < move->number; ++i) {
                        if (move->movements[i].from != 25) {
                                move->status = GIBBON_MOVE_DANCING;
                                return move;
                        }
                }
        }

        if (die1 != die2) {
                if (move->number == 0) {
                        if (gibbon_position_can_move (after, die1)
                            || gibbon_position_can_move (after, die2)) {
                                move->status = GIBBON_MOVE_USE_ALL;
                                return move;
                        }
                } else if (move->number == 1) {
                        if (move->movements[0].die == die1) {
                                this_die = die1;
                                other_die = die2;
                        } else {
                                this_die = die2;
                                other_die = die1;
                        }
                        if (gibbon_position_can_move (after, other_die)) {
                                move->status = GIBBON_MOVE_USE_ALL;
                                return move;
                        }

                        if (gibbon_position_can_move2 (before, die1, die2)
                            || gibbon_position_can_move2 (before, die2,
                                                          die1)) {
                                move->status = GIBBON_MOVE_TRY_SWAP;
                                return move;
                        }

                        /* It would be a lot more efficient to do this test
                         * before the "try swap" test because it is the
                         * cheaper one.  However, the information that the
                         * numbers used have to be swapped is much better for
                         * the user who moved wrong, and it better matches
                         * the phrasing of the backgammon rules.
                         */
                        if (this_die < other_die
                            && gibbon_position_can_move (before,
                                                         other_die)) {
                                move->status = GIBBON_MOVE_USE_HIGHER;

                                /* Special case: If this was a bear-off, and
                                 * and the checker may be borne off with either
                                 * number then either number has to be accepted.
                                 */
                                if (move->movements[0].to == 0) {
                                        if (this_die >=
                                            find_backmost_checker (before))
                                                move->status =
                                                        GIBBON_MOVE_LEGAL;
                                }
                                return move;
                        }
                }
        } else {
                if (move->number != 4
                    && gibbon_position_can_move (after, die1)) {
                        move->status = GIBBON_MOVE_USE_ALL;
                        return move;
                }
        }

        return move;
}

static gboolean
gibbon_position_can_move (const gint board[26], gint die)
{
        gint i;
        gint backmost;

        /* Dance? */
        if (board[25]) {
                if (board[25 - die] < -1)
                        return FALSE;
                else
                        return TRUE;
        }

        /* Move freely?  */
        for (i = 24; i > die; --i)
                if (board[i] >= 1 && board[i - die] >= -1)
                        return TRUE;

        /* Can we bear-off? */
        backmost = find_backmost_checker (board);
        if (backmost > 6)
                return FALSE;

        /* Direct bear-off? */
        if (board[die] >= 1)
                return TRUE;

        /* Wasteful bear-off? */
        if (die > backmost)
                return TRUE;

        return FALSE;
}

static gboolean
gibbon_position_can_move_checker (const gint board[26], gint point,
                                  gint backmost, gint die)
{
        if (board[point] < 1)
                return FALSE;

        if (point > die) {
                if (board[point - die] >= -1)
                        return TRUE;
                else
                        return FALSE;
        }

        /* Bear-off.  */
        if (backmost > 6)
                return FALSE;

        if (point == die)
                return TRUE;

        if (die > backmost)
                return TRUE;

        return FALSE;
}

static gboolean
gibbon_position_can_move2 (gint board[26], gint die1, gint die2)
{
        gint i;
        gint backmost = 0;
        gboolean can_move;
        gint saved_from, saved_to;

        /* If there are two or more checkers on the bar, we have an early
         * exit because this case was certainly checked already.
         */
        if (board[25] >= 2)
                return FALSE;

        if (board[25] == 1) {
                if (board[25 - die1] < -1)
                        return FALSE;


                board[25] = 0;
                saved_to = board[25 - die1];
                if (saved_to < 0)
                        board[25 - die1] = 0;
                ++board[25 - die1];

                can_move = gibbon_position_can_move_checker (board, 25,
                                                             25, die1 + die2);
                if (!can_move)
                        can_move = gibbon_position_can_move (board, die2);
                board[25] = 1;
                board[25 - die1] = saved_to;
                if (can_move)
                        return TRUE;

                return FALSE;
        }

        for (i = 24; i > die1; --i) {
                if (board[i] <= 0)
                        continue;

                if (!backmost)
                        backmost = i;

                if (!gibbon_position_can_move_checker (board, i,
                                                       backmost, die1))
                        continue;

                saved_from = board[i];
                saved_to = board[i - die1];
                if (saved_to < 0)
                        board[i - die1] = 0;
                --board[i];
                ++board[i - die1];

                can_move = gibbon_position_can_move_checker (board, i, backmost,
                                                             die2);

                /* FIXME! We know already the backmost checker and
                 * that information could be used here.
                 */
                if (!can_move)
                        can_move = gibbon_position_can_move (board, die2);
                board[i] = saved_from;
                board[i - die1] = saved_to;
                if (can_move)
                        return TRUE;
        }

        return FALSE;
}

static gboolean
gibbon_position_is_diff (const gint _before[26], const gint after[26],
                         GibbonMove *move)
{
        gint before[26];
        gint i, from, to;
        const GibbonMovement *movement;
        gint backmost;

        /* dump_move (move); */
        memcpy (before, _before, sizeof before);

        for (i = 0; i < move->number; ++i) {
                movement = move->movements + i;

                from = movement->from;
                if (from <= 0) {
                        /* Actually this move is a duplicate.  */
                        move->status = GIBBON_MOVE_ILLEGAL;
                        return FALSE;
                }

                to = movement->to;

                if (to == 0) {
                        backmost = find_backmost_checker (before);

                        if (backmost > 6)
                                move->status = GIBBON_MOVE_PREMATURE_BEAR_OFF;
                        else if (from != movement->die && from != backmost)
                                move->status = GIBBON_MOVE_ILLEGAL_WASTE;
                        /* Even when this move is illegal, we continue.  If
                         * the resulting position matches, we know that we
                         * intentionally bore off the checker, and we can
                         * inform about the error.
                         */
                } else if (before[to] < -1) {
                        /* Is the target point occupied?  */
                        move->status = GIBBON_MOVE_BLOCKED;
                        return FALSE;
                } else if (before[to] == -1) {
                        before[to] = 0;
                        ++before[0];
                }

                if (to)
                        ++before[to];
                /* Else: No need to know about it.  We count the checkers on
                 * the board instead, and calculate the difference.
                 */
                --before[from];
        }

        if (memcmp (before, after, sizeof before))
                return FALSE;

        /* At this point we know:
         *
         * 1) All intermediate points and the landing point are free.
         * 2) The resulting position exactly reflects the effect of the move.
         *
         * After this, we only have to check these constraints:
         *
         * - No legal moves, while checkers on the bar.
         * - Maximize number of pips used.
         */

        return TRUE;
}

static void
gibbon_position_fill_movement (GibbonMove *move, guint point, guint die)
{
        guint movement_num = move->number;

        move->movements[movement_num].from = point;
        move->movements[movement_num].to = point - die;
        if (move->movements[movement_num].to < 0)
                move->movements[movement_num].to = 0;
        move->movements[movement_num].die = die;
        ++move->number;
}

static GList *
gibbon_position_find_non_double (const gint *before,
                                 const gint *after,
                                 guint _die1, guint _die2,
                                 gsize num_froms, const guint *froms)
{
        GibbonMove *move;
        GList *moves = NULL;
        guint die1, die2;

        if (_die1 < _die2) {
                die1 = _die1;
                die2 = _die2;
        } else {
                die1 = _die2;
                die2 = _die1;
        }

        if (!num_froms) {
                move = gibbon_move_new (die1, die2, 0);
                moves = g_list_append (moves, move);

                return moves;
        }

        /* Two possibilities.  */
        if (2 == num_froms) {
                move = gibbon_move_new (die1, die2, 2);
                gibbon_position_fill_movement (move, froms[0], die1);
                gibbon_position_fill_movement (move, froms[1], die2);
                moves = g_list_append (moves, move);

                move = gibbon_move_new (die1, die2, 2);
                gibbon_position_fill_movement (move, froms[0], die2);
                gibbon_position_fill_movement (move, froms[1], die1);
                moves = g_list_append (moves, move);

                return moves;
        }

        /* Only one checker was moved.  This can happen in five distinct
         * ways.
         */
        if (froms[0] > die1) {
                move = gibbon_move_new (die1, die2, 2);
                gibbon_position_fill_movement (move, froms[0], die1);
                gibbon_position_fill_movement (move, froms[0] - die1, die2);
                moves = g_list_append (moves, move);
        }
        if (froms[0] > die2) {
                move = gibbon_move_new (die1, die2, 2);
                gibbon_position_fill_movement (move, froms[0], die2);
                gibbon_position_fill_movement (move, froms[0] - die2, die1);
                moves = g_list_append (moves, move);
        }

        move = gibbon_move_new (die1, die2, 1);
        gibbon_position_fill_movement (move, froms[0], die1);
        moves = g_list_append (moves, move);

        move = gibbon_move_new (die1, die2, 1);
        gibbon_position_fill_movement (move, froms[0], die2);
        moves = g_list_append (moves, move);

        move = gibbon_move_new (die1, die2, 2);
        gibbon_position_fill_movement (move, froms[0], die1);
        gibbon_position_fill_movement (move, froms[0], die2);
        moves = g_list_append (moves, move);

        return moves;
}

static GList *
gibbon_position_find_double (const gint *before,
                             const gint *after,
                             guint die,
                             gsize num_froms, const guint *froms)
{
        guint *move_patterns;
        GList *moves = NULL;
        gsize i, j, num_patterns;
        GibbonMove *move;
        gsize num_steps;
        guint pattern;
        guint from_index;
        gint from;
        gboolean is_bear_off;

        switch (num_froms) {
                case 0:
                        move = gibbon_move_new (die, die, 0);
                        moves = g_list_append (moves, move);

                        return moves;
                case 1:
                        move_patterns = move_patterns1;
                        num_patterns = (sizeof move_patterns1)
                                        / sizeof *move_patterns1;
                        break;
                case 2:
                        move_patterns = move_patterns2;
                        num_patterns = (sizeof move_patterns2)
                                        / sizeof *move_patterns2;
                        break;
                case 3:
                        move_patterns = move_patterns3;
                        num_patterns = (sizeof move_patterns3)
                                        / sizeof *move_patterns3;
                        break;
                default:
                        move_patterns = move_patterns4;
                        num_patterns = (sizeof move_patterns4)
                                        / sizeof *move_patterns4;
                        break;
        }

        for (i = 0; i < num_patterns; ++i) {
                pattern = move_patterns[i];
                /* This may allocate too much but calculating the correct size
                 * would never pay out.
                 */
                move = gibbon_move_new (die, die, 4);
                is_bear_off = FALSE;

                while (pattern) {
                        num_steps = pattern & 0xf;
                        from_index = (pattern & 0xf0) >> 4;
                        from = froms[from_index];
                        for (j = 0; j < num_steps; ++j) {
                                gibbon_position_fill_movement (move, from, die);
                                from -= die;

                                if (from <= 0)
                                        is_bear_off = TRUE;
                        }

                        pattern >>= 8;
                }

                if (is_bear_off)
                        order_movements (move);

                moves = g_list_append (moves, move);
        }

        return moves;
}

#if (0)
static void
dump_move (const GibbonMove *move)
{
        int i;

        g_printerr ("Move:");
        if (move->status)
                g_printerr (" error %d:", move->status);

        for (i = 0; i < move->number; ++i) {
                g_printerr (" %d/%d",
                            move->movements[i].from,
                            move->movements[i].to);
        }

        g_printerr ("\n");
}
#endif

static gint
find_backmost_checker (const gint board[26])
{
        gint i;

        for (i = 25; i > 0; --i)
                if (board[i] > 0)
                        break;

        if (!i)
                return 26;

        return i;
}

static void
swap_movements (GibbonMovement *m1, GibbonMovement *m2)
{
        GibbonMovement tmp;

        memcpy (&tmp, m1, sizeof tmp);
        memcpy (m1, m2, sizeof *m1);
        memcpy (m2, &tmp, sizeof *m2);
}

static void
order_movements (GibbonMove *move)
{
        gint i;

        for (i = 1; i < move->number; ++i) {
                if (move->movements[i].from > move->movements[i - 1].from)
                        swap_movements (move->movements + i,
                                        move->movements + i - 1);
        }
}

gboolean
gibbon_position_equals_technically (const GibbonPosition *self,
                                    const GibbonPosition *other)
{
#define GIBBON_POSITION_DEBUG_EQUALS_TECHNICALLY 0
#if GIBBON_POSITION_DEBUG_EQUALS_TECHNICALLY
        if (!self && !other)
                g_printerr ("*** !self && !other\n");
#endif
        if (!self && !other)
                return TRUE;
#if GIBBON_POSITION_DEBUG_EQUALS_TECHNICALLY
        if (!self)
                g_printerr ("*** !self\n");
#endif
        if (!self)
                return FALSE;
#if GIBBON_POSITION_DEBUG_EQUALS_TECHNICALLY
        if (!other)
                g_printerr ("*** !other\n");
#endif
        if (!other)
                return FALSE;
        if (memcmp (self, other, (gpointer) &self->players - (gpointer) self))
                return FALSE;

        return TRUE;
}

gboolean
gibbon_position_apply_move (GibbonPosition *self, GibbonMove *move,
                            GibbonPositionSide side, gboolean reverse)
{
        gint m, c;
        gint i, from, to;
        GibbonMovement *movement;
        gboolean from_bar, bear_off;
        guint *my_bar, *her_bar;
        gint score;

        g_return_val_if_fail (side, FALSE);

        if (side > 0)
                side = GIBBON_POSITION_SIDE_WHITE;
        else
                side = GIBBON_POSITION_SIDE_BLACK;

        if (reverse) {
                m = -1;
                c = 24;
        } else {
                m = 1;
                c = -1;
        }

        for (i = 0; i < move->number; ++i) {
                movement = move->movements + i;
                from = m * movement->from + c;

                to = m * movement->to + c;

                /*
                 * Normalize the move on the fly.
                 */
                movement->from = from + 1;
                movement->to = to + 1;

                from_bar = bear_off = FALSE;

                if (side == GIBBON_POSITION_SIDE_WHITE) {
                        g_return_val_if_fail (from <= 24, FALSE);
                        g_return_val_if_fail (from >= 0, FALSE);
                        g_return_val_if_fail (to <= 23, FALSE);
                        g_return_val_if_fail (to >= -1, FALSE);

                        from_bar = from == 24;
                        bear_off = to == -1;
                        my_bar = self->bar + 0;
                        her_bar = self->bar + 1;
                } else {
                        g_return_val_if_fail (from >= -1, FALSE);
                        g_return_val_if_fail (from <= 23, FALSE);
                        g_return_val_if_fail (to >= 0, FALSE);
                        g_return_val_if_fail (to <= 24, FALSE);

                        from_bar = from == -1;
                        bear_off = to == 24;
                        my_bar = self->bar + 1;
                        her_bar = self->bar + 0;
                }

                if (from_bar) {
                        g_return_val_if_fail (*my_bar > 0, FALSE);
                        --(*my_bar);
                } else {
                        g_return_val_if_fail (self->points[from] != 0, FALSE);

                        /* This tests for the same sign.  */
                        g_return_val_if_fail ((side ^ self->points[from]) >= 0,
                                              FALSE);
                        self->points[from] -= side;
                }

                /* If this is a bear-off, it is sufficient to remove the
                 * checker from the source point.
                 */
                if (!bear_off) {
                        if (self->points[to] == 0
                            || (side ^ self->points[to]) >= 0) {
                                self->points[to] += side;
                        } else if (self->points[to] == -side) {
                                ++(*her_bar);
                                self->points[to] = side;
                        } else {
                                g_critical ("gibbon_position_apply_move:"
                                            " Target point is blocked.");
                        }
                }
        }

        self->dice[0] = self->dice[1] = 0;
        self->turn = -side;

        score = gibbon_position_game_over (self);
        if (!score)
                return TRUE;

        self->score = score;

        if (score < 0) {
                self->scores[1] -= score;
        } else if (score > 0) {
                self->scores[0] += score;
        }

        return TRUE;
}

gint
gibbon_position_game_over (const GibbonPosition *position)
{
        guint white_borne_off =
                gibbon_position_get_borne_off (position,
                                               GIBBON_POSITION_SIDE_WHITE);
        guint black_borne_off;
        gint i;
        guint cube = position->cube;

        if (position->score)
                return position->score;

        if (white_borne_off >= 15) {
                black_borne_off =
                        gibbon_position_get_borne_off (position,
                                                    GIBBON_POSITION_SIDE_BLACK);
                if (black_borne_off)
                        return cube;
                if (position->bar[1])
                        return 3 * cube;
                for (i = 0; i < 6; ++i)
                        if (position->points[i])
                                return 3 * cube;
                return 2 * cube;
        }

        black_borne_off =
                gibbon_position_get_borne_off (position,
                                               GIBBON_POSITION_SIDE_BLACK);

        if (black_borne_off >= 15) {
                white_borne_off =
                        gibbon_position_get_borne_off (position,
                                                    GIBBON_POSITION_SIDE_WHITE);
                if (white_borne_off)
                        return -cube;
                if (position->bar[0])
                        return -3 * cube;
                for (i = 23; i > 17; --i)
                        if (position->points[i])
                                return -3 * cube;
                return -2 * cube;
        }

        return 0;
}

static GRegex *re_adjacent1 = NULL;
static GRegex *re_adjacent2 = NULL;
static GRegex *re_dup2 = NULL;
static GRegex *re_dup3 = NULL;
static GRegex *re_dup4 = NULL;
static GRegex *re_prune_intermediate = NULL;

gchar *
gibbon_position_format_move (const GibbonPosition *self,
                             const GibbonMove *move,
                             GibbonPositionSide side,
                             gboolean reverse)
{
        gint i, j;
        gint from, to;
        gchar buf[29];
        GError *error = NULL;
        gchar *new_buf = NULL;
        gchar *old_buf = NULL;
        GString *string;
        gint blots[24];
        gboolean do_reverse = FALSE;

        g_return_val_if_fail (side, g_strdup (_("invalid")));

        if (move->number == 0)
                return g_strdup ("-");
        else if (move->number > 4)
                return g_strdup (_("invalid"));

        if ((side == GIBBON_POSITION_SIDE_BLACK && !reverse)
            || (side == GIBBON_POSITION_SIDE_WHITE && reverse))
                do_reverse = TRUE;

        /* We first convert the complete move into two-character tokens.
         * The first character of each token represents the starting point
         * in the range of 'a'-'z', the second token represents the landing
         * point, again in the range of 'a'-'z'.
         */

        /* First convert it into our character form.  */
        for (i = 0, j = 0; i < move->number; ++i) {
                from = move->movements[i].from;
                to = move->movements[i].to;

                if (do_reverse) {
                        from = 25 - from;
                        to = 25 - to;
                }
                buf[j++] = 'a' + from;
                buf[j++] = '/';
                buf[j++] = 'a' + to;
                buf[j++] = ' ';
        }
        buf[j - 1] = 0;

        if (!re_adjacent1) {
                re_adjacent1 = g_regex_new ("([a-z]) \\1",
                                            0, 0, &error);
                if (error)
                        return g_strdup (error->message);
        }
        if (!re_adjacent2) {
                re_adjacent2 = g_regex_new ("([a-z])/([a-z])(.*) \\2/([a-z])",
                                            0, 0, &error);
                if (error)
                        return g_strdup (error->message);
        }
        if (!re_dup4) {
                re_dup4 = g_regex_new ("([a-z]/[a-z]) \\1 \\1 \\1",
                                       0, 0, &error);
                if (error)
                        return g_strdup (error->message);
        }
        if (!re_dup3) {
                re_dup3 = g_regex_new ("([a-z]/[a-z]) \\1 \\1",
                                       0, 0, &error);
                if (error)
                        return g_strdup (error->message);
        }
        if (!re_dup2) {
                re_dup2 = g_regex_new ("([a-z](?:/[a-z])+) \\1",
                                       0, 0, &error);
                if (error)
                        return g_strdup (error->message);
        }
        if (!re_prune_intermediate) {
                re_prune_intermediate = g_regex_new ("/[1-9][0-9]*/",
                                                     0, 0, &error);
                if (error)
                        return g_strdup (error->message);
        }

        /* First compression step.  Condense ab bc into abc, or
         * ab bc cd into abcd.
         */
        new_buf = g_regex_replace (re_adjacent1, buf, -1, 0, "\\1", 0, &error);
        if (error)
                return g_strdup (error->message);
        old_buf = new_buf;
        new_buf = g_regex_replace (re_adjacent2, old_buf, -1, 0,
                                   "\\1/\\2/\\4\\3", 0, &error);
        if (error)
                return g_strdup (error->message);

        /* Now group identical checker movements.  */
        old_buf = new_buf;
        new_buf = g_regex_replace (re_dup4, old_buf, -1, 0, "\\1(4)", 0,
                                   &error);
        g_free (old_buf);
        if (error)
                return g_strdup (error->message);

        old_buf = new_buf;
        new_buf = g_regex_replace (re_dup3, old_buf, -1, 0, "\\1(3)", 0,
                                   &error);
        g_free (old_buf);
        if (error)
                return g_strdup (error->message);

        old_buf = new_buf;
        new_buf = g_regex_replace (re_dup2, old_buf, -1, 0, "\\1(2)", 0,
                                   &error);
        g_free (old_buf);
        if (error)
                return g_strdup (error->message);

        string = g_string_new ("");

        /* Create a virtual table with opposing blots.  */
        memset (blots, 0, sizeof blots);
        for (i = 0; i < 24; ++i) {
                j = (side == GIBBON_POSITION_SIDE_BLACK) ? 23 - i : i;
                if (self->points[j] == -side)
                        blots[i] = 1;
        }

        /* And finally convert the string back into the numerical form.  */
        for (i = 0; new_buf[i]; ++i) {
                if ('a' == new_buf[i]) {
                        g_string_append (string, "off");
                } else if ('z' == new_buf[i]) {
                        g_string_append (string, "bar");
                } else if ('a' < new_buf[i] && 'z' > new_buf[i]) {
                        g_string_append_printf (string, "%u", new_buf[i] - 'a');
                        if (blots[new_buf[i] - 'a' - 1]) {
                                g_string_append_c (string, '*');
                                blots[new_buf[i] - 'a' - 1] = 0;
                        }
                } else {
                        g_string_append_c (string, new_buf[i]);
                }
        }
        g_free (new_buf);

        /* And finally prune out unneeded intermediate points.  */
        new_buf = g_regex_replace (re_prune_intermediate,
                                   string->str, -1, 0, "/", 0, &error);
        g_string_free (string, TRUE);
        if (error)
                return g_strdup (error->message);

        return new_buf;
}

gchar *
gibbon_position_fibs_move (const GibbonPosition *self, const GibbonMove *move,
                           GibbonPositionSide side, gboolean reverse)
{
        GString *string = g_string_new ("");
        const GibbonMovement *movement;
        gchar *result;
        guint i;

        for (i = 0; i < move->number; ++i) {
                if (i) string = g_string_append_c (string, ' ');

                movement = move->movements + i;

                if (movement->from == 0 || movement->from == 25)
                        string = g_string_append (string, "bar");
                else if (reverse)
                        g_string_append_printf (string, "%d",
                                                25 - movement->from);
                else
                        g_string_append_printf (string, "%d",
                                                movement->from);
                string = g_string_append_c (string, '-');

                if (movement->to == 0 || movement->to == 25)
                        string = g_string_append (string, "off");
                else if (reverse)
                        g_string_append_printf (string, "%d",
                                                25 - movement->to);
                else
                        g_string_append_printf (string, "%d",
                                                movement->to);
        }

        result = string->str;
        g_string_free (string, FALSE);

        return result;
}

void
gibbon_position_dump (const GibbonPosition *self)
{
        GValue vp = G_VALUE_INIT;
        GValue vs = G_VALUE_INIT;

        g_value_init (&vp, GIBBON_TYPE_POSITION);
        g_value_init (&vs, G_TYPE_STRING);

        g_value_set_static_boxed (&vp, self);
        g_value_transform (&vp, &vs);

        g_printerr ("%s", g_value_get_string (&vs));

        g_value_unset (&vs);
}

void
gibbon_position_reset (GibbonPosition *self)
{
        self->turn = GIBBON_POSITION_SIDE_NONE;
        memcpy (self->points, initial.points, sizeof self->points);
        memset (self->bar, 0, sizeof self->bar);
        memset (self->dice, 0, sizeof self->dice);
        self->cube = 1;
        self->cube_turned = GIBBON_POSITION_SIDE_NONE;
        self->resigned = 0;
        self->score = 0;
        self->may_double[0] = self->may_double[1] = TRUE;
}

GibbonPositionSide
gibbon_position_match_over (const GibbonPosition *self)
{
        if (self->match_length <= 0)
                return GIBBON_POSITION_SIDE_NONE;

        if (self->scores[0] >= self->match_length)
                return GIBBON_POSITION_SIDE_WHITE;
        else if (self->scores[1] >= self->match_length)
                return GIBBON_POSITION_SIDE_BLACK;

        return GIBBON_POSITION_SIDE_NONE;
}

const GibbonPosition *
gibbon_position_initial ()
{
        return &initial;
}

gboolean
gibbon_position_is_initial (const GibbonPosition *self)
{
        if (memcmp (self->points, initial.points, sizeof initial.points))
                return FALSE;

        if (self->scores[0])
                return FALSE;
        if (self->scores[1])
                return FALSE;
        if (self->dice[0])
                return FALSE;
        if (self->dice[1])
                return FALSE;
        if (self->cube > 1)
                return FALSE;
        if (self->cube_turned)
                return FALSE;
        if (self->turn)
                return FALSE;

        /*
         * Why not check the bar? We already compared the position to the
         * backgammon starting position.  If there are checkers on the bar
         * these are excess checkers, and we are in trouble anyway.  There is
         * no point in doing further consistency checks here.
         */

        return TRUE;
}

void
gibbon_position_reset_unused_dice (GibbonPosition *self)
{
        g_return_if_fail (self != NULL);

        self->unused_dice[0] = self->unused_dice[2] = 0;

        self->unused_dice[0] = self->dice[0];
        self->unused_dice[1] = self->dice[1];
        if (self->dice[0] == self->dice[1])
                self->unused_dice[2] = self->unused_dice[3]
                                     = self->unused_dice[0];
}

void
gibbon_position_transform_to_string_value (const GValue *position_value,
                                           GValue *string_value)
{
        GibbonPosition *self =
                (GibbonPosition *) g_value_get_boxed (position_value);
        GString *s = g_string_new ("=== Position ===\n");
        gint i;

        g_string_append_printf (s, "Opponent: %s, %llu/%llu points, %u pips\n",
                                self->players[1],
                                (unsigned long long) self->scores[1],
                                (unsigned long long) self->match_length,
                                gibbon_position_get_pip_count (self,
                                                   GIBBON_POSITION_SIDE_BLACK));
        g_string_append (s, "\
  +-13-14-15-16-17-18-------19-20-21-22-23-24-+ negative: black or X\n");
        g_string_append (s, "  |");
        for (i = 12; i < 18; ++i)
                if (self->points[i])
                        g_string_append_printf (s, "%+3d", self->points[i]);
                else
                        g_string_append_printf (s, "%s", "   ");
        g_string_append_printf (s, " |%+3d|", self->bar[1]);
        for (i = 18; i < 24; ++i)
                if (self->points[i])
                        g_string_append_printf (s, "%+3d", self->points[i]);
                else
                        g_string_append_printf (s, "%s", "   ");
        g_string_append_printf (s, " | May double: %s\n",
                                self->may_double[1] ? "yes" : "no");
        if (!self->dice[0] && !self->dice[1])
                g_string_append_printf (s, " v| dice: +0 : +0     ");
        else if (self->turn > 0)
                g_string_append_printf (s, " v| dice: +%u : +%u     ",
                                        self->dice[0], self->dice[1]);
        else if (self->turn < 0)
                g_string_append_printf (s, " v| dice: -%u : -%u     ",
                                        self->dice[0], self->dice[1]);
        else
                g_string_append_printf (s, " v| dice: +%u : -%u     ",
                                        self->dice[0], self->dice[1]);
        g_string_append (s, "|BAR|                   | ");
        g_string_append_printf (s, " Cube: %llu\n",
                                (unsigned long long) self->cube);
        g_string_append (s, "  |");
        for (i = 11; i >= 6; --i)
                if (self->points[i])
                        g_string_append_printf (s, "%+3d", self->points[i]);
                else
                        g_string_append_printf (s, "%s", "   ");
        g_string_append_printf (s, " |%+3d|", self->bar[0]);
        for (i = 5; i >= 0; --i)
                if (self->points[i])
                        g_string_append_printf (s, "%+3d", self->points[i]);
                else
                        g_string_append_printf (s, "%s", "   ");
        g_string_append_printf (s, " | May double: %s\n",
                                self->may_double[0] ? "yes" : "no");
        g_string_append (s, "\
  +-12-11-10--9--8--7--------6--5--4--3--2--1-+ positive: white or O\n");
        g_string_append_printf (s, "Player: %s, %llu/%llu points, %u pips\n",
                    self->players[0],
                    (unsigned long long) self->scores[0],
                    (unsigned long long) self->match_length,
                    gibbon_position_get_pip_count (self,
                                                   GIBBON_POSITION_SIDE_WHITE));
        g_string_append_printf (s, "Game info: %s\n", self->game_info);
        g_string_append_printf (s, "Status: %s\n", self->status);
        g_string_append_printf (s, "Turn: %d, cube turned: %d,"
                                   " resigned: %llu, score: %llu\n",
                                self->turn, self->cube_turned,
                                (unsigned long long) self->resigned,
                                (unsigned long long) self->score);

        g_value_set_static_string (string_value, g_string_free (s, FALSE));
}
