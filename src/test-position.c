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

#include <string.h>

#include <glib.h>

#include <gibbon-position.h>
#include <gibbon-move.h>

static gboolean test_constructor (void);
static gboolean test_copy_constructor (void);
static gboolean test_compare (void);
static gboolean test_apply_move (void);
static gboolean test_game_over (void);

int
main(int argc, char *argv[])
{
	int status = 0;

        g_type_init ();

        if (!test_constructor ())
                status = -1;
        if (!test_copy_constructor ())
                status = -1;
        if (!test_compare ())
                status = -1;
        if (!test_apply_move ())
                status = -1;
        if (!test_game_over ())
                status = -1;

        return status;
}

static gboolean
test_constructor ()
{
        GibbonPosition *pos = gibbon_position_new ();

        g_return_val_if_fail (pos != NULL, FALSE);

        g_return_val_if_fail (pos->players[0] == NULL, FALSE);
        g_return_val_if_fail (pos->players[1] == NULL, FALSE);

        g_return_val_if_fail (pos->points[0] == -2, FALSE);
        g_return_val_if_fail (pos->points[1] == 0, FALSE);
        g_return_val_if_fail (pos->points[2] == 0, FALSE);
        g_return_val_if_fail (pos->points[3] == 0, FALSE);
        g_return_val_if_fail (pos->points[4] == 0, FALSE);
        g_return_val_if_fail (pos->points[5] == 5, FALSE);
        g_return_val_if_fail (pos->points[6] == 0, FALSE);
        g_return_val_if_fail (pos->points[7] == 3, FALSE);
        g_return_val_if_fail (pos->points[8] == 0, FALSE);
        g_return_val_if_fail (pos->points[9] == 0, FALSE);
        g_return_val_if_fail (pos->points[10] == 0, FALSE);
        g_return_val_if_fail (pos->points[11] == -5, FALSE);
        g_return_val_if_fail (pos->points[12] == 5, FALSE);
        g_return_val_if_fail (pos->points[13] == 0, FALSE);
        g_return_val_if_fail (pos->points[14] == 0, FALSE);
        g_return_val_if_fail (pos->points[15] == 0, FALSE);
        g_return_val_if_fail (pos->points[16] == -3, FALSE);
        g_return_val_if_fail (pos->points[17] == 0, FALSE);
        g_return_val_if_fail (pos->points[18] == -5, FALSE);
        g_return_val_if_fail (pos->points[19] == 0, FALSE);
        g_return_val_if_fail (pos->points[20] == 0, FALSE);
        g_return_val_if_fail (pos->points[21] == 0, FALSE);
        g_return_val_if_fail (pos->points[22] == 0, FALSE);
        g_return_val_if_fail (pos->points[23] == 2, FALSE);

        g_return_val_if_fail (pos->bar[0] == 0, FALSE);
        g_return_val_if_fail (pos->bar[1] == 0, FALSE);

        g_return_val_if_fail (pos->dice[0] == 0, FALSE);
        g_return_val_if_fail (pos->dice[1] == 0, FALSE);

        g_return_val_if_fail (pos->cube == 1, FALSE);
        g_return_val_if_fail (pos->may_double[0] == TRUE, FALSE);
        g_return_val_if_fail (pos->may_double[1] == TRUE, FALSE);
        g_return_val_if_fail (pos->game_info == NULL, FALSE);

        gibbon_position_free (pos);

        return TRUE;
}

static gboolean
test_copy_constructor (void)
{
        GibbonPosition *orig = gibbon_position_new ();
        GibbonPosition *copy;
        gchar *player1;
        gchar *player2;

        orig->players[0] = g_strdup ("foo");
        orig->players[1] = g_strdup ("bar");

        copy = gibbon_position_copy (orig);
        g_return_val_if_fail (copy != NULL, FALSE);

        g_return_val_if_fail (orig->players[0] != copy->players[0], FALSE);
        g_return_val_if_fail (orig->players[1] != copy->players[1], FALSE);

        g_return_val_if_fail (!g_strcmp0 (orig->players[0],
                                          copy->players[0]), FALSE);
        g_return_val_if_fail (!g_strcmp0 (orig->players[1],
                                          copy->players[1]), FALSE);

        /* Compare the structure expect for the pointers.  */
        player1 = copy->players[0];
        player2 = copy->players[1];

        copy->players[0] = orig->players[0];
        copy->players[1] = orig->players[1];

        g_return_val_if_fail (!memcmp (orig, copy, sizeof *orig), FALSE);

        copy->players[0] = player1;
        copy->players[1] = player2;

        gibbon_position_free (orig);
        gibbon_position_free (copy);

        return TRUE;
}

