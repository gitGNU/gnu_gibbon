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

#include <glib-object.h>

#include "gibbon-clip-parse.h"

struct token_pair {
        enum GibbonClipType type;
        const gchar *value;
};

struct test_case {
        const gchar *line;
        struct token_pair tokens[];
};

struct test_case test_case01 = {
                "1 gflohr 1306865048 gibbon.example.com",
                {
                                { GIBBON_CLIP_TYPE_UINT, "1" },
                                { GIBBON_CLIP_TYPE_NAME, "gflohr" },
                                { GIBBON_CLIP_TYPE_TIMESTAMP, "1306865048" },
                                { GIBBON_CLIP_TYPE_STRING,
                                                "gibbon.example.com" },
                                { GIBBON_CLIP_TYPE_END, NULL }
                }
};

struct test_case test_case02 = {
                "2 gflohr 1 1 0 0 0 0 1 1 2396 0 1 0 1 3457.85 0 0 0 0 0"
                " Europe/Sofia",
                {
                                { GIBBON_CLIP_TYPE_UINT, "2" },
                                { GIBBON_CLIP_TYPE_NAME, "gflohr" },
                                { GIBBON_CLIP_TYPE_BOOLEAN, "TRUE" },
                                { GIBBON_CLIP_TYPE_BOOLEAN, "TRUE" },
                                { GIBBON_CLIP_TYPE_BOOLEAN, "FALSE" },
                                { GIBBON_CLIP_TYPE_BOOLEAN, "FALSE" },
                                { GIBBON_CLIP_TYPE_BOOLEAN, "FALSE" },
                                { GIBBON_CLIP_TYPE_BOOLEAN, "FALSE" },
                                { GIBBON_CLIP_TYPE_BOOLEAN, "TRUE" },
                                { GIBBON_CLIP_TYPE_BOOLEAN, "TRUE" },
                                { GIBBON_CLIP_TYPE_UINT, "2396" },
                                { GIBBON_CLIP_TYPE_BOOLEAN, "FALSE" },
                                { GIBBON_CLIP_TYPE_BOOLEAN, "TRUE" },
                                { GIBBON_CLIP_TYPE_BOOLEAN, "FALSE" },
                                { GIBBON_CLIP_TYPE_BOOLEAN, "TRUE" },
                                { GIBBON_CLIP_TYPE_DOUBLE, "3457.850000" },
                                { GIBBON_CLIP_TYPE_BOOLEAN, "FALSE" },
                                { GIBBON_CLIP_TYPE_BOOLEAN, "FALSE" },
                                { GIBBON_CLIP_TYPE_BOOLEAN, "FALSE" },
                                { GIBBON_CLIP_TYPE_BOOLEAN, "FALSE" },
                                { GIBBON_CLIP_TYPE_BOOLEAN, "FALSE" },
                                { GIBBON_CLIP_TYPE_STRING, "Europe/Sofia" },
                                { GIBBON_CLIP_TYPE_END, NULL }
                }
};

struct test_case *test_cases[] = {
                &test_case01,
                &test_case02
};

static gboolean test_single_case (struct test_case *test_case);
static gboolean check_result (const gchar *line, gsize num,
                              struct token_pair *token_pair,
                              struct GibbonClipTokenSet *token_set);
static gchar *stringify_token_type (enum GibbonClipType type);
static gchar *stringify_token_value (struct GibbonClipTokenSet *token_set);

int
main (int argc, char *argv[])
{
        gint status = 0;
        gsize i;

        g_type_init ();

        for (i = 0; i < sizeof test_cases / sizeof test_cases[0]; ++i) {
                if (!test_single_case (test_cases[i]))
                        status = -1;
        }

        return status;
}

