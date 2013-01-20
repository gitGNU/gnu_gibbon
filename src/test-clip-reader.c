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
                "1 GibbonTestA 1306865048 gibbon.example.com",
                {
                                { G_TYPE_UINT, "1" },
                                { G_TYPE_STRING, "GibbonTestA" },
                                { G_TYPE_INT64, "1306865048" },
                                { G_TYPE_STRING, "gibbon.example.com" },
                                { G_TYPE_INVALID }
                }
};

static struct test_case test_clip01_1 = {
                "1 GibbonTestB 1306865049 127.128.129.130",
                {
                                { G_TYPE_UINT, "1" },
                                { G_TYPE_STRING, "GibbonTestB" },
                                { G_TYPE_INT64, "1306865049" },
                                { G_TYPE_STRING, "127.128.129.130" },
                                { G_TYPE_INVALID }
                }
};

static struct test_case test_clip02_0 = {
                "2 GibbonTestA 1 1 0 0 0 0 1 1 2396 0 1 0 1 3457.85 0 0 0 0 0"
                " Europe/Sofia",
                {
                                { G_TYPE_UINT, "2" },
                                { G_TYPE_STRING, "GibbonTestA" },
                                { G_TYPE_BOOLEAN, "TRUE" },
                                { G_TYPE_BOOLEAN, "TRUE" },
                                { G_TYPE_BOOLEAN, "FALSE" },
                                { G_TYPE_BOOLEAN, "FALSE" },
                                { G_TYPE_BOOLEAN, "FALSE" },
                                { G_TYPE_BOOLEAN, "FALSE" },
                                { G_TYPE_BOOLEAN, "TRUE" },
                                { G_TYPE_BOOLEAN, "TRUE" },
                                { G_TYPE_UINT64, "2396" },
                                { G_TYPE_BOOLEAN, "FALSE" },
                                { G_TYPE_BOOLEAN, "TRUE" },
                                { G_TYPE_BOOLEAN, "FALSE" },
                                { G_TYPE_BOOLEAN, "TRUE" },
                                { G_TYPE_DOUBLE, "3457.850000" },
                                { G_TYPE_BOOLEAN, "FALSE" },
                                { G_TYPE_BOOLEAN, "FALSE" },
                                { G_TYPE_INT64, "0" },
                                { G_TYPE_BOOLEAN, "FALSE" },
                                { G_TYPE_BOOLEAN, "FALSE" },
                                { G_TYPE_STRING, "Europe/Sofia" },
                                { G_TYPE_INVALID }
                }
};

static struct test_case test_clip02_1 = {
                "2 GibbonTestB 1 1 0 0 0 0 1 1 2396 0 1 0 1 3457.85 0 0 2 0 0"
                " Europe/Sofia",
                {
                                { G_TYPE_UINT, "2" },
                                { G_TYPE_STRING, "GibbonTestB" },
                                { G_TYPE_BOOLEAN, "TRUE" },
                                { G_TYPE_BOOLEAN, "TRUE" },
                                { G_TYPE_BOOLEAN, "FALSE" },
                                { G_TYPE_BOOLEAN, "FALSE" },
                                { G_TYPE_BOOLEAN, "FALSE" },
                                { G_TYPE_BOOLEAN, "FALSE" },
                                { G_TYPE_BOOLEAN, "TRUE" },
                                { G_TYPE_BOOLEAN, "TRUE" },
                                { G_TYPE_UINT64, "2396" },
                                { G_TYPE_BOOLEAN, "FALSE" },
                                { G_TYPE_BOOLEAN, "TRUE" },
                                { G_TYPE_BOOLEAN, "FALSE" },
                                { G_TYPE_BOOLEAN, "TRUE" },
                                { G_TYPE_DOUBLE, "3457.850000" },
                                { G_TYPE_BOOLEAN, "FALSE" },
                                { G_TYPE_BOOLEAN, "FALSE" },
                                { G_TYPE_INT64, "2" },
                                { G_TYPE_BOOLEAN, "FALSE" },
                                { G_TYPE_BOOLEAN, "FALSE" },
                                { G_TYPE_STRING, "Europe/Sofia" },
                                { G_TYPE_INVALID }
                }
};

