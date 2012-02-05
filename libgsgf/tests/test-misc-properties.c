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

char *filename = "misc-properties.sgf";

static gboolean test_prop_FG (const GSGFNode *node);
static gboolean test_prop_empty_FG (const GSGFNode *node);
static gboolean test_prop_PM (const GSGFNode *node);
static gboolean test_prop_VW (const GSGFNode *node);
static gboolean test_prop_empty_VW (const GSGFNode *node);

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

        item = g_list_nth_data (nodes, 2);
        if (!item) {
                g_printerr ("Property #2 not found.\n");
                return -1;
        }
        node = GSGF_NODE (item);
        if (!test_prop_empty_FG (node))
                retval = -1;

        item = g_list_nth_data (nodes, 3);
        if (!item) {
                g_printerr ("Property #3 not found.\n");
                return -1;
        }
        node = GSGF_NODE (item);
        if (!test_prop_PM (node))
                retval = -1;

        item = g_list_nth_data (nodes, 4);
        if (!item) {
                g_printerr ("Property #4 not found.\n");
                return -1;
        }
        node = GSGF_NODE (item);
        if (!test_prop_VW (node))
                retval = -1;

        item = g_list_nth_data (nodes, 5);
        if (!item) {
                g_printerr ("Property #5 not found.\n");
                return -1;
        }
        node = GSGF_NODE (item);
        if (!test_prop_empty_VW (node))
                retval = -1;

        return retval;
}

static gboolean
test_prop_FG (const GSGFNode *node)
{
        const GSGFValue *value =
                        gsgf_node_get_property_value (node, "FG");
        gint64 number_value;
        gchar *simple_text_value;
        gsize length;
        const GSGFCookedValue *subvalue;

        if (!value) {
                g_printerr ("No property 'FG'!\n");
                return FALSE;
        }

        if (!GSGF_IS_COMPOSE (value)) {
                g_printerr ("Property 'FG' is not a GSGFCompose ");
                g_printerr ("but a '%s'.\n", G_OBJECT_TYPE_NAME (value));
                return FALSE;
        }

        length = gsgf_compose_get_number_of_values(GSGF_COMPOSE(value));
        if (length != 2) {
                g_printerr ("Property 'FG': Expected 2 subvalues, got %llu.\n",
                            (unsigned long long) length);
                return FALSE;
        }

        subvalue = gsgf_compose_get_value (GSGF_COMPOSE (value), 1);
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

        subvalue = gsgf_compose_get_value (GSGF_COMPOSE (value), 0);
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
                g_printerr ("Expected 7, not %lld!\n",
                            (long long) number_value);
                return FALSE;
        }

        return TRUE;
}

static gboolean
test_prop_empty_FG (const GSGFNode *node)
{
        const GSGFValue *value =
                        gsgf_node_get_property_value (node, "FG");

        if (!value) {
                g_printerr ("No property 'FG'!\n");
                return FALSE;
        }

        if (!GSGF_IS_EMPTY (value)) {
                g_printerr ("Property 'FG' is not a GSGFEmpty ");
                g_printerr ("but a '%s'.\n", G_OBJECT_TYPE_NAME (value));
                return FALSE;
        }

        return TRUE;
}

static gboolean
test_prop_PM (const GSGFNode *node)
{
        const GSGFValue *value =
                        gsgf_node_get_property_value (node, "PM");
        GSGFNumber *number;

        if (!value) {
                g_printerr ("No property 'PM'!\n");
                return FALSE;
        }

        if (!GSGF_IS_NUMBER (value)) {
                g_printerr ("Property 'PM' is not a GSGFNumber ");
                g_printerr ("but a '%s'.\n", G_OBJECT_TYPE_NAME (value));
                return FALSE;
        }

        number = GSGF_NUMBER (value);
        if (1 != gsgf_number_get_value (number)) {
                g_printerr ("Property 'PM': expected 1, got %lld\n",
                            (long long) gsgf_number_get_value (number));
                return FALSE;
        }

        return TRUE;
}

static gboolean
test_prop_VW (const GSGFNode *node)
{
        const GSGFValue *value =
                        gsgf_node_get_property_value (node, "VW");
        const GSGFListOf *list_of;
        GType type;
        GSGFCookedValue *cooked_point;
        gsize num_points;
        guint point;
        gint values[] = { 0, 1, 2, 3 };
        gsize expect_num_points, i;

        if (!value) {
                g_printerr ("No property 'VW'!\n");
                return FALSE;
        }

        if (!GSGF_IS_LIST_OF (value)) {
                g_printerr ("Property 'VW' is not a GSGFListOf ");
                g_printerr ("but a '%s'.\n", G_OBJECT_TYPE_NAME (value));
                return FALSE;
        }

        list_of = GSGF_LIST_OF (value);
        type = gsgf_list_of_get_item_type (list_of);
        if (type != gsgf_point_backgammon_get_type ()) {
                g_printerr ("Property 'VW': Expected GSGFPointBackgammon, not %s!\n",
                            g_type_name(type));
                return FALSE;
        }

        num_points = gsgf_list_of_get_number_of_items(list_of);
        expect_num_points = (sizeof values) / (sizeof *values);
        if (num_points != expect_num_points) {
                g_printerr ("Property 'VW': Expected %llu points, got %llu!\n",
                            (unsigned long long) expect_num_points,
                            (unsigned long long) num_points);
                return FALSE;
        }

        for (i = 0; i < expect_num_points; ++i) {
                cooked_point = gsgf_list_of_get_nth_item(list_of, i);
                if (!GSGF_IS_POINT_BACKGAMMON (cooked_point)) {
                        g_printerr ("Property 'VW': Item #%llu is not a"
                                    " GSGFSPointBackgammon!\n",
                                    (unsigned long long) i);
                        return FALSE;
                }
                point = gsgf_point_backgammon_get_point
                                (GSGF_POINT_BACKGAMMON (cooked_point));
                if (point != values[i]) {
                        g_printerr ("Property 'VW': Item #%llu is not a %d"
                                    " point but a %d point!\n",
                                    (unsigned long long) i, values[i], point);
                        return FALSE;
                }
        }

        return TRUE;
}

static gboolean
test_prop_empty_VW (const GSGFNode *node)
{
        const GSGFValue *value =
                        gsgf_node_get_property_value (node, "VW");
        const GSGFListOf *list_of;
        GType type;
        gsize num_points;

        if (!value) {
                g_printerr ("No empty property 'VW'!\n");
                return FALSE;
        }

        if (!GSGF_IS_LIST_OF (value)) {
                g_printerr ("Empty property 'VW' is not a GSGFListOf ");
                g_printerr ("but a '%s'.\n", G_OBJECT_TYPE_NAME (value));
                return FALSE;
        }

        list_of = GSGF_LIST_OF (value);
        type = gsgf_list_of_get_item_type (list_of);
        if (type != gsgf_empty_get_type ()) {
                g_printerr ("Empty property 'VW': Expected GSGFEmpty, not %s!\n",
                            g_type_name(type));
                return FALSE;
        }

        num_points = gsgf_list_of_get_number_of_items(list_of);
        if (num_points != 1) {
                g_printerr ("Property 'VW': Expected one point, got %llu!\n",
                            (unsigned long long) num_points);
                return FALSE;
        }

        return TRUE;
}
