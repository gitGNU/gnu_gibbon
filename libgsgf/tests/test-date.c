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

static gboolean test_simple (void);
static gboolean test_shortcut_MM_DD (void);
static gboolean test_shortcut_DD (void);

int
main(int argc, char *argv[])
{
        int status = 0;

        g_type_init ();

        if (!test_simple ())
                status = -1;
        if (!test_shortcut_MM_DD ())
                status = -1;
        if (!test_shortcut_DD ())
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
