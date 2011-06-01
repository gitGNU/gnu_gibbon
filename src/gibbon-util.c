/*
 * This file is part of gibbon.
 * Gibbon is a Gtk+ frontend for the First Internet Backgammon Server FIBS.
 * Copyright (C) 2009-2011 Guido Flohr, http://guido-flohr.net/.
 *
 * gibbon is free software: you can redistribute it and/or modify 
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * gibbon is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with gibbon.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <glib/gi18n.h>
#include "gibbon-util.h"


gchar **
gibbon_strsplit_ws (const gchar *string)
{
        gchar **vector = NULL;
        GSList *list = NULL;
        GSList *iter;
        gsize i, num_tokens = 0;
        const gchar *start;
        const gchar *ptr;

        if (!string) {
                vector = g_new (gchar *, num_tokens + 1);
                vector[0] = NULL;
                return vector;
        }

        ptr = string;

        while (1) {
                while (*ptr == ' ' || *ptr == '\t' || *ptr == '\n'
                       || *ptr == '\v' || *ptr == '\f' || *ptr == '\r')
                        ++ptr;
                if (!*ptr)
                        break;
                start = ptr;
                while (*ptr && *ptr != ' ' && *ptr != '\t' && *ptr != '\n'
                       && *ptr != '\v' && *ptr != '\f' && *ptr != '\r')
                        ++ptr;
                list = g_slist_prepend (list, g_strndup (start, ptr - start));
                ++num_tokens;
        }

        vector = g_new (gchar *, num_tokens + 1);
        iter = list;
        for (i = 0; i < num_tokens; ++i) {
                vector[num_tokens - i - 1] = iter->data;
                iter = iter->next;
        }
        vector[num_tokens] = NULL;

        g_slist_free (list);

        return vector;
}

const gchar *
gibbon_skip_ws_tokens (const gchar *string, const gchar * const * const tokens,
                       gsize num)
{
        gsize i;
        gsize length;
        const gchar *previous;
        const gchar *retval;

        retval = string;

        while (*retval == ' ' || *retval == '\t' || *retval == '\n'
               || *retval == '\v' || *retval == '\f' || *retval == '\r')
                ++retval;

        if (!*retval)
                return NULL;

        for (i = 0; i < num; ++i) {
                if (!tokens[i])
                        return NULL;
                length = strlen (tokens[i]);
                if (strncmp (tokens[i], retval, length))
                        return NULL;
                retval += length;
                while (*retval == ' ' || *retval == '\t' || *retval == '\n'
                       || *retval == '\v' || *retval == '\f' || *retval == '\r')
                        ++retval;
        }

        if (!*retval)
                return NULL;

        do {
                previous = retval - 1;
                if (*previous != ' ' && *previous != '\t' && *previous != '\n'
                    && *previous != '\v' && *previous != '\f' && *previous != '\r')
                        break;
                --retval;
        } while (1);
        if (*retval == ' ' || *retval == '\t' || *retval == '\n'
            || *retval == '\v' || *retval == '\f' || *retval == '\r')
                ++retval;

        return retval;
}
