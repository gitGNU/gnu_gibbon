/*
 * This file is part of Gibbon, a graphical frontend to the First Internet 
 * Backgammon Server FIBS.
 * Copyright (C) 2009-2010 Guido Flohr, http://guido-flohr.net/.
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
#include <stdio.h>

#include "test.h"

char *filename = "root-properties.sgf";

int 
test_collection(GSGFCollection *collection, GError *error)
{
        GSGFGameTree *game_tree;
        GSGFNode *root_node;

        game_tree = GSGF_GAME_TREE(
                g_list_first(gsgf_collection_get_game_trees(collection)));
        if (!game_tree) {
                fprintf(stderr, "No game trees found.\n");
                return -1;
        }

        root_node = GSGF_NODE(
                g_list_first(gsgf_game_tree_get_nodes(game_tree)));

        return expect_error(error, NULL);
}
