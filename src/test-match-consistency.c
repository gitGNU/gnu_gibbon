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

#include "gibbon-match.h"
#include "gibbon-game.h"
#include "gibbon-position.h"
#include "gibbon-game-actions.h"

#define gibbon_error_reset(error)       \
	{                               \
                g_error_free (error);   \
                error = NULL;           \
        }

typedef gboolean (*test_function) (GibbonMatch *match, GError **error);

static gboolean check_opening (GibbonMatch *match, GError **error);
static gboolean check_roll (GibbonMatch *match, GError **error);
static gboolean check_move (GibbonMatch *match, GError **error);
static gboolean check_double (GibbonMatch *match, GError **error);
static gboolean check_resignation (GibbonMatch *match, GError **error);
static gboolean check_crawford (GibbonMatch *match, GError **error);
static gboolean check_premature_double (GibbonMatch *match, GError **error);

static test_function tests[] = {
    check_opening,
    check_roll,
    check_move,
    check_double,
    check_resignation,
    check_crawford,
    check_premature_double
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
                                      G_MININT64, error)) {
                g_printerr ("Adding opening double failed: %s\n",
                            (*error)->message);
                g_object_unref (action);
                return FALSE;
        }
        action = GIBBON_GAME_ACTION (gibbon_move_newv (1, 1, 8, 7, 8, 7,
                                                       6, 5, 6, 5, -1));
        if (gibbon_match_add_action (match, GIBBON_POSITION_SIDE_WHITE,
                                     action, G_MININT64, error)) {
                g_printerr ("White move after opening double succeded!\n");
                return FALSE;
        }
        gibbon_error_reset (*error);
        g_object_unref (action);
        action = GIBBON_GAME_ACTION (gibbon_move_newv (1, 1, 17, 21, 17, 21,
                                                       19, 21, 19, 21, -1));
        if (gibbon_match_add_action (match, GIBBON_POSITION_SIDE_BLACK,
                                      action, G_MININT64, error)) {
                g_printerr ("Black move after opening double succeded!\n");
                return FALSE;
        }
        gibbon_error_reset (*error);
        g_object_unref (action);

        action = GIBBON_GAME_ACTION (gibbon_roll_new (3, 1));
        if (!gibbon_match_add_action (match, GIBBON_POSITION_SIDE_NONE, action,
                                      G_MININT64, error)) {
                g_printerr ("Adding second opening roll after double failed: %s\n",
                            (*error)->message);
                g_object_unref (action);
                return FALSE;
        }

        action = GIBBON_GAME_ACTION (gibbon_move_newv (3, 1, 17, 21, 17, 21,
                                                       19, 21, 19, 21, -1));
        if (gibbon_match_add_action (match, GIBBON_POSITION_SIDE_BLACK,
                                     action, G_MININT64, error)) {
                g_printerr ("Black opening move although not on turn!\n");
                return FALSE;
        }
        gibbon_error_reset (*error);
        g_object_unref (action);

        action = GIBBON_GAME_ACTION (gibbon_move_newv (3, 1, 8, 7, 8, 7,
                                                       6, 5, 6, 5, -1));
        if (!gibbon_match_add_action (match, GIBBON_POSITION_SIDE_WHITE,
                                      action, G_MININT64, error)) {
                g_printerr ("White opening move failed: %s!\n",
                            (*error)->message);
                g_object_unref (action);
                return FALSE;
        }

        return TRUE;
}

