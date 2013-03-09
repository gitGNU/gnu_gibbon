/*
 * This file is part of Gibbon, a graphical frontend to the First Internet 
 * Backgammon Server FIBS.
 * Copyright (C) 2009-2012 Guido Flohr, http://guido-flohr.net/.
 *
 * Gibbon is free software: you can redistribute it and/or modify 
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Gibbon is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Gibbon.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <string.h>

#include <glib.h>

#include <gibbon-position.h>
#include <gibbon-move.h>

static gboolean expect_move (const GibbonMove *expect,
                             GibbonMove *got, const gchar *msg);
static gboolean test_too_many_moves (void);
static gboolean test_use_all (void);
static gboolean test_try_swap1 (void);
static gboolean test_try_swap2 (void);
static gboolean test_try_dance (void);
static gboolean test_illegal_waste (void);
static gboolean test_use_higher (void);
static gboolean test_not_use_higher (void);
static gboolean test_try_swap_bug1 (void);
static gboolean test_try_swap_bug2 (void);
static gboolean test_try_swap_bug3 (void);
static gboolean test_ordered_bear_off (void);
static gboolean test_bear_off_bug1 (void);

int
main(int argc, char *argv[])
{
	int status = 0;

        g_type_init ();

        if (!test_too_many_moves ())
                status = -1;
        if (!test_use_all ())
                status = -1;
        if (!test_try_swap1 ())
                status = -1;
        if (!test_try_swap2 ())
                status = -1;
        if (!test_try_dance ())
                status = -1;
        if (!test_illegal_waste ())
                status = -1;
        if (!test_use_higher ())
                status = -1;
        if (!test_not_use_higher ())
                status = -1;
        if (!test_try_swap_bug1 ())
                status = -1;
        if (!test_try_swap_bug2 ())
                status = -1;
        if (!test_try_swap_bug3 ())
                status = -1;
        if (!test_ordered_bear_off ())
                status = -1;
        if (!test_bear_off_bug1 ())
                status = -1;

        return status;
}

static gboolean
expect_move (const GibbonMove *expect,
             GibbonMove *got, const gchar *msg)
{
        gboolean retval = TRUE;
        guint i;
        guint got_from, got_to;
        const GibbonMovement *got_movement;
        const GibbonMovement *expect_movement;
        guint expect_from, expect_to;

        if (!got) {
                g_printerr ("%s: Returned move is NULL!\n", msg);
                return FALSE;
        }

        if (got->status != expect->status) {
                g_printerr ("%s: Expected status %d, got %d.\n",
                            msg, expect->status, got->status);
                retval = FALSE;
        }

        if (expect->status == GIBBON_MOVE_LEGAL) {
                if (expect->number != got->number) {
                        g_printerr ("%s: Expected %llu movements, got %llu.\n",
                                    msg,
                                    (unsigned long long) expect->number,
                                    (unsigned long long) got->number);
                        retval = FALSE;
                }

                for (i = 0; i < got->number && i < expect->number; ++i) {
                        got_movement = got->movements + i;
                        got_from = got_movement->from;
                        got_to = got_movement->to;
                        expect_movement = expect->movements + i;
                        expect_from = expect_movement->from;
                        expect_to = expect_movement->to;
                        if (got_from != expect_from || got_to != expect_to) {
                                g_printerr ("%s: Movement %u: "
                                            "Expected %u/%u,"
                                            " got %u/%u.\n",
                                            msg, i,
                                            expect_from , expect_to,
                                            got_from, got_to);
                                retval = FALSE;
                        }
                }
        }

        g_object_unref (got);

        return retval;
}

static gboolean
test_too_many_moves ()
{
        GibbonPosition *before = gibbon_position_new ();
        GibbonPosition *after;
        GibbonMove *move;
        GibbonMove *expect;
        gboolean retval = TRUE;

        expect = gibbon_move_new (2, 2, 4);

        before->match_length = 1;
        before->dice[0] = 2;
        before->dice[1] = 2;

        memset (before->points, 0, sizeof before->points);

        /* Black has two checkers on her ace-point.  White has one checker
         * on his 16-point, one on his 13-point, one on his 10-point, one on
         * his 7-point, and one on his 4-point..
         */
        before->points[23] = -2;
        before->points[15] = 1;
        before->points[12] = 1;
        before->points[9] = 1;
        before->points[6] = 1;
        before->points[3] = 1;

        /* White moves each of his checkers 2 pips.  */
        after = gibbon_position_copy (before);
        after->points[15] = 0;
        after->points[13] = 1;
        after->points[12] = 0;
        after->points[10] = 1;
        after->points[9] = 0;
        after->points[7] = 1;
        after->points[6] = 0;
        after->points[4] = 1;
        after->points[3] = 0;
        after->points[1] = 1;

        expect->number = 0;
        expect->status = GIBBON_MOVE_TOO_MANY_MOVES;
        move = gibbon_position_check_move (before, after,
                                           GIBBON_POSITION_SIDE_WHITE);

        if (!expect_move (expect, move, "White moved 5 checkers after 22"))
                retval = FALSE;

        /* Move the extra checker back.  */
        after->points[15] = 1;
        after->points[13] = 0;
        move = gibbon_position_check_move (before, after,
                                           GIBBON_POSITION_SIDE_WHITE);
        expect->number = 4;
        expect->status = GIBBON_MOVE_LEGAL;
        expect->movements[0].from = 13;
        expect->movements[0].to = 11;
        expect->movements[1].from = 10;
        expect->movements[1].to = 8;
        expect->movements[2].from = 7;
        expect->movements[2].to = 5;
        expect->movements[3].from = 4;
        expect->movements[3].to = 2;

        if (!expect_move (expect, move, "White moved 4 checkers after 22"))
                retval = FALSE;

        gibbon_position_free (after);

        g_object_unref (expect);

        return retval;
}

