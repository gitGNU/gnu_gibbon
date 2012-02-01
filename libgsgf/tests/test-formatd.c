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

#include <stdio.h>

#include <glib.h>

struct test_case {
        const gchar *expect;
        gdouble d;
        gint width;
        gint precision;
        gboolean zeropad;
        gboolean zerotrim;
};

static struct test_case simple = {
                "1234.000000", 1234.0, -1, -1, FALSE, FALSE
};

static struct test_case *test_cases[] = {
                &simple,
};

static gboolean test_single_case (struct test_case *test_case);

int
main(int argc, char *argv[])
{
        int status = 0;
        gsize i;

        for (i = 0; i < sizeof test_cases / sizeof test_cases[0]; ++i) {
                if (!test_single_case (test_cases[i]))
                        status = -1;
        }

        return status;
}

static gboolean
test_single_case (struct test_case *test_case)
{
        return FALSE;
}
