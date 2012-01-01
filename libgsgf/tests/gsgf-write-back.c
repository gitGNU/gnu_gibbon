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
#include <string.h>

#include <glib/gi18n.h>

#include <libgsgf/gsgf.h>

static char *program_name;

/*
 * This program does essentially the following:
 *
 * It reads the first argument on the command line as an SGF file and
 * fails gracefully if reading or parsing that file fails.  Such failures
 * are already detected by other tests.
 *
 * It then re-writes the collection to another file, the second command
 * line argument and exits with an error if that fails.
 *
 * Then, it re-reads the second file, and writes it back to a file
 * specified as the third command line argument.  If either of that fails,
 * the program exits with an error.
 *
 * First, we check that all classes properly implement the write_stream()
 * method.  Second, we check that files created by libgsgf are round-trip-safe.
 */
int
main (int argc, char *argv[])
{
        GError *error = NULL;
        GSGFCollection *collection = NULL;
        gchar *sgf_filename;
        GFile *sgf_file;
        gchar *tmp1_filename;
        GFile *tmp1_file;
        GOutputStream *tmp1_out;
        gchar *tmp2_filename;
        GFile *tmp2_file;
        GOutputStream *tmp2_out;
        gsize bytes_written;

        g_type_init ();

        program_name = argv[0];
        if (argc != 4) {
                fprintf (stderr, "Usage: %s SGF_FILE TMP_FILE1 TMP_FILE2\n",
                                program_name);
                return 1;
        }

        sgf_filename = argv[1];
        sgf_file = g_file_new_for_commandline_arg (sgf_filename);
        collection = gsgf_collection_parse_file (sgf_file, NULL, &error);
        if (sgf_file) g_object_unref (sgf_file);
        /* Parse errors are checked by other tests.  If we get a parse
         * error here, then the SGF file is intentionally broken.
         */
        if (error || !collection)
                return 0;

        tmp1_filename = argv[2];
        tmp1_file = g_file_new_for_commandline_arg (tmp1_filename);
        if (g_file_query_exists (tmp1_file, NULL)) {
                /* Racy but that is tolerable for our test case.  */
                if (!g_file_delete (tmp1_file, NULL, &error)) {
                        fprintf(stderr, "%s: Cannot delete: %s!\n",
                                tmp1_filename, error->message);
                        return 1;
                }
        }
        tmp1_out = G_OUTPUT_STREAM (g_file_create(tmp1_file,
                        G_FILE_CREATE_REPLACE_DESTINATION,
                        NULL, &error));
        if (!tmp1_out) {
                fprintf(stderr, "%s: Cannot create: %s!\n",
                        tmp1_filename, error->message);
                return 1;
        }
        if (!gsgf_component_write_stream (GSGF_COMPONENT (collection),
                                          tmp1_out, &bytes_written,
                                          NULL, &error)) {
                fprintf(stderr, "%s: Cannot write to stream: %s!\n",
                        tmp1_filename, error->message);
                return 1;
        }
        if (!g_output_stream_close (tmp1_out, NULL, &error)) {
                g_printerr ("%s: Cannot close stream: %s!\n",
                            tmp1_filename, error->message);
                return 1;
        }
        g_object_unref (tmp1_out);
        g_object_unref (collection);

        collection = gsgf_collection_parse_file (tmp1_file, NULL, &error);
        if (tmp1_file) g_object_unref (tmp1_file);
        if (!collection) {
                fprintf(stderr, "%s (%s): Cannot re-read collection: %s!\n",
                        sgf_filename, tmp1_filename, error->message);
                return 1;
        }

        tmp2_filename = argv[3];
        tmp2_file = g_file_new_for_commandline_arg (tmp2_filename);
        if (g_file_query_exists (tmp2_file, NULL)) {
                /* Racy but that is tolerable for our test case.  */
                if (!g_file_delete (tmp2_file, NULL, &error)) {
                        fprintf(stderr, "%s: Cannot delete: %s!\n",
                                tmp2_filename, error->message);
                        return 1;
                }
        }
        tmp2_out = G_OUTPUT_STREAM (g_file_create(tmp2_file,
                        G_FILE_CREATE_REPLACE_DESTINATION,
                        NULL, &error));
        if (!tmp2_out) {
                fprintf(stderr, "%s: Cannot create: %s!\n",
                        tmp2_filename, error->message);
                return 1;
        }
        if (!gsgf_component_write_stream (GSGF_COMPONENT (collection),
                                          tmp2_out, &bytes_written,
                                          NULL, &error)) {
                fprintf(stderr, "%s: Cannot write to stream: %s!\n",
                        tmp2_filename, error->message);
                return 1;
        }
        g_object_unref (tmp2_out);
        g_object_unref (tmp2_file);
        g_object_unref (collection);

        return 0;
}
