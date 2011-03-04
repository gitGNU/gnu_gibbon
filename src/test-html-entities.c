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

#include <glib.h>

#include "html-entities.h"

static gboolean test_encode_decimal (void);

int
main(int argc, char *argv[])
{
	int status = 0;

        g_type_init ();

        if (!test_encode_decimal ())
                status = -1;

        return status;
}

static gboolean
test_encode_decimal (void)
{
        const gchar *original = "My name is Мечо Пух";
        const gchar *expect =
                "My name is &#1052;&#1077;&#1095;&#1086; &#1055;&#1091;&#1093;";
        gchar *got = encode_html_entities (original);
        gboolean retval = TRUE;

        if (g_strcmp0 (expect, got)) {
                g_printerr ("Expected '%s', got '%s'.\n", expect, got);
                retval = FALSE;
        }

        g_free (got);

        return retval;
}
