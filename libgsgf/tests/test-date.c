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

#include "test.h"

static gboolean test_simple (void);
static gboolean test_shortcut_MM_DD (void);
static gboolean test_shortcut_DD (void);
static gboolean test_partial (void);
static gboolean test_parse_YYYY (void);
static gboolean test_parse_YYYY_MM (void);
static gboolean test_parse_YYYY_MM_DD (void);
static gboolean test_parse_multiple (void);

int
main(int argc, char *argv[])
{
        int status = 0;

        g_type_init ();

        path = "[in memory]";

        if (!test_simple ())
                status = -1;
        if (!test_partial ())
                status = -1;
        if (!test_shortcut_MM_DD ())
                status = -1;
        if (!test_shortcut_DD ())
                status = -1;
        if (!test_parse_YYYY ())
                status = -1;
        if (!test_parse_YYYY_MM ())
                status = -1;
        if (!test_parse_YYYY_MM_DD ())
                status = -1;
        if (!test_parse_multiple ())
                status = -1;

        return status;
}

static gboolean
test_simple (void)
{
        GSGFDate *gsgf_date;
        gchar *got;
        gchar *expect;
        gboolean retval = TRUE;

        gsgf_date = gsgf_date_new (g_date_new_dmy (26, 4, 2007), NULL);
        expect = "2007-04-26";
        got = gsgf_text_get_value (GSGF_TEXT (gsgf_date));
        if (g_strcmp0 (expect, got)) {
                g_printerr ("Expected %s, got %s\n", expect, got);
                retval = FALSE;
        }

        gsgf_date_append (gsgf_date, g_date_new_dmy (26, 5, 2008), NULL);

        expect = "2007-04-26,2008-05-26";
        got = gsgf_text_get_value (GSGF_TEXT (gsgf_date));
        if (g_strcmp0 (expect, got)) {
                g_printerr ("Expected %s, got %s\n", expect, got);
                retval = FALSE;
        }
        g_object_unref(gsgf_date);

        return retval;
}

static gboolean
test_shortcut_MM_DD (void)
{
        GSGFDate *gsgf_date;
        gchar *got;
        gchar *expect;
        gboolean retval = TRUE;

        gsgf_date = gsgf_date_new (g_date_new_dmy (19, 2, 2011), NULL);
        gsgf_date_append (gsgf_date, g_date_new_dmy (20, 3, 2011), NULL);
        expect = "2011-02-19,03-20";
        got = gsgf_text_get_value (GSGF_TEXT (gsgf_date));
        if (g_strcmp0 (expect, got)) {
                g_printerr ("Expected %s, got %s\n", expect, got);
                retval = FALSE;
        }

        gsgf_date_append (gsgf_date, g_date_new_dmy (21, 4, 2011), NULL);
        expect = "2011-02-19,03-20,04-21";
        got = gsgf_text_get_value (GSGF_TEXT (gsgf_date));
        if (g_strcmp0 (expect, got)) {
                g_printerr ("Expected %s, got %s\n", expect, got);
                retval = FALSE;
        }

        gsgf_date_append (gsgf_date, g_date_new_dmy (22, 4, 2011), NULL);
        gsgf_date_append (gsgf_date, g_date_new_dmy (23, 5, 2011), NULL);
        expect = "2011-02-19,03-20,04-21,22,05-23";
        got = gsgf_text_get_value (GSGF_TEXT (gsgf_date));
        if (g_strcmp0 (expect, got)) {
                g_printerr ("Expected %s, got %s\n", expect, got);
                retval = FALSE;
        }

        g_object_unref(gsgf_date);

        return retval;
}

static gboolean
test_shortcut_DD (void)
{
        GSGFDate *gsgf_date;
        gchar *got;
        gchar *expect;
        gboolean retval = TRUE;

        gsgf_date = gsgf_date_new (g_date_new_dmy (19, 2, 2011), NULL);
        gsgf_date_append (gsgf_date, g_date_new_dmy (20, 2, 2011), NULL);
        expect = "2011-02-19,20";
        got = gsgf_text_get_value (GSGF_TEXT (gsgf_date));
        if (g_strcmp0 (expect, got)) {
                g_printerr ("Expected %s, got %s\n", expect, got);
                retval = FALSE;
        }

        gsgf_date_append (gsgf_date, g_date_new_dmy (21, 2, 2011), NULL);
        expect = "2011-02-19,20,21";
        got = gsgf_text_get_value (GSGF_TEXT (gsgf_date));
        if (g_strcmp0 (expect, got)) {
                g_printerr ("Expected %s, got %s\n", expect, got);
                retval = FALSE;
        }

        gsgf_date_append (gsgf_date, g_date_new_dmy (22, 3, 2011), NULL);
        gsgf_date_append (gsgf_date, g_date_new_dmy (23, 3, 2011), NULL);
        expect = "2011-02-19,20,21,03-22,23";
        got = gsgf_text_get_value (GSGF_TEXT (gsgf_date));
        if (g_strcmp0 (expect, got)) {
                g_printerr ("Expected %s, got %s\n", expect, got);
                retval = FALSE;
        }

        g_object_unref(gsgf_date);

        return retval;
}

