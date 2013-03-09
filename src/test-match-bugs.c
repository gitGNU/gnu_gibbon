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

#include <gio/gio.h>
#include <string.h>

#include "gibbon-match.h"
#include "gibbon-game.h"
#include "gibbon-position.h"
#include "gibbon-roll.h"

gboolean
test_bug01 ()
{
        GibbonMatch *match;
        GibbonGame *game;
        GibbonPosition *from;
        GibbonPosition *to;
        GSList *result = NULL;
        GibbonRoll *roll;

        match = gibbon_match_new ("guido", "gnubg_supremo", 17, TRUE);
        g_return_val_if_fail (gibbon_match_add_game (match, NULL), -1);
        game = gibbon_match_get_current_game (match);
        from = gibbon_game_get_initial_position_editable (game);

        from->turn = GIBBON_POSITION_SIDE_BLACK;
        memset (from->points, 0, sizeof from->points);
        from->cube = 2;
        from->points[0] = +1;
        from->points[1] = +3;
        from->points[2] = +2;
        from->points[3] = +3;
        from->points[23] = -2;
        from->scores[0] = 2;
        from->scores[1] = 6;

        roll = gibbon_roll_new (6, 4);
        g_return_val_if_fail (
                        gibbon_game_add_action (game,
                                                GIBBON_POSITION_SIDE_BLACK,
                                                GIBBON_GAME_ACTION (roll),
                                                1234567890,
                                                NULL), -1);

        to = gibbon_position_copy (gibbon_match_get_current_position (match));
        to->turn = GIBBON_POSITION_SIDE_WHITE;
        to->dice[0] = to->dice[1] = 0;
        to->points[23] = 0;
        to->scores[1] = 8;
        to->score = -2;

        if (!gibbon_match_get_missing_actions (match, to, &result))
                return FALSE;

        gibbon_position_free (to);
        g_object_unref (match);

        return TRUE;
}

gboolean
test_bug02 ()
{
        GibbonMatch *match;
        GibbonGame *game;
        GibbonPosition *from;
        GibbonPosition *to;
        GSList *result = NULL;

        match = gibbon_match_new ("gnubg_grandmaster", "gnubg_advanced",
                                  11, TRUE);
        g_return_val_if_fail (gibbon_match_add_game (match, NULL), -1);
        game = gibbon_match_get_current_game (match);
        from = gibbon_game_get_initial_position_editable (game);

        from->turn = GIBBON_POSITION_SIDE_WHITE;
        memset (from->points, 0, sizeof from->points);
        from->dice[0] = from->dice[1] = +5;
        from->cube = 2;
        from->may_double[0] = FALSE;
        from->may_double[1] = TRUE;
        from->points[2] = +2;
        from->points[4] = +3;
        from->points[5] = +3;
        from->points[7] = +3;
        from->points[9] = +2;
        from->points[11] = -2;
        from->points[12] = +2;
        from->points[13] = -1;
        from->points[16] = -1;
        from->points[18] = -5;
        from->points[19] = -2;
        from->points[20] = -2;
        from->points[21] = -2;

        to = gibbon_position_copy (gibbon_match_get_current_position (match));
        to->turn = GIBBON_POSITION_SIDE_BLACK;
        to->dice[0] = to->dice[1] = 0;
        to->points[12] = 0;
        to->points[9] = +1;
        to->points[7] = +4;
        to->points[4] = +4;
        to->points[2] = +3;

        if (!gibbon_match_get_missing_actions (match, to, &result))
                return FALSE;

        gibbon_position_free (to);
        g_object_unref (match);

        return TRUE;
}

int
main (int argc, char *argv[])
{
        g_type_init ();

        if (!test_bug01 ())
                return -1;
        if (!test_bug02 ())
                return -1;

        return 0;
}
