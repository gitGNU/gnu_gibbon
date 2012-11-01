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
#include <gio/gio.h>

#include "gibbon-match.h"
#include "gibbon-game.h"
#include "gibbon-roll.h"
#include "gibbon-move.h"
#include "gibbon-double.h"
#include "gibbon-drop.h"

int
main(int argc, char *argv[])
{
        GibbonMatch *match;
        const GibbonGame *game;
        const GibbonGame *last_game;
        GibbonGameAction *action;
        GError *error = NULL;

        g_type_init ();

        match = gibbon_match_new (NULL, NULL, 0, FALSE);
        last_game = gibbon_match_get_current_game (match);

        action = GIBBON_GAME_ACTION (gibbon_roll_new (2, 1));
        if (last_game) {
                g_printerr ("Current game before move.\n");
                return 1;
        }
        if (!gibbon_match_add_action (match, GIBBON_POSITION_SIDE_WHITE,
                                      action, G_MININT64, &error)) {
                g_printerr ("Error applying roll 21: %s\n", error->message);
                return 1;
        }
        game = gibbon_match_get_current_game (match);
        if (!game) {
                g_printerr ("No current game after roll.\n");
                return 1;
        }
        last_game = game;

        action = GIBBON_GAME_ACTION (gibbon_move_newv (2, 1, 13, 11, 24, 21, -1));
        if (!gibbon_match_add_action (match, GIBBON_POSITION_SIDE_WHITE,
                                      action, G_MININT64, &error)) {
                g_printerr ("Error applying move: %s\n", error->message);
                return 1;
        }
        game = gibbon_match_get_current_game (match);
        if (game != last_game) {
                g_printerr ("Game changed after move.\n");
                return 1;
        }
        last_game = game;

        action = GIBBON_GAME_ACTION (gibbon_double_new ());
        if (!gibbon_match_add_action (match, GIBBON_POSITION_SIDE_BLACK,
                                      action, G_MININT64, &error)) {
                g_printerr ("Error offering double: %s\n", error->message);
                return 1;
        }
        game = gibbon_match_get_current_game (match);
        if (game != last_game) {
                g_printerr ("Game changed after double.\n");
                return 1;
        }
        last_game = game;

        action = GIBBON_GAME_ACTION (gibbon_drop_new ());
        if (!gibbon_match_add_action (match, GIBBON_POSITION_SIDE_WHITE,
                                      action, G_MININT64, &error)) {
                g_printerr ("Error dropping double: %s\n", error->message);
                return 1;
        }
        game = gibbon_match_get_current_game (match);
        if (game == last_game) {
                g_printerr ("Game did not change after double.\n");
                return 1;
        }
        last_game = game;

        action = GIBBON_GAME_ACTION (gibbon_roll_new (3, 1));
        if (!gibbon_match_add_action (match, GIBBON_POSITION_SIDE_WHITE,
                                      action, G_MININT64, &error)) {
                g_printerr ("Error applying roll 31: %s\n", error->message);
                return 1;
        }
        game = gibbon_match_get_current_game (match);
        if (game != last_game) {
                g_printerr ("Game changed after first roll in 2nd game.\n");
                return 1;
        }
        last_game = game;

        g_object_unref (match);

        return 0;
}
