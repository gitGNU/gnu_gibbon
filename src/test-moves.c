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

static gboolean test_white_opening_31 (void);

int
main(int argc, char *argv[])
{
	int status = 0;

        g_type_init ();

        if (!test_white_opening_31 ())
                status = -1;

        return status;
}

static gboolean
test_white_opening_31 ()
{
        GibbonPosition *pos = gibbon_position_new ();
        GSList *moves;

        pos->match_length = 1;
        pos->turn = GIBBON_POSITION_SIDE_WHITE;
        pos->dice[0] = 3;
        pos->dice[1] = 1;

        moves = gibbon_position_get_moves (pos);
        g_return_val_if_fail (moves != NULL, FALSE);

        gibbon_position_free_moves (moves);

        return TRUE;
}
