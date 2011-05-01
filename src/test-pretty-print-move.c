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

static gboolean test_very_basic (void);

int
main(int argc, char *argv[])
{
	int status = 0;

        g_type_init ();

        if (!test_very_basic ())
                status = -1;

        return status;
}

static gboolean
test_very_basic ()
{
        gboolean retval = TRUE;
        GibbonPosition *position = gibbon_position_new ();
        GibbonMove *move = gibbon_position_alloc_move (4);
        gchar *expect = "8/5 6/5";
        gchar *got = NULL;

        got = gibbon_position_format_move (position, move,
                                           GIBBON_POSITION_SIDE_WHITE, FALSE);
        if (g_strcmp0 (expect, got)) {
                retval = FALSE;
                g_printerr ("Expected '%s', got '%s'.\n",
                            expect, got);
        }

        gibbon_position_free (position);
        g_free (got);
        g_free (move);

        return retval;
}