static gboolean
check_roll (GibbonMatch *match, GError **error)
{
        GibbonGameAction *action;

        action = GIBBON_GAME_ACTION (gibbon_roll_new (3, 1));
        if (!gibbon_match_add_action (match, GIBBON_POSITION_SIDE_NONE, action,
                                      G_MININT64, error)) {
                g_printerr ("Adding normal opening roll failed: %s\n",
                            (*error)->message);
                g_object_unref (action);
                return FALSE;
        }

        action = GIBBON_GAME_ACTION (gibbon_roll_new (3, 1));
        if (gibbon_match_add_action (match, GIBBON_POSITION_SIDE_BLACK,
                                     action, G_MININT64, error)) {
                g_printerr ("Black roll after white roll succeded!\n");
                return FALSE;
        }
        gibbon_error_reset (*error);

        action = GIBBON_GAME_ACTION (gibbon_roll_new (3, 1));
        if (gibbon_match_add_action (match, GIBBON_POSITION_SIDE_WHITE,
                                     action, G_MININT64, error)) {
                g_printerr ("White rolling twice succeded!\n");
                return FALSE;
        }
        gibbon_error_reset (*error);
        g_object_unref (action);

        action = GIBBON_GAME_ACTION (gibbon_move_newv (3, 1, 8, 5, 6, 5,
                                                       -1));
        if (!gibbon_match_add_action (match, GIBBON_POSITION_SIDE_WHITE, action,
                                      G_MININT64, error)) {
                g_printerr ("Adding white move failed: %s\n",
                            (*error)->message);
                g_object_unref (action);
                return FALSE;
        }

        action = GIBBON_GAME_ACTION (gibbon_roll_new (3, 1));
        if (gibbon_match_add_action (match, GIBBON_POSITION_SIDE_WHITE,
                                     action, G_MININT64, error)) {
                g_printerr ("White roll after white move succeded!\n");
                return FALSE;
        }
        if (!gibbon_match_add_action (match, GIBBON_POSITION_SIDE_BLACK, action,
                                      G_MININT64, error)) {
                g_printerr ("Black roll after white move failed: %s\n",
                            (*error)->message);
                g_object_unref (action);
                return FALSE;
        }

        return TRUE;
}

static gboolean
check_move (GibbonMatch *match, GError **error)
{
        GibbonGameAction *action;

        action = GIBBON_GAME_ACTION (gibbon_roll_new (3, 1));
        if (!gibbon_match_add_action (match, GIBBON_POSITION_SIDE_NONE, action,
                                      G_MININT64, error)) {
                g_printerr ("Adding normal opening roll failed: %s\n",
                            (*error)->message);
                g_object_unref (action);
                return FALSE;
        }

        action = GIBBON_GAME_ACTION (gibbon_move_newv (3, 1, 17, 21, 19, 21,
                                                       -1));
        if (gibbon_match_add_action (match, GIBBON_POSITION_SIDE_BLACK,
                                     action, G_MININT64, error)) {
                g_printerr ("Black move after white roll succeded!\n");
                return FALSE;
        }
        gibbon_error_reset (*error);
        g_object_unref (action);

        action = GIBBON_GAME_ACTION (gibbon_move_newv (3, 1, 8, 5, 6, 5,
                                                       -1));
        if (!gibbon_match_add_action (match, GIBBON_POSITION_SIDE_WHITE, action,
                                      G_MININT64, error)) {
                g_printerr ("Adding white move failed: %s\n",
                            (*error)->message);
                g_object_unref (action);
                return FALSE;
        }

        action = GIBBON_GAME_ACTION (gibbon_move_newv (3, 1, 8, 5, 6, 5,
                                                       -1));
        if (gibbon_match_add_action (match, GIBBON_POSITION_SIDE_WHITE,
                                     action, G_MININT64, error)) {
                g_printerr ("White moving twice succeded!\n");
                return FALSE;
        }
        gibbon_error_reset (*error);
        g_object_unref (action);

        return TRUE;
}

