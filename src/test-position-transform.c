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

#include <gibbon-position.h>

static gboolean test_initial (void);

int
main(int argc, char *argv[])
{
	int status = 0;

        g_type_init ();

        g_value_register_transform_func (
                GIBBON_TYPE_POSITION, G_TYPE_STRING,
                gibbon_position_transform_to_string_value);

        if (!test_initial ())
                status = -1;

        return status;
}

static gboolean
test_initial ()
{
        GibbonPosition *position = gibbon_position_new ();
        GValue position_value = G_VALUE_INIT;
        GValue string_value = G_VALUE_INIT;
        gchar *expect = "\
=== Position ===\n\
Opponent: (null), 0/0 points, 167 pips\n\
  +-13-14-15-16-17-18-------19-20-21-22-23-24-+ negative: black or X\n\
  | +5          -3    | +0| -5             +2 | May double: yes\n\
 v| dice: +0 : +0     |BAR|                   |  Cube: 1\n\
  | -5          +3    | +0| +5             -2 | May double: yes\n\
  +-12-11-10--9--8--7--------6--5--4--3--2--1-+ positive: white or O\n\
Player: (null), 0/0 points, 167 pips\n\
Game info: (null)\n\
Status: (null)\n\
Turn: 0, cube turned: 0, resigned: 0, score: 0\n";
        gchar *got;

        g_return_val_if_fail (position != NULL, FALSE);

        g_value_init (&position_value, GIBBON_TYPE_POSITION);
        g_value_take_boxed (&position_value, position);

        g_value_init (&string_value, G_TYPE_STRING);
        g_return_val_if_fail (g_value_transform (&position_value, &string_value),
                              FALSE);

        got = g_value_get_string (&string_value);

        if (g_strcmp0 (expect, got)) {
            g_printerr ("Expected:\n%s", expect);
            g_printerr ("Got:\n%s", got);
            return FALSE;
        }

        g_value_unset (&position_value);
        g_value_unset (&string_value);

        return TRUE;
}