static gboolean
test_compare (void)
{
        gboolean retval = TRUE;
        GibbonPosition *ref = gibbon_position_new ();
        GibbonPosition *def = gibbon_position_copy (ref);

        if (!gibbon_position_equals_technically (ref, def)) {
                g_printerr ("Equal positions differ technically.\n");
                retval = FALSE;
        }

        def->match_length = 7;
        if (gibbon_position_equals_technically (ref, def)) {
                g_printerr ("Positions with different match lengths do not"
                            " differ technically.\n");
                retval = FALSE;
        }
        def->match_length = 0;

        def->scores[0] = 1;
        if (gibbon_position_equals_technically (ref, def)) {
                g_printerr ("Positions with different scores for white do not"
                            " differ technically.\n");
                retval = FALSE;
        }
        def->scores[0] = 0;

        def->scores[1] = 1;
        if (gibbon_position_equals_technically (ref, def)) {
                g_printerr ("Positions with different scores for black do not"
                            " differ technically.\n");
                retval = FALSE;
        }
        def->scores[1] = 0;

        --def->points[12];
        if (gibbon_position_equals_technically (ref, def)) {
                g_printerr ("Positions with different points do not differ"
                            " technically.\n");
                retval = FALSE;
        }
        ++def->points[12];

        def->bar[0] = 1;
        if (gibbon_position_equals_technically (ref, def)) {
                g_printerr ("Positions with different bars for white do not"
                            " differ technically.\n");
                retval = FALSE;
        }
        def->bar[0] = 0;

        def->bar[1] = 1;
        if (gibbon_position_equals_technically (ref, def)) {
                g_printerr ("Positions with different bars for black do not"
                            " differ technically.\n");
                retval = FALSE;
        }
        def->bar[1] = 0;


        def->dice[0] = 2;
        if (gibbon_position_equals_technically (ref, def)) {
                g_printerr ("Positions with different first die do not"
                            " differ technically.\n");
                retval = FALSE;
        }
        def->dice[0] = 0;

        def->dice[1] = 3;
        if (gibbon_position_equals_technically (ref, def)) {
                g_printerr ("Positions with different second die do not"
                            " differ technically.\n");
                retval = FALSE;
        }
        def->dice[1] = 0;

        def->cube = 2;
        if (gibbon_position_equals_technically (ref, def)) {
                g_printerr ("Positions with different second die do not"
                            " differ technically.\n");
                retval = FALSE;
        }
        def->cube = ref->cube;

        def->may_double[0] = !def->may_double[0];
        if (gibbon_position_equals_technically (ref, def)) {
                g_printerr ("Positions with different white doubling do not"
                            " differ technically.\n");
                retval = FALSE;
        }
        def->may_double[0] = !def->may_double[0];

        def->may_double[1] = !def->may_double[1];
        if (gibbon_position_equals_technically (ref, def)) {
                g_printerr ("Positions with different black doubling do not"
                            " differ technically.\n");
                retval = FALSE;
        }
        def->may_double[1] = !def->may_double[1];

        def->game_info = g_strdup ("Crawford game");
        if (!gibbon_position_equals_technically (ref, def)) {
                g_printerr ("Positions with different game info do differ"
                            " technically.\n");
                retval = FALSE;
        }
        g_free (def->game_info);
        def->game_info = NULL;

        def->status = g_strdup ("It is your turn.");
        if (!gibbon_position_equals_technically (ref, def)) {
                g_printerr ("Positions with different status do differ"
                            " technically.\n");
                retval = FALSE;
        }
        g_free (def->status);
        def->status = NULL;

        def->resigned = 3;
        if (gibbon_position_equals_technically (ref, def)) {
                g_printerr ("Positions with and without resignation do"
                            " not differ!\n");
                retval = FALSE;
        }
        def->resigned = 0;

        def->turn = GIBBON_POSITION_SIDE_BLACK;
        if (gibbon_position_equals_technically (ref, def)) {
                g_printerr ("Positions do not differ technically after turn"
                            " changed although they should!\n");
                retval = FALSE;
        }
        def->turn = GIBBON_POSITION_SIDE_NONE;

        if (!gibbon_position_equals_technically (ref, def)) {
                g_printerr ("Test case for compare function must be fixed.\n");
                retval = FALSE;
        }

        gibbon_position_free (ref);
        gibbon_position_free (def);

        return retval;
}