static gboolean
test_use_all ()
{
        GibbonPosition *before = gibbon_position_new ();
        GibbonPosition *after;
        GibbonMove *move;
        GibbonMove *expect;
        gboolean retval = TRUE;

        expect = gibbon_move_new (3, 2, 4);

        before->match_length = 1;
        before->dice[0] = 3;
        before->dice[1] = 2;

        memset (before->points, 0, sizeof before->points);

        before->points[3] = -2;
        before->points[10] = +1;

        after = gibbon_position_copy (before);
        after->points[3] = -1;
        after->points[6] = -1;

        expect->number = 0;
        expect->status = GIBBON_MOVE_USE_ALL;

        move = gibbon_position_check_move (before, after,
                                           GIBBON_POSITION_SIDE_BLACK);
        if (!expect_move (expect, move,
                          "Black moved only one checker after 32"))
                retval = FALSE;

        /* Move one white checker in the way.  */
        before->points[5] = 1;
        after->points[5] = 1;
        before->points[8] = 2;
        after->points[8] = 2;

        move = gibbon_position_check_move (before, after,
                                           GIBBON_POSITION_SIDE_BLACK);
        if (!expect_move (expect, move,
                          "Black moved only one checker after 32,"
                          " and did not hit"))
                retval = FALSE;

        /* Now really block the point.  */
        before->points[5] = 2;
        after->points[5] = 2;

        move = gibbon_position_check_move (before, after,
                                           GIBBON_POSITION_SIDE_BLACK);
        expect->status = GIBBON_MOVE_LEGAL;
        expect->number = 1;
        expect->movements[0].from = 21;
        expect->movements[0].to = 18;
        if (!expect_move (expect, move,
                          "Black could not move the 2 after 32"))
                retval = FALSE;

        gibbon_position_free (after);

        g_object_unref (expect);

        return retval;
}