static struct test_case test_clip02_2 = {
                "2 GibbonTestC 1 1 0 0 0 0 1 1 2396 0 1 0 1 3457.85 0 0"
                " unlimited 0 0 Europe/Sofia",
                {
                                { G_TYPE_UINT, "2" },
                                { G_TYPE_STRING, "GibbonTestC" },
                                { G_TYPE_BOOLEAN, "TRUE" },
                                { G_TYPE_BOOLEAN, "TRUE" },
                                { G_TYPE_BOOLEAN, "FALSE" },
                                { G_TYPE_BOOLEAN, "FALSE" },
                                { G_TYPE_BOOLEAN, "FALSE" },
                                { G_TYPE_BOOLEAN, "FALSE" },
                                { G_TYPE_BOOLEAN, "TRUE" },
                                { G_TYPE_BOOLEAN, "TRUE" },
                                { G_TYPE_UINT64, "2396" },
                                { G_TYPE_BOOLEAN, "FALSE" },
                                { G_TYPE_BOOLEAN, "TRUE" },
                                { G_TYPE_BOOLEAN, "FALSE" },
                                { G_TYPE_BOOLEAN, "TRUE" },
                                { G_TYPE_DOUBLE, "3457.850000" },
                                { G_TYPE_BOOLEAN, "FALSE" },
                                { G_TYPE_BOOLEAN, "FALSE" },
                                { G_TYPE_INT64, "-1" },
                                { G_TYPE_BOOLEAN, "FALSE" },
                                { G_TYPE_BOOLEAN, "FALSE" },
                                { G_TYPE_STRING, "Europe/Sofia" },
                                { G_TYPE_INVALID }
                }
};

static struct test_case test_clip03 = {
                "3",
                {
                                { G_TYPE_UINT, "3" },
                                { G_TYPE_INVALID }
                }
};

static struct test_case test_clip03a = {
                "+------------------------------+",
                {
                                { G_TYPE_UINT, "413" },
                                { G_TYPE_STRING,
                                  "+------------------------------+" },
                                { G_TYPE_INVALID }
                }
};

static struct test_case test_clip03b = {
                "| This is the motto of the day. |",
                {
                                { G_TYPE_UINT, "413" },
                                { G_TYPE_STRING,
                                  "| This is the motto of the day. |"
                                },
                                { G_TYPE_INVALID }
                }
};

static struct test_case test_clip04 = {
                "4",
                {
                                { G_TYPE_UINT, "4" },
                                { G_TYPE_INVALID }
                }
};

static struct test_case *test_cases[] = {
                &test_clip01_0,
                &test_clip01_1,
                &test_clip02_0,
                &test_clip02_1,
                &test_clip02_2,
                &test_clip03,
                &test_clip03a,
                &test_clip03b,
                &test_clip04
};

static gboolean test_single_case (GibbonCLIPReader *reader,
                                  struct test_case *test_case);
static gboolean check_result (const gchar *line, gsize num,
                              struct token_pair *token_pair,
                              GValue *value);

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

        gibbon_clip_reader_free_result (reader, result);

        return retval;
}

static gboolean
check_result (const gchar *line, gsize num,
              struct token_pair *token_pair,
              GValue *value)
{
        gboolean retval = TRUE;
        GValue stringified = G_VALUE_INIT;
        const gchar *got_value;
        const gchar *expect_value;

        g_value_init (&stringified, G_TYPE_STRING);
        g_return_val_if_fail (g_value_transform (value, &stringified), FALSE);

        got_value = g_value_get_string (&stringified);
        expect_value = token_pair->value;

        if (token_pair->type != G_VALUE_TYPE (value)
            || g_strcmp0 (got_value, expect_value)) {
                g_printerr ("%s: token #%llu:"
                            " expected `%s' (token type %s),"
                            " got `%s' (token type %s).\n",
                            line, (unsigned long long) num,
                            expect_value, g_type_name (token_pair->type),
                            got_value, G_VALUE_TYPE_NAME (value));
                retval = FALSE;
        }

        return retval;
}
