/*
 * This file is part of Gibbon, a graphical frontend to the First Internet 
 * Backgammon Server FIBS.
 * Copyright (C) 2009 Guido Flohr, http://guido-flohr.net/.
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

#include <gsgf.h>

#include "gsgf-private.h"

/* FIXME: This function should be optimized.  It currently copies (and
 * allocates the memory for it) byte by byte.
 */
gchar *
gsgf_util_read_simpletext (const gchar *raw, const gchar **end, 
                           gchar delim)
{
        GString *string = g_string_new("");
        const gchar *ptr = raw;
        gchar *result;
        gboolean escaped = FALSE;
        gchar c;

        while ((escaped || (*ptr != delim)) && *ptr) {
                c = *ptr;

                if (c == '\n' || c == '\t' 
                    || c == '\v' || c == '\f') {
                        c = ' ';
                }

                if (escaped) {
                        escaped = FALSE;
                } else if (c == '\\') {
                        escaped = TRUE;
                }

                if (!escaped) g_string_append_c(string, c);

                ++ptr;
        }

        if (end)
                *end = ptr;
        result = string->str;

        g_string_free(string, FALSE);

        return result;
}

/* FIXME: This function should be optimized.  It currently copies (and
 * allocates the memory for it) byte by byte.
 */
gchar *
gsgf_util_read_text (const gchar *raw, const gchar **end, 
                     gchar delim)
{
        GString *string = g_string_new("");
        const gchar *ptr = raw;
        gchar *result;
        gboolean escaped = FALSE;
        gboolean softbreak;
        gchar c;

        while ((escaped || (*ptr != delim)) && *ptr) {
                c = *ptr;

                softbreak = FALSE;

                if (c == '\t' || c == '\v' || c == '\f')
                        c = ' ';

                if (escaped) {
                        escaped = FALSE;
                        if (c == '\n') softbreak = TRUE;
                } else if (c == '\\') {
                        escaped = TRUE;
                }

                if (!escaped && !softbreak) g_string_append_c(string, c);

                ++ptr;
        }

        if (end)
                *end = ptr;
        result = string->str;

        g_string_free(string, FALSE);

        return result;
}
