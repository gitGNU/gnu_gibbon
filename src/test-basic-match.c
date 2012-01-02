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

static GibbonMatch *fill_match (void);
static gboolean check_match (const GibbonMatch *match);
static gboolean serialize_match (const GibbonMatch *match);

int
main(int argc, char *argv[])
{
	int status = 0;
	GibbonMatch *match;

        g_type_init ();

        match = fill_match ();
        if (!match)
                return -1;

        if (!check_match (match))
                status = -1;

        if (!serialize_match (match))
                status = -1;

        g_object_unref (match);

        return status;
}

static GibbonMatch *
fill_match (void)
{
        GibbonMatch *match = gibbon_match_new ("Snow White", "Joe Black",
                                               0, TRUE);
        GibbonGame *game;
        GibbonGameAction *action;
        gint score;
        GibbonPosition *pos;

        game = gibbon_match_get_current_game (match);
        if (!game)
                g_printerr ("Fresh match has no game.\n");

        action = GIBBON_GAME_ACTION (gibbon_roll_new (3, 1));
        gibbon_game_add_action (game, GIBBON_POSITION_SIDE_WHITE, action);

        action = GIBBON_GAME_ACTION (gibbon_move_newv (3, 1, 8, 5, 6, 5, -1));
        gibbon_game_add_action (game, GIBBON_POSITION_SIDE_WHITE, action);

        action = GIBBON_GAME_ACTION (gibbon_roll_new (5, 2));
        gibbon_game_add_action (game, GIBBON_POSITION_SIDE_BLACK, action);

        action = GIBBON_GAME_ACTION (gibbon_move_newv (5, 2, 12, 17, 1, 3, -1));
        gibbon_game_add_action (game, GIBBON_POSITION_SIDE_BLACK, action);

        action = GIBBON_GAME_ACTION (gibbon_roll_new (5, 5));
        gibbon_game_add_action (game, GIBBON_POSITION_SIDE_WHITE, action);

        action = GIBBON_GAME_ACTION (gibbon_move_newv (5, 5, 8, 3, 8, 3, 6, 1,
                                                       6, 1, -1));
        gibbon_game_add_action (game, GIBBON_POSITION_SIDE_WHITE, action);

        action = GIBBON_GAME_ACTION (gibbon_roll_new (4, 5));
        gibbon_game_add_action (game, GIBBON_POSITION_SIDE_BLACK, action);

        action = GIBBON_GAME_ACTION (gibbon_move_newv (4, 5, 0, 4, -1));
        gibbon_game_add_action (game, GIBBON_POSITION_SIDE_BLACK, action);

        action = GIBBON_GAME_ACTION (gibbon_double_new ());
        gibbon_game_add_action (game, GIBBON_POSITION_SIDE_WHITE, action);

        if (gibbon_game_over (game)) {
                g_object_unref (match);
                g_printerr ("Premature end of game before drop.\n");
                return NULL;
        }

        action = GIBBON_GAME_ACTION (gibbon_drop_new ());
        gibbon_game_add_action (game, GIBBON_POSITION_SIDE_BLACK, action);

        score = gibbon_game_over (game);
        if (1 != score) {
                g_object_unref (match);
                g_printerr ("Score should be %d, not %d after black's"
                             " drop!\n", 1, score);
                return NULL;
        }

        game = gibbon_match_add_game (match);
        if (!game) {
                g_object_unref (match);
                g_printerr ("Cannot add 2nd game!\n");
                return NULL;
        }

        action = GIBBON_GAME_ACTION (gibbon_roll_new (5, 2));
        gibbon_game_add_action (game, GIBBON_POSITION_SIDE_BLACK, action);

        action = GIBBON_GAME_ACTION (gibbon_move_newv (5, 2, 12, 17, 1, 3, -1));
        gibbon_game_add_action (game, GIBBON_POSITION_SIDE_BLACK, action);

        action = GIBBON_GAME_ACTION (gibbon_roll_new (5, 5));
        gibbon_game_add_action (game, GIBBON_POSITION_SIDE_WHITE, action);

        action = GIBBON_GAME_ACTION (gibbon_move_newv (5, 5, 8, 3, 8, 3, 6, 1,
                                                       6, 1, -1));
        gibbon_game_add_action (game, GIBBON_POSITION_SIDE_WHITE, action);

        action = GIBBON_GAME_ACTION (gibbon_double_new ());
        gibbon_game_add_action (game, GIBBON_POSITION_SIDE_BLACK, action);

        action = GIBBON_GAME_ACTION (gibbon_double_new ());
        gibbon_game_add_action (game, GIBBON_POSITION_SIDE_WHITE, action);

        action = GIBBON_GAME_ACTION (gibbon_take_new ());
        gibbon_game_add_action (game, GIBBON_POSITION_SIDE_BLACK, action);

        action = GIBBON_GAME_ACTION (gibbon_roll_new (3, 1));
        gibbon_game_add_action (game, GIBBON_POSITION_SIDE_BLACK, action);
        action = GIBBON_GAME_ACTION (gibbon_move_newv (3, 1, -1));
        gibbon_game_add_action (game, GIBBON_POSITION_SIDE_BLACK, action);

        action = GIBBON_GAME_ACTION (gibbon_resign_new (1));
        gibbon_game_add_action (game, GIBBON_POSITION_SIDE_WHITE, action);

        action = GIBBON_GAME_ACTION (gibbon_reject_new ());
        gibbon_game_add_action (game, GIBBON_POSITION_SIDE_BLACK, action);

        action = GIBBON_GAME_ACTION (gibbon_resign_new (2));
        gibbon_game_add_action (game, GIBBON_POSITION_SIDE_WHITE, action);

        action = GIBBON_GAME_ACTION (gibbon_accept_new ());
        gibbon_game_add_action (game, GIBBON_POSITION_SIDE_BLACK, action);

        score = gibbon_game_over (game);
        if (-8 != score) {
                g_object_unref (match);
                g_printerr ("Score should be %d, not %d after white's"
                             " resignation!\n", -8, score);
                return NULL;
        }

        game = gibbon_match_add_game (match);
        if (!game) {
                g_object_unref (match);
                g_printerr ("Cannot add 3rd game!\n");
                return NULL;
        }

        pos = gibbon_position_new ();
        pos->players[0] = g_strdup ("Snow White");
        pos->players[1] = g_strdup ("Joe Black");
        memset (pos->points, 0, sizeof pos->points);
        pos->points[0] = 1;
        pos->points[1] = 1;
        pos->points[17] = -2;
        pos->points[18] = -7;
        pos->points[19] = -6;

        action = GIBBON_GAME_ACTION (gibbon_setup_new (pos));
        gibbon_game_add_action (game, GIBBON_POSITION_SIDE_BLACK, action);

        action = GIBBON_GAME_ACTION (gibbon_roll_new (3, 1));
        gibbon_game_add_action (game, GIBBON_POSITION_SIDE_WHITE, action);
        action = GIBBON_GAME_ACTION (gibbon_move_newv (3, 1, 2, 0, 1, 0, -1));
        gibbon_game_add_action (game, GIBBON_POSITION_SIDE_WHITE, action);

        return match;
}

