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

#include "test.h"

static int test_compose();

int
main(int argc, char *argv[])
{
        int status;

        g_type_init();

        status = test_compose();
        if (status)
                return status;

        return 0;
}

static int
test_compose(void)
{
        GSGFCookedValue *number = GSGF_COOKED_VALUE(gsgf_number_new(13));
        GSGFCookedValue *text = GSGF_COOKED_VALUE(gsgf_text_new("foo"));
        GSGFCookedValue *simple_text = GSGF_COOKED_VALUE(gsgf_simple_text_new("bar"));
        GSGFCompose *compose = gsgf_compose_new(number, text, simple_text, NULL);
        gsize i;
        GOutputStream *out;
        gsize written;
        gboolean success;
        gchar *expect = "13:foo:bar";
        gchar *got;
        GError *error = NULL;

        if (!compose) {
                fprintf(stderr, "Constructor gsgf_compose_new failed.\n");
                return -1;
        }

        i = gsgf_compose_get_number_of_values(compose);
        if (i != 3) {
                fprintf(stderr, "Expected 3 values, got %lld.\n",
                        (long long) i);
                return -1;
        }

        if (number != gsgf_compose_get_value(compose, 0)) {
                fprintf(stderr, "Wrong element #0.\n");
                return -1;
        }

        if (text != gsgf_compose_get_value(compose, 1)) {
                fprintf(stderr, "Wrong element #1.\n");
                return -1;
        }

        if (simple_text != gsgf_compose_get_value(compose, 2)) {
                fprintf(stderr, "Wrong element #0.\n");
                return -1;
        }

        out = g_memory_output_stream_new(NULL, 0, g_realloc, g_free);
        success = gsgf_value_write_stream (GSGF_VALUE (compose),
                                           out, &written,
                                           NULL, &error);
        if (!success) {
                fprintf(stderr, "write_stream failed: %s\n", error->message);
                return-1;
        }

        got = (gchar*) g_memory_output_stream_get_string(G_MEMORY_OUTPUT_STREAM(out));
        if (strcmp(got, expect)) {
                fprintf(stderr, "Expected '%s', got '%s'.\n", expect, got);
                return -1;
        }

        g_free(got);
        g_object_unref(out);
        g_object_unref(compose);

        return 0;
}