static gboolean
test_single_case (struct test_case *test_case)
{
        GSList *result = gibbon_clip_parse (test_case->line);
        GSList *iter;
        gboolean retval = TRUE;
        struct token_pair *expect;
        gsize i = 0;
        gchar *expect_type;

        if (!result) {
                g_printerr ("%s: parser returned NULL!\n", test_case->line);

                return FALSE;
        }

        iter = result;
        expect = test_case->tokens;
        while (iter) {
                ++i;
                if (!check_result (test_case->line, i, expect,
                                   (struct GibbonClipTokenSet *) iter->data))
                        retval = FALSE;
                if (expect->type == GIBBON_CLIP_TYPE_END)
                        break;
                iter = iter->next;
                ++expect;
        }

        if (expect->type != GIBBON_CLIP_TYPE_END) {
                expect_type = stringify_token_type (expect->type);

                g_printerr ("%s: token #%u: expected `%s' (token type %s)"
                            " got nothing.\n",
                            test_case->line, i + 1,
                            expect->value, expect_type);
                g_free (expect_type);

                retval = FALSE;
        }

        gibbon_clip_free_result (result);

        return retval;
}

static gboolean
check_result (const gchar *line, gsize num, struct token_pair *token_pair,
              struct GibbonClipTokenSet *token_set)
{
        gchar *got_value;
        gchar *got_type = NULL;
        gchar *expect_type = NULL;
        const gchar *expect_value = NULL;

        gboolean retval = TRUE;

        got_value = stringify_token_value (token_set);
        expect_value = token_pair->value;

        if (token_pair->type != token_set->type
            || g_strcmp0 (got_value, expect_value)) {
                got_type = stringify_token_type (token_set->type);
                expect_type = stringify_token_type (token_pair->type);
                g_printerr ("%s: token #%u:"
                            " expected `%s' (token type %s),"
                            " got `%s' (token type %s).\n",
                            line, num,
                            expect_value, expect_type,
                            got_value, got_type);
                retval = FALSE;
        }

        g_free (got_value);
        g_free (got_type);
        g_free (expect_type);

        return retval;
}

static gchar *
stringify_token_type (enum GibbonClipType type)
{
        switch (type) {
                case GIBBON_CLIP_TYPE_END:
                        return g_strdup ("end");
                case GIBBON_CLIP_TYPE_UINT:
                        return g_strdup ("unsigned integer");
                case GIBBON_CLIP_TYPE_INT:
                        return g_strdup ("integer");
                case GIBBON_CLIP_TYPE_DOUBLE:
                        return g_strdup ("double");
                case GIBBON_CLIP_TYPE_BOOLEAN:
                        return g_strdup ("boolean");
                case GIBBON_CLIP_TYPE_STRING:
                        return g_strdup ("string");
                case GIBBON_CLIP_TYPE_NAME:
                        return g_strdup ("name");
                case GIBBON_CLIP_TYPE_TIMESTAMP:
                        return g_strdup ("timestamp");
        }

        return NULL;
}

static gchar *
stringify_token_value (struct GibbonClipTokenSet *token_set)
{
        switch (token_set->type) {
                case GIBBON_CLIP_TYPE_END:
                        return NULL;
                case GIBBON_CLIP_TYPE_UINT:
                        return g_strdup_printf ("%llu",
                                                (guint64) token_set->v.i64);
                case GIBBON_CLIP_TYPE_INT:
                        return g_strdup_printf ("%lld",
                                                (gint64) token_set->v.i64);
                case GIBBON_CLIP_TYPE_DOUBLE:
                        return g_strdup_printf ("%f",
                                                (gdouble) token_set->v.d);
                case GIBBON_CLIP_TYPE_BOOLEAN:
                        if (token_set->v.i64)
                                return g_strdup ("TRUE");
                        else
                                return g_strdup ("FALSE");
                case GIBBON_CLIP_TYPE_STRING:
                case GIBBON_CLIP_TYPE_NAME:
                        return g_strdup (token_set->v.s);
                case GIBBON_CLIP_TYPE_TIMESTAMP:
                        return g_strdup_printf ("%lld",
                                                (gint64) token_set->v.i64);
        }

        return NULL;
}
