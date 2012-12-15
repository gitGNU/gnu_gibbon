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
#include "gibbon-gmd-writer.h"

int
main(int argc, char *argv[])
{
        GibbonMatch *match;
        const GibbonGame *game;
        const GibbonGame *last_game;
        GibbonGameAction *action;
        GError *error = NULL;
        GOutputStream *out;
        GibbonGMDWriter *writer;
        gchar *gmd_out;
        gchar *wanted;

        g_type_init ();

        out = G_OUTPUT_STREAM (g_memory_output_stream_new (NULL, 0,
                                                           g_realloc,
                                                           g_free));
        writer = gibbon_gmd_writer_new ();

        match = gibbon_match_new (NULL, NULL, 0, FALSE);
        if (!gibbon_match_writer_write_stream (GIBBON_MATCH_WRITER (writer),
                                               out, match, &error)) {
                g_printerr ("Error writing initial match: %s\n",
                            error->message);
                return 1;
        }
        if (!gibbon_gmd_writer_add_game (writer, out, &error)) {
                g_printerr ("Error writing 1st game start: %s\n",
                            error->message);
                return 1;
        }

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
        if (!gibbon_gmd_writer_write_action (writer, out, game, action,
                                             GIBBON_POSITION_SIDE_WHITE,
                                             G_MININT64, &error)) {
                g_printerr ("Cannot write roll 21: %s\n", error->message);
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
        if (!gibbon_gmd_writer_write_action (writer, out, game, action,
                                             GIBBON_POSITION_SIDE_WHITE,
                                             G_MININT64, &error)) {
                g_printerr ("Cannot write move: %s\n", error->message);
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
        if (!gibbon_gmd_writer_write_action (writer, out, game, action,
                                             GIBBON_POSITION_SIDE_WHITE,
                                             G_MININT64, &error)) {
                g_printerr ("Cannot write double: %s\n", error->message);
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
        if (!gibbon_game_over (game)) {
                g_printerr ("Game not over after drop after drop.\n");
                return 1;
        }
        if (!gibbon_gmd_writer_write_action (writer, out, game, action,
                                             GIBBON_POSITION_SIDE_WHITE,
                                             G_MININT64, &error)) {
                g_printerr ("Cannot write drop: %s\n", error->message);
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
        if (game == last_game) {
                g_printerr ("Game did not change after first roll"
                            " in 2nd game.\n");
                return 1;
        }
        if (!gibbon_gmd_writer_add_game (writer, out, &error)) {
                g_printerr ("Error writing 2nd game start: %s\n",
                            error->message);
                return 1;
        }
        if (!gibbon_gmd_writer_write_action (writer, out, game, action,
                                             GIBBON_POSITION_SIDE_WHITE,
                                             G_MININT64, &error)) {
                g_printerr ("Cannot first roll in 2nd game: %s\n",
                            error->message);
                return 1;
        }
        last_game = game;

        g_object_unref (match);

        gmd_out = g_memory_output_stream_get_data (G_MEMORY_OUTPUT_STREAM (out));
        wanted = g_strdup_printf ("GMD-2 # Created by Gibbon version %s\n"
                        "Length: 0\n"
                        "Player:W: white\n"
                        "Player:B: black\n"
                        "Game:\n"
                        "Roll:W:: 2 1\n"
                        "Move:W:: 13/11 24/21\n"
                        "Double:W:\n"
                        "Drop:W:\n"
                        "Game:\n"
                        "Roll:W:: 3 1\n", VERSION);
        if (g_strcmp0 (gmd_out, wanted)) {
                g_printerr ("Recorded match differs, wanted: %s", wanted);
                g_printerr ("Got: %s", gmd_out);
                return 1;
        }

        g_object_unref (out);

        return 0;
}