static gboolean
check_match (const GibbonMatch *match)
{
        gboolean retval = TRUE;
        const gchar *got;
        const gchar *expect;
        gint expect_int, got_int;
        const GibbonPosition *pos;

        pos = gibbon_match_get_current_position (match);
        g_return_val_if_fail (pos != NULL, FALSE);

        got = pos->players[1];
        expect = "Joe Black";
        if (g_strcmp0 (expect, got)) {
                g_printerr ("Expected `%s', got `%s'!\n", expect, got);
                retval = FALSE;
        }

        got = pos->players[0];
        expect = "Snow White";
        if (g_strcmp0 (expect, got)) {
                g_printerr ("Expected `%s', got `%s'!\n", expect, got);
                retval = FALSE;
        }

        expect_int = 0;
        got_int = pos->match_length;
        if (expect_int != got_int) {
                g_printerr ("Expected match length %d, got %d.\n",
                            expect_int, got_int);
                retval = FALSE;
        }

        return retval;
}

static gboolean
serialize_match (const GibbonMatch *match)
{
        const GSGFCollection *collection = gibbon_match_get_collection (match);
        GOutputStream *out = g_memory_output_stream_new (NULL, 0,
                                                         g_realloc, g_free);
        GError *error = NULL;
        gsize written;

        if (!gsgf_component_write_stream (GSGF_COMPONENT (collection), out,
                                          &written, NULL, &error)) {
                g_printerr ("Writing basic match failed: %s.\n",
                            error->message);
                return FALSE;
        }

#if (1)
        g_printerr ("%s",
                    (gchar *) g_memory_output_stream_get_data  (
                                    G_MEMORY_OUTPUT_STREAM (out)));
#endif

        g_object_unref (out);

        return TRUE;
}
