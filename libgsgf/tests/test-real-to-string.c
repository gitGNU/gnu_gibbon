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
#include <string.h>

#include <glib/gi18n.h>

#include <libgsgf/gsgf.h>

#include <math.h>
#include <locale.h>

struct test_case {
        gdouble input;
        const gchar *expect;
};

gdouble test_cases[] = {
                123.000,
                -123456789,
                3.1415927,
                -12345.6789,
                1e-24
};

static gboolean test_it (gdouble num);
static gboolean test_nan (void);
static gboolean test_positive_infinity (void);
static gboolean test_negative_infinity (void);

int
main (int argc, char *argv[])
{
        gsize num_cases = (sizeof test_cases) / (sizeof *test_cases);
        gsize i;
        int status = 0;

        setlocale (LC_ALL, "");

        g_type_init ();

        for (i = 0; i < num_cases; ++i) {
                if (!test_it (test_cases[i]))
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
test_it (gdouble num)
{
        GSGFReal *real = gsgf_real_new (num);
        gchar *got = gsgf_real_to_string (real);
        gboolean result;
        gdouble expect;
        gdouble diff;

        g_object_unref (real);

        if (!got) {
                result = FALSE;
                g_printerr ("%g triggered NULL pointer!\n", num);
        }

        expect = g_ascii_strtod (got, NULL);
        diff = num - expect;
        if (diff < 0)
                diff = -diff;

        if (diff != 0 && diff > 0.000000000001) {
                result = FALSE;
                g_printerr ("Expected '%g', got '%s' (difference: %g).\n",
                            num, got, diff);
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
        gchar *got = gsgf_real_to_string (real);
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

        if (max_str && '-' == max_str[0]) {
                result = FALSE;
                g_printerr ("G_MAXDOUBLE starts with a minus: %s\n",
                            max_str);
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

        if (min_str && '-' != min_str[0]) {
                result = FALSE;
                g_printerr ("-G_MAXDOUBLE does not start with a minus: %s\n",
                            min_str);
        }

        if (min_str)
                g_free (min_str);
        if (infinity_str)
                g_free (infinity_str);

        return result;
}