static gboolean
check_double (GibbonMatch *match, GError **error)
{
        GibbonGameAction *action;

        action = GIBBON_GAME_ACTION (gibbon_roll_new (3, 1));
        if (!gibbon_match_add_action (match, GIBBON_POSITION_SIDE_NONE, action,
                                      G_MININT64, error)) {
                g_printerr ("Adding normal opening roll failed: %s\n",
                            (*error)->message);
                g_object_unref (action);
                return FALSE;
        }

        action = GIBBON_GAME_ACTION (gibbon_move_newv (3, 1, 8, 5, 6, 5,
                                                       -1));
        if (!gibbon_match_add_action (match, GIBBON_POSITION_SIDE_WHITE, action,
                                      G_MININT64, error)) {
                g_printerr ("Adding white move failed: %s\n",
                            (*error)->message);
                g_object_unref (action);
                return FALSE;
        }

        action = GIBBON_GAME_ACTION (gibbon_roll_new (3, 1));
        if (!gibbon_match_add_action (match, GIBBON_POSITION_SIDE_BLACK, action,
                                      G_MININT64, error)) {
                g_printerr ("Black's first roll failed: %s\n",
                            (*error)->message);
                g_object_unref (action);
                return FALSE;
        }

        action = GIBBON_GAME_ACTION (gibbon_double_new ());
        if (gibbon_match_add_action (match, GIBBON_POSITION_SIDE_BLACK,
                                     action, G_MININT64, error)) {
                g_printerr ("Double after roll succeded!\n");
                return FALSE;
        }
        gibbon_error_reset (*error);
        g_object_unref (action);

        action = GIBBON_GAME_ACTION (gibbon_move_newv (3, 1, 17, 21, 19, 21,
                                                       -1));
        if (!gibbon_match_add_action (match, GIBBON_POSITION_SIDE_BLACK, action,
                                      G_MININT64, error)) {
                g_printerr ("Adding black's first move failed: %s\n",
                            (*error)->message);
                g_object_unref (action);
                return FALSE;
        }

        action = GIBBON_GAME_ACTION (gibbon_double_new ());
        if (!gibbon_match_add_action (match, GIBBON_POSITION_SIDE_WHITE, action,
                                      G_MININT64, error)) {
                g_printerr ("Adding white's first double failed: %s\n",
                            (*error)->message);
                g_object_unref (action);
                return FALSE;
        }

        action = GIBBON_GAME_ACTION (gibbon_roll_new (6, 5));
        if (gibbon_match_add_action (match, GIBBON_POSITION_SIDE_WHITE,
                                     action, G_MININT64, error)) {
                g_printerr ("White roll after unresponded double succeded!\n");
                return FALSE;
        }
        gibbon_error_reset (*error);
        if (gibbon_match_add_action (match, GIBBON_POSITION_SIDE_BLACK,
                                     action, G_MININT64, error)) {
                g_printerr ("Black roll after unresponded double succeded!\n");
                return FALSE;
        }
        gibbon_error_reset (*error);
        g_object_unref (action);

        action = GIBBON_GAME_ACTION (gibbon_take_new ());
        if (gibbon_match_add_action (match, GIBBON_POSITION_SIDE_WHITE,
                                     action, G_MININT64, error)) {
                g_printerr ("White taking own double succeded!\n");
                return FALSE;
        }
        gibbon_error_reset (*error);
        g_object_unref (action);

        action = GIBBON_GAME_ACTION (gibbon_drop_new ());
        if (gibbon_match_add_action (match, GIBBON_POSITION_SIDE_WHITE,
                                     action, G_MININT64, error)) {
                g_printerr ("White dropping own double succeded!\n");
                return FALSE;
        }
        gibbon_error_reset (*error);
        g_object_unref (action);

        action = GIBBON_GAME_ACTION (gibbon_take_new ());
        if (!gibbon_match_add_action (match, GIBBON_POSITION_SIDE_BLACK, action,
                                      G_MININT64, error)) {
                g_printerr ("Adding black's first take failed: %s\n",
                            (*error)->message);
                g_object_unref (action);
                return FALSE;
        }

        action = GIBBON_GAME_ACTION (gibbon_roll_new (6, 5));
        if (!gibbon_match_add_action (match, GIBBON_POSITION_SIDE_WHITE, action,
                                      G_MININT64, error)) {
                g_printerr ("Adding white's roll failed: %s\n",
                            (*error)->message);
                g_object_unref (action);
                return FALSE;
        }

        action = GIBBON_GAME_ACTION (gibbon_move_newv (6, 5, 24, 18, 18, 13,
                                                       -1));
        if (!gibbon_match_add_action (match, GIBBON_POSITION_SIDE_WHITE, action,
                                      G_MININT64, error)) {
                g_printerr ("Adding white move failed: %s\n",
                            (*error)->message);
                g_object_unref (action);
                return FALSE;
        }

        action = GIBBON_GAME_ACTION (gibbon_roll_new (6, 5));
        if (!gibbon_match_add_action (match, GIBBON_POSITION_SIDE_BLACK, action,
                                      G_MININT64, error)) {
                g_printerr ("Adding black's roll failed: %s\n",
                            (*error)->message);
                g_object_unref (action);
                return FALSE;
        }

        action = GIBBON_GAME_ACTION (gibbon_move_newv (6, 5, 1, 7, 7, 12,
                                                       -1));
        if (!gibbon_match_add_action (match, GIBBON_POSITION_SIDE_BLACK, action,
                                      G_MININT64, error)) {
                g_printerr ("Adding black move failed: %s\n",
                            (*error)->message);
                g_object_unref (action);
                return FALSE;
        }

        action = GIBBON_GAME_ACTION (gibbon_double_new ());
        if (gibbon_match_add_action (match, GIBBON_POSITION_SIDE_WHITE,
                                     action, G_MININT64, error)) {
                g_printerr ("White can double, when black owns the cube!\n");
                return FALSE;
        }
        gibbon_error_reset (*error);
        g_object_unref (action);

        action = GIBBON_GAME_ACTION (gibbon_double_new ());
        if (gibbon_match_add_action (match, GIBBON_POSITION_SIDE_BLACK,
                                     action, G_MININT64, error)) {
                g_printerr ("Black can double, when not on turn!\n");
                return FALSE;
        }
        gibbon_error_reset (*error);
        g_object_unref (action);

        return TRUE;
}

