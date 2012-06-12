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

typedef gboolean (*test_function) (GibbonMatch *match);

static gboolean check_opening_roll (GibbonMatch *match);

static test_function tests[] = {
    check_opening_roll,
};

int
main(int argc, char *argv[])
{
	GibbonMatch *match;
	GibbonGame *game;
	gsize i;
	int status = 0;

        g_type_init ();

        for (i = 0; i < sizeof tests / sizeof tests[0]; ++i) {
                match = gibbon_match_new ("SnowWhite", "JoeBlack", 5, TRUE);
                g_return_val_if_fail (match != NULL, -1);
                game = gibbon_match_add_game (match, NULL);
                g_return_val_if_fail (game != NULL, -1);
                if (!tests[i] (match))
                        status = -1;
                g_object_unref (match);
        }

        return status;
}

static gboolean
check_opening_roll (GibbonMatch *match)
{
        g_printerr ("Test not yet implemented!\n");

        return FALSE;
}
