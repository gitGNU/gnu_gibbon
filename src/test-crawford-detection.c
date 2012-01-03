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

#include <glib.h>
#include <string.h>

#include <gibbon-match.h>
#include <gibbon-game.h>
#include <gibbon-game-actions.h>

static gboolean test_to_nil (void);
static gboolean test_regular_match (void);

int
main(int argc, char *argv[])
{
	int status = 0;

        g_type_init ();

        if (!test_to_nil ())
                status = -1;

        if (!test_regular_match ())
                status = -1;

        return status;
}

static gboolean
test_to_nil (void)
{
        GibbonMatch *match = gibbon_match_new ("Snow White", "Joe Black",
                                               3, TRUE);
        GibbonGame *game;
        GibbonGameAction *action;

        game = gibbon_match_get_current_game (match);

        if (gibbon_game_is_crawford (game)) {
                g_object_unref (match);
                g_printerr ("First game cannot be Crawford.\n");
                return FALSE;
        }

        action = GIBBON_GAME_ACTION (gibbon_roll_new (3, 1));
        gibbon_game_add_action (game, GIBBON_POSITION_SIDE_WHITE, action);

        action = GIBBON_GAME_ACTION (gibbon_move_newv (3, 1, 8, 5, 6, 5, -1));
        gibbon_game_add_action (game, GIBBON_POSITION_SIDE_WHITE, action);

        action = GIBBON_GAME_ACTION (gibbon_roll_new (5, 2));
        gibbon_game_add_action (game, GIBBON_POSITION_SIDE_BLACK, action);

        action = GIBBON_GAME_ACTION (gibbon_move_newv (5, 2, 12, 17, 1, 3, -1));
        gibbon_game_add_action (game, GIBBON_POSITION_SIDE_BLACK, action);

        action = GIBBON_GAME_ACTION (gibbon_double_new ());
        gibbon_game_add_action (game, GIBBON_POSITION_SIDE_WHITE, action);

        action = GIBBON_GAME_ACTION (gibbon_take_new ());
        gibbon_game_add_action (game, GIBBON_POSITION_SIDE_BLACK, action);

        action = GIBBON_GAME_ACTION (gibbon_roll_new (5, 5));
        gibbon_game_add_action (game, GIBBON_POSITION_SIDE_WHITE, action);

        action = GIBBON_GAME_ACTION (gibbon_move_newv (5, 5, 8, 3, 8, 3, 6, 1,
                                                       6, 1, -1));
        gibbon_game_add_action (game, GIBBON_POSITION_SIDE_WHITE, action);

        action = GIBBON_GAME_ACTION (gibbon_resign_new (1));
        gibbon_game_add_action (game, GIBBON_POSITION_SIDE_BLACK, action);

        action = GIBBON_GAME_ACTION (gibbon_accept_new ());
        gibbon_game_add_action (game, GIBBON_POSITION_SIDE_WHITE, action);

        game = gibbon_match_add_game (match);

        if (!gibbon_game_is_crawford (game)) {
                g_object_unref (match);
                g_printerr ("Crawford game to_nill not detected.\n");
                return FALSE;
        }

        g_object_unref (match);

        return TRUE;
}


static gboolean
test_regular_match (void)
{
        GibbonMatch *match = gibbon_match_new ("Snow White", "Joe Black",
                                               4, TRUE);
        GibbonGame *game;
        GibbonGameAction *action;

        game = gibbon_match_get_current_game (match);
        if (gibbon_game_is_crawford (game)) {
                g_object_unref (match);
                g_printerr ("First game cannot be Crawford at %s:%d.\n",
                            __FILE__, __LINE__);
                return FALSE;
        }

        action = GIBBON_GAME_ACTION (gibbon_resign_new (2));
        gibbon_game_add_action (game, GIBBON_POSITION_SIDE_WHITE, action);
        action = GIBBON_GAME_ACTION (gibbon_accept_new ());
        gibbon_game_add_action (game, GIBBON_POSITION_SIDE_BLACK, action);

        game = gibbon_match_add_game (match);
        if (gibbon_game_is_crawford (game)) {
                g_object_unref (match);
                g_printerr ("False positive for Crawford at %s:%d.\n",
                            __FILE__, __LINE__);
                return FALSE;
        }
        action = GIBBON_GAME_ACTION (gibbon_resign_new (1));
        gibbon_game_add_action (game, GIBBON_POSITION_SIDE_BLACK, action);
        action = GIBBON_GAME_ACTION (gibbon_accept_new ());
        gibbon_game_add_action (game, GIBBON_POSITION_SIDE_WHITE, action);

        game = gibbon_match_add_game (match);
        if (gibbon_game_is_crawford (game)) {
                g_object_unref (match);
                g_printerr ("False positive for Crawford at %s:%d.\n",
                            __FILE__, __LINE__);
                return FALSE;
        }

        action = GIBBON_GAME_ACTION (gibbon_resign_new (1));
        gibbon_game_add_action (game, GIBBON_POSITION_SIDE_WHITE, action);
        action = GIBBON_GAME_ACTION (gibbon_accept_new ());
        gibbon_game_add_action (game, GIBBON_POSITION_SIDE_BLACK, action);
        game = gibbon_match_add_game (match);
        if (!gibbon_game_is_crawford (game)) {
                g_object_unref (match);
                g_printerr ("Crawford not detected at %s:%d.\n",
                            __FILE__, __LINE__);
                return FALSE;
        }

        action = GIBBON_GAME_ACTION (gibbon_resign_new (1));
        gibbon_game_add_action (game, GIBBON_POSITION_SIDE_WHITE, action);
        action = GIBBON_GAME_ACTION (gibbon_accept_new ());
        gibbon_game_add_action (game, GIBBON_POSITION_SIDE_BLACK, action);
        game = gibbon_match_add_game (match);
        if (gibbon_game_is_crawford (game)) {
                g_object_unref (match);
                g_printerr ("False positive for post-Crawford at %s:%d.\n",
                            __FILE__, __LINE__);
                return FALSE;
        }

        g_object_unref (match);

        return TRUE;
}
