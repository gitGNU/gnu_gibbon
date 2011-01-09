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

char *filename = "setup-properties.sgf";

static gboolean test_prop_AB(const GSGFNode *node);
static gboolean test_prop_AW(const GSGFNode *node);

int 
test_collection(GSGFCollection *collection, GError *error)
{
        GList *game_trees;
        GSGFGameTree *game_tree;
        GList *nodes;
        GList *item;
        GSGFNode *node;

        if (error) {
                fprintf(stderr, "%s: %s\n", filename, error->message);
                return -1;
        }

        game_trees = gsgf_collection_get_game_trees(collection);
        if (!game_trees) {
                fprintf(stderr, "No game trees found.\n");
                return -1;
        }
        game_tree = GSGF_GAME_TREE(game_trees->data);

        nodes = gsgf_game_tree_get_nodes(game_tree);
        if (!nodes) {
                fprintf(stderr, "No nodes found.\n");
                return -1;
        }

        item = g_list_nth_data(nodes, 1);
        if (!item) {
                fprintf(stderr, "Node #1 not found.\n");
                return -1;
        }
        node = GSGF_NODE(item);
        if (!test_prop_AB(node))
                return -1;
        if (!test_prop_AW(node))
                return -1;

        return expect_error(error, NULL);
}

static gboolean
test_prop_AB(const GSGFNode *node)
{
        const GSGFCookedValue *cooked_value = gsgf_node_get_property_cooked(node, "AB");
        const GSGFListOf *list_of;
        GType type;
        GSGFCookedValue *cooked_point;
        gsize num_points;
        guint point;

        if (!cooked_value) {
                fprintf(stderr, "No property 'AB'!\n");
                return FALSE;
        }

        if (!GSGF_IS_LIST_OF(cooked_value)) {
                fprintf(stderr, "Property 'AB' is not a GSGFListOf!\n");
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
        if (num_points != 2) {
                fprintf(stderr, "Expected two points, got %u!\n", num_points);
                return FALSE;
        }

        cooked_point = gsgf_list_of_get_nth_item(list_of, 0);
        if (!GSGF_IS_POINT_BACKGAMMON(cooked_point)) {
                fprintf(stderr, "Item #0 is not a GSGFPointBackgammon!\n");
                return FALSE;
        }
        point = gsgf_point_backgammon_get_point(GSGF_POINT_BACKGAMMON(cooked_point));
        if (point != 16) {
                fprintf(stderr, "Item #0 is not a 16 point but a %u point!\n",
                                point);
                return FALSE;
        }

        cooked_point = gsgf_list_of_get_nth_item(list_of, 1);
        if (!GSGF_IS_POINT_BACKGAMMON(cooked_point)) {
                fprintf(stderr, "Item #0 is not a GSGFPointBackgammon!\n");
                return FALSE;
        }
        point = gsgf_point_backgammon_get_point(GSGF_POINT_BACKGAMMON(cooked_point));
        if (point != 16) {
                fprintf(stderr, "Item #0 is not a 16 point but a %u point!\n",
                                point);
                return FALSE;
        }

        return TRUE;
}

static gboolean
test_prop_AW(const GSGFNode *node)
{
        const GSGFCookedValue *cooked_value = gsgf_node_get_property_cooked(node, "AW");
        const GSGFListOf *list_of;
        GType type;
        GSGFCookedValue *cooked_point;
        gsize num_points;
        guint point;

        if (!cooked_value) {
                fprintf(stderr, "No property 'AW'!\n");
                return FALSE;
        }

        if (!GSGF_IS_LIST_OF(cooked_value)) {
                fprintf(stderr, "Property 'AW' is not a GSGFListOf!\n");
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
        if (num_points != 3) {
                fprintf(stderr, "Expected three points, got %u!\n", num_points);
                return FALSE;
        }

        cooked_point = gsgf_list_of_get_nth_item(list_of, 0);
        if (!GSGF_IS_POINT_BACKGAMMON(cooked_point)) {
                fprintf(stderr, "Item #0 is not a GSGFPointBackgammon!\n");
                return FALSE;
        }
        point = gsgf_point_backgammon_get_point(GSGF_POINT_BACKGAMMON(cooked_point));
        if (point != 6) {
                fprintf(stderr, "Item #0 is not a 6 point but a %u point!\n",
                                point);
                return FALSE;
        }

        cooked_point = gsgf_list_of_get_nth_item(list_of, 1);
        if (!GSGF_IS_POINT_BACKGAMMON(cooked_point)) {
                fprintf(stderr, "Item #0 is not a GSGFPointBackgammon!\n");
                return FALSE;
        }
        point = gsgf_point_backgammon_get_point(GSGF_POINT_BACKGAMMON(cooked_point));
        if (point != 6) {
                fprintf(stderr, "Item #0 is not a 6 point but a %u point!\n",
                                point);
                return FALSE;
        }

        cooked_point = gsgf_list_of_get_nth_item(list_of, 2);
        if (!GSGF_IS_POINT_BACKGAMMON(cooked_point)) {
                fprintf(stderr, "Item #0 is not a GSGFPointBackgammon!\n");
                return FALSE;
        }
        point = gsgf_point_backgammon_get_point(GSGF_POINT_BACKGAMMON(cooked_point));
        if (point != 11) {
                fprintf(stderr, "Item #0 is not a 11 point but a %u point!\n",
                                point);
                return FALSE;
        }

        return TRUE;
}
