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

#include "gibbon-gmd-reader.h"
#include "gibbon-gmd-writer.h"

int
main (int argc, char *argv[])
{
        gint status = 0;
        gchar *input_file;
        GibbonMatchReader *reader;
        GibbonMatchWriter *writer;
        GibbonMatch *match;
        gchar *wanted, *got;
        GError *error = NULL;
        GOutputStream *out;

        g_type_init ();

        reader = GIBBON_MATCH_READER (gibbon_gmd_reader_new (NULL, NULL));
        g_return_val_if_fail (reader != NULL, -1);

        input_file = g_build_filename (ABS_SRCDIR, "7point.gmd", NULL);
        match = gibbon_match_reader_parse (reader, input_file);

        g_return_val_if_fail (match != NULL, -1);

        if (!g_file_get_contents (input_file, &wanted, NULL, &error)) {
                g_printerr ("Error reading `%s': %s\n",
                            input_file, error->message);
                return -1;
        }

        writer = GIBBON_MATCH_WRITER (gibbon_gmd_writer_new ());
        g_return_val_if_fail (writer != NULL, -1);

        out = G_OUTPUT_STREAM (g_memory_output_stream_new (NULL, 0,
                                                           g_realloc,
                                                           g_free));
        g_return_val_if_fail (out != NULL, -1);

        if (!gibbon_match_writer_write_stream (writer, out, match, &error)) {
                g_printerr ("%s.\n", error->message);
                return 1;
        }

        got = g_memory_output_stream_get_data (G_MEMORY_OUTPUT_STREAM (out));
        if (g_strcmp0 (wanted, got)) {
                g_printerr ("Wanted contents of `%s', got:\n%s--- END ---\n",
                            input_file, got);
                return -1;
        }


        g_free (wanted);
        g_free (input_file);

        g_object_unref (match);
        g_object_unref (reader);

        return status;
}