static gboolean
test_try_swap1 ()
{
        GibbonPosition *before = gibbon_position_new ();
        GibbonPosition *after;
        GibbonMove *move;
        GibbonMove *expect;
        gboolean retval = TRUE;

        expect = gibbon_move_new (6, 3, 4);

        before->match_length = 1;
        before->dice[0] = 6;
        before->dice[1] = 3;

        memset (before->points, 0, sizeof before->points);

        before->points[23] = -1;
        before->points[21] = +2;
        before->points[20] = -3;
        before->points[19] = -1;
        before->points[18] = -2;
        before->points[12] = +2;
        before->points[7] = +2;
        before->points[6] = -1;
        before->points[5] = +2;
        before->points[3] = +2;
        before->points[2] = +2;

        /* Black now moves only the three but could use the three and
         * the six.
         */
        after = gibbon_position_copy (before);
        after->points[20] = -2;
        after->points[23] = -2;

        expect->number = 0;
        expect->status = GIBBON_MOVE_TRY_SWAP;
        move = gibbon_position_check_move (before, after,
                                           GIBBON_POSITION_SIDE_BLACK);

        if (!expect_move (expect, move, "White must use the 3 before the 6"))
                retval = FALSE;

        gibbon_position_free (after);

        g_object_unref (expect);

        return retval;
}

static gboolean
test_try_swap2 ()
{
        GibbonPosition *before = gibbon_position_new ();
        GibbonPosition *after;
        GibbonMove *move;
        GibbonMove *expect;
        gboolean retval = TRUE;

        expect = gibbon_move_new (1, 4, 4);

        before->match_length = 1;
        before->dice[0] = 1;
        before->dice[1] = 4;

        memset (before->points, 0, sizeof before->points);

        before->points[21] = -2;
        before->points[20] = -2;
        before->points[18] = -4;
        before->points[16] = +1;
        before->points[14] = -2;
        before->points[12] = -1;
        before->points[11] = -3;
        before->points[8] = -1;
        before->points[2] = +1;
        before->points[1] = +4;
        before->points[0] = +4;

        /* White can move the one and the four with the checker on his
         * 17-point.  But he must use the four so that he can move the one
         * within his home board.
         *
         * This case would already be caught by the rule that the higher
         * number has to be used.  But since the "try swap" error message
         * contains better information for the user, we expect this error
         * instead.
         */
        after = gibbon_position_copy (before);
        after->points[16] = 0;
        after->points[15] = +1;

        expect->number = 0;
        expect->status = GIBBON_MOVE_TRY_SWAP;
        move = gibbon_position_check_move (before, after,
                                           GIBBON_POSITION_SIDE_WHITE);

        if (!expect_move (expect, move, "White must move the one in his home"
                                        " board"))
                retval = FALSE;

        gibbon_position_free (after);

        g_object_unref (expect);

        return retval;
}

static gboolean
test_try_dance ()
{
        GibbonPosition *before = gibbon_position_new ();
        GibbonPosition *after;
        GibbonMove *move;
        GibbonMove *expect;
        gboolean retval = TRUE;

        expect = gibbon_move_new (1, 2, 4);

        before->match_length = 1;
        before->dice[0] = 1;
        before->dice[1] = 2;

        memset (before->points, 0, sizeof before->points);

        before->bar[0] = 3;
        before->points[21] = -2;
        before->points[6] = 1;

        /* White comes in with only one checker, and moves 6/5.  */
        after = gibbon_position_copy (before);
        after->bar[0] = 2;
        after->points[22] = +1;
        after->points[6] = 0;
        after->points[5] = +1;

        expect->number = 0;
        expect->status = GIBBON_MOVE_DANCING;
        move = gibbon_position_check_move (before, after,
                                           GIBBON_POSITION_SIDE_WHITE);

        if (!expect_move (expect, move, "White moved although dancing"))
                retval = FALSE;

        /* Now correct the move.  */
        after->bar[0] = 1;
        after->points[23] = +1;
        after->points[6] = +1;
        after->points[5] = 0;

        expect->number = 2;
        expect->movements[0].from = 25;
        expect->movements[0].to = 24;
        expect->movements[1].from = 25;
        expect->movements[1].to = 23;
        expect->status = GIBBON_MOVE_LEGAL;

        move = gibbon_position_check_move (before, after,
                                           GIBBON_POSITION_SIDE_WHITE);

        if (!expect_move (expect, move, "White coming in from the bar with"
                                        " two checkers"))
                retval = FALSE;

        gibbon_position_free (after);

        g_object_unref (expect);

        return retval;
}

