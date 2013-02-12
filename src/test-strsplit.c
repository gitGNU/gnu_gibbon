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

#include <glib-object.h>
#include <gibbon-util.h>

static gboolean test_strsplit_ws (const gchar *in, ...);
static gboolean test_strsplit_set (const gchar *in, const gchar *set,
                                   gint max_tokens, ...);

int
main (int argc, char *argv[])
{
        gint status = 0;

        g_type_init ();

        if (!test_strsplit_ws ("", NULL))
                status = -1;
        if (!test_strsplit_ws ("foo bar baz", "foo", "bar", "baz", NULL))
                status = -1;
        if (!test_strsplit_ws ("multiple  delimiters", 
                               "multiple", "delimiters", NULL))
                status = -1;
        if (!test_strsplit_ws ("   leading whitespace", 
                               "leading", "whitespace", NULL))
                status = -1;
        if (!test_strsplit_ws ("   trailing whitespace", 
                               "trailing", "whitespace", NULL))
                status = -1;
        if (!test_strsplit_ws ("  leading and trailing whitespace    ", 
                               "leading", "and", "trailing", "whitespace", 
                               NULL))
                status = -1;
        if (!test_strsplit_ws ("horizontal\ttab", 
                               "horizontal", "tab", NULL))
                status = -1;
        if (!test_strsplit_ws ("line\nfeed", 
                               "line", "feed", NULL))
                status = -1;
        if (!test_strsplit_ws ("vertical\vtab", 
                               "vertical", "tab", NULL))
                status = -1;
        if (!test_strsplit_ws ("form\ffeed", 
                               "form", "feed", NULL))
                status = -1;
        if (!test_strsplit_ws ("carriage\rreturn", 
                               "carriage", "return", NULL))
                status = -1;
        if (!test_strsplit_ws ("all \t\ndelimiters\v\f\rmixed", 
                               "all", "delimiters", "mixed", NULL))
                status = -1;

        if (!test_strsplit_ws ("all \t\ndelimiters\v\f\rmixed\v\f\r",
                               "all", "delimiters", "mixed", NULL))
                status = -1;

        if (!test_strsplit_set ("foo bar. baz",
                                " .", 4, "foo", "bar", "baz", NULL))
                status = -1;

        if (!test_strsplit_set ("foo bar. baz",
                                " .", 3, "foo", "bar", "baz", NULL))
                status = -1;

        if (!test_strsplit_set ("foo bar. baz",
                                " .", 2, "foo", "bar. baz", NULL))
                status = -1;

        if (!test_strsplit_set ("foo bar. baz ",
                                " .", 2, "foo", "bar. baz", NULL))
                status = -1;

        return status;
}

static gboolean
test_strsplit_ws (const gchar *in, ...)
{
        gchar **tokens;
        va_list list;
        const gchar *arg;
        const gchar *prefix = "Splitting at whitespace";
        gsize i = 0;
        gboolean retval = TRUE;

        tokens = gibbon_strsplit_ws (in);
        if (!tokens) {
                g_printerr ("%s: %s: no vector returned.\n", prefix, in);
                return FALSE;
        }

        va_start (list, in);
        do {
                arg = va_arg (list, const gchar *);
                if (g_strcmp0 (arg, tokens[i])) {
                        g_printerr ("%s: %s: token #%llu:"
                                    "expected '%s', got '%s'.\n", 
                                    prefix, in, (unsigned long long) i,
                                    arg, tokens[i]);
                        retval = FALSE;
                        break;
                }
                ++i;
        } while (arg);

        va_end (list);

        g_strfreev (tokens);

        return retval;
}

static gboolean
test_strsplit_set (const gchar *in, const gchar *set, gint max_tokens, ...)
{
        gchar **tokens;
        va_list list;
        const gchar *arg;
        const gchar *prefix = "Splitting at ";
        gsize i = 0;
        gboolean retval = TRUE;

        tokens = gibbon_strsplit_set (in, set, max_tokens);
        if (!tokens) {
                g_printerr ("%s: %s: no vector returned.\n", prefix, in);
                return FALSE;
        }

        va_start (list, max_tokens);
        do {
                arg = va_arg (list, const gchar *);
                if (g_strcmp0 (arg, tokens[i])) {
                        g_printerr ("%s '%s': %s (max_tokens: %d): token #%llu:"
                                    "expected '%s', got '%s'.\n",
                                    prefix, set, in, max_tokens,
                                    (unsigned long long) i, arg, tokens[i]);
                        retval = FALSE;
                        break;
                }
                ++i;
        } while (arg);

        va_end (list);

        g_strfreev (tokens);

        return retval;
}
