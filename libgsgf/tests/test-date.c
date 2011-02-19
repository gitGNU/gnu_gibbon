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

static gboolean test_creation (void);

int
main(int argc, char *argv[])
{
        GFile *file = NULL;
        GError *error = NULL;
        GSGFCollection *collection = NULL;
        int status = 0;

        g_type_init ();

        if (!test_creation ())
                status = -1;

        return status;
}

static gboolean
test_creation (void)
{
        GSGFDate *gsgf_date;
        GDate *g_date;
        gchar *got;
        gchar *expect;
        gboolean retval = TRUE;

        g_date = g_date_new_dmy (26, 4, 1977);
        gsgf_date = gsgf_date_new (g_date);
        expect = "1977-04-26";
        got = gsgf_text_get_value (GSGF_TEXT (gsgf_date));
        if (g_strcmp0 (expect, got)) {
                g_printerr ("Expected %s, got %s\n", expect, got);
                retval = FALSE;
        }
        g_object_unref(gsgf_date);

        return retval;
}