static gboolean
test_illegal_waste ()
{
        GibbonPosition *before = gibbon_position_new ();
        GibbonPosition *after;
        GibbonMove *move;
        GibbonMove *expect;
        gboolean retval = TRUE;

        expect = gibbon_move_new (1, 5, 4);

        before->match_length = 1;
        before->dice[0] = 1;
        before->dice[1] = 5;

        memset (before->points, 0, sizeof before->points);

        before->points[22] = -2;
        before->points[20] = -2;
        before->points[18] = -4;
        before->points[16] = -4;
        before->points[13] = -1;
        before->points[5] = -1;
        before->points[4] = +3;
        before->points[3] = +4;
        before->points[2] = +3;
        before->points[1] = +2;
        before->points[0] = +2;

        /* White now moves 1/off and 3/2.  */
        after = gibbon_position_copy (before);
        after->points[2] = +2;
        after->points[1] = +3;
        after->points[0] = +1;

        expect->number = 0;
        expect->status = GIBBON_MOVE_ILLEGAL_WASTE;
        move = gibbon_position_check_move (before, after,
                                           GIBBON_POSITION_SIDE_WHITE);

        if (!expect_move (expect, move, "Illegal bear-off by white"))
                retval = FALSE;

        gibbon_position_free (after);

        g_object_unref (expect);

        return retval;
}

static gboolean
test_use_higher ()
{
        GibbonPosition *before = gibbon_position_new ();
        GibbonPosition *after;
        GibbonMove *move;
        GibbonMove *expect;
        gboolean retval = TRUE;

        expect = gibbon_move_new (6, 4, 4);

        before->match_length = 1;
        before->dice[0] = 6;
        before->dice[1] = 4;

        memset (before->points, 0, sizeof before->points);

        before->bar[0] = 1;
        before->points[23] = -2;
        before->points[22] = -2;
        before->points[17] = -1;
        before->points[16] = -1;
        before->points[14] = -2;
        before->points[8] = -1;
        before->points[6] = -1;
        before->points[4] = +2;
        before->points[3] = +3;
        before->points[2] = +2;
        before->points[1] = +2;
        before->points[0] = -5;

        /* White can come in from the bar with the six and the four.  But
         * it must use the six because it is higher, and both numbers cannot
         * be used elsewhere.
         */
        after = gibbon_position_copy (before);
        after->bar[0] = 0;
        after->points[20] = +1;

        expect->number = 0;
        expect->status = GIBBON_MOVE_USE_HIGHER;
        move = gibbon_position_check_move (before, after,
                                           GIBBON_POSITION_SIDE_WHITE);

        if (!expect_move (expect, move, "White used lower number"))
                retval = FALSE;

        gibbon_position_free (after);

        g_object_unref (expect);

        return retval;
}

