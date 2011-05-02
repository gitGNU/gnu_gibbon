/*
 * This file is part of Gibbon, a graphical frontend to the First Internet 
 * Backgammon Server FIBS.
 * Copyright (C) 2009-2011 Guido Flohr, http://guido-flohr.net/.
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

static gboolean test_cannot_move (void);
static gboolean test_basic_move (void);

int
main(int argc, char *argv[])
{
	int status = 0;

        g_type_init ();

        if (!test_cannot_move ())
                status = -1;
        if (!test_basic_move ())
                status = -1;

        return status;
}

static gboolean
test_cannot_move ()
{
        gboolean retval = TRUE;
        GibbonPosition *position = gibbon_position_new ();
        GibbonMove *move = gibbon_position_alloc_move (0);
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
        g_free (move);

        return retval;
}

static gboolean
test_basic_move ()
{
        gboolean retval = TRUE;
        GibbonPosition *position = gibbon_position_new ();
        GibbonMove *move = gibbon_position_alloc_move (2);
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

        gibbon_position_free (position);
        g_free (move);

        return retval;
}
