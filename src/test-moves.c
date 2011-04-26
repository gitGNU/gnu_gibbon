/*
 * This file is part of Gibbon, a graphical frontend to the First Internet 
 * Backgammon Server FIBS.
 * Copyright (C) 2009-2011 Guido Flohr, http://guido-flohr.net/.
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

static gboolean expect_move (const GibbonMove *expect,
                             GibbonMove *got, const gchar *msg);
static gboolean test_too_many_moves (void);
static gboolean test_use_all (void);
static gboolean test_try_swap1 (void);
static gboolean test_try_swap2 (void);

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
                        g_printerr ("%s: Expected %u movements, got %u.\n",
                                    msg, expect->number, got->number);
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

        g_free (got);

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

        expect = g_alloca (sizeof expect->number
                           + 4 * sizeof *expect->movements
                           + sizeof expect->status);

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

        expect = g_alloca (sizeof expect->number
                           + 4 * sizeof *expect->movements
                           + sizeof expect->status);

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

        expect = g_alloca (sizeof expect->number
                           + 4 * sizeof *expect->movements
                           + sizeof expect->status);

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

        expect = g_alloca (sizeof expect->number
                           + 4 * sizeof *expect->movements
                           + sizeof expect->status);

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
         */
        after = gibbon_position_copy (before);
        after->points[16] = 0;
        after->points[15] = +1;

        expect->number = 0;
        expect->status = GIBBON_MOVE_TRY_SWAP;
        move = gibbon_position_check_move (before, after,
                                           GIBBON_POSITION_SIDE_WHITE);

        if (!expect_move (expect, move, "White must the one in his home board"))
                retval = FALSE;

        gibbon_position_free (after);

        return retval;
}
