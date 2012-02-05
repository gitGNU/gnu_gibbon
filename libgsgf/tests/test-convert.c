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

#include "test.h"

char *filename = "cp1251.sgf";

int 
test_collection(GSGFCollection *collection, GError *error)
{
        GOutputStream *out = g_memory_output_stream_new(NULL, 0, 
                                                        g_realloc, g_free);
        gsize written;
        gboolean success =
                gsgf_component_write_stream (GSGF_COMPONENT (collection), out,
                                             &written, NULL, &error);
        gchar *expect;
        gchar *got;
        gchar *expect_path;

        if (error) return expect_error(error, NULL);
        if (!success) {
                fprintf (stderr, "Failure without error!\n");
                return -1;
        }

        expect_path = build_filename("utf-8.sgf.in");

        if (!g_file_get_contents(expect_path, &expect, NULL, &error))
                return expect_error(error, NULL);

        got = (gchar*) g_memory_output_stream_get_string(G_MEMORY_OUTPUT_STREAM(out));
        if (strcmp(got, expect)) {
                fprintf(stderr, "Expected '%s', got '%s'.\n", expect, got);
                return -1;
        }

        return 0;
}
