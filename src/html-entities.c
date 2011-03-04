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

#include <string.h>
#include "html-entities.h"

gchar *
encode_html_entities (const gchar *original)
{
        GString *string = g_string_sized_new (strlen (original));
        const gchar *ptr = original;
        gunichar next_char;
        gchar *retval;

        while (*ptr) {
                next_char = g_utf8_get_char_validated (ptr, -1);
                if (next_char < 0x80) {
                        string = g_string_append_unichar (string, next_char);
                        ++ptr;
                } else {
                        g_string_append_printf (string, "&#%u;", next_char);
                        /* A NULL parameter for the output buffer causes
                         * the function to just compute the length in bytes.
                         */
                        ptr += g_unichar_to_utf8 (next_char, NULL);
                }
        }

        retval = string->str;
        g_string_free (string, FALSE);

        return retval;
}
