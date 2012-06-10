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
#include <gibbon-position.h>
#include <gibbon-move.h>
#include <gibbon-double.h>
#include <gibbon-drop.h>
#include <gibbon-roll.h>
#include <gibbon-take.h>
#include <gibbon-resign.h>
#include <gibbon-reject.h>
#include <gibbon-accept.h>
#include <gibbon-setup.h>

#define gibbon_error_reset(error)       \
	{                               \
                g_error_free (error);   \
                error = NULL;           \
        }

typedef gboolean (*test_function) (GibbonMatch *match, GError **error);

static gboolean check_opening (GibbonMatch *match, GError **error);

static test_function tests[] = {
    check_opening
};

int
main(int argc, char *argv[])
{
	GibbonMatch *match;
	GibbonGame *game;
	GError *error = NULL;
	gsize i;
	int status = 0;

        g_type_init ();

        for (i = 0; i < sizeof tests / sizeof tests[0]; ++i) {
                match = gibbon_match_new ("SnowWhite", "JoeBlack", 5, TRUE);
                g_return_val_if_fail (match != NULL, -1);
                game = gibbon_match_add_game (match, NULL);
                g_return_val_if_fail (game != NULL, -1);
                if (!tests[i] (match, &error))
                        status = -1;
                g_object_unref (match);
                if (error)
                        g_error_free (error);
                error = NULL;
        }

        return status;
}

static gboolean
check_opening (GibbonMatch *match, GError **error)
{
        GibbonGameAction *action;

        action = GIBBON_GAME_ACTION (gibbon_roll_new (1, 1));
        if (!gibbon_match_add_action (match, GIBBON_POSITION_SIDE_NONE, action,
                                      error)) {
                g_printerr ("Adding opening double failed: %s\n",
                            (*error)->message);
                g_object_unref (action);
                return FALSE;
        }
        action = GIBBON_GAME_ACTION (gibbon_move_newv (1, 1, 8, 7, 8, 7,
                                                       6, 5, 6, 5, -1));
        if (gibbon_match_add_action (match, GIBBON_POSITION_SIDE_WHITE,
                                      action, error)) {
                g_printerr ("White move after opening double succeded!\n");
                return FALSE;
        }
        gibbon_error_reset (*error);
        g_object_unref (action);
        action = GIBBON_GAME_ACTION (gibbon_move_newv (1, 1, 17, 21, 17, 21,
                                                       19, 21, 19, 21, -1));
        if (gibbon_match_add_action (match, GIBBON_POSITION_SIDE_BLACK,
                                      action, error)) {
                g_printerr ("White move after opening double succeded!\n");
                return FALSE;
        }
        gibbon_error_reset (*error);
        g_object_unref (action);

        action = GIBBON_GAME_ACTION (gibbon_roll_new (3, 1));
        if (!gibbon_match_add_action (match, GIBBON_POSITION_SIDE_NONE, action,
                                      error)) {
                g_printerr ("Adding second opening roll after double failed: %s\n",
                            (*error)->message);
                g_object_unref (action);
                return FALSE;
        }

        return TRUE;
}
