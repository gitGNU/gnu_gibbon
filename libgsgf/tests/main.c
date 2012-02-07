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

#include "test.h"

static char *program_name;

int
main(int argc, char *argv[])
{
        GFile *file = NULL;
        GError *error = NULL;
        GSGFCollection *collection = NULL;
        GSGFComponent *culprit;
        int status;

        g_type_init ();

        program_name = argv[0];

        if (filename) {
                path = build_filename (filename);

                file = g_file_new_for_commandline_arg (path);
                collection = gsgf_collection_parse_file (file, NULL, &error);
                if (collection)
                        (void) gsgf_component_cook (GSGF_COMPONENT (collection),
                                                    &culprit, &error);
        }

        if (!path)
                path = "";

        status = test_collection (collection, error);

        if (file) g_object_unref (file);
        if (collection) g_object_unref (collection);
        if (error) g_error_free (error);

        return status;
}
