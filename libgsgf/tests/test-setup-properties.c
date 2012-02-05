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

#include <glib/gi18n.h>
#include <stdio.h>

#include "test.h"

char *filename = "setup-properties.sgf";

static gboolean test_prop_AB(const GSGFNode *node);
static gboolean test_prop_AW(const GSGFNode *node);
static gboolean test_prop_AE(const GSGFNode *node);
static gboolean test_prop_PL(const GSGFNode *node, GSGFColorEnum expect,
                             const gchar *name);

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
        if (!test_prop_AE(node))
                return -1;

        if (!test_prop_PL(node, GSGF_COLOR_BLACK, "black"))
                return -1;

        item = g_list_nth_data(nodes, 2);
        if (!item) {
                fprintf(stderr, "Node #2 not found.\n");
                return -1;
        }
        node = GSGF_NODE(item);
        if (!test_prop_PL(node, GSGF_COLOR_BLACK, "black"))
                return -1;

        item = g_list_nth_data(nodes, 3);
        if (!item) {
                fprintf(stderr, "Node #3 not found.\n");
                return -1;
        }
        node = GSGF_NODE(item);
        if (!test_prop_PL(node, GSGF_COLOR_BLACK, "black"))
                return -1;

        item = g_list_nth_data(nodes, 4);
        if (!item) {
                fprintf(stderr, "Node #4 not found.\n");
                return -1;
        }
        node = GSGF_NODE(item);
        if (!test_prop_PL(node, GSGF_COLOR_WHITE, "white"))
                return -1;

        item = g_list_nth_data(nodes,5);
        if (!item) {
                fprintf(stderr, "Node #5 not found.\n");
                return -1;
        }
        node = GSGF_NODE(item);
        if (!test_prop_PL(node, GSGF_COLOR_WHITE, "white"))
                return -1;

        item = g_list_nth_data(nodes,6);
        if (!item) {
                fprintf(stderr, "Node #6 not found.\n");
                return -1;
        }
        node = GSGF_NODE(item);
        if (!test_prop_PL(node, GSGF_COLOR_WHITE, "white"))
                return -1;

        return expect_error(error, NULL);
}

static gboolean
test_prop_AB(const GSGFNode *node)
{
        const GSGFValue *value = gsgf_node_get_property_value(node, "AB");
        const GSGFListOf *list_of;
        GType type;
        GSGFCookedValue *cooked_stone;
        gsize num_stones;
        guint stone;

        if (!value) {
                fprintf(stderr, "No property 'AB'!\n");
                return FALSE;
        }

        if (!GSGF_IS_LIST_OF(value)) {
                fprintf(stderr, "Property 'AB' is not a GSGFListOf!\n");
                return FALSE;
        }

        list_of = GSGF_LIST_OF(value);
        type = gsgf_list_of_get_item_type(list_of);
        if (type != gsgf_stone_backgammon_get_type ()) {
                fprintf(stderr, "Expected GSGFStoneBackgammon, not %s!\n",
                        g_type_name(type));
                return FALSE;
        }

        num_stones = gsgf_list_of_get_number_of_items(list_of);
        if (num_stones != 2) {
                fprintf(stderr, "Expected two stones, got %llu!\n",
                        (unsigned long long) num_stones);
                return FALSE;
        }

        cooked_stone = gsgf_list_of_get_nth_item(list_of, 0);
        if (!GSGF_IS_STONE_BACKGAMMON(cooked_stone)) {
                fprintf(stderr, "Item #0 is not a GSGFStoneBackgammon!\n");
                return FALSE;
        }
        stone = gsgf_stone_backgammon_get_stone(GSGF_STONE_BACKGAMMON(cooked_stone));
        if (stone != 16) {
                fprintf(stderr, "Item #0 is not a 16 stone but a %d stone!\n",
                                stone);
                return FALSE;
        }

        cooked_stone = gsgf_list_of_get_nth_item(list_of, 1);
        if (!GSGF_IS_STONE_BACKGAMMON(cooked_stone)) {
                fprintf(stderr, "Item #1 is not a GSGFStoneBackgammon!\n");
                return FALSE;
        }
        stone = gsgf_stone_backgammon_get_stone(GSGF_STONE_BACKGAMMON(cooked_stone));
        if (stone != 11) {
                fprintf(stderr, "Item #1 is not a 16 stone but a %d stone!\n",
                                stone);
                return FALSE;
        }

        return TRUE;
}

