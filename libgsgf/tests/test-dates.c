/*
 * This file is part of Gibbon, a graphical frontend to the First Internet 
 * Backgammon Server FIBS.
 * Copyright (C) 2009-2011 Guido Flohr, http://guido-flohr.net/.
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

char *filename = "dates.sgf";

static gboolean test_single_ymd (const GSGFNode *node);

int 
test_collection(GSGFCollection *collection, GError *error)
{
        GList *game_trees;
        GSGFGameTree *game_tree;
        GList *nodes;
        gpointer item;
        GSGFNode *node;
        gint retval = 0;

        if (error) {
                g_printerr ("%s: %s\n", filename, error->message);
                return -1;
        }

        game_trees = gsgf_collection_get_game_trees (collection);
        if (!game_trees) {
                g_printerr ("No game trees found.\n");
                return -1;
        }
        game_tree = GSGF_GAME_TREE (game_trees->data);

        nodes = gsgf_game_tree_get_nodes (game_tree);
        if (!nodes) {
                g_printerr ("No nodes found.\n");
                return -1;
        }

        item = g_list_nth_data (nodes, 0);
        if (!item) {
                g_printerr ("Property #0 not found.\n");
                return -1;
        }
        node = GSGF_NODE (item);
        if (!test_single_ymd (node))
                retval = -1;

        return retval;
}

static gboolean
test_single_ymd (const GSGFNode *node)
{
        const GSGFCookedValue *cooked_value =
                        gsgf_node_get_property_cooked (node, "DT");
        const gchar *expect = "2011-02-16";
        const gchar *got;

        if (!cooked_value) {
                g_printerr ("No property 'DT'!\n");
                return FALSE;
        }

        if (!GSGF_IS_SIMPLE_TEXT (cooked_value)) {
                g_printerr ("Property 'DT' is not a GSGFSimpleText!\n");
                return FALSE;
        }

        got = gsgf_text_get_value (GSGF_TEXT (cooked_value));
        if (g_strcmp0 (expect, got)) {
                g_printerr ("Property 'DT': Expected '%s', got '%s'!\n",
                             expect, got);
                return FALSE;
        }

        return TRUE;
}
