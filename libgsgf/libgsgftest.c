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

#include <libgsgf/gsgf-collection.h>

int
main(int argc, char *argv[])
{
        GFile *file;
        GError *error = NULL;
        GInputStream *stream;
        GSGFCollection *collection;

        if (argc != 2) {
                fprintf (stderr, "Usage: %s INPUT_FILE\n", argv[0]);
                return 1;
        }

        g_type_init ();

        file = g_file_new_for_commandline_arg (argv[1]);
        collection = gsgf_collection_parse_file (file, NULL, &error);

        if (error) {
                fprintf (stderr, "%s: %s\n", argv[1], error->message);
                g_error_free (error);
                return 1;
        }

        return 0;
}
