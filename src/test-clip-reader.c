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

#include "gibbon-clip-reader.h"

struct token_pair {
        GType type;
        const gchar *value;
};

struct test_case {
        const gchar *line;
        struct token_pair values[];
};

static struct test_case test_clip01_0 = {
                "1 gflohr 1306865048 gibbon.example.com",
                {
                                { G_TYPE_UINT, "1" },
                                { G_TYPE_STRING, "gflohr" },
                                { G_TYPE_INT, "1306865048" },
                                { G_TYPE_STRING, "gibbon.example.com" },
                                { G_TYPE_INVALID }
                }
};

static gboolean test_single_case (GibbonCLIPReader *reader,
                                  struct test_case *test_case);
static gboolean check_result (const gchar *line, gsize num,
                              struct token_pair *token_pair,
                              GValue *value);

static struct test_case *test_cases[] = {
                &test_clip01_0
};

int
main (int argc, char *argv[])
{
        gint status = 0;
        gint i;
        GibbonCLIPReader *reader;

        g_type_init ();

        reader = gibbon_clip_reader_new ();

        for (i = 0; i < sizeof test_cases / sizeof test_cases[0]; ++i) {
                if (!test_single_case (reader, test_cases[i]))
                        status = -1;
        }

        return status;
}

static gboolean
test_single_case (GibbonCLIPReader *reader, struct test_case *test_case)
{
        GSList *result = gibbon_clip_reader_parse (reader, test_case->line);
        GSList *iter;
        gboolean retval = TRUE;
        struct token_pair *expect;
        gsize i = 0;
        gchar *expect_type;

        expect = test_case->values;

        iter = result;
        while (iter) {
                ++i;
                if (!check_result (test_case->line, i, expect,
                                   (GValue *) iter->data))
                        retval = FALSE;
                iter = iter->next;
                ++expect;
        }

        while (expect->type != G_TYPE_INVALID) {
                retval = FALSE;
                g_printerr ("%s: token #%llu: expected `%s' (token type %s)"
                            " got nothing.\n",
                            test_case->line, (unsigned long long) ++i,
                            expect->value, g_type_name (expect->type));
                ++expect;
        }

        g_slist_free_full (result, g_free);

        return retval;
}

static gboolean
check_result (const gchar *line, gsize num,
              struct token_pair *token_pair,
              GValue *value)
{
        g_printerr ("Cannot check result at %s:%d.\n", __FILE__, __LINE__);

        return FALSE;
}
