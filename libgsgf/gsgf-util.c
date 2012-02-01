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

#include <gsgf.h>

#include "gsgf-private.h"

/**
 * SECTION:gsgf-util
 * @short_description: Utility Functions
 * @title: Utility Functions
 *
 * Some utility functions for libgsgf.
 */

/**
 * gsgf_util_read_simple_text:
 * @raw: The string to read from.
 * @end: Optional location to store an end pointer.
 * @delim: Stop reading, when encountering this #gchar.
 *
 * Extracts a SGF simple text from a string.  The function unescapes everything
 * according to the SGF specification.  It stops reading at the first
 * occurence of the unescaped delimiter @delim or a null byte, whichever
 * comes first.
 *
 * If you want to just unescape a string without a delimiter, simply pass
 * 0 for @delim.
 *
 * The string returned must be freed with g_free().  The function cannot
 * fail unless you pass a #NULL pointer in @raw.
 *
 * Returns: The extracted string copied to a newly allocated buffer.
 */
/* FIXME: This function should be optimized.  It currently copies (and
 * allocates the memory for it) byte by byte.
 */
gchar *
gsgf_util_read_simple_text (const gchar *raw, const gchar **end, gchar delim)
{
        GString *string;
        const gchar *ptr = raw;
        gchar *result;
        gboolean escaped = FALSE;
        gchar c;

        if (end)
                *end = raw;
        g_return_val_if_fail(raw != NULL, NULL);

        string = g_string_new("");

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

/**
 * gsgf_util_read_text:
 * @raw: The string to read from.
 * @end: Optional location to store an end pointer.
 * @delim: Stop reading, when encountering this #gchar.
 *
 * Extracts a SGF text value from a string.  The function unescapes everything
 * according to the SGF specification.  It stops reading at the first
 * occurence of the unescaped delimiter @delim or a null byte, whichever
 * comes first.
 *
 * If you want to just unescape a string without a delimiter, simply pass
 * 0 for @delim.
 *
 * The string returned must be freed with g_free().  The function cannot
 * fail unless you pass a #NULL pointer in @raw.
 *
 * Returns: The extracted string copied to a newly allocated buffer.
 */
/* FIXME: This function should be optimized.  It currently copies (and
 * allocates the memory for it) byte by byte.
 */
gchar *
gsgf_util_read_text (const gchar *raw, const gchar **end, gchar delim)
{
        GString *string = g_string_new("");
        const gchar *ptr = raw;
        gchar *result;
        gboolean escaped = FALSE;
        gboolean softbreak;
        gchar c;

        if (end)
                *end = (gchar *) raw;
        g_return_val_if_fail(raw != NULL, NULL);

        string = g_string_new("");

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

/**
 * gsgf_ascii_dtostring:
 * @d: The #gdouble to format.
 * @width: (Maximum) output width before zero-trimming or -1 for default
 * @precision: Decimal precision before zero-trimming or -1 for default
 * @zeropad: Left-pad with zeros if #TRUE
 * @zerotrim: Right
 *
 * Formats @d as a string using the portable POSIX locale, i.e. using the
 * dot ('.') as the decimal point, and no thousands separator.  The function
 * is threadsafe.
 *
 * Invalid values for @width or @precision are silently replaced by -1.
 *
 * The number is first formatted using standard printf formatting directives.
 * If both @width and @precision are given, the format string is
 * "\%@width.@precision f" (without the space in front of the f).  If @zeropad
 * is TRUE, the format string is "\%0@width.@precision f" instead (again
 * without the gratuitous space in front of the f).
 *
 * If @zerotrim is true, trailing zeros after the decimal point are removed.
 * If no decimal digits are left, the decimal point is removed as well.
 *
 * Example: If you zero-trim the intermediate output "123.45600", the result
 * will be "123.456".  Zero-trimming the output "123.000" will result in "123".
 * Zero-trimming the output "123.000000001" will not change the string at
 * all.
 *
 * Returns: The formatted value in a newly allocated buffer.
 */
gchar *
gsgf_ascii_dtostring (gdouble d, gint width, gint precision,
                      gboolean zeropad, gboolean zerotrim)
{
        gchar *format = "%f";
        gchar *output;

        output = g_strdup_printf (format, d);

        return output;
}
