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

#include "gibbon-java-fibs-reader.h"

int
main (int argc, char *argv[])
{
        gint status = 0;
        gchar *input_file;
        GibbonMatchReader *reader;
        GibbonMatch *match;

        g_type_init ();

        reader = GIBBON_MATCH_READER (gibbon_java_fibs_reader_new (NULL, NULL));
        g_return_val_if_fail (reader != NULL, -1);

        input_file = g_build_filename (ABS_BUILDDIR, "complete.match", NULL);
        match = gibbon_match_reader_parse (reader, input_file);
        g_free (input_file);

        g_return_val_if_fail (match != NULL, -1);

        g_object_unref (match);
        g_object_unref (reader);

        return status;
}