static gboolean
test_partial (void)
{
        GSGFDate *gsgf_date;
        GDate *date;
        gchar *got;
        gchar *expect;
        gboolean retval = TRUE;
        GError *error, *expect_error;

        error = NULL;
        date = g_date_new ();
        g_date_clear (date, 1);
        g_date_set_year (date, 2011);
        g_date_set_month (date, 3);
        gsgf_date = gsgf_date_new (date, &error);
        if (!gsgf_date) {
                g_printerr ("Cannot create GSGFDate with YYYY-MM: %s\n",
                            error->message);
                retval = FALSE;
        } else {
                expect = "2011-03";
                got = gsgf_text_get_value (GSGF_TEXT (gsgf_date));
                if (g_strcmp0 (expect, got)) {
                        g_printerr ("Expected %s, got %s\n", expect, got);
                        retval = FALSE;
                }
        }

        error = NULL;
        date = g_date_new ();
        g_date_clear (date, 1);
        g_date_set_year (date, 2011);
        gsgf_date = gsgf_date_new (date, &error);
        if (!gsgf_date) {
                g_printerr ("Cannot create GSGFDate with YYYY: %s\n",
                            error->message);
                retval = FALSE;
        } else {
                expect = "2011";
                got = gsgf_text_get_value (GSGF_TEXT (gsgf_date));
                if (g_strcmp0 (expect, got)) {
                        g_printerr ("Expected %s, got %s\n", expect, got);
                        retval = FALSE;
                }
        }

        error = NULL;
        expect_error = NULL;
        g_set_error (&expect_error, GSGF_ERROR, GSGF_ERROR_SEMANTIC_ERROR,
                     "Invalid year in date specification");
        date = g_date_new ();
        g_date_clear (date, 1);
        gsgf_date = gsgf_date_new (date, &error);
        if (0 != expect_error_conditional (!gsgf_date,
                                           "GSGFDate without year is creatable",
                                           error, expect_error))
                retval = FALSE;

        return retval;
}

static gboolean
test_parse_YYYY (void)
{
        GSGFDate *gsgf_date;
        GDate *date;
        gchar *got;
        gchar *expect;
        gboolean retval = TRUE;
        GError *error;
        GError *expected_error;

        date = g_date_new ();
        g_date_clear (date, 1);
        g_date_set_year (date, 1976);

        gsgf_date = gsgf_date_new (date, NULL);

        error = NULL;
        expect = "2011";
        gsgf_text_set_value (GSGF_TEXT (gsgf_date), expect, TRUE, &error);
        got = gsgf_text_get_value (GSGF_TEXT (gsgf_date));
        if (expect_error (error, NULL)) {
                g_printerr ("  (failed string was: %s)\n", expect);
                retval = FALSE;
        } else if (g_strcmp0 (expect, got)) {
                g_printerr ("Expected %s, got %s\n", expect, got);
                retval = FALSE;
        }

        error = NULL;
        expect = "0";
        gsgf_text_set_value (GSGF_TEXT (gsgf_date), expect, TRUE, &error);
        got = gsgf_text_get_value (GSGF_TEXT (gsgf_date));
        expected_error = NULL;
        g_set_error (&expected_error,
                     GSGF_ERROR, GSGF_ERROR_INVALID_DATE_FORMAT,
                     "Invalid date specification '%s' or out of range",
                     expect);
        if (expect_error (error, expected_error)) {
                g_printerr ("  (failed string was: %s)\n", expect);
                retval = FALSE;
        }

        error = NULL;
        expect = "";
        gsgf_text_set_value (GSGF_TEXT (gsgf_date), expect, TRUE, &error);
        got = gsgf_text_get_value (GSGF_TEXT (gsgf_date));
        expected_error = NULL;
        g_set_error (&expected_error,
                     GSGF_ERROR, GSGF_ERROR_INVALID_DATE_FORMAT,
                     "Empty dates are not allowed");
        if (expect_error (error, expected_error)) {
                g_printerr ("  (failed string was empty)\n");
                retval = FALSE;
        }

        g_object_unref(gsgf_date);

        return retval;
}

