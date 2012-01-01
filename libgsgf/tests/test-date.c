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

#include "test.h"

static gboolean test_simple (void);
static gboolean test_shortcut_MM_DD (void);
static gboolean test_shortcut_DD (void);
static gboolean test_partial (void);
static gboolean test_parse_YYYY (void);
static gboolean test_parse_YYYY_MM (void);
static gboolean test_parse_YYYY_MM_DD (void);
static gboolean test_parse_multiple (void);
static gboolean test_parse_MM_DD (void);
static gboolean test_parse_MM (void);
static gboolean test_parse_DD (void);

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
        if (!test_parse_MM_DD ())
                status = -1;
        if (!test_parse_MM ())
                status = -1;
        if (!test_parse_DD ())
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
        GSGFDateDMY date;

        date.day = 26;
        date.month = 4;
        date.year = 2007;
        gsgf_date = gsgf_date_new (&date, NULL);
        expect = "2007-04-26";
        got = gsgf_text_get_value (GSGF_TEXT (gsgf_date));
        if (g_strcmp0 (expect, got)) {
                g_printerr ("Expected %s, got %s\n", expect, got);
                retval = FALSE;
        }

        date.day = 26;
        date.month = 5;
        date.year = 2008;
        gsgf_date_append (gsgf_date, &date, NULL);

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
        GSGFDateDMY dmy;

        dmy.day = 19;
        dmy.month = 2;
        dmy.year = 2011;
        gsgf_date = gsgf_date_new (&dmy, NULL);
        dmy.day = 20;
        dmy.month = 3;
        dmy.year = 2011;
        gsgf_date_append (gsgf_date, &dmy, NULL);
        expect = "2011-02-19,03-20";
        got = gsgf_text_get_value (GSGF_TEXT (gsgf_date));
        if (g_strcmp0 (expect, got)) {
                g_printerr ("Expected %s, got %s\n", expect, got);
                retval = FALSE;
        }

        dmy.day = 21;
        dmy.month = 4;
        dmy.year = 2011;
        gsgf_date_append (gsgf_date, &dmy, NULL);
        expect = "2011-02-19,03-20,04-21";
        got = gsgf_text_get_value (GSGF_TEXT (gsgf_date));
        if (g_strcmp0 (expect, got)) {
                g_printerr ("Expected %s, got %s\n", expect, got);
                retval = FALSE;
        }

        dmy.day = 22;
        dmy.month = 4;
        dmy.year = 2011;
        gsgf_date_append (gsgf_date, &dmy, NULL);
        dmy.day = 23;
        dmy.month = 5;
        dmy.year = 2011;
        gsgf_date_append (gsgf_date, &dmy, NULL);
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
        GSGFDateDMY dmy;

        dmy.day = 19;
        dmy.month = 2;
        dmy.year = 2011;
        gsgf_date = gsgf_date_new (&dmy, NULL);
        dmy.day = 20;
        dmy.month = 2;
        dmy.year = 2011;
        gsgf_date_append (gsgf_date, &dmy, NULL);
        expect = "2011-02-19,20";
        got = gsgf_text_get_value (GSGF_TEXT (gsgf_date));
        if (g_strcmp0 (expect, got)) {
                g_printerr ("Expected %s, got %s\n", expect, got);
                retval = FALSE;
        }

        dmy.day = 21;
        dmy.month = 2;
        dmy.year = 2011;
        gsgf_date_append (gsgf_date, &dmy, NULL);
        expect = "2011-02-19,20,21";
        got = gsgf_text_get_value (GSGF_TEXT (gsgf_date));
        if (g_strcmp0 (expect, got)) {
                g_printerr ("Expected %s, got %s\n", expect, got);
                retval = FALSE;
        }

        dmy.day = 22;
        dmy.month = 3;
        dmy.year = 2011;
        gsgf_date_append (gsgf_date, &dmy, NULL);
        dmy.day = 23;
        dmy.month = 3;
        dmy.year = 2011;
        gsgf_date_append (gsgf_date, &dmy, NULL);
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
        GSGFDateDMY dmy;
        gchar *got;
        gchar *expect;
        gboolean retval = TRUE;
        GError *error, *expect_error;

        error = NULL;
        dmy.year = 2011;
        dmy.month = 3;
        dmy.day = 0;
        gsgf_date = gsgf_date_new (&dmy, &error);
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
        dmy.year = 2011;
        dmy.month = 0;
        dmy.day = 0;
        gsgf_date = gsgf_date_new (&dmy, &error);
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

        dmy.year = dmy.month = dmy.day = 0;
        gsgf_date = gsgf_date_new (&dmy, &error);
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
        GSGFDateDMY dmy;
        gchar *got;
        gchar *expect;
        gboolean retval = TRUE;
        GError *error;
        GError *expected_error;

        dmy.day = 0;
        dmy.month = 0;
        dmy.year = 1976;

        gsgf_date = gsgf_date_new (&dmy, NULL);

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
        GSGFDateDMY dmy;
        gchar *got;
        gchar *expect;
        gboolean retval = TRUE;
        GError *error;
        GError *expected_error;

        dmy.day = 0;
        dmy.month = 0;
        dmy.year = 1976;

        gsgf_date = gsgf_date_new (&dmy, NULL);

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
        GSGFDateDMY dmy;
        gchar *got;
        gchar *expect;
        gboolean retval = TRUE;
        GError *error;
        GError *expected_error;

        dmy.day = 0;
        dmy.month = 0;
        dmy.year = 1976;
        gsgf_date = gsgf_date_new (&dmy, NULL);

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
        GSGFDateDMY dmy;
        gchar *got;
        gchar *expect;
        gboolean retval = TRUE;
        GError *error;
        GError *expected_error;

        dmy.day = 0;
        dmy.month = 0;
        dmy.year = 1976;

        gsgf_date = gsgf_date_new (&dmy, NULL);

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

        /* Test for trailing comma.  */
        error = NULL;
        expect = "2011-03-13,2012-03-13,";
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

        g_object_unref(gsgf_date);

        return retval;
}

