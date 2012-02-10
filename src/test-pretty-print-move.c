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

static gboolean test_cannot_move (void);
static gboolean test_basic_move (void);
static gboolean test_double2 (void);
static gboolean test_double3 (void);
static gboolean test_double4 (void);
static gboolean test_from_bar (void);
static gboolean test_bear_off (void);
static gboolean test_black_move (void);
static gboolean test_black_move_reverse (void);
static gboolean test_double_bug1 (void);

int
main(int argc, char *argv[])
{
	int status = 0;

        g_type_init ();

        if (!test_cannot_move ())
                status = -1;
        if (!test_basic_move ())
                status = -1;
        if (!test_double2 ())
                status = -1;
        if (!test_double3 ())
                status = -1;
        if (!test_double4 ())
                status = -1;
        if (!test_from_bar ())
                status = -1;
        if (!test_bear_off ())
                status = -1;
        if (!test_black_move ())
                status = -1;
        if (!test_black_move_reverse ())
                status = -1;
        if (!test_double_bug1 ())
                status = -1;

        return status;
}

static gboolean
test_cannot_move ()
{
        gboolean retval = TRUE;
        GibbonPosition *position = gibbon_position_new ();
        GibbonMove *move = gibbon_move_new (3, 3, 0);
        gchar *expect = "-";
        gchar *got = NULL;

        got = gibbon_position_format_move (position, move,
                                           GIBBON_POSITION_SIDE_WHITE, FALSE);
        if (g_strcmp0 (expect, got)) {
                retval = FALSE;
                g_printerr ("Expected '%s', got '%s'.\n",
                            expect, got);
        }
        g_free (got);

        gibbon_position_free (position);
        g_object_unref (move);

        return retval;
}

static gboolean
test_basic_move ()
{
        gboolean retval = TRUE;
        GibbonPosition *position = gibbon_position_new ();
        GibbonMove *move = gibbon_move_new (3, 1, 2);
        gchar *expect;
        gchar *got = NULL;

        move->number = 2;
        move->movements[0].from = 8;
        move->movements[0].to = 5;
        move->movements[1].from = 6;
        move->movements[1].to = 5;
        expect = "8/5 6/5";
        got = gibbon_position_format_move (position, move,
                                           GIBBON_POSITION_SIDE_WHITE, FALSE);
        if (g_strcmp0 (expect, got)) {
                retval = FALSE;
                g_printerr ("Expected '%s', got '%s'.\n",
                            expect, got);
        }
        g_free (got);

        /* Test compression.  */
        move->movements[0].from = 24;
        move->movements[0].to = 18;
        move->movements[1].from = 18;
        move->movements[1].to = 13;
        expect = "24/13";
        got = gibbon_position_format_move (position, move,
                                           GIBBON_POSITION_SIDE_WHITE, FALSE);
        if (g_strcmp0 (expect, got)) {
                retval = FALSE;
                g_printerr ("Expected '%s', got '%s'.\n",
                            expect, got);
        }
        g_free (got);

        /* Now make this a hit.  */
        position->points[18] = -4;
        position->points[17] = -1;
        expect = "24/18*/13";
        got = gibbon_position_format_move (position, move,
                                           GIBBON_POSITION_SIDE_WHITE, FALSE);
        if (g_strcmp0 (expect, got)) {
                retval = FALSE;
                g_printerr ("Expected '%s', got '%s'.\n",
                            expect, got);
        }
        g_free (got);

        gibbon_position_free (position);
        g_object_unref (move);

        return retval;
}

static gboolean
test_double2 ()
{
        gboolean retval = TRUE;
        GibbonPosition *position = gibbon_position_new ();
        GibbonMove *move = gibbon_move_new (6, 6, 4);
        gchar *expect;
        gchar *got = NULL;

        move->number = 4;
        move->movements[0].from = 24;
        move->movements[0].to = 18;
        move->movements[1].from = 24;
        move->movements[1].to = 18;
        move->movements[2].from = 13;
        move->movements[2].to = 7;
        move->movements[3].from = 13;
        move->movements[3].to = 7;
        expect = "24/18(2) 13/7(2)";
        got = gibbon_position_format_move (position, move,
                                           GIBBON_POSITION_SIDE_WHITE, FALSE);
        if (g_strcmp0 (expect, got)) {
                retval = FALSE;
                g_printerr ("Expected '%s', got '%s'.\n",
                            expect, got);
        }
        g_free (got);

        /* Now make one a hit.  */
        position->points[18] = -4;
        position->points[6] = -1;
        expect = "24/18(2) 13/7*(2)";
        got = gibbon_position_format_move (position, move,
                                           GIBBON_POSITION_SIDE_WHITE, FALSE);
        if (g_strcmp0 (expect, got)) {
                retval = FALSE;
                g_printerr ("Expected '%s', got '%s'.\n",
                            expect, got);
        }
        g_free (got);

        gibbon_position_free (position);

        g_object_unref (move);

        return retval;
}

