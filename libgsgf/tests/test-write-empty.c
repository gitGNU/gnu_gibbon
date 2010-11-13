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

#include <glib/gi18n.h>

#include "test.h"

char *filename = NULL;

int 
test_collection(GSGFCollection *collection, GError *error)
{
        GSGFCollection *empty = gsgf_collection_new();
        GError *expect = NULL;
        GOutputStream *out = g_memory_output_stream_new(NULL, 0, NULL, NULL);
        gssize written = gsgf_collection_write_stream(empty, out, NULL, &error);

        g_set_error(&expect, GSGF_ERROR, GSGF_ERROR_EMPTY_COLLECTION,
                    _("Attempt to write an empty collection"));

        return expect_error_conditional(written == -1,
                                        "Expected written size to be -1",
                                        error, expect);
}