static gboolean
test_not_use_higher ()
{
        GibbonPosition *before = gibbon_position_new ();
        GibbonPosition *after;
        GibbonMove *move;
        GibbonMove *expect;
        gboolean retval = TRUE;

        expect = gibbon_move_new (4, 6, 4);

        before->match_length = 1;
        before->dice[0] = 4;
        before->dice[1] = 6;

        memset (before->points, 0, sizeof before->points);

        before->points[23] = -2;
        before->points[20] = -1;
        before->points[0] = +1;

        /* White bears off his last checker from the ace point.  Even if the
         * legality checker assumed that the lower number was used for
         * bearing off, this move is legal.
         */
        after = gibbon_position_copy (before);
        after->points[0] = 0;

        expect->number = 1;
        expect->movements[0].from = 1;
        expect->movements[0].to = 0;
        expect->status = GIBBON_MOVE_LEGAL;
        move = gibbon_position_check_move (before, after,
                                           GIBBON_POSITION_SIDE_WHITE);

        if (!expect_move (expect, move, "False positive for use higher"))
                retval = FALSE;

        /* Now swap the two dice.  Independently of the internal implementation,
         * the above described bug should strike in one of the two cases
         * if it is not handled.
         */
        before->dice[0] = 6;
        before->dice[1] = 4;
        move = gibbon_position_check_move (before, after,
                                           GIBBON_POSITION_SIDE_WHITE);

        if (!expect_move (expect, move, "False positive for use higher"))
                retval = FALSE;

        gibbon_position_free (after);

        g_object_unref (expect);

        return retval;
}

static gboolean
test_try_swap_bug1 ()
{
        GibbonPosition *before = gibbon_position_new ();
        GibbonPosition *after;
        GibbonMove *move;
        GibbonMove *expect;
        gboolean retval = TRUE;

        expect = gibbon_move_new (6, 3, 4);

        before->match_length = 1;
        before->dice[0] = 6;
        before->dice[1] = 3;

        memset (before->points, 0, sizeof before->points);

        before->points[21] = -1;
        before->points[20] = -1;
        before->points[19] = -2;
        before->points[18] = -6;
        before->points[17] = -2;
        before->points[16] = -1;
        before->points[6] = +4;
        before->points[5] = +5;
        before->points[4] = +2;
        before->points[3] = +1;
        before->points[2] = +2;
        before->points[1] = +1;
        before->points[0] = -2;

        /* White can only move the three.  */
        after = gibbon_position_copy (before);
        after->points[6] = +3;
        after->points[3] = +2;

        expect->number = 1;
        expect->movements[0].from = 7;
        expect->movements[0].to = 4;
        expect->status = GIBBON_MOVE_LEGAL;
        move = gibbon_position_check_move (before, after,
                                           GIBBON_POSITION_SIDE_WHITE);
        if (!expect_move (expect, move, "False positive for try swap"))
                retval = FALSE;

        gibbon_position_free (after);

        g_object_unref (expect);

        return retval;
}

static gboolean
test_try_swap_bug2 ()
{
        GibbonPosition *before = gibbon_position_new ();
        GibbonPosition *after;
        GibbonMove *move;
        GibbonMove *expect;
        gboolean retval = TRUE;

        expect = gibbon_move_new (5, 3, 4);

        before->match_length = 1;
        before->dice[0] = 5;
        before->dice[1] = 3;

        memset (before->points, 0, sizeof before->points);

        /* Starting position:
         *  === Position ===
         *  +-13-14-15-16-17-18-------19-20-21-22-23-24-+ negative: black or X
         *  | -1    -2    -1    | +0| -4 +1 -2 -2       |
         * v| dice: +5 : +3     |BAR|                   |  Cube: 1
         *  | -3                | +0|          +1 +4 +4 |
         *  +-12-11-10--9--8--7--------6--5--4--3--2--1-+ positive: white or O
         * End position:
         * === Position ===
         *  +-13-14-15-16-17-18-------19-20-21-22-23-24-+ negative: black or X
         *  | -1    -2    +1    | +1| -4    -2 -2       |
         * v| dice: +5 : +3     |BAR|                   |  Cube: 1
         *  | -3                | +0|          +1 +4 +4 |
         *  +-12-11-10--9--8--7--------6--5--4--3--2--1-+ positive: white or O
         */

        before->points[21] = -2;
        before->points[20] = -2;
        before->points[19] = +1;
        before->points[18] = -4;
        before->points[16] = -1;
        before->points[14] = -2;
        before->points[12] = -1;
        before->points[11] = -3;
        before->points[2] = +1;
        before->points[1] = +4;
        before->points[0] = +4;

        /* White can only move the three.  */
        after = gibbon_position_copy (before);
        after->points[19] = 0;
        after->points[16] = +1;
        after->bar[1] = 1;

        expect->number = 1;
        expect->movements[0].from = 20;
        expect->movements[0].to = 17;
        expect->status = GIBBON_MOVE_LEGAL;
        move = gibbon_position_check_move (before, after,
                                           GIBBON_POSITION_SIDE_WHITE);
        if (!expect_move (expect, move, "False positive #2 for try swap"))
                retval = FALSE;

        gibbon_position_free (after);

        g_object_unref (expect);

        return retval;
}

