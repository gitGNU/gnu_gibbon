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

static gboolean test_white_simple_doubles (void);
static gboolean test_black_simple_doubles (void);
static gboolean test_white_3moves_doubles (void);

int
main(int argc, char *argv[])
{
	int status = 0;

        g_type_init ();

        if (!test_white_simple_doubles ())
                status = -1;
        if (!test_black_simple_doubles ())
                status = -1;
        if (!test_white_3moves_doubles ())
                status = -1;

        return status;
}

static gboolean
test_white_simple_doubles ()
{
        GibbonPosition *before = gibbon_position_new ();
        GibbonPosition *after;
        GibbonMove *move;
        GibbonMove *expect;
        gint i;
        gboolean retval = TRUE;

        expect = g_alloca (sizeof expect->number
                           + 4 * sizeof *expect->movements
                           + sizeof expect->status);

        before->match_length = 1;
        before->dice[0] = 2;
        before->dice[1] = 2;

        for (i = 0; i < 24; ++i)
                before->points[i] = 0;

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

        /* Put two black checkers in the way.  */
        before->points[7] = -2;
        move = gibbon_position_check_move (before, after,
                                           GIBBON_POSITION_SIDE_WHITE);

        expect->number = 0;
        expect->status = GIBBON_MOVE_ILLEGAL;
        if (!expect_move (expect, move, "White was blocked after 22"))
                retval = FALSE;

        /* Remove one of these black checkers again.  */
        before->points[7] = -1;
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

        if (!expect_move (expect, move, "Hitting one black checker after 22"))
                retval = FALSE;

        gibbon_position_free (after);

        return retval;
}

static gboolean
test_black_simple_doubles ()
{
        GibbonPosition *before = gibbon_position_new ();
        GibbonPosition *after;
        GibbonMove *move;
        GibbonMove *expect;
        gint i;
        gboolean retval = TRUE;

        expect = g_alloca (sizeof expect->number
                           + 4 * sizeof *expect->movements
                           + sizeof expect->status);

        before->match_length = 1;
        before->dice[0] = -2;
        before->dice[1] = -2;

        for (i = 0; i < 24; ++i)
                before->points[i] = 0;

        /* White has two checkers on her ace-point.  Black has one checker
         * on his 16-point, one on his 13-point, one on his 10-point, one on
         * his 7-point, and one on his 4-point..
         */
        before->points[0] = 2;
        before->points[8] = -1;
        before->points[11] = -1;
        before->points[14] = -1;
        before->points[17] = -1;
        before->points[20] = -1;

        /* Black moves each of his checkers 2 pips.  */
        after = gibbon_position_copy (before);
        after->points[8] = 0;
        after->points[10] = -1;
        after->points[11] = 0;
        after->points[13] = -1;
        after->points[14] = 0;
        after->points[16] = -1;
        after->points[17] = 0;
        after->points[19] = -1;
        after->points[20] = 0;
        after->points[22] = -1;

        expect->number = 0;
        expect->status = GIBBON_MOVE_TOO_MANY_MOVES;
        move = gibbon_position_check_move (before, after,
                                           GIBBON_POSITION_SIDE_BLACK);

        if (!expect_move (expect, move, "Black moved 5 checkers after 22"))
                retval = FALSE;

        /* Move the extra checker back.  */
        after->points[8] = -1;
        after->points[10] = 0;
        move = gibbon_position_check_move (before, after,
                                           GIBBON_POSITION_SIDE_BLACK);
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

        if (!expect_move (expect, move, "Black moved 4 checkers after 22"))
                retval = FALSE;

        /* Put two white checkers in the way.  */
        before->points[16] = 2;
        move = gibbon_position_check_move (before, after,
                                           GIBBON_POSITION_SIDE_BLACK);

        expect->number = 0;
        expect->status = GIBBON_MOVE_ILLEGAL;
        if (!expect_move (expect, move, "Black was blocked after 22"))
                retval = FALSE;

        /* Remove one of these white checkers again.  */
        before->points[16] = 1;
        move = gibbon_position_check_move (before, after,
                                           GIBBON_POSITION_SIDE_BLACK);
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

        if (!expect_move (expect, move, "Hitting one white checker after 22"))
                retval = FALSE;

        gibbon_position_free (after);

        return retval;
}

