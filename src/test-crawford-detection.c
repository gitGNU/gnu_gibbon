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
        GError *error = NULL;

        game = gibbon_match_add_game (match, &error);
        if (!game) {
                g_object_unref (match);
                g_printerr ("%s:%d: Cannot add 1st game: %s.\n",
                            __FILE__, __LINE__, error->message);
                g_error_free (error);
                return FALSE;
        }

        if (gibbon_game_is_crawford (game)) {
                g_object_unref (match);
                g_printerr ("%s:%d: First game cannot be Crawford.\n",
                            __FILE__, __LINE__);
                return FALSE;
        }

        action = GIBBON_GAME_ACTION (gibbon_roll_new (3, 1));
        if (!gibbon_game_add_action (game, GIBBON_POSITION_SIDE_WHITE, action,
                                     G_MININT64, &error)) {
                g_object_unref (match);
                g_printerr ("%s:%d: Cannot add game action: %s\n",
                            __FILE__, __LINE__, error->message);
                g_error_free (error);
                return FALSE;
        }

        action = GIBBON_GAME_ACTION (gibbon_move_newv (3, 1, 8, 5, 6, 5, -1));
        if (!gibbon_game_add_action (game, GIBBON_POSITION_SIDE_WHITE, action,
                                     G_MININT64, &error)) {
                g_object_unref (match);
                g_printerr ("%s:%d: Cannot add game action: %s\n",
                            __FILE__, __LINE__, error->message);
                g_error_free (error);
                return FALSE;
        }

        action = GIBBON_GAME_ACTION (gibbon_roll_new (5, 2));
        if (!gibbon_game_add_action (game, GIBBON_POSITION_SIDE_BLACK, action,
                                     G_MININT64, &error)) {
                g_object_unref (match);
                g_printerr ("%s:%d: Cannot add game action: %s\n",
                            __FILE__, __LINE__, error->message);
                g_error_free (error);
                return FALSE;
        }

        action = GIBBON_GAME_ACTION (gibbon_move_newv (5, 2, 12, 17, 1, 3, -1));
        if (!gibbon_game_add_action (game, GIBBON_POSITION_SIDE_BLACK, action,
                                     G_MININT64, &error)) {
                g_object_unref (match);
                g_printerr ("%s:%d: Cannot add game action: %s\n",
                            __FILE__, __LINE__, error->message);
                g_error_free (error);
                return FALSE;
        }

        action = GIBBON_GAME_ACTION (gibbon_double_new ());
        if (!gibbon_game_add_action (game, GIBBON_POSITION_SIDE_WHITE, action,
                                     G_MININT64, &error)) {
                g_object_unref (match);
                g_printerr ("%s:%d: Cannot add game action: %s\n",
                            __FILE__, __LINE__, error->message);
                g_error_free (error);
                return FALSE;
        }

        action = GIBBON_GAME_ACTION (gibbon_take_new ());
        if (!gibbon_game_add_action (game, GIBBON_POSITION_SIDE_BLACK, action,
                                     G_MININT64, &error)) {
                g_object_unref (match);
                g_printerr ("%s:%d: Cannot add game action: %s\n",
                            __FILE__, __LINE__, error->message);
                g_error_free (error);
                return FALSE;
        }

        action = GIBBON_GAME_ACTION (gibbon_roll_new (5, 5));
        if (!gibbon_game_add_action (game, GIBBON_POSITION_SIDE_WHITE, action,
                                     G_MININT64, &error)) {
                g_object_unref (match);
                g_printerr ("%s:%d: Cannot add game action: %s\n",
                            __FILE__, __LINE__, error->message);
                g_error_free (error);
                return FALSE;
        }

        action = GIBBON_GAME_ACTION (gibbon_move_newv (5, 5, 8, 3, 8, 3, 6, 1,
                                                       6, 1, -1));
        if (!gibbon_game_add_action (game, GIBBON_POSITION_SIDE_WHITE, action,
                                     G_MININT64, &error)) {
                g_object_unref (match);
                g_printerr ("%s:%d: Cannot add game action: %s\n",
                            __FILE__, __LINE__, error->message);
                g_error_free (error);
                return FALSE;
        }

        action = GIBBON_GAME_ACTION (gibbon_resign_new (2));
        if (!gibbon_game_add_action (game, GIBBON_POSITION_SIDE_BLACK, action,
                                     G_MININT64, &error)) {
                g_object_unref (match);
                g_printerr ("%s:%d: Cannot add game action: %s\n",
                            __FILE__, __LINE__, error->message);
                g_error_free (error);
                return FALSE;
        }

        action = GIBBON_GAME_ACTION (gibbon_accept_new ());
        if (!gibbon_game_add_action (game, GIBBON_POSITION_SIDE_WHITE, action,
                                     G_MININT64, &error)) {
                g_object_unref (match);
                g_printerr ("%s:%d: Cannot add game action: %s\n",
                            __FILE__, __LINE__, error->message);
                g_error_free (error);
                return FALSE;
        }

        game = gibbon_match_add_game (match, &error);
        if (!game) {
                g_object_unref (match);
                g_printerr ("%s:%d: Cannot add game: %s\n",
                            __FILE__, __LINE__, error->message);
                g_error_free (error);
                return FALSE;
        }

        if (!gibbon_game_is_crawford (game)) {
                g_object_unref (match);
                g_printerr ("%s:%d: Crawford game to_nil not detected.\n",
                            __FILE__, __LINE__);
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
        GError *error = NULL;

        game = gibbon_match_add_game (match, &error);
        if (!game) {
                g_object_unref (match);
                g_printerr ("%s:%d: Cannot add 1st game: %s!\n",
                            __FILE__, __LINE__, error->message);
                return FALSE;
        }

        if (gibbon_game_is_crawford (game)) {
                g_object_unref (match);
                g_printerr ("%s:%d: First game cannot be Crawford.\n",
                            __FILE__, __LINE__);
                return FALSE;
        }

        action = GIBBON_GAME_ACTION (gibbon_resign_new (2));
        if (!gibbon_game_add_action (game, GIBBON_POSITION_SIDE_WHITE, action,
                                     G_MININT64, &error)) {
                g_object_unref (match);
                g_printerr ("%s:%d: Cannot add game action: %s\n",
                            __FILE__, __LINE__, error->message);
                g_error_free (error);
                return FALSE;
        }
        action = GIBBON_GAME_ACTION (gibbon_accept_new ());
        if (!gibbon_game_add_action (game, GIBBON_POSITION_SIDE_BLACK, action,
                                     G_MININT64, &error)) {
                g_object_unref (match);
                g_printerr ("%s:%d: Cannot add game action: %s\n",
                            __FILE__, __LINE__, error->message);
                g_error_free (error);
                return FALSE;
        }

        game = gibbon_match_add_game (match, &error);
        if (!game) {
                g_object_unref (match);
                g_printerr ("%s:%d: Cannot add game: %s\n",
                            __FILE__, __LINE__, error->message);
                g_error_free (error);
                return FALSE;
        }
        if (gibbon_game_is_crawford (game)) {
                g_object_unref (match);
                g_printerr ("%s:%d: False positive for Crawford.\n",
                            __FILE__, __LINE__);
                return FALSE;
        }
        action = GIBBON_GAME_ACTION (gibbon_resign_new (1));
        if (!gibbon_game_add_action (game, GIBBON_POSITION_SIDE_BLACK, action,
                                     G_MININT64, &error)) {
                g_object_unref (match);
                g_printerr ("%s:%d: Cannot add game action: %s\n",
                            __FILE__, __LINE__, error->message);
                g_error_free (error);
                return FALSE;
        }
        action = GIBBON_GAME_ACTION (gibbon_accept_new ());
        if (!gibbon_game_add_action (game, GIBBON_POSITION_SIDE_WHITE, action,
                                     G_MININT64, &error)) {
                g_object_unref (match);
                g_printerr ("%s:%d: Cannot add game action: %s\n",
                            __FILE__, __LINE__, error->message);
                g_error_free (error);
                return FALSE;
        }

        game = gibbon_match_add_game (match, &error);
        if (!game) {
                g_object_unref (match);
                g_printerr ("%s:%d: Cannot add game: %s\n",
                            __FILE__, __LINE__, error->message);
                g_error_free (error);
                return FALSE;
        }
        if (gibbon_game_is_crawford (game)) {
                g_object_unref (match);
                g_printerr ("%s:%d: False positive for Crawford.\n",
                            __FILE__, __LINE__);
                return FALSE;
        }

        action = GIBBON_GAME_ACTION (gibbon_resign_new (1));
        if (!gibbon_game_add_action (game, GIBBON_POSITION_SIDE_WHITE, action,
                                     G_MININT64, &error)) {
                g_object_unref (match);
                g_printerr ("%s:%d: Cannot add game action: %s\n",
                            __FILE__, __LINE__, error->message);
                g_error_free (error);
                return FALSE;
        }
        action = GIBBON_GAME_ACTION (gibbon_accept_new ());
        if (!gibbon_game_add_action (game, GIBBON_POSITION_SIDE_BLACK, action,
                                     G_MININT64, &error)) {
                g_object_unref (match);
                g_printerr ("%s:%d: Cannot add game action: %s\n",
                            __FILE__, __LINE__, error->message);
                g_error_free (error);
                return FALSE;
        }

        game = gibbon_match_add_game (match, &error);
        if (!game) {
                g_object_unref (match);
                g_printerr ("%s:%d: Cannot add game: %s\n",
                            __FILE__, __LINE__, error->message);
                g_error_free (error);
                return FALSE;
        }
        if (!gibbon_game_is_crawford (game)) {
                g_object_unref (match);
                g_printerr ("%s:%d: Crawford not detected.\n",
                            __FILE__, __LINE__);
                return FALSE;
        }

        action = GIBBON_GAME_ACTION (gibbon_resign_new (1));
        if (!gibbon_game_add_action (game, GIBBON_POSITION_SIDE_BLACK, action,
                                     G_MININT64, &error)) {
                g_object_unref (match);
                g_printerr ("%s:%d: Cannot add game action: %s\n",
                            __FILE__, __LINE__, error->message);
                g_error_free (error);
                return FALSE;
        }
        action = GIBBON_GAME_ACTION (gibbon_accept_new ());
        if (!gibbon_game_add_action (game, GIBBON_POSITION_SIDE_WHITE, action,
                                     G_MININT64, &error)) {
                g_object_unref (match);
                g_printerr ("%s:%d: Cannot add game action: %s\n",
                            __FILE__, __LINE__, error->message);
                g_error_free (error);
                return FALSE;
        }

        game = gibbon_match_add_game (match, &error);
        if (!game) {
                g_object_unref (match);
                g_printerr ("%s:%d: Cannot add game: %s\n",
                            __FILE__, __LINE__, error->message);
                g_error_free (error);
                return FALSE;
        }

        if (gibbon_game_is_crawford (game)) {
                g_object_unref (match);
                g_printerr ("%s:%d: False positive for post-Crawford.\n",
                            __FILE__, __LINE__);
                return FALSE;
        }

        g_object_unref (match);

        return TRUE;
}
