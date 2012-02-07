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

char *filename = NULL;

int 
test_collection(GSGFCollection *collection, GError *error)
{
        GError *expect = NULL;
        GSGFGameTree* game_tree;
        GSGFNode* node;
        GSGFProperty* property;
        GOutputStream *out = g_memory_output_stream_new(NULL, 0,
                                                        g_realloc, g_free);
        gsize written = 12345;
        gchar *version_string = "AP[libgsgf:" VERSION "]";
        GSGFCollection *empty = gsgf_collection_new(&error);

        game_tree = gsgf_collection_add_game_tree (empty, NULL);
        node = gsgf_game_tree_add_node(game_tree);
        property = gsgf_node_add_property(node, "EMPTY", &error);

        if (error)
                return expect_error(error, NULL);

        if (gsgf_component_write_stream (GSGF_COMPONENT (empty), out, &written,
                                         NULL, &error)) {
                fprintf(stderr, "Writing empty properties did not fail.\n");
                return -1;
        }

        g_set_error(&expect, GSGF_ERROR, GSGF_ERROR_EMPTY_PROPERTY,
                    ("Attempt to write empty property"));

        return expect_error_conditional(strlen(version_string),
                                        "Expected written size to be"
                                        " length of version string",
                                        error, expect);
}