static gboolean
test_parse_MM_DD (void)
{
        GSGFDate *gsgf_date;
        GSGFDateDMY dmy;
        gchar *got;
        gchar *expect;
        gboolean retval = TRUE;
        GError *error;

        dmy.day = 0;
        dmy.month = 0;
        dmy.year = 1976;

        gsgf_date = gsgf_date_new (&dmy, NULL);

        /* MM-DD is valid after YYYY-MM-DD.  */
        error = NULL;
        expect = "2011-03-13,04-14";
        gsgf_text_set_value (GSGF_TEXT (gsgf_date), expect, TRUE, &error);
        got = gsgf_text_get_value (GSGF_TEXT (gsgf_date));
        if (expect_error (error, NULL)) {
                g_printerr ("  (failed string was: %s)\n", expect);
                retval = FALSE;
        } else if (g_strcmp0 (expect, got)) {
                g_printerr ("Expected %s, got %s\n", expect, got);
                retval = FALSE;
        }

        /* MM-DD is valid after YYYY-MM.  */
        error = NULL;
        expect = "2011-03,04-14";
        gsgf_text_set_value (GSGF_TEXT (gsgf_date), expect, TRUE, &error);
        got = gsgf_text_get_value (GSGF_TEXT (gsgf_date));
        if (expect_error (error, NULL)) {
                g_printerr ("  (failed string was: %s)\n", expect);
                retval = FALSE;
        } else if (g_strcmp0 (expect, got)) {
                g_printerr ("Expected %s, got %s\n", expect, got);
                retval = FALSE;
        }

        /* MM-DD is valid after MM-DD.  */
        error = NULL;
        expect = "2011-03,04-14,05-14";
        gsgf_text_set_value (GSGF_TEXT (gsgf_date), expect, TRUE, &error);
        got = gsgf_text_get_value (GSGF_TEXT (gsgf_date));
        if (expect_error (error, NULL)) {
                g_printerr ("  (failed string was: %s)\n", expect);
                retval = FALSE;
        } else if (g_strcmp0 (expect, got)) {
                g_printerr ("Expected %s, got %s\n", expect, got);
                retval = FALSE;
        }

        /* MM-DD is valid after MM.  */
        error = NULL;
        expect = "2011-03,04,05-14";
        gsgf_text_set_value (GSGF_TEXT (gsgf_date), expect, TRUE, &error);
        got = gsgf_text_get_value (GSGF_TEXT (gsgf_date));
        if (expect_error (error, NULL)) {
                g_printerr ("  (failed string was: %s)\n", expect);
                retval = FALSE;
        } else if (g_strcmp0 (expect, got)) {
                g_printerr ("Expected %s, got %s\n", expect, got);
                retval = FALSE;
        }

        /* MM-DD is valid after DD.  */
        error = NULL;
        expect = "2011-03-13,14,04-14";
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

static gboolean
test_parse_MM (void)
{
        GSGFDate *gsgf_date;
        GSGFDateDMY dmy;
        gchar *got;
        gchar *expect;
        gboolean retval = TRUE;
        GError *error;

        dmy.day = 0;
        dmy.month = 0;
        dmy.year = 1976;

        gsgf_date = gsgf_date_new (&dmy, NULL);

        /* MM is valid after YYYY-MM.  */
        error = NULL;
        expect = "2011-03,04";
        gsgf_text_set_value (GSGF_TEXT (gsgf_date), expect, TRUE, &error);
        got = gsgf_text_get_value (GSGF_TEXT (gsgf_date));
        if (expect_error (error, NULL)) {
                g_printerr ("  (failed string was: %s)\n", expect);
                retval = FALSE;
        } else if (g_strcmp0 (expect, got)) {
                g_printerr ("Expected %s, got %s\n", expect, got);
                retval = FALSE;
        }

        /* MM is valid after MM.  */
        error = NULL;
        expect = "2011-03,04,06";
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

static gboolean
test_parse_DD (void)
{
        GSGFDate *gsgf_date;
        GSGFDateDMY dmy;
        gchar *got;
        gchar *expect;
        gboolean retval = TRUE;
        GError *error;

        dmy.day = 0;
        dmy.month = 0;
        dmy.year = 1976;

        gsgf_date = gsgf_date_new (&dmy, NULL);

        /* DD is valid after YYYY-MM-DD.  */
        error = NULL;
        expect = "2011-03-13,14";
        gsgf_text_set_value (GSGF_TEXT (gsgf_date), expect, TRUE, &error);
        got = gsgf_text_get_value (GSGF_TEXT (gsgf_date));
        if (expect_error (error, NULL)) {
                g_printerr ("  (failed string was: %s)\n", expect);
                retval = FALSE;
        } else if (g_strcmp0 (expect, got)) {
                g_printerr ("Expected %s, got %s\n", expect, got);
                retval = FALSE;
        }

        /* DD is valid after MM-DD.  */
        error = NULL;
        expect = "2011-03,04-14,15";
        gsgf_text_set_value (GSGF_TEXT (gsgf_date), expect, TRUE, &error);
        got = gsgf_text_get_value (GSGF_TEXT (gsgf_date));
        if (expect_error (error, NULL)) {
                g_printerr ("  (failed string was: %s)\n", expect);
                retval = FALSE;
        } else if (g_strcmp0 (expect, got)) {
                g_printerr ("Expected %s, got %s\n", expect, got);
                retval = FALSE;
        }

        /* DD is valid after DD.  */
        error = NULL;
        expect = "2011-03-13,14,15";
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