static gboolean
check_resignation (GibbonMatch *match, GError **error)
{
        GibbonGameAction *action;

        action = GIBBON_GAME_ACTION (gibbon_roll_new (3, 1));
        if (!gibbon_match_add_action (match, GIBBON_POSITION_SIDE_NONE, action,
                                      G_MININT64, error)) {
                g_printerr ("Adding normal opening roll failed: %s\n",
                            (*error)->message);
                g_object_unref (action);
                return FALSE;
        }

        action = GIBBON_GAME_ACTION (gibbon_resign_new (1));
        if (!gibbon_match_add_action (match, GIBBON_POSITION_SIDE_WHITE, action,
                                      G_MININT64, error)) {
                g_printerr ("Adding white resignation failed: %s\n",
                            (*error)->message);
                g_object_unref (action);
                return FALSE;
        }

        action = GIBBON_GAME_ACTION (gibbon_roll_new (6, 5));
        if (gibbon_match_add_action (match, GIBBON_POSITION_SIDE_WHITE,
                                     action, G_MININT64, error)) {
                g_printerr ("White roll after unresponded resignation succeded!\n");
                return FALSE;
        }
        gibbon_error_reset (*error);
        if (gibbon_match_add_action (match, GIBBON_POSITION_SIDE_BLACK,
                                     action, G_MININT64, error)) {
                g_printerr ("Black roll after unresponded resignation succeded!\n");
                return FALSE;
        }
        gibbon_error_reset (*error);
        g_object_unref (action);

        action = GIBBON_GAME_ACTION (gibbon_move_newv (6, 5, 8, 5, 6, 5, -1));
        if (gibbon_match_add_action (match, GIBBON_POSITION_SIDE_WHITE,
                                     action, G_MININT64, error)) {
                g_printerr ("White move after unresponded resignation succeded!\n");
                return FALSE;
        }
        gibbon_error_reset (*error);
        if (gibbon_match_add_action (match, GIBBON_POSITION_SIDE_BLACK,
                                     action, G_MININT64, error)) {
                g_printerr ("Black move after unresponded resignation succeded!\n");
                return FALSE;
        }
        gibbon_error_reset (*error);
        g_object_unref (action);

        action = GIBBON_GAME_ACTION (gibbon_double_new ());
        if (gibbon_match_add_action (match, GIBBON_POSITION_SIDE_WHITE,
                                     action, G_MININT64, error)) {
                g_printerr ("White double after unresponded resignation succeded!\n");
                return FALSE;
        }
        gibbon_error_reset (*error);
        if (gibbon_match_add_action (match, GIBBON_POSITION_SIDE_BLACK,
                                     action, G_MININT64, error)) {
                g_printerr ("Black double after unresponded resignation succeded!\n");
                return FALSE;
        }
        gibbon_error_reset (*error);
        g_object_unref (action);

        action = GIBBON_GAME_ACTION (gibbon_resign_new (1));
        if (gibbon_match_add_action (match, GIBBON_POSITION_SIDE_WHITE,
                                     action, G_MININT64, error)) {
                g_printerr ("White resignation after unresponded resignation succeded!\n");
                return FALSE;
        }
        gibbon_error_reset (*error);
        if (gibbon_match_add_action (match, GIBBON_POSITION_SIDE_BLACK,
                                     action, G_MININT64, error)) {
                g_printerr ("Black resignation after unresponded resignation succeded!\n");
                return FALSE;
        }
        gibbon_error_reset (*error);
        g_object_unref (action);

        action = GIBBON_GAME_ACTION (gibbon_accept_new ());
        if (gibbon_match_add_action (match, GIBBON_POSITION_SIDE_WHITE,
                                     action, G_MININT64, error)) {
                g_printerr ("White accepting own resignation succeded!\n");
                return FALSE;
        }
        gibbon_error_reset (*error);
        g_object_unref (action);

        action = GIBBON_GAME_ACTION (gibbon_reject_new ());
        if (gibbon_match_add_action (match, GIBBON_POSITION_SIDE_WHITE,
                                     action, G_MININT64, error)) {
                g_printerr ("White rejecting own resignation succeded!\n");
                return FALSE;
        }
        gibbon_error_reset (*error);
        g_object_unref (action);

        return TRUE;
}

