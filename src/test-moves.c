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

static gboolean test_white_simple_doubles (void);

int
main(int argc, char *argv[])
{
	int status = 0;

        g_type_init ();

        if (!test_white_simple_doubles ())
                status = -1;

        return status;
}

static gboolean
test_white_simple_doubles ()
{
        GibbonPosition *before = gibbon_position_new ();
        GibbonPosition *after;
        GibbonMove *move;
        GibbonMovement *movement;
        gint i;

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

        move = gibbon_position_check_move (before, after,
                                           GIBBON_POSITION_SIDE_WHITE);
        g_return_val_if_fail (move != NULL, FALSE);
        g_return_val_if_fail (move->status == GIBBON_MOVE_TOO_MANY_MOVES,
                              FALSE);
        g_free (move);

        /* Move the extra checker back.  */
        after->points[15] = 1;
        after->points[13] = 0;
        move = gibbon_position_check_move (before, after,
                                           GIBBON_POSITION_SIDE_WHITE);
        g_return_val_if_fail (move != NULL, FALSE);
        g_return_val_if_fail (move->status == GIBBON_MOVE_LEGAL, FALSE);
        g_free (move);

        gibbon_position_free (after);

        return TRUE;
}