static gboolean
test_double3 ()
{
        gboolean retval = TRUE;
        GibbonPosition *position = gibbon_position_new ();
        GibbonMove *move = gibbon_move_new (6, 6, 4);
        gchar *expect;
        gchar *got = NULL;

        move->number = 4;
        move->movements[0].from = 24;
        move->movements[0].to = 18;
        move->movements[1].from = 13;
        move->movements[1].to = 7;
        move->movements[2].from = 13;
        move->movements[2].to = 7;
        move->movements[3].from = 13;
        move->movements[3].to = 7;
        expect = "24/18 13/7(3)";
        got = gibbon_position_format_move (position, move,
                                           GIBBON_POSITION_SIDE_WHITE, FALSE);
        if (g_strcmp0 (expect, got)) {
                retval = FALSE;
                g_printerr ("Expected '%s', got '%s'.\n",
                            expect, got);
        }
        g_free (got);

        /* Now make this a hit.  */
        position->points[18] = -4;
        position->points[6] = -1;
        expect = "24/18 13/7*(3)";
        got = gibbon_position_format_move (position, move,
                                           GIBBON_POSITION_SIDE_WHITE, FALSE);
        if (g_strcmp0 (expect, got)) {
                retval = FALSE;
                g_printerr ("Expected '%s', got '%s'.\n",
                            expect, got);
        }
        g_free (got);

        gibbon_position_free (position);

        g_object_unref (move);

        return retval;
}

static gboolean
test_double4 ()
{
        gboolean retval = TRUE;
        GibbonPosition *position = gibbon_position_new ();
        GibbonMove *move = gibbon_move_new (6, 6, 4);
        gchar *expect;
        gchar *got = NULL;

        move->number = 4;
        move->movements[0].from = 13;
        move->movements[0].to = 7;
        move->movements[1].from = 13;
        move->movements[1].to = 7;
        move->movements[2].from = 13;
        move->movements[2].to = 7;
        move->movements[3].from = 13;
        move->movements[3].to = 7;
        expect = "13/7(4)";
        got = gibbon_position_format_move (position, move,
                                           GIBBON_POSITION_SIDE_WHITE, FALSE);
        if (g_strcmp0 (expect, got)) {
                retval = FALSE;
                g_printerr ("Expected '%s', got '%s'.\n",
                            expect, got);
        }
        g_free (got);

        /* Now make this a hit.  */
        position->points[18] = -4;
        position->points[6] = -1;
        expect = "13/7*(4)";
        got = gibbon_position_format_move (position, move,
                                           GIBBON_POSITION_SIDE_WHITE, FALSE);
        if (g_strcmp0 (expect, got)) {
                retval = FALSE;
                g_printerr ("Expected '%s', got '%s'.\n",
                            expect, got);
        }
        g_free (got);

        gibbon_position_free (position);

        g_object_unref (move);

        return retval;
}

static gboolean
test_from_bar ()
{
        gboolean retval = TRUE;
        GibbonPosition *position = gibbon_position_new ();
        GibbonMove *move = gibbon_move_new (6, 2, 1);
        gchar *expect;
        gchar *got = NULL;

        move->number = 1;
        move->movements[0].from = 25;
        move->movements[0].to = 23;
        expect = "bar/23";
        got = gibbon_position_format_move (position, move,
                                           GIBBON_POSITION_SIDE_WHITE, FALSE);
        if (g_strcmp0 (expect, got)) {
                retval = FALSE;
                g_printerr ("Expected '%s', got '%s'.\n",
                            expect, got);
        }
        g_free (got);

        gibbon_position_free (position);

        g_object_unref (move);

        return retval;
}