static gboolean
test_try_swap_bug3 ()
{
        GibbonPosition *before = gibbon_position_new ();
        GibbonPosition *after;
        GibbonMove *move;
        GibbonMove *expect;
        gboolean retval = TRUE;

        expect = gibbon_move_new (5, 3, 4);

        before->match_length = 1;
        before->dice[0] = 5;
        before->dice[1] = 3;

        memset (before->points, 0, sizeof before->points);

        /* Starting position:
         * === Position ===
         *  +-13-14-15-16-17-18-------19-20-21-22-23-24-+ negative: black or X
         *  | +1             +1 | +0| -2       +3 -7 -5 |
         * v| dice: -5 : -3     |BAR|                   |  Cube: 1
         *  | +1    +1    +3    | +0| +1    +1 +1    +2 |
         *  +-12-11-10--9--8--7--------6--5--4--3--2--1-+ positive: white or O
         * End position:
         * === Position ===
         * +-13-14-15-16-17-18-------19-20-21-22-23-24-+ negative: black or X
         *  | +1             +1 | +0| -1       +3 -7 -6 |
         * v| dice: -5 : -3     |BAR|                   |  Cube: 1
         *  | +1    +1    +3    | +0| +1    +1 +1    +2 |
         *  +-12-11-10--9--8--7--------6--5--4--3--2--1-+ positive: white or O
         */
        before->points[23] = -5;
        before->points[22] = -7;
        before->points[21] = +3;
        before->points[18] = -2;
        before->points[17] = +1;
        before->points[12] = +1;
        before->points[11] = +1;
        before->points[9] = +1;
        before->points[7] = +3;
        before->points[5] = +1;
        before->points[3] = +1;
        before->points[2] = +1;
        before->points[0] = +2;

        /* Black moves 6/1.  */
        after = gibbon_position_copy (before);
        after->points[23] = -6;
        after->points[18] = -1;

        expect->number = 1;
        expect->movements[0].from = 6;
        expect->movements[0].to = 1;
        expect->status = GIBBON_MOVE_LEGAL;
        move = gibbon_position_check_move (before, after,
                                           GIBBON_POSITION_SIDE_BLACK);
        if (!expect_move (expect, move, "False positive #3 for try swap"))
                retval = FALSE;

        gibbon_position_free (after);

        g_object_unref (expect);

        return retval;
}

