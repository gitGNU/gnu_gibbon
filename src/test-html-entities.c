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
static gboolean test_encode_named (void);

int
main(int argc, char *argv[])
{
	int status = 0;

        g_type_init ();

        if (!test_encode_decimal ())
                status = -1;
        if (!test_encode_named ())
                status = -1;

        return status;
}

static gboolean
test_encode_decimal (void)
{
        const gchar *original = "My name is \xd0\x9c\xd0\xb5\xd1\x87\xd0\xbe"
                                 " \xd0\x9f\xd1\x83\xd1\x85";
        const gchar *expect =
                "My name is &#x41c;&#x435;&#x447;&#x43e; &#x41f;&#x443;&#x445;";
        gchar *got = encode_html_entities (original);
        gboolean retval = TRUE;

        if (g_strcmp0 (expect, got)) {
                g_printerr ("Expected '%s', got '%s'.\n", expect, got);
                retval = FALSE;
        }

        g_free (got);

        return retval;
}

static gboolean
test_encode_named (void)
{
        const gchar *original = "Fünf Äpfel für 2 €";
        const gchar *expect = "F&uuml;nf &Auml;pfel f&uuml;r 2 &euro;";
        gchar *got = encode_html_entities (original);
        gboolean retval = TRUE;

        if (g_strcmp0 (expect, got)) {
                g_printerr ("Expected '%s', got '%s'.\n", expect, got);
                retval = FALSE;
        }

        g_free (got);

        return retval;
}