static gboolean
test_bear_off ()
{
        gboolean retval = TRUE;
        GibbonPosition *position = gibbon_position_new ();
        GibbonMove *move = gibbon_move_new (6, 3, 1);
        gchar *expect;
        gchar *got = NULL;

        move->number = 1;
        move->movements[0].from = 3;
        move->movements[0].to = 0;
        expect = "3/off";
        got = gibbon_position_format_move (position, move,
                                           GIBBON_POSITION_SIDE_WHITE, FALSE);
        if (g_strcmp0 (expect, got)) {
                retval = FALSE;
                g_printerr ("Expected '%s', got '%s'.\n",
                            expect, got);
        }
        g_free (got);

        gibbon_position_free (position);

        g_object_unref (move);

        return retval;
}

static gboolean
test_black_move ()
{
        gboolean retval = TRUE;
        GibbonPosition *position = gibbon_position_new ();
        GibbonMove *move = gibbon_move_new (6, 3, 2);
        gchar *expect;
        gchar *got = NULL;

        move->number = 2;
        move->movements[0].from = 0;
        move->movements[0].to = 6;
        move->movements[1].from = 22;
        move->movements[1].to = 25;
        expect = "bar/19 3/off";
        got = gibbon_position_format_move (position, move,
                                           GIBBON_POSITION_SIDE_BLACK, FALSE);
        if (g_strcmp0 (expect, got)) {
                retval = FALSE;
                g_printerr ("Expected '%s', got '%s'.\n",
                            expect, got);
        }
        g_free (got);

        /* Make it a hit.  */
        position->points[5] = +1;
        expect = "bar/19* 3/off";
        got = gibbon_position_format_move (position, move,
                                           GIBBON_POSITION_SIDE_BLACK, FALSE);
        if (g_strcmp0 (expect, got)) {
                retval = FALSE;
                g_printerr ("Expected '%s', got '%s'.\n",
                            expect, got);
        }
        g_free (got);

        gibbon_position_free (position);

        g_object_unref (move);

        return retval;
}


static gboolean
test_black_move_reverse ()
{
        gboolean retval = TRUE;
        GibbonPosition *position = gibbon_position_new ();
        GibbonMove *move = gibbon_move_new (5, 3, 2);
        gchar *expect;
        gchar *got = NULL;

        move->number = 2;
        move->movements[0].from = 25;
        move->movements[0].to = 20;
        move->movements[1].from = 3;
        move->movements[1].to = 0;
        expect = "bar/20 3/off";
        got = gibbon_position_format_move (position, move,
                                           GIBBON_POSITION_SIDE_BLACK, TRUE);
        if (g_strcmp0 (expect, got)) {
                retval = FALSE;
                g_printerr ("Expected '%s', got '%s'.\n",
                            expect, got);
        }
        g_free (got);

        /* Make it a hit.  */
        position->points[4] = +1;
        expect = "bar/20* 3/off";
        got = gibbon_position_format_move (position, move,
                                           GIBBON_POSITION_SIDE_BLACK, TRUE);
        if (g_strcmp0 (expect, got)) {
                retval = FALSE;
                g_printerr ("Expected '%s', got '%s'.\n",
                            expect, got);
        }
        g_free (got);

        gibbon_position_free (position);

        g_object_unref (move);

        return retval;
}

static gboolean
test_double_bug1 ()
{
        gboolean retval = TRUE;
        GibbonPosition *position = gibbon_position_new ();
        GibbonMove *move = gibbon_move_new (5, 5, 4);
        gchar *expect;
        gchar *got = NULL;

        move->number = 4;
        move->movements[0].from = 18;
        move->movements[0].to = 13;
        move->movements[1].from = 18;
        move->movements[1].to = 13;
        move->movements[2].from = 13;
        move->movements[2].to = 8;
        move->movements[3].from = 13;
        move->movements[3].to = 8;
        expect = "18/8(2)";
        got = gibbon_position_format_move (position, move,
                                           GIBBON_POSITION_SIDE_BLACK, TRUE);
        if (g_strcmp0 (expect, got)) {
                retval = FALSE;
                g_printerr ("Expected '%s', got '%s'.\n",
                            expect, got);
        }
        g_free (got);

        position->points[11] = +1;
        expect = "18/13*/8(2)";
        got = gibbon_position_format_move (position, move,
                                           GIBBON_POSITION_SIDE_BLACK, TRUE);
        if (g_strcmp0 (expect, got)) {
                retval = FALSE;
                g_printerr ("Expected '%s', got '%s'.\n",
                            expect, got);
        }
        g_free (got);

        g_object_unref (move);

        return retval;
}