static gboolean expect_move (const GibbonMove *expect,
                             GibbonMove *got, const gchar *msg)
{
        gboolean retval = TRUE;
        guint i;
        guint got_from, got_to, got_num;
        const GibbonMovement *got_movement;
        const GibbonMovement *expect_movement;
        guint expect_from, expect_to, expect_num;

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
                        if (got_from != expect_from
                            || got_to != expect_to
                            || got_num != expect_num) {
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
test_white_3moves_doubles ()
{
        GibbonPosition *before = gibbon_position_new ();
        GibbonPosition *after;
        GibbonMove *move;
        GibbonMove *expect;
        gint i;
        gboolean retval = TRUE;

        expect = g_alloca (sizeof expect->number
                           + 4 * sizeof *expect->movements
                           + sizeof expect->status);

        before->match_length = 1;
        before->dice[0] = 3;
        before->dice[1] = 3;

        for (i = 0; i < 24; ++i)
                before->points[i] = 0;

        /*
         *  +12-13-14-15-16-17-------18-19-20-21-22-23-+ negative: black or X
         *  |   -2    +1       |   |    +1 -2       +1 |
         * v|                  |BAR|                   |
         *  |+1    -2          |   | -2                |
         *  +11-10--9--8--7--6--------5--4--3--2--1--0-+ positive: white or O
         */
        before->points[23] = +1;
        before->points[20] = -2;
        before->points[19] = +1;
        before->points[15] = +1;
        before->points[13] = -2;
        before->points[11] = +1;
        before->points[9] = -2;
        before->points[5] = -2;

        /* White moves each of his movable checkers 2 pips.  */
        after = gibbon_position_copy (before);
        after->points[19] = 0;
        after->points[16] = 1;
        after->points[15] = 0;
        after->points[12] = 1;
        after->points[11] = 0;
        after->points[8] = 1;

        expect->number = 3;
        expect->status = GIBBON_MOVE_LEGAL;
        expect->movements[0].from = 20;
        expect->movements[0].to = 17;
        expect->movements[1].from = 16;
        expect->movements[1].to = 13;
        expect->movements[2].from = 12;
        expect->movements[2].to = 9;
        move = gibbon_position_check_move (before, after,
                                           GIBBON_POSITION_SIDE_WHITE);

        if (!expect_move (expect, move, "White moved 3 checkers after 33"))
                retval = FALSE;

        /* We double the most backward movable white checkers so that we can
         * move two checkers at once.
         */
        before->points[19] = +2;
        after->points[16] = +2;
        move = gibbon_position_check_move (before, after,
                                           GIBBON_POSITION_SIDE_WHITE);
        if (!expect_move (expect, move, "White moved 4a checkers after 33"))
                retval = FALSE;

        /* Same with the next possibility.  */
        before->points[19] = +1;
        after->points[16] = +1;
        before->points[15] = +2;
        after->points[12] = +2;
        move = gibbon_position_check_move (before, after,
                                           GIBBON_POSITION_SIDE_WHITE);
        if (!expect_move (expect, move, "White moved 4b checkers after 33"))
                retval = FALSE;

        /* And once more.  */
        before->points[15] = +1;
        after->points[12] = +1;
        before->points[11] = +2;
        after->points[8] = +2;
        move = gibbon_position_check_move (before, after,
                                           GIBBON_POSITION_SIDE_WHITE);
        if (!expect_move (expect, move, "White moved 4c checkers after 33"))
                retval = FALSE;

        /* Reset.  */
        for (i = 0; i < 24; ++i)
                before->points[i] = 0;

        /*
         *  +12-13-14-15-16-17-------18-19-20-21-22-23-+ negative: black or X
         *  |                  |   |          +1 +1 +1 |
         * v|                  |BAR|                   |
         *  |                  |   |                   |
         *  +11-10--9--8--7--6--------5--4--3--2--1--0-+ positive: white or O
         */
        before->points[23] = +1;
        before->points[22] = +1;
        before->points[21] = +1;
        gibbon_position_free (after);
        after = gibbon_position_copy (before);

        /* Let the most backward checker move twice, the others once.  */
        after->points[23] = 0;
        after->points[17] = +1;
        after->points[22] = 0;
        after->points[19] = +1;
        after->points[21] = 0;
        after->points[18] = +1;
        expect->number = 4;
        expect->status = GIBBON_MOVE_LEGAL;
        expect->movements[0].from = 24;
        expect->movements[0].to = 21;
        expect->movements[1].from = 21;
        expect->movements[1].to = 18;
        expect->movements[2].from = 23;
        expect->movements[2].to = 20;
        expect->movements[3].from = 22;
        expect->movements[3].to = 19;
        move = gibbon_position_check_move (before, after,
                                           GIBBON_POSITION_SIDE_WHITE);
        if (!expect_move (expect, move, "White moved 4d checkers after 33"))
                retval = FALSE;

        /* Now the middle checker move twice, the others once.  */
        after->points[23] = 0;
        after->points[20] = 1;
        after->points[17] = 0;
        after->points[22] = 0;
        after->points[19] = 0;
        after->points[16] = 1;
        after->points[21] = 0;
        after->points[18] = 1;
        after->points[15] = 0;
        expect->number = 4;
        expect->status = GIBBON_MOVE_LEGAL;
        expect->movements[0].from = 24;
        expect->movements[0].to = 21;
        expect->movements[1].from = 23;
        expect->movements[1].to = 20;
        expect->movements[2].from = 20;
        expect->movements[2].to = 17;
        expect->movements[3].from = 22;
        expect->movements[3].to = 19;
        move = gibbon_position_check_move (before, after,
                                           GIBBON_POSITION_SIDE_WHITE);
        if (!expect_move (expect, move, "White moved 4e checkers after 33"))
                retval = FALSE;

        /* Now the forward checker move twice, the others once.  */
        after->points[23] = 0;
        after->points[20] = 1;
        after->points[17] = 0;
        after->points[22] = 0;
        after->points[19] = 1;
        after->points[16] = 0;
        after->points[21] = 0;
        after->points[18] = 0;
        after->points[15] = 1;
        expect->number = 4;
        expect->status = GIBBON_MOVE_LEGAL;
        expect->movements[0].from = 24;
        expect->movements[0].to = 21;
        expect->movements[1].from = 23;
        expect->movements[1].to = 20;
        expect->movements[2].from = 22;
        expect->movements[2].to = 19;
        expect->movements[3].from = 19;
        expect->movements[3].to = 16;
        move = gibbon_position_check_move (before, after,
                                           GIBBON_POSITION_SIDE_WHITE);
        if (!expect_move (expect, move, "White moved 4e checkers after 33"))
                retval = FALSE;

        gibbon_position_free (after);

        return retval;
}
