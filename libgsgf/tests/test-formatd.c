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
#include <locale.h>

#include <glib.h>

#include "../gsgf-util.h"

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

static struct test_case trim = {
                "1234.56", 1234.56, -1, -1, FALSE, TRUE
};

static struct test_case truncate_decimal_point = {
                "1234", 1234.0, -1, -1, FALSE, TRUE
};

static struct test_case zeropad = {
                "00001234.560000", 1234.56, 15, -1, TRUE, FALSE
};

static struct test_case *test_cases[] = {
                &simple,
                &trim,
                &truncate_decimal_point,
                &zeropad
};

static gboolean test_single_case (struct test_case *test_case);

int
main(int argc, char *argv[])
{
        int status = 0;
        gsize i;

        /*
         * Try to select a locale with a decimal comma.
         */
#ifdef G_OS_WIN32
        if (!setlocale(LC_ALL, "Spanish_Spain"))
        if (!setlocale(LC_ALL, "French_France"))
        if (!setlocale(LC_ALL, "German_Germany"))
        {}
#else
        if (!setlocale(LC_ALL, "es_ES"))
        if (!setlocale(LC_ALL, "fr_FR"))
        if (!setlocale(LC_ALL, "de_DE"))
        if (!setlocale(LC_ALL, "it_IT"))
        if (!setlocale(LC_ALL, "sv_SE"))
        {}
#endif

        for (i = 0; i < sizeof test_cases / sizeof test_cases[0]; ++i) {
                if (!test_single_case (test_cases[i]))
                        status = -1;
        }

        return status;
}

static gboolean
test_single_case (struct test_case *tc)
{
        gchar *got = gsgf_ascii_dtostring (tc->d, tc->width,
                                           tc->precision, tc->zeropad,
                                           tc->zerotrim);
        gboolean result = TRUE;
        gchar *saved_locale;

        if (g_strcmp0 (tc->expect, got)) {
                saved_locale = setlocale (LC_ALL, NULL);
                setlocale (LC_ALL, "C");
                g_printerr ("%f, %d, %d, %s, %s: expected '%s', got '%s'!\n",
                            tc->d, tc->precision, tc->width,
                            tc->zeropad ? "TRUE" : "FALSE",
                            tc->zerotrim ? "TRUE" : "FALSE",
                            tc->expect, got);
                result = FALSE;
                if (saved_locale)
                        setlocale (LC_ALL, saved_locale);
        }
        g_free (got);

        return result;
}
