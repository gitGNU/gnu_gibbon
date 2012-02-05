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

#include <glib/gi18n.h>

#include <libgsgf/gsgf.h>

struct gsgf_util_test {
        const gchar *raw;
        const gchar delim;
        const gchar *expect;
        const gssize offset;
};

static struct gsgf_util_test tests[] = {
        { "basic", 0, "basic", 5 },
        { "before:after", ':', "before", 6 },
        { "line\\\nfeed", 0, "linefeed", 10 },
        { "line\nfeed", 0, "line\nfeed", 9 },
        { "tab\tstop", 0, "tab stop", 8 },
        { "\\e\\s\\c\\a\\p\\e\\\\", 0, "escape\\", 14 },
        { "before\\:before:after", ':', "before:before", 14 },
};

int
main(int argc, char *argv[])
{
        struct gsgf_util_test *current = tests;
        const gchar *end;
        gchar *result;
        int status = 0;
        gssize offset;
        int i;

        for (i = 0; i < sizeof tests / sizeof tests[0]; ++i) {
                current = tests + i;
                result = gsgf_util_read_text(current->raw,
                                             &end, current->delim);
                if (!result) {
                        fprintf(stderr, "Test %d: got NULL.\n", i + 1);
                        status = 1;
                } else {
                        if (strcmp (current->expect,
                                    result)) {
                                fprintf(stderr,
                                        "Test %d: got \"%s\", not \"%s\"\n",
                                        i + 1, result, current->expect);
                                status = 1;
                        } else {
                                offset = end - current->raw;
                                if (offset != current->offset) {
                                        fprintf (stderr,
                                                 "Test %d: expected offset"
                                                 "%lld not %lld\n",
                                                 i + 1,
                                                 (long long) current->offset,
                                                 (long long) offset);
                                        status = 1;
                                }
                        }
                }

                if (result)
                        g_free(result);

                ++current;
        }

        return status;
}
