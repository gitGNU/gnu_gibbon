/*
 * This file is part of Gibbon, a graphical frontend to the First Internet 
 * Backgammon Server FIBS.
 * Copyright (C) 2009 Guido Flohr, http://guido-flohr.net/.
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

#include "test.h"

static char *program_name;
static char *path;

int
main(int argc, char *argv[])
{
        GFile *file;
        GError *error = NULL;
        GSGFCollection *collection;
        int status;

        g_type_init ();

        program_name = argv[0];
        path = g_build_filename(TEST_DIR, filename, NULL);

        file = g_file_new_for_commandline_arg (filename);
        collection = gsgf_collection_parse_file (file, NULL, &error);

        status = test_collection(collection, error);

        if (file) g_object_unref (file);
        if (collection) g_object_unref (collection);
        if (error) g_error_free(error);

        return status;
}

int
expect_error(GError *error, GError *expect)
{
        if (!error && !expect)
                return 0;

        if (!error && expect) {
                fprintf(stderr, "%s: Expected error '%s' but got none.\n",
                        path, expect->message);
                g_error_free(expect);
                return -1;
        }

        if (error && !expect) {
                fprintf(stderr, "%s: %s\n", path, error->message);
                return -1;
        }

        if (error && expect) {
                if (strcmp (error->message, expect->message)) {
                        fprintf(stderr, 
                                "%s: Expected error '%s' but got '%s'\n",
                                path, expect->message, error->message);
                        g_error_free(expect);
                        return -1;
                }
                if (error->domain != expect->domain) {
                        fprintf(stderr, 
                                "%s: Expected error domain '%s' but got '%s'\n",
                                path,
                                g_quark_to_string(expect->domain),
                                g_quark_to_string(error->domain));
                        g_error_free(expect);
                        return -1;
                }
                if (error->code != expect->code) {
                        fprintf(stderr, 
                                "%s: Expected error code %d but got %d.\n",
                                path, expect->code, error->code);
                        g_error_free(expect);
                        return -1;
                }

                g_error_free(expect);
        }

        return 0;
}
