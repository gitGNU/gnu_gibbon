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

char *filename = "markup-properties.sgf";

static gboolean test_prop_AR (const GSGFNode *node);
static gboolean test_prop_CR (const GSGFNode *node);

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
        if (!test_prop_AR (node))
                retval = -1;
        if (!test_prop_CR (node))
                retval = -1;

        return retval;
}

static gboolean
test_prop_AR (const GSGFNode *node)
{
        const GSGFCookedValue *cooked_value =
                        gsgf_node_get_property_cooked (node, "AR");
        GSGFListOf *list_of;
        GType type;
        gsize num_arrows;
        GError *expect;

        if (!cooked_value) {
                g_printerr ("No property 'AR'!\n");
                return FALSE;
        }

        if (!GSGF_IS_LIST_OF (cooked_value)) {
                g_printerr ("Property 'AR' is not a GSGFListOf!\n");
                return FALSE;
        }
        list_of = GSGF_LIST_OF (cooked_value);

        type = gsgf_list_of_get_item_type (list_of);
        if (type != GSGF_TYPE_COMPOSE) {
                g_printerr ("Expected item type 'GSGFCompose', not '%s'.\n",
                            g_type_name (type));
                return FALSE;
        }

        num_arrows = gsgf_list_of_get_number_of_items (list_of);
        if (num_arrows != 3) {
                g_printerr ("Expected 3 arrows, got %u.\n", num_arrows);
                return FALSE;
        }

        expect = NULL;
        g_set_error (&expect, GSGF_ERROR, GSGF_ERROR_SEMANTIC_ERROR,
                     "Property 'AR': Start and end point must differ");
        if (!expect_error_from_sgf ("(;GM[6];AR[a:b][c:c][c:d])", expect))
                return FALSE;

        expect = NULL;
        g_set_error (&expect, GSGF_ERROR, GSGF_ERROR_SEMANTIC_ERROR,
                     "Property 'AR': Arrows must be unique");
        if (!expect_error_from_sgf ("(;GM[6];AR[a:b][c:d][a:b])", expect))
                return FALSE;

        return TRUE;
}

static gboolean
test_prop_CR (const GSGFNode *node)
{
        const GSGFCookedValue *cooked_value =
                        gsgf_node_get_property_cooked (node, "CR");
        const GSGFListOf *list_of;
        GType type;
        GSGFCookedValue *cooked_point;
        gsize num_points;
        guint point;
        gint values[] = { 5, 6, 7, 9, 11, 12 };
        gsize expect_num_points, i;

        if (!cooked_value) {
                fprintf(stderr, "No property 'CR'!\n");
                return FALSE;
        }

        if (!GSGF_IS_LIST_OF(cooked_value)) {
                fprintf(stderr, "Property 'CR' is not a GSGFListOf!\n");
                return FALSE;
        }

        list_of = GSGF_LIST_OF(cooked_value);
        type = gsgf_list_of_get_item_type(list_of);
        if (type != gsgf_point_backgammon_get_type ()) {
                fprintf(stderr, "Expected GSGFPointBackgammon, not %s!\n",
                        g_type_name(type));
                return FALSE;
        }

        num_points = gsgf_list_of_get_number_of_items(list_of);
        expect_num_points = (sizeof values) / (sizeof *values);
        if (num_points != expect_num_points) {
                fprintf(stderr, "Expected %u points, got %u!\n",
                                expect_num_points, num_points);
                return FALSE;
        }

        for (i = 0; i < expect_num_points; ++i) {
                cooked_point = gsgf_list_of_get_nth_item(list_of, i);
                if (!GSGF_IS_POINT_BACKGAMMON (cooked_point)) {
                        g_printerr ("Item #%u is not a GSGFSPointBackgammon!\n",
                                    i);
                        return FALSE;
                }
                point = gsgf_point_backgammon_get_point
                                (GSGF_POINT_BACKGAMMON (cooked_point));
                if (point != values[i]) {
                        g_printerr ( "Item #%u is not a %d"
                                     " point but a %d point!\n",
                                     i, values[i], point);
                        return FALSE;
                }
        }

        return TRUE;
}