static gboolean
test_apply_move (void)
{
        gboolean retval = TRUE;
        GibbonPosition *expect = gibbon_position_new ();
        GibbonPosition *got = gibbon_position_new ();
        GibbonMove *move = gibbon_move_new (3, 1, 4);

        move->number = 2;
        move->movements[0].from = 8;
        move->movements[0].to = 5;
        move->movements[0].die = 3;
        move->movements[1].from = 6;
        move->movements[1].to = 5;
        move->movements[1].die = 1;
        expect->points[7] = 2;
        expect->points[5] = 4;
        expect->points[4] = 2;
        expect->turn = GIBBON_POSITION_SIDE_NONE;
        if (!gibbon_position_apply_move (got, move,
                                         GIBBON_POSITION_SIDE_WHITE, FALSE)) {
                g_printerr ("Cannot apply white's 1) 31: 8/5 6/5.\n");
                retval = FALSE;
        }
        if (gibbon_position_equals_technically (got, expect)) {
                g_printerr ("Positions do not differ after white's 1) 31: 8/5 6/5.\n");
                retval = FALSE;
        }

        move->number = 2;
        move->movements[0].from = 24;
        move->movements[0].to = 18;
        move->movements[0].die = 6;
        move->movements[1].from = 18;
        move->movements[1].to = 14;
        move->movements[1].die = 4;
        expect->points[0] = -1;
        expect->points[10] = -1;
        if (!gibbon_position_apply_move (got, move,
                                         GIBBON_POSITION_SIDE_BLACK, TRUE)) {
                g_printerr ("Cannot apply black's 1) 64: 24/14.\n");
                retval = FALSE;
        }
        if (gibbon_position_equals_technically (got, expect)) {
                g_printerr ("Positions do not differ after black's 1) 64: 24/14.\n");
                retval = FALSE;
        }

        move->number = 2;
        move->movements[0].from = 13;
        move->movements[0].to = 11;
        move->movements[0].die = 2;
        move->movements[1].from = 6;
        move->movements[1].to = 1;
        move->movements[1].die = 5;
        expect->points[12] = +4;
        expect->points[10] = +1;
        expect->points[5] = +3;
        expect->points[0] = +1;
        expect->bar[1] = 2;
        if (!gibbon_position_apply_move (got, move,
                                         GIBBON_POSITION_SIDE_WHITE, FALSE)) {
                g_printerr ("Cannot apply white's 2) 25: 13/11* 6/1*.\n");
                retval = FALSE;
        }
        if (gibbon_position_equals_technically (got, expect)) {
                g_printerr ("Positions do not differ after white's 2)"
                            " 25: 13/11* 6/1*.\n");
                retval = FALSE;
        }

        /* From here on, we will not reverse black's moves.  */
        move->number = 1;
        move->movements[0].from = 0;
        move->movements[0].to = 1;
        move->movements[0].die = 1;
        expect->points[0] = -1;
        expect->bar[0] = 1;
        expect->bar[1] = 1;
        if (!gibbon_position_apply_move (got, move,
                                         GIBBON_POSITION_SIDE_BLACK, FALSE)) {
                g_printerr ("Cannot apply black's 2) 61: bar/24*.\n");
                retval = FALSE;
        }
        if (gibbon_position_equals_technically (got, expect)) {
                g_printerr ("Positions do not differ after black's 2) 61: bar/24.\n");
                retval = FALSE;
        }

        move->number = 4;
        move->movements[0].from = 25;
        move->movements[0].to = 20;
        move->movements[0].die = 5;
        move->movements[1].from = 20;
        move->movements[1].to = 15;
        move->movements[1].die = 5;
        move->movements[2].from = 15;
        move->movements[2].to = 10;
        move->movements[2].die = 5;
        move->movements[3].from = 10;
        move->movements[3].to = 5;
        move->movements[3].die = 5;
        expect->points[4] = +3;
        expect->bar[0] = 0;
        if (!gibbon_position_apply_move (got, move,
                                         GIBBON_POSITION_SIDE_WHITE, FALSE)) {
                g_printerr ("Cannot apply white's 3: 55 bar/5.\n");
                retval = FALSE;
        }
        if (gibbon_position_equals_technically (got, expect)) {
                g_printerr ("Positions do not differ after white's 3)"
                            " 55: bar/5.\n");
                retval = FALSE;
        }

        move->number = 4;
        move->movements[0].from = 0;
        move->movements[0].to = 4;
        move->movements[0].die = 4;
        move->movements[1].from = 12;
        move->movements[1].to = 16;
        move->movements[1].die = 4;
        move->movements[2].from = 12;
        move->movements[2].to = 16;
        move->movements[2].die = 4;
        move->movements[3].from = 12;
        move->movements[3].to = 16;
        move->movements[3].die = 4;
        expect->points[3] = -1;
        expect->points[11] = -2;
        expect->points[15] = -3;
        expect->bar[1] = 0;
        if (!gibbon_position_apply_move (got, move,
                                         GIBBON_POSITION_SIDE_BLACK, FALSE)) {
                g_printerr ("Cannot apply black's 3) 44: bar/21 12/9(3).\n");
                retval = FALSE;
        }
        if (gibbon_position_equals_technically (got, expect)) {
                g_printerr ("Positions do not differ after black's"
                            " 3) 44: bar/21 12/9(3).\n");
                retval = FALSE;
        }

        /* Some moves later ... */
        gibbon_position_free (got);
        gibbon_position_free (expect);

        got = gibbon_position_new ();
        memset (got->points, 0, sizeof got->points);
        got->points[0] = +1;
        got->points[2] = +1;
        got->points[23] = -2;
        expect = gibbon_position_copy (got);

        move->number = 2;
        move->movements[0].from = 3;
        move->movements[0].to = 0;
        move->movements[0].die = 6;
        move->movements[1].from = 1;
        move->movements[1].to = 0;
        move->movements[1].die = 4;

        expect->points[0] = 0;
        expect->points[2] = 0;

        if (!gibbon_position_apply_move (got, move,
                                         GIBBON_POSITION_SIDE_WHITE, FALSE)) {
                g_printerr ("Cannot apply white's n: 64 3/off 1/off.\n");
                retval = FALSE;
        }
        if (gibbon_position_equals_technically (got, expect)) {
                g_printerr ("Positions do not differ after white's n)"
                            " 64: 3/off 1/off.\n");
                retval = FALSE;
        }

        g_object_unref (move);
        gibbon_position_free (got);
        gibbon_position_free (expect);

        return retval;
}