static gboolean
check_crawford (GibbonMatch *match, GError **error)
{
        GibbonGameAction *action;

        gibbon_match_set_length (match, 5);
        gibbon_match_set_crawford (match, TRUE);

        action = GIBBON_GAME_ACTION (gibbon_roll_new (3, 1));
        if (!gibbon_match_add_action (match, GIBBON_POSITION_SIDE_NONE, action,
                                      G_MININT64, error)) {
                g_printerr ("Adding normal opening roll failed: %s\n",
                            (*error)->message);
                g_object_unref (action);
                return FALSE;
        }

        action = GIBBON_GAME_ACTION (gibbon_move_newv (3, 1, 8, 5, 6, 5,
                                                       -1));
        if (!gibbon_match_add_action (match, GIBBON_POSITION_SIDE_WHITE, action,
                                      G_MININT64, error)) {
                g_printerr ("Adding white move failed: %s\n",
                            (*error)->message);
                g_object_unref (action);
                return FALSE;
        }

        action = GIBBON_GAME_ACTION (gibbon_double_new ());
        if (!gibbon_match_add_action (match, GIBBON_POSITION_SIDE_BLACK, action,
                                      G_MININT64, error)) {
                g_printerr ("Normal black double failed: %s\n",
                            (*error)->message);
                g_object_unref (action);
                return FALSE;
        }

        action = GIBBON_GAME_ACTION (gibbon_take_new ());
        if (!gibbon_match_add_action (match, GIBBON_POSITION_SIDE_WHITE, action,
                                      G_MININT64, error)) {
                g_printerr ("Normal white take failed: %s\n",
                            (*error)->message);
                g_object_unref (action);
                return FALSE;
        }

        action = GIBBON_GAME_ACTION (gibbon_resign_new (4));
        if (!gibbon_match_add_action (match, GIBBON_POSITION_SIDE_BLACK, action,
                                      G_MININT64, error)) {
                g_printerr ("Normal black resign failed: %s\n",
                            (*error)->message);
                g_object_unref (action);
                return FALSE;
        }

        action = GIBBON_GAME_ACTION (gibbon_accept_new ());
        if (!gibbon_match_add_action (match, GIBBON_POSITION_SIDE_WHITE, action,
                                      G_MININT64, error)) {
                g_printerr ("Normal white accept failed: %s\n",
                            (*error)->message);
                g_object_unref (action);
                return FALSE;
        }

        action = GIBBON_GAME_ACTION (gibbon_roll_new (3, 1));
        if (!gibbon_match_add_action (match, GIBBON_POSITION_SIDE_NONE, action,
                                      G_MININT64, error)) {
                g_printerr ("Adding normal opening roll failed: %s\n",
                            (*error)->message);
                g_object_unref (action);
                return FALSE;
        }

        action = GIBBON_GAME_ACTION (gibbon_move_newv (3, 1, 8, 5, 6, 5,
                                                       -1));
        if (!gibbon_match_add_action (match, GIBBON_POSITION_SIDE_WHITE, action,
                                      G_MININT64, error)) {
                g_printerr ("Adding white move failed: %s\n",
                            (*error)->message);
                g_object_unref (action);
                return FALSE;
        }

        action = GIBBON_GAME_ACTION (gibbon_double_new ());
        if (gibbon_match_add_action (match, GIBBON_POSITION_SIDE_BLACK,
                                     action, G_MININT64, error)) {
                g_printerr ("Double during Crawford game possible!\n");
                return FALSE;
        }
        gibbon_error_reset (*error);
        g_object_unref (action);

        return TRUE;
}