static gboolean
test_prop_AW(const GSGFNode *node)
{
        const GSGFValue *value = gsgf_node_get_property_value(node, "AW");
        const GSGFListOf *list_of;
        GType type;
        GSGFCookedValue *cooked_stone;
        gsize num_stones;
        guint stone;

        if (!value) {
                fprintf(stderr, "No property 'AW'!\n");
                return FALSE;
        }

        if (!GSGF_IS_LIST_OF(value)) {
                fprintf(stderr, "Property 'AW' is not a GSGFListOf!\n");
                return FALSE;
        }

        list_of = GSGF_LIST_OF(value);
        type = gsgf_list_of_get_item_type(list_of);
        if (type != gsgf_stone_backgammon_get_type ()) {
                fprintf(stderr, "Expected GSGFStoneBackgammon, not %s!\n",
                        g_type_name(type));
                return FALSE;
        }

        num_stones = gsgf_list_of_get_number_of_items(list_of);
        if (num_stones != 3) {
                fprintf(stderr, "Expected three stones, got %llu!\n",
                        (unsigned long long) num_stones);
                return FALSE;
        }

        cooked_stone = gsgf_list_of_get_nth_item(list_of, 0);
        if (!GSGF_IS_STONE_BACKGAMMON(cooked_stone)) {
                fprintf(stderr, "Item #0 is not a GSGFStoneBackgammon!\n");
                return FALSE;
        }
        stone = gsgf_stone_backgammon_get_stone(GSGF_STONE_BACKGAMMON(cooked_stone));
        if (stone != 7) {
                fprintf(stderr, "Item #0 is not a 7 stone but a %d stone!\n",
                                stone);
                return FALSE;
        }

        cooked_stone = gsgf_list_of_get_nth_item(list_of, 1);
        if (!GSGF_IS_STONE_BACKGAMMON(cooked_stone)) {
                fprintf(stderr, "Item #1 is not a GSGFStoneBackgammon!\n");
                return FALSE;
        }
        stone = gsgf_stone_backgammon_get_stone(GSGF_STONE_BACKGAMMON(cooked_stone));
        if (stone != 7) {
                fprintf(stderr, "Item #1 is not a 7 stone but a %d stone!\n",
                                stone);
                return FALSE;
        }

        cooked_stone = gsgf_list_of_get_nth_item(list_of, 2);
        if (!GSGF_IS_STONE_BACKGAMMON(cooked_stone)) {
                fprintf(stderr, "Item #2 is not a GSGFStoneBackgammon!\n");
                return FALSE;
        }
        stone = gsgf_stone_backgammon_get_stone(GSGF_STONE_BACKGAMMON(cooked_stone));
        if (stone != 12) {
                fprintf(stderr, "Item #2 is not a 12 stone but a %d stone!\n",
                                stone);
                return FALSE;
        }

        return TRUE;
}

static gboolean
test_prop_AE(const GSGFNode *node)
{
        const GSGFValue *value = gsgf_node_get_property_value(node, "AE");
        const GSGFListOf *list_of;
        GType type;
        GSGFCookedValue *cooked_point;
        gsize num_points;
        guint point;
        gint values[] = { 0, 1, 2, 3, 4, 5, 6, 8, 9, 10, 11, 13, 14,
                          15, 17, 18, 19, 20, 21, 22, 23, 24, 25 };
        gsize expect_num_points, i;

        if (!value) {
                fprintf(stderr, "No property 'AE'!\n");
                return FALSE;
        }

        if (!GSGF_IS_LIST_OF(value)) {
                fprintf(stderr, "Property 'AE' is not a GSGFListOf!\n");
                return FALSE;
        }

        list_of = GSGF_LIST_OF(value);
        type = gsgf_list_of_get_item_type(list_of);
        if (type != gsgf_point_backgammon_get_type ()) {
                fprintf(stderr, "Expected GSGFPointBackgammon, not %s!\n",
                        g_type_name(type));
                return FALSE;
        }

        num_points = gsgf_list_of_get_number_of_items(list_of);
        expect_num_points = (sizeof values) / (sizeof *values);
        if (num_points != expect_num_points) {
                fprintf(stderr, "Expected %llu points, got %llu!\n",
                        (unsigned long long) expect_num_points,
                        (unsigned long long) num_points);
                return FALSE;
        }

        for (i = 0; i < expect_num_points; ++i) {
                cooked_point = gsgf_list_of_get_nth_item(list_of, i);
                if (!GSGF_IS_POINT_BACKGAMMON (cooked_point)) {
                        g_printerr ("Item #%llu is not a"
                                    " GSGFSPointBackgammon!\n",
                                    (unsigned long long) i);
                        return FALSE;
                }
                point = gsgf_point_backgammon_get_point
                                (GSGF_POINT_BACKGAMMON (cooked_point));
                if (point != values[i]) {
                        g_printerr ( "Item #%llu is not a %d"
                                     " point but a %d point!\n",
                                     (unsigned long long) i, values[i], point);
                        return FALSE;
                }
        }

        return TRUE;
}

static gboolean
test_prop_PL (const GSGFNode *node, GSGFColorEnum expect, const gchar *name)
{
        const GSGFValue *value =
                        gsgf_node_get_property_value(node, "PL");
        GSGFColor *color;

        if (!value) {
                fprintf (stderr, "No property 'PL'!\n");
                return FALSE;
        }

        if (!GSGF_IS_COLOR (value)) {
                fprintf (stderr, "Property 'PL' is not a GSGFColor!\n");
                return FALSE;
        }

        color = GSGF_COLOR (value);
        if (expect != gsgf_color_get_color (color)) {
                fprintf (stderr, "Property 'PL' is not %s!\n", name);
                return FALSE;
        }

        return TRUE;
}
