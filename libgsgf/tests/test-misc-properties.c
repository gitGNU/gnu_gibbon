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
#include <stdio.h>

#include "test.h"

char *filename = "misc-properties.sgf";

static gboolean test_prop_FG (const GSGFNode *node);

int 
test_collection (GSGFCollection *collection, GError *error)
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

        item = g_list_nth_data (nodes, 1);
        if (!item) {
                g_printerr ("Property #1 not found.\n");
                return -1;
        }
        node = GSGF_NODE (item);

        if (!test_prop_FG (node))
                retval = -1;

        return retval;
}

static gboolean
test_prop_FG (const GSGFNode *node)
{
        const GSGFCookedValue *cooked_value =
                        gsgf_node_get_property_cooked (node, "FG");
        gint64 number_value;
        gchar *simple_text_value;
        gsize length;
        const GSGFCookedValue *subvalue;

        if (!cooked_value) {
                g_printerr ("No root property 'FG'!\n");
                return FALSE;
        }

        if (!GSGF_IS_COMPOSE (cooked_value)) {
                g_printerr ("Property 'FG' is not a GSGFCompose ");
                g_printerr ("but a '%s'.\n", G_OBJECT_TYPE_NAME (cooked_value));
                return FALSE;
        }

        length = gsgf_compose_get_number_of_values(GSGF_COMPOSE(cooked_value));
        if (length != 2) {
                g_printerr ("Property 'FG': Expected 2 subvalues, got %u.\n",
                            length);
                return FALSE;
        }

        subvalue = gsgf_compose_get_value (GSGF_COMPOSE (cooked_value), 1);
        if (!subvalue) {
                g_printerr ("No first 'FG' subvalue!\n");
                return FALSE;
        }
        if (!GSGF_IS_SIMPLE_TEXT (subvalue)) {
                g_printerr ("Second 'FG' subvalue is not a GSGFSimpleText!\n");
                return FALSE;
        }
        simple_text_value = gsgf_text_get_value (GSGF_TEXT (subvalue));
        if (g_strcmp0 ("diagram 1", simple_text_value)) {
                g_printerr ("Expected 'diagram 1', not %s!\n",
                            simple_text_value);
                return FALSE;
        }

        subvalue = gsgf_compose_get_value (GSGF_COMPOSE (cooked_value), 0);
        if (!subvalue) {
                g_printerr ("No first 'FG' subvalue!\n");
                return FALSE;
        }

        if (!GSGF_IS_NUMBER (subvalue)) {
                g_printerr ("First 'FG' subvalue is not a GSGFNumber!\n");
                return FALSE;
        }
        number_value = gsgf_number_get_value (GSGF_NUMBER (subvalue));
        if (7 != number_value) {
                g_printerr ("Expected 7, not %lld!\n", number_value);
                return FALSE;
        }

        return TRUE;
}