static gboolean
check_premature_double (GibbonMatch *match, GError **error)
{
        GibbonGameAction *action;

        gibbon_match_set_length (match, 5);
        gibbon_match_set_crawford (match, TRUE);

        action = GIBBON_GAME_ACTION (gibbon_double_new ());
        if (gibbon_match_add_action (match, GIBBON_POSITION_SIDE_WHITE,
                                     action, G_MININT64, error)) {
                g_printerr ("White double before opening roll possible!\n");
                return FALSE;
        }
        gibbon_error_reset (*error);
        g_object_unref (action);

        action = GIBBON_GAME_ACTION (gibbon_double_new ());
        if (gibbon_match_add_action (match, GIBBON_POSITION_SIDE_WHITE,
                                     action, G_MININT64, error)) {
                g_printerr ("Black double before opening roll possible!\n");
                return FALSE;
        }
        gibbon_error_reset (*error);
        g_object_unref (action);

        action = GIBBON_GAME_ACTION (gibbon_resign_new (1));
        if (!gibbon_match_add_action (match, GIBBON_POSITION_SIDE_WHITE, action,
                                      G_MININT64, error)) {
                g_printerr ("White resignation failed: %s\n",
                            (*error)->message);
                g_object_unref (action);
                return FALSE;
        }
        action = GIBBON_GAME_ACTION (gibbon_reject_new ());
        if (!gibbon_match_add_action (match, GIBBON_POSITION_SIDE_BLACK, action,
                                      G_MININT64, error)) {
                g_printerr ("Black rejection failed: %s\n",
                            (*error)->message);
                g_object_unref (action);
                return FALSE;
        }

        action = GIBBON_GAME_ACTION (gibbon_resign_new (2));
        if (!gibbon_match_add_action (match, GIBBON_POSITION_SIDE_BLACK, action,
                                      G_MININT64, error)) {
                g_printerr ("Black resignation failed: %s\n",
                            (*error)->message);
                g_object_unref (action);
                return FALSE;
        }
        action = GIBBON_GAME_ACTION (gibbon_reject_new ());
        if (!gibbon_match_add_action (match, GIBBON_POSITION_SIDE_WHITE, action,
                                      G_MININT64, error)) {
                g_printerr ("White rejection failed: %s\n",
                            (*error)->message);
                g_object_unref (action);
                return FALSE;
        }


        action = GIBBON_GAME_ACTION (gibbon_double_new ());
        if (gibbon_match_add_action (match, GIBBON_POSITION_SIDE_WHITE,
                                     action, G_MININT64, error)) {
                g_printerr ("White double before opening roll after"
                            " rejection possible!\n");
                return FALSE;
        }
        gibbon_error_reset (*error);
        g_object_unref (action);

        action = GIBBON_GAME_ACTION (gibbon_double_new ());
        if (gibbon_match_add_action (match, GIBBON_POSITION_SIDE_WHITE,
                                     action, G_MININT64, error)) {
                g_printerr ("Black double before opening roll after"
                            " rejection possible!\n");
                return FALSE;
        }
        gibbon_error_reset (*error);
        g_object_unref (action);

        return TRUE;
}