static gboolean
test_game_over ()
{
        gboolean retval = TRUE;
        GibbonPosition *position = gibbon_position_new ();
        gint score, expect;

        score = gibbon_position_game_over (position);

        if (score) {
                g_printerr ("False positive for game over for initial"
                            " position.\n");
                retval = FALSE;
        }
        memset (position->points, 0, sizeof position->points);

        position->points[23] = -1;
        score = gibbon_position_game_over (position);
        expect = 1;
        if (score != expect) {
                g_printerr ("Expected score %d, got %d.\n", expect, score);
                retval = FALSE;
        }
        position->points[23] = 0;

        position->points[0] = +1;
        score = gibbon_position_game_over (position);
        expect = -1;
        if (score != expect) {
                g_printerr ("Expected score %d, got %d.\n", expect, score);
                retval = FALSE;
        }
        position->points[0] = 0;

        position->points[23] = -15;
        score = gibbon_position_game_over (position);
        expect = 2;
        if (score != expect) {
                g_printerr ("Expected score %d, got %d.\n", expect, score);
                retval = FALSE;
        }
        position->points[23] = 0;

        position->points[0] = 15;
        score = gibbon_position_game_over (position);
        expect = -2;
        if (score != expect) {
                g_printerr ("Expected score %d, got %d.\n", expect, score);
                retval = FALSE;
        }
        position->points[0] = 0;

        position->points[23] = -14;
        position->points[0] = -1;
        score = gibbon_position_game_over (position);
        expect = 3;
        if (score != expect) {
                g_printerr ("Expected score %d, got %d.\n", expect, score);
                retval = FALSE;
        }
        position->points[23] = 0;
        position->points[0] = 0;

        position->points[0] = 1;
        position->points[23] = 14;
        score = gibbon_position_game_over (position);
        expect = -3;
        if (score != expect) {
                g_printerr ("Expected score %d, got %d.\n", expect, score);
                retval = FALSE;
        }
        position->points[0] = 0;
        position->points[23] = 0;

        gibbon_position_free (position);

        return retval;
}
