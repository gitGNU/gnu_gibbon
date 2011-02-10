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

#include <stdio.h>
#include <string.h>

#include <glib/gi18n.h>

#include <libgsgf/gsgf.h>

#include <math.h>

struct test_case {
        gdouble input;
        const gchar *expect;
};

struct test_case test_cases[] = {
                { 123.000, "123" },
                { -123, "-123" },
                { 3.14, "3.14" },
                { -45.6, "-45.6" },
};

static gboolean test_it (struct test_case *test_case);
static gboolean test_nan (void);
static gboolean test_positive_infinity (void);
static gboolean test_negative_infinity (void);

int
main (int argc, char *argv[])
{
        gsize num_cases = (sizeof test_cases) / (sizeof *test_cases);
        gsize i;
        int status = 0;

        g_type_init ();

        for (i = 0; i < num_cases; ++i) {
                if (!test_it (test_cases + i))
                        status = -1;
        }

        if (!test_nan ())
                status = -1;

        if (!test_positive_infinity ())
                status = -1;
        if (!test_negative_infinity ())
                status = -1;

        return status;
}

static gboolean
test_it (struct test_case *test_case)
{
        GSGFReal *real = gsgf_real_new (test_case->input);
        const gchar *got = gsgf_real_to_string (real);
        gboolean result;

        if (g_strcmp0 (test_case->expect, got)) {
                result = FALSE;
                g_printerr ("Expected '%s', got '%s' for %g.\n",
                            test_case->expect, got, test_case->input);
        } else {
                result = TRUE;
        }
        if (got)
                g_free (got);

        return result;
}

static gboolean
test_nan (void)
{
        GSGFReal *real = gsgf_real_new (NAN);
        const gchar *got = gsgf_real_to_string (real);
        gboolean result;

        if (got) {
                result = FALSE;
                g_printerr ("Expected NULL, got '%s' for NaN.\n", got);
        } else {
                result = TRUE;
        }
        if (got)
                g_free (got);

        return result;
}

static gboolean
test_positive_infinity (void)
{
        GSGFReal *max = gsgf_real_new (G_MAXDOUBLE);
        GSGFReal *infinity = gsgf_real_new (+INFINITY);
        gchar *max_str = gsgf_real_to_string (max);
        gchar *infinity_str = gsgf_real_to_string (infinity);
        gboolean result;

        if (g_strcmp0 (max_str, infinity_str)) {
                result = FALSE;
                g_printerr ("Expected '%s', got '%s' for +INFINITY.\n",
                            max_str, infinity_str);
        } else {
                result = TRUE;
        }
        if (max_str)
                g_free (max_str);
        if (infinity_str)
                g_free (infinity_str);

        return result;
}

static gboolean
test_negative_infinity (void)
{
        GSGFReal *min = gsgf_real_new (-G_MAXDOUBLE);
        GSGFReal *infinity = gsgf_real_new (-INFINITY);
        gchar *min_str = gsgf_real_to_string (min);
        gchar *infinity_str = gsgf_real_to_string (infinity);
        gboolean result;

        if (g_strcmp0 (min_str, infinity_str)) {
                result = FALSE;
                g_printerr ("Expected '%s', got '%s' for -INFINITY.\n",
                            min_str, infinity_str);
        } else {
                result = TRUE;
        }
        if (min_str)
                g_free (min_str);
        if (infinity_str)
                g_free (infinity_str);

        return result;
}