static gboolean
test_ordered_bear_off ()
{
        GibbonPosition *before = gibbon_position_new ();
        GibbonPosition *after;
        GibbonMove *move;
        GibbonMove *expect;
        gboolean retval = TRUE;

        expect = gibbon_move_new (3, 3, 4);

        before->match_length = 1;
        before->dice[0] = 3;
        before->dice[1] = 3;

        memset (before->points, 0, sizeof before->points);

        /*
         * Starting position:
         * === Position ===
         *  +-13-14-15-16-17-18-------19-20-21-22-23-24-+ negative: black or X
         *  |             -1    | +0| -2       -2    -2 |
         * v| dice: +3 : +3     |BAR|                   |  Cube: 1
         *  |    -3 -2 -2    -1 | +0|    +1    +2 +4    |
         *  +-12-11-10--9--8--7--------6--5--4--3--2--1-+ positive: white or O
         * End position:
         * === Position ===
         *  +-13-14-15-16-17-18-------19-20-21-22-23-24-+ negative: black or X
         *  |             -1    | +0| -2       -2    -2 |
         * v| dice: +3 : +3     |BAR|                   |  Cube: 1
         *  |    -3 -2 -2    -1 | +0|             +4    |
         *  +-12-11-10--9--8--7--------6--5--4--3--2--1-+ positive: white or O
         */

        before->points[23] = -2;
        before->points[21] = -2;
        before->points[18] = -2;
        before->points[16] = -1;
        before->points[10] = -3;
        before->points[9] = -2;
        before->points[8] = -2;
        before->points[6] = -1;
        before->points[4] = +1;
        before->points[2] = +2;
        before->points[1] = +4;

        /* White cannot bear off directly from the five point.  The correct
         * order must be 5/2, 3/off, 3/off, and then 2/off.
         */
        after = gibbon_position_copy (before);
        after->points[4] = 0;
        after->points[2] = 0;
        after->points[1] = +4;

        expect->number = 4;
        expect->movements[0].from = 5;
        expect->movements[0].to = 2;
        expect->movements[1].from = 3;
        expect->movements[1].to = 0;
        expect->movements[2].from = 3;
        expect->movements[2].to = 0;
        expect->movements[3].from = 2;
        expect->movements[3].to = 0;
        expect->status = GIBBON_MOVE_LEGAL;
        move = gibbon_position_check_move (before, after,
                                           GIBBON_POSITION_SIDE_WHITE);
        if (!expect_move (expect, move, "Bear-off in wrong order"))
                retval = FALSE;

        gibbon_position_free (after);

        g_object_unref (expect);

        return retval;
}

static gboolean
test_bear_off_bug1 ()
{
        GibbonPosition *before = gibbon_position_new ();
        GibbonPosition *after;
        GibbonMove *move;
        GibbonMove *expect;
        gboolean retval = TRUE;

        expect = gibbon_move_new (1, 4, 4);

        before->match_length = 1;
        before->dice[0] = 1;
        before->dice[1] = 4;

        memset (before->points, 0, sizeof before->points);

        /*
         * Starting position:
         * === Position ===
         *   +-13-14-15-16-17-18-------19-20-21-22-23-24-+ negative: black or X
         *   |                +2 | +0| -2 +3 -1 -3 +3    |
         *  v| dice: -1 : -4     |BAR|                   |  Cube: 1
         *   |                   | +0|       +2 +5       |
         *   +-12-11-10--9--8--7--------6--5--4--3--2--1-+ positive: white or O
         *  End position:
         * === Position ===
         *   +-13-14-15-16-17-18-------19-20-21-22-23-24-+ negative: black or X
         *   |                +2 | +0| -2 +3    -3 +3    |
         *  v| dice: -1 : -4     |BAR|                   |  Cube: 1
         *   |                   | +0|       +2 +5       |
         *   +-12-11-10--9--8--7--------6--5--4--3--2--1-+ positive: white or O
         */

        before->points[22] = +3;
        before->points[21] = -3;
        before->points[20] = -1;
        before->points[19] = +3;
        before->points[18] = -2;
        before->points[17] = +2;
        before->points[3] = +2;
        before->points[2] = +5;

        /* White uses only the four, and bears off with it.  */
        after = gibbon_position_copy (before);
        after->points[20] = 0;

        expect->number = 1;
        expect->movements[0].from = 4;
        expect->movements[0].to = 0;
        expect->status = GIBBON_MOVE_LEGAL;
        move = gibbon_position_check_move (before, after,
                                           GIBBON_POSITION_SIDE_BLACK);
        if (!expect_move (expect, move, "Overlooked direct bear-off"))
                retval = FALSE;

        gibbon_position_free (after);

        g_object_unref (expect);

        return retval;
}