static gboolean
test_parse_YYYY_MM (void)
{
        GSGFDate *gsgf_date;
        GDate *date;
        gchar *got;
        gchar *expect;
        gboolean retval = TRUE;
        GError *error;
        GError *expected_error;

        date = g_date_new ();
        g_date_clear (date, 1);
        g_date_set_year (date, 1976);

        gsgf_date = gsgf_date_new (date, NULL);

        error = NULL;
        expect = "2011-03";
        gsgf_text_set_value (GSGF_TEXT (gsgf_date), expect, TRUE, &error);
        got = gsgf_text_get_value (GSGF_TEXT (gsgf_date));
        if (expect_error (error, NULL)) {
                g_printerr ("  (failed string was: %s)\n", expect);
                retval = FALSE;
        } else if (g_strcmp0 (expect, got)) {
                g_printerr ("Expected %s, got %s\n", expect, got);
                retval = FALSE;
        }

        error = NULL;
        expect = "2011-00";
        gsgf_text_set_value (GSGF_TEXT (gsgf_date), expect, TRUE, &error);
        got = gsgf_text_get_value (GSGF_TEXT (gsgf_date));
        expected_error = NULL;
        g_set_error (&expected_error,
                     GSGF_ERROR, GSGF_ERROR_INVALID_DATE_FORMAT,
                     "Invalid date specification '%s' or out of range",
                     expect);
        if (expect_error (error, expected_error)) {
                g_printerr ("  (failed string was: %s)\n", expect);
                retval = FALSE;
        }

#if (0)
        /* This test causes glib to print ugly output.  */
        error = NULL;
        expect = "2011-13";
        gsgf_text_set_value (GSGF_TEXT (gsgf_date), expect, TRUE, &error);
        got = gsgf_text_get_value (GSGF_TEXT (gsgf_date));
        expected_error = NULL;
        g_set_error (&expected_error,
                     GSGF_ERROR, GSGF_ERROR_INVALID_DATE_FORMAT,
                     "Invalid date specification '%s' or out of range",
                     expect);
        if (expect_error (error, expected_error)) {
                g_printerr ("  (failed string was: %s)\n", expect);
                retval = FALSE;
        }
#endif

        g_object_unref(gsgf_date);

        return retval;
}

static gboolean
test_parse_YYYY_MM_DD (void)
{
        GSGFDate *gsgf_date;
        GDate *date;
        gchar *got;
        gchar *expect;
        gboolean retval = TRUE;
        GError *error;
        GError *expected_error;

        date = g_date_new ();
        g_date_clear (date, 1);
        g_date_set_year (date, 1976);

        gsgf_date = gsgf_date_new (date, NULL);

        error = NULL;
        expect = "2011-03-13";
        gsgf_text_set_value (GSGF_TEXT (gsgf_date), expect, TRUE, &error);
        got = gsgf_text_get_value (GSGF_TEXT (gsgf_date));
        if (expect_error (error, NULL)) {
                g_printerr ("  (failed string was: %s)\n", expect);
                retval = FALSE;
        } else if (g_strcmp0 (expect, got)) {
                g_printerr ("Expected %s, got %s\n", expect, got);
                retval = FALSE;
        }

        error = NULL;
        expect = "2011-03-00";
        gsgf_text_set_value (GSGF_TEXT (gsgf_date), expect, TRUE, &error);
        got = gsgf_text_get_value (GSGF_TEXT (gsgf_date));
        expected_error = NULL;
        g_set_error (&expected_error,
                     GSGF_ERROR, GSGF_ERROR_INVALID_DATE_FORMAT,
                     "Invalid date specification '%s' or out of range",
                     expect);
        if (expect_error (error, expected_error)) {
                g_printerr ("  (failed string was: %s)\n", expect);
                retval = FALSE;
        }

#if (0)
        /* This test causes glib to print ugly output.  */
        error = NULL;
        expect = "2011-03-32";
        gsgf_text_set_value (GSGF_TEXT (gsgf_date), expect, TRUE, &error);
        got = gsgf_text_get_value (GSGF_TEXT (gsgf_date));
        expected_error = NULL;
        g_set_error (&expected_error,
                     GSGF_ERROR, GSGF_ERROR_INVALID_DATE_FORMAT,
                     "Invalid date specification '%s' or out of range",
                     expect);
        if (expect_error (error, expected_error)) {
                g_printerr ("  (failed string was: %s)\n", expect);
                retval = FALSE;
        }
#endif

        g_object_unref(gsgf_date);

        return retval;
}

static gboolean
test_parse_multiple (void)
{
        GSGFDate *gsgf_date;
        GDate *date;
        gchar *got;
        gchar *expect;
        gboolean retval = TRUE;
        GError *error;
        GError *expected_error;

        date = g_date_new ();
        g_date_clear (date, 1);
        g_date_set_year (date, 1976);

        gsgf_date = gsgf_date_new (date, NULL);

        error = NULL;
        expect = "2011-03-13,2012-03-13";
        gsgf_text_set_value (GSGF_TEXT (gsgf_date), expect, TRUE, &error);
        got = gsgf_text_get_value (GSGF_TEXT (gsgf_date));
        if (expect_error (error, NULL)) {
                g_printerr ("  (failed string was: %s)\n", expect);
                retval = FALSE;
        } else if (g_strcmp0 (expect, got)) {
                g_printerr ("Expected %s, got %s\n", expect, got);
                retval = FALSE;
        }

        g_object_unref(gsgf_date);

        return retval;
}
