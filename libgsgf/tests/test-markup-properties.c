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

char *filename = "markup-properties.sgf";

static gboolean test_prop_AR (const GSGFNode *node);
static gboolean test_constraints_AR (void);
static gboolean test_prop_CR (const GSGFNode *node);
static gboolean test_unique_points_CR (void);
static gboolean test_prop_MA (const GSGFNode *node);
static gboolean test_prop_DD (const GSGFNode *node);
static gboolean test_prop_DD_empty (const GSGFNode *node);
static gboolean test_prop_LB (const GSGFNode *node);
static gboolean test_unique_points_LB (void);
static gboolean test_prop_LN (const GSGFNode *node);
static gboolean test_constraints_LN (void);
static gboolean test_unique_points_MA (void);
static gboolean test_prop_SL (const GSGFNode *node);
static gboolean test_unique_points_SL (void);
static gboolean test_prop_SQ (const GSGFNode *node);
static gboolean test_unique_points_SQ (void);
static gboolean test_prop_TR (const GSGFNode *node);
static gboolean test_unique_points_TR (void);

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
        if (!test_constraints_AR ())
                retval = -1;

        if (!test_prop_CR (node))
                retval = -1;
        if (!test_unique_points_CR ())
                retval = -1;

        if (!test_prop_MA (node))
                retval = -1;
        if (!test_unique_points_MA ())
                retval = -1;

        if (!test_prop_SL (node))
                retval = -1;
        if (!test_unique_points_SL ())
                retval = -1;

        if (!test_prop_SQ (node))
                retval = -1;
        if (!test_unique_points_SQ ())
                retval = -1;

        if (!test_prop_TR (node))
                retval = -1;
        if (!test_unique_points_TR ())
                retval = -1;

        item = g_list_nth_data (nodes, 2);
        if (!item) {
                g_printerr ("Property #2 not found.\n");
                return -1;
        }
        node = GSGF_NODE (item);
        if (!test_prop_DD (node))
                retval = -1;

        item = g_list_nth_data (nodes, 3);
        if (!item) {
                g_printerr ("Property #3 not found.\n");
                return -1;
        }
        node = GSGF_NODE (item);
        if (!test_prop_DD_empty (node))
                retval = -1;

        item = g_list_nth_data (nodes, 4);
        if (!item) {
                g_printerr ("Property #4 not found.\n");
                return -1;
        }
        node = GSGF_NODE (item);
        if (!test_prop_LB (node))
                retval = -1;
        if (!test_unique_points_LB ())
                retval = -1;

        item = g_list_nth_data (nodes, 5);
        if (!item) {
                g_printerr ("Property #5 not found.\n");
                return -1;
        }
        node = GSGF_NODE (item);
        if (!test_prop_LN (node))
                retval = -1;
        if (!test_constraints_LN ())
                retval = -1;

        return retval;
}

static gboolean
test_prop_AR (const GSGFNode *node)
{
        const GSGFValue *value =
                        gsgf_node_get_property_value (node, "AR");
        GSGFListOf *list_of;
        GType type;
        gsize num_lines;
        gint expect[3][2] = {{ 0, 1}, { 2, 3 }, { 3, 4 }};
        gint got;
        GSGFCompose *compose;
        GSGFCookedValue *item;
        gsize i;

        if (!value) {
                g_printerr ("No property 'AR'!\n");
                return FALSE;
        }

        if (!GSGF_IS_LIST_OF (value)) {
                g_printerr ("Property 'AR' is not a GSGFListOf!\n");
                return FALSE;
        }
        list_of = GSGF_LIST_OF (value);

        type = gsgf_list_of_get_item_type (list_of);
        if (type != GSGF_TYPE_COMPOSE) {
                g_printerr ("Expected item type 'GSGFCompose', not '%s'.\n",
                            g_type_name (type));
                return FALSE;
        }

        num_lines = gsgf_list_of_get_number_of_items (list_of);
        if (num_lines != 3) {
                g_printerr ("Expected 3 lines, got %llu.\n",
                            (unsigned long long) num_lines);
                return FALSE;
        }

        for (i = 0; i < 3; ++i) {
                compose = GSGF_COMPOSE (gsgf_list_of_get_nth_item (list_of, i));

                item = gsgf_compose_get_value (compose, 0);
                if (!GSGF_IS_POINT_BACKGAMMON (item)) {
                        g_printerr ("Expected GSGFPointBackgammon for"
                                    " item #0 of pair #%lld, not '%s':\n",
                                    (long long) i, G_OBJECT_TYPE_NAME (item));
                        return FALSE;
                }
                got = gsgf_point_get_normalized_value (GSGF_POINT (item));

                if (expect[i][0] != got) {
                        g_printerr ("Expected %d not %d for"
                                    " item #0 of pair #%lld.\n",
                                    expect[i][0], got, (unsigned long long) i);
                        return FALSE;
                }

                item = gsgf_compose_get_value (compose, 1);
                if (!GSGF_IS_POINT_BACKGAMMON (item)) {
                        g_printerr ("Expected GSGFPointBackgammon for"
                                    " item #1 of pair #%lld, not '%s':\n",
                                    (long long) i, G_OBJECT_TYPE_NAME (item));
                        return FALSE;
                }
                got = gsgf_point_get_normalized_value (GSGF_POINT (item));

                if (expect[i][1] != got) {
                        g_printerr ("Expected %d not %d for"
                                    " item #1 of pair #%lld.\n",
                                    expect[i][1], got, (long long) i);
                        return FALSE;
                }
        }

        return TRUE;
}

static gboolean
test_constraints_AR (void)
{
        GError *expect;

        expect = NULL;
        g_set_error (&expect, GSGF_ERROR, GSGF_ERROR_SEMANTIC_ERROR,
                     "Property 'AR': Start and end point must differ");
        if (!expect_error_from_sgf ("(;GM[6];AR[a:b][c:c][c:d])", expect))
                return FALSE;

        expect = NULL;
        g_set_error (&expect, GSGF_ERROR, GSGF_ERROR_SEMANTIC_ERROR,
                     "Property 'AR': Lines and arrows must be unique");
        if (!expect_error_from_sgf ("(;GM[6];AR[a:b][c:d][a:b])", expect))
                return FALSE;

        return TRUE;
}

static gboolean
test_prop_CR (const GSGFNode *node)
{
        const GSGFValue *value =
                        gsgf_node_get_property_value (node, "CR");
        const GSGFListOf *list_of;
        GType type;
        GSGFCookedValue *cooked_point;
        gsize num_points;
        guint point;
        gint values[] = { 0, 1, 2, 3 };
        gsize expect_num_points, i;

        if (!value) {
                g_printerr ("No property 'CR'!\n");
                return FALSE;
        }

        if (!GSGF_IS_LIST_OF(value)) {
                g_printerr ("Property 'CR' is not a GSGFListOf!\n");
                return FALSE;
        }

        list_of = GSGF_LIST_OF(value);
        type = gsgf_list_of_get_item_type(list_of);
        if (type != gsgf_point_backgammon_get_type ()) {
                g_printerr ("Property 'CR': Expected GSGFPointBackgammon, not %s!\n",
                            g_type_name(type));
                return FALSE;
        }

        num_points = gsgf_list_of_get_number_of_items(list_of);
        expect_num_points = (sizeof values) / (sizeof *values);
        if (num_points != expect_num_points) {
                g_printerr ("Property 'CR': Expected %llu points, got %llu!\n",
                            (unsigned long long) expect_num_points,
                            (unsigned long long) num_points);
                return FALSE;
        }

        for (i = 0; i < expect_num_points; ++i) {
                cooked_point = gsgf_list_of_get_nth_item(list_of, i);
                if (!GSGF_IS_POINT_BACKGAMMON (cooked_point)) {
                        g_printerr ("Property 'CR': Item #%llu is not a"
                                    "GSGFSPointBackgammon!\n",
                                    (unsigned long long) i);
                        return FALSE;
                }
                point = gsgf_point_backgammon_get_point
                                (GSGF_POINT_BACKGAMMON (cooked_point));
                if (point != values[i]) {
                        g_printerr ("Property 'CR': Item #%llu is not a %d"
                                    " point but a %d point!\n",
                                    (unsigned long long) i, values[i], point);
                        return FALSE;
                }
        }

        return TRUE;
}

static gboolean
test_unique_points_CR (void)
{
        GError *expect1 = NULL;
        GError *expect2 = NULL;
        gboolean retval = TRUE;

        g_set_error (&expect1, GSGF_ERROR, GSGF_ERROR_SEMANTIC_ERROR,
                     "Property 'CR': The properties 'CR' and 'MA' are not"
                     " allowed on the same point within one node");
        g_set_error (&expect2, GSGF_ERROR, GSGF_ERROR_SEMANTIC_ERROR,
                     "Property 'MA': The properties 'MA' and 'CR' are not"
                     " allowed on the same point within one node");
        if (!expect_errors_from_sgf ("(;GM[6];CR[a:h]MA[h:m])",
                                     expect1, expect2))
                retval = FALSE;

        expect1 = NULL;
        expect2 = NULL;
        g_set_error (&expect1, GSGF_ERROR, GSGF_ERROR_SEMANTIC_ERROR,
                     "Property 'CR': The properties 'CR' and 'SL' are not"
                     " allowed on the same point within one node");
        g_set_error (&expect2, GSGF_ERROR, GSGF_ERROR_SEMANTIC_ERROR,
                     "Property 'SL': The properties 'SL' and 'CR' are not"
                     " allowed on the same point within one node");
        if (!expect_errors_from_sgf ("(;GM[6];CR[a:h]SL[h:m])",
                                     expect1, expect2))
                retval = FALSE;

        expect1 = NULL;
        expect2 = NULL;
        g_set_error (&expect1, GSGF_ERROR, GSGF_ERROR_SEMANTIC_ERROR,
                     "Property 'CR': The properties 'CR' and 'SQ' are not"
                     " allowed on the same point within one node");
        g_set_error (&expect2, GSGF_ERROR, GSGF_ERROR_SEMANTIC_ERROR,
                     "Property 'SQ': The properties 'SQ' and 'CR' are not"
                     " allowed on the same point within one node");
        if (!expect_errors_from_sgf ("(;GM[6];CR[a:h]SQ[h:m])",
                                     expect1, expect2))
                retval = FALSE;

        expect1 = NULL;
        expect2 = NULL;
        g_set_error (&expect1, GSGF_ERROR, GSGF_ERROR_SEMANTIC_ERROR,
                     "Property 'CR': The properties 'CR' and 'TR' are not"
                     " allowed on the same point within one node");
        g_set_error (&expect2, GSGF_ERROR, GSGF_ERROR_SEMANTIC_ERROR,
                     "Property 'TR': The properties 'TR' and 'CR' are not"
                     " allowed on the same point within one node");
        if (!expect_errors_from_sgf ("(;GM[6];CR[a:h]TR[h:m])",
                                     expect1, expect2))
                retval = FALSE;

        return retval;
}

static gboolean
test_prop_DD (const GSGFNode *node)
{
        const GSGFValue *value =
                        gsgf_node_get_property_value (node, "DD");
        const GSGFListOf *list_of;
        GType type;
        GSGFCookedValue *cooked_point;
        gsize num_points;
        guint point;
        gint values[] = { 0, 1, 2, 3 };
        gsize expect_num_points, i;

        if (!value) {
                g_printerr ("No property 'DD'!\n");
                return FALSE;
        }

        if (!GSGF_IS_LIST_OF (value)) {
                g_printerr ("Property 'DD' is not a GSGFListOf!\n");
                return FALSE;
        }

        list_of = GSGF_LIST_OF (value);
        type = gsgf_list_of_get_item_type (list_of);
        if (type != gsgf_point_backgammon_get_type ()) {
                g_printerr ("Property 'DD': Expected GSGFPointBackgammon, not %s!\n",
                            g_type_name(type));
                return FALSE;
        }

        num_points = gsgf_list_of_get_number_of_items(list_of);
        expect_num_points = (sizeof values) / (sizeof *values);
        if (num_points != expect_num_points) {
                g_printerr ("Property 'DD': Expected %llu points, got %llu!\n",
                            (unsigned long long) expect_num_points,
                            (unsigned long long) num_points);
                return FALSE;
        }

        for (i = 0; i < expect_num_points; ++i) {
                cooked_point = gsgf_list_of_get_nth_item(list_of, i);
                if (!GSGF_IS_POINT_BACKGAMMON (cooked_point)) {
                        g_printerr ("Property 'DD': Item #%llu is not a"
                                    " GSGFSPointBackgammon!\n",
                                    (unsigned long long) i);
                        return FALSE;
                }
                point = gsgf_point_backgammon_get_point
                                (GSGF_POINT_BACKGAMMON (cooked_point));
                if (point != values[i]) {
                        g_printerr ("Property 'DD': Item #%llu is not a %d"
                                    " point but a %d point!\n",
                                    (unsigned long long) i, values[i], point);
                        return FALSE;
                }
        }

        return TRUE;
}

static gboolean
test_prop_DD_empty (const GSGFNode *node)
{
        const GSGFValue *value =
                        gsgf_node_get_property_value (node, "DD");
        const GSGFListOf *list_of;
        GType type;
        gsize num_points;

        if (!value) {
                g_printerr ("No empty property 'DD'!\n");
                return FALSE;
        }

        if (!GSGF_IS_LIST_OF (value)) {
                g_printerr ("Empty property 'DD' is not a GSGFListOf!\n");
                return FALSE;
        }

        list_of = GSGF_LIST_OF (value);
        type = gsgf_list_of_get_item_type (list_of);
        if (type != gsgf_empty_get_type ()) {
                g_printerr ("Empty property 'DD': Expected GSGFEmpty, not %s!\n",
                            g_type_name(type));
                return FALSE;
        }

        num_points = gsgf_list_of_get_number_of_items(list_of);
        if (num_points != 1) {
                g_printerr ("Property 'DD': Expected one point, got %llu!\n",
                            (unsigned long long) num_points);
                return FALSE;
        }

        return TRUE;
}

struct point_simpletext {
        gint point;
        const gchar *label;
};

static gboolean
test_prop_LB (const GSGFNode *node)
{
        const GSGFValue *value =
                        gsgf_node_get_property_value (node, "LB");
        GSGFListOf *list_of;
        GType type;
        gsize num_pairs, i;
        GSGFCompose *compose;
        GSGFCookedValue *item;
        struct point_simpletext expect[2] = {
                        { 0, "This is point a" },
                        { 25, "And this is point z" },
        };
        gint got_point;
        const gchar *got_label;

        if (!value) {
                g_printerr ("No property 'LB'!\n");
                return FALSE;
        }

        if (!GSGF_IS_LIST_OF (value)) {
                g_printerr ("Property 'LB' is not a GSGFListOf!\n");
                return FALSE;
        }
        list_of = GSGF_LIST_OF (value);

        type = gsgf_list_of_get_item_type (list_of);
        if (type != GSGF_TYPE_COMPOSE) {
                g_printerr ("Expected item type 'GSGFCompose', not '%s'.\n",
                            g_type_name (type));
                return FALSE;
        }

        num_pairs = gsgf_list_of_get_number_of_items (list_of);
        if (num_pairs != 2) {
                g_printerr ("Expected 2 pairs, got %llu.\n",
                            (unsigned long long) num_pairs);
                return FALSE;
        }

        for (i = 0; i < 2; ++i) {
                compose = GSGF_COMPOSE (gsgf_list_of_get_nth_item (list_of, i));

                item = gsgf_compose_get_value (compose, 0);
                if (!GSGF_IS_POINT_BACKGAMMON (item)) {
                        g_printerr ("Expected GSGFPointBackgammon for"
                                    " item #0 of pair #%lld, not '%s':\n",
                                    (long long) i, G_OBJECT_TYPE_NAME (item));
                        return FALSE;
                }
                got_point = gsgf_point_get_normalized_value (GSGF_POINT (item));

                if (expect[i].point != got_point) {
                        g_printerr ("Expected %d not %d for"
                                    " item #0 of pair #%lld.\n",
                                    expect[i].point, got_point,
                                    (long long) i);
                        return FALSE;
                }

                item = gsgf_compose_get_value (compose, 1);
                if (!GSGF_IS_SIMPLE_TEXT (item)) {
                        g_printerr ("Expected GSGFSimpleText for"
                                    " item #1 of pair #%lld, not '%s':\n",
                                    (long long) i,
                                    G_OBJECT_TYPE_NAME (item));
                        return FALSE;
                }

                got_label = gsgf_text_get_value (GSGF_TEXT (item));
                if (strcmp (expect[i].label, got_label)) {
                        g_printerr ("Expected '%s' not '%s' for"
                                    " item #1 of pair #%lld.\n",
                                    expect[i].label, got_label,
                                    (long long) i);
                        return FALSE;
                }
        }

        return TRUE;
}

static gboolean
test_unique_points_LB (void)
{
        GError *expect;

        expect = NULL;
        g_set_error (&expect, GSGF_ERROR, GSGF_ERROR_SEMANTIC_ERROR,
                     "Property 'LB': Points must be unique");
        if (!expect_error_from_sgf ("(;GM[6];LB[a:foo][a:bar])", expect))
                return FALSE;

        return TRUE;
}

static gboolean
test_prop_LN (const GSGFNode *node)
{
        const GSGFValue *value =
                        gsgf_node_get_property_value (node, "LN");
        GSGFListOf *list_of;
        GType type;
        gsize num_lines;
        gint expect[3][2] = {{ 0, 1}, { 2, 3 }, { 3, 4 }};
        gint got;
        GSGFCompose *compose;
        GSGFCookedValue *item;
        gsize i;

        if (!value) {
                g_printerr ("No property 'LN'!\n");
                return FALSE;
        }

        if (!GSGF_IS_LIST_OF (value)) {
                g_printerr ("Property 'LN' is not a GSGFListOf!\n");
                return FALSE;
        }
        list_of = GSGF_LIST_OF (value);

        type = gsgf_list_of_get_item_type (list_of);
        if (type != GSGF_TYPE_COMPOSE) {
                g_printerr ("Expected item type 'GSGFCompose', not '%s'.\n",
                            g_type_name (type));
                return FALSE;
        }

        num_lines = gsgf_list_of_get_number_of_items (list_of);
        if (num_lines != 3) {
                g_printerr ("Expected 3 lines, got %llu.\n",
                            (unsigned long long) num_lines);
                return FALSE;
        }

        for (i = 0; i < 3; ++i) {
                compose = GSGF_COMPOSE (gsgf_list_of_get_nth_item (list_of, i));

                item = gsgf_compose_get_value (compose, 0);
                if (!GSGF_IS_POINT_BACKGAMMON (item)) {
                        g_printerr ("Expected GSGFPointBackgammon for"
                                    " item #0 of pair #%lld, not '%s':\n",
                                    (long long) i, G_OBJECT_TYPE_NAME (item));
                        return FALSE;
                }
                got = gsgf_point_get_normalized_value (GSGF_POINT (item));

                if (expect[i][0] != got) {
                        g_printerr ("Expected %d not %d for"
                                    " item #0 of pair #%lld.\n",
                                    expect[i][0], got, (long long) i);
                        return FALSE;
                }

                item = gsgf_compose_get_value (compose, 1);
                if (!GSGF_IS_POINT_BACKGAMMON (item)) {
                        g_printerr ("Expected GSGFPointBackgammon for"
                                    " item #1 of pair #%lld, not '%s':\n",
                                    (long long) i, G_OBJECT_TYPE_NAME (item));
                        return FALSE;
                }
                got = gsgf_point_get_normalized_value (GSGF_POINT (item));

                if (expect[i][1] != got) {
                        g_printerr ("Expected %d not %d for"
                                    " item #1 of pair #%lld.\n",
                                    expect[i][1], got,
                                    (long long) i);
                        return FALSE;
                }
        }

        return TRUE;
}

static gboolean
test_constraints_LN (void)
{
        GError *expect;

        expect = NULL;
        g_set_error (&expect, GSGF_ERROR, GSGF_ERROR_SEMANTIC_ERROR,
                     "Property 'LN': Start and end point must differ");
        if (!expect_error_from_sgf ("(;GM[6];LN[a:b][c:c][c:d])", expect))
                return FALSE;

        expect = NULL;
        g_set_error (&expect, GSGF_ERROR, GSGF_ERROR_SEMANTIC_ERROR,
                     "Property 'LN': Lines and arrows must be unique");
        if (!expect_error_from_sgf ("(;GM[6];LN[a:b][c:d][a:b])", expect))
                return FALSE;

        return TRUE;
}

static gboolean
test_prop_MA (const GSGFNode *node)
{
        const GSGFValue *value =
                        gsgf_node_get_property_value (node, "MA");
        const GSGFListOf *list_of;
        GType type;
        GSGFCookedValue *cooked_point;
        gsize num_points;
        guint point;
        gint values[] = { 4, 5, 6, 7 };
        gsize expect_num_points, i;

        if (!value) {
                g_printerr ("No property 'MA'!\n");
                return FALSE;
        }

        if (!GSGF_IS_LIST_OF(value)) {
                g_printerr ("Property 'MA' is not a GSGFListOf!\n");
                return FALSE;
        }

        list_of = GSGF_LIST_OF(value);
        type = gsgf_list_of_get_item_type(list_of);
        if (type != gsgf_point_backgammon_get_type ()) {
                g_printerr ("Property 'MA': Expected GSGFPointBackgammon, not %s!\n",
                            g_type_name(type));
                return FALSE;
        }

        num_points = gsgf_list_of_get_number_of_items(list_of);
        expect_num_points = (sizeof values) / (sizeof *values);
        if (num_points != expect_num_points) {
                g_printerr ("Property 'MA': Expected %llu points, got %llu!\n",
                            (unsigned long long) expect_num_points,
                            (unsigned long long) num_points);
                return FALSE;
        }

        for (i = 0; i < expect_num_points; ++i) {
                cooked_point = gsgf_list_of_get_nth_item(list_of, i);
                if (!GSGF_IS_POINT_BACKGAMMON (cooked_point)) {
                        g_printerr ("Property 'MA': Item #%llu is not a"
                                    " GSGFSPointBackgammon!\n",
                                    (unsigned long long) i);
                        return FALSE;
                }
                point = gsgf_point_backgammon_get_point
                                (GSGF_POINT_BACKGAMMON (cooked_point));
                if (point != values[i]) {
                        g_printerr ("Property 'MA': Item #%llu is not a %d"
                                    " point but a %d point!\n",
                                    (unsigned long long) i, values[i], point);
                        return FALSE;
                }
        }

        return TRUE;
}

static gboolean
test_unique_points_MA (void)
{
        GError *expect1 = NULL;
        GError *expect2 = NULL;
        gboolean retval = TRUE;

        g_set_error (&expect1, GSGF_ERROR, GSGF_ERROR_SEMANTIC_ERROR,
                     "Property 'MA': The properties 'MA' and 'CR' are not"
                     " allowed on the same point within one node");
        g_set_error (&expect2, GSGF_ERROR, GSGF_ERROR_SEMANTIC_ERROR,
                     "Property 'CR': The properties 'CR' and 'MA' are not"
                     " allowed on the same point within one node");
        if (!expect_errors_from_sgf ("(;GM[6];MA[a:h]CR[h:m])",
                                     expect1, expect2))
                retval = FALSE;

        expect1 = NULL;
        expect2 = NULL;
        g_set_error (&expect1, GSGF_ERROR, GSGF_ERROR_SEMANTIC_ERROR,
                     "Property 'MA': The properties 'MA' and 'SL' are not"
                     " allowed on the same point within one node");
        g_set_error (&expect2, GSGF_ERROR, GSGF_ERROR_SEMANTIC_ERROR,
                     "Property 'SL': The properties 'SL' and 'MA' are not"
                     " allowed on the same point within one node");
        if (!expect_errors_from_sgf ("(;GM[6];MA[a:h]SL[h:m])",
                                     expect1, expect2))
                retval = FALSE;

        expect1 = NULL;
        expect2 = NULL;
        g_set_error (&expect1, GSGF_ERROR, GSGF_ERROR_SEMANTIC_ERROR,
                     "Property 'MA': The properties 'MA' and 'SQ' are not"
                     " allowed on the same point within one node");
        g_set_error (&expect2, GSGF_ERROR, GSGF_ERROR_SEMANTIC_ERROR,
                     "Property 'SQ': The properties 'SQ' and 'MA' are not"
                     " allowed on the same point within one node");
        if (!expect_errors_from_sgf ("(;GM[6];MA[a:h]SQ[h:m])",
                                     expect1, expect2))
                retval = FALSE;

        expect1 = NULL;
        expect2 = NULL;
        g_set_error (&expect1, GSGF_ERROR, GSGF_ERROR_SEMANTIC_ERROR,
                     "Property 'MA': The properties 'MA' and 'TR' are not"
                     " allowed on the same point within one node");
        g_set_error (&expect2, GSGF_ERROR, GSGF_ERROR_SEMANTIC_ERROR,
                     "Property 'TR': The properties 'TR' and 'MA' are not"
                     " allowed on the same point within one node");
        if (!expect_errors_from_sgf ("(;GM[6];MA[a:h]TR[h:m])",
                                     expect1, expect2))
                retval = FALSE;

        return retval;
}

static gboolean
test_prop_SL (const GSGFNode *node)
{
        const GSGFValue *value =
                        gsgf_node_get_property_value (node, "SL");
        const GSGFListOf *list_of;
        GType type;
        GSGFCookedValue *cooked_point;
        gsize num_points;
        guint point;
        gint values[] = { 8, 9, 10, 11 };
        gsize expect_num_points, i;

        if (!value) {
                g_printerr ("No property 'SL'!\n");
                return FALSE;
        }

        if (!GSGF_IS_LIST_OF(value)) {
                g_printerr ("Property 'SL' is not a GSGFListOf!\n");
                return FALSE;
        }

        list_of = GSGF_LIST_OF(value);
        type = gsgf_list_of_get_item_type(list_of);
        if (type != gsgf_point_backgammon_get_type ()) {
                g_printerr ("Property 'SL': Expected GSGFPointBackgammon, not %s!\n",
                            g_type_name(type));
                return FALSE;
        }

        num_points = gsgf_list_of_get_number_of_items(list_of);
        expect_num_points = (sizeof values) / (sizeof *values);
        if (num_points != expect_num_points) {
                g_printerr ("Property 'SL': Expected %llu points, got %llu!\n",
                            (unsigned long long) expect_num_points,
                            (unsigned long long) num_points);
                return FALSE;
        }

        for (i = 0; i < expect_num_points; ++i) {
                cooked_point = gsgf_list_of_get_nth_item(list_of, i);
                if (!GSGF_IS_POINT_BACKGAMMON (cooked_point)) {
                        g_printerr ("Property 'SL': Item #%llu is not a"
                                    " GSGFSPointBackgammon!\n",
                                    (unsigned long long) i);
                        return FALSE;
                }
                point = gsgf_point_backgammon_get_point
                                (GSGF_POINT_BACKGAMMON (cooked_point));
                if (point != values[i]) {
                        g_printerr ("Property 'SL': Item #%llu is not a %d"
                                    " point but a %d point!\n",
                                    (unsigned long long) i, values[i], point);
                        return FALSE;
                }
        }

        return TRUE;
}

static gboolean
test_unique_points_SL (void)
{
        GError *expect1 = NULL;
        GError *expect2 = NULL;
        gboolean retval = TRUE;

        g_set_error (&expect1, GSGF_ERROR, GSGF_ERROR_SEMANTIC_ERROR,
                     "Property 'SL': The properties 'SL' and 'CR' are not"
                     " allowed on the same point within one node");
        g_set_error (&expect2, GSGF_ERROR, GSGF_ERROR_SEMANTIC_ERROR,
                     "Property 'CR': The properties 'CR' and 'SL' are not"
                     " allowed on the same point within one node");
        if (!expect_errors_from_sgf ("(;GM[6];SL[a:h]CR[h:m])",
                                     expect1, expect2))
                retval = FALSE;

        expect1 = NULL;
        expect2 = NULL;
        g_set_error (&expect1, GSGF_ERROR, GSGF_ERROR_SEMANTIC_ERROR,
                     "Property 'SL': The properties 'SL' and 'MA' are not"
                     " allowed on the same point within one node");
        g_set_error (&expect2, GSGF_ERROR, GSGF_ERROR_SEMANTIC_ERROR,
                     "Property 'MA': The properties 'MA' and 'SL' are not"
                     " allowed on the same point within one node");
        if (!expect_errors_from_sgf ("(;GM[6];SL[a:h]MA[h:m])",
                                     expect1, expect2))
                retval = FALSE;

        expect1 = NULL;
        expect2 = NULL;
        g_set_error (&expect1, GSGF_ERROR, GSGF_ERROR_SEMANTIC_ERROR,
                     "Property 'SL': The properties 'SL' and 'SQ' are not"
                     " allowed on the same point within one node");
        g_set_error (&expect2, GSGF_ERROR, GSGF_ERROR_SEMANTIC_ERROR,
                     "Property 'SQ': The properties 'SQ' and 'SL' are not"
                     " allowed on the same point within one node");
        if (!expect_errors_from_sgf ("(;GM[6];SL[a:h]SQ[h:m])",
                                     expect1, expect2))
                retval = FALSE;

        expect1 = NULL;
        expect2 = NULL;
        g_set_error (&expect1, GSGF_ERROR, GSGF_ERROR_SEMANTIC_ERROR,
                     "Property 'SL': The properties 'SL' and 'TR' are not"
                     " allowed on the same point within one node");
        g_set_error (&expect2, GSGF_ERROR, GSGF_ERROR_SEMANTIC_ERROR,
                     "Property 'TR': The properties 'TR' and 'SL' are not"
                     " allowed on the same point within one node");
        if (!expect_errors_from_sgf ("(;GM[6];SL[a:h]TR[h:m])",
                                     expect1, expect2))
                retval = FALSE;

        return retval;
}

static gboolean
test_prop_SQ (const GSGFNode *node)
{
        const GSGFValue *value =
                        gsgf_node_get_property_value (node, "SQ");
        const GSGFListOf *list_of;
        GType type;
        GSGFCookedValue *cooked_point;
        gsize num_points;
        guint point;
        gint values[] = { 12, 13, 14, 15 };
        gsize expect_num_points, i;

        if (!value) {
                g_printerr ("No property 'SQ'!\n");
                return FALSE;
        }

        if (!GSGF_IS_LIST_OF(value)) {
                g_printerr ("Property 'SQ' is not a GSGFListOf!\n");
                return FALSE;
        }

        list_of = GSGF_LIST_OF(value);
        type = gsgf_list_of_get_item_type(list_of);
        if (type != gsgf_point_backgammon_get_type ()) {
                g_printerr ("Property 'SQ': Expected GSGFPointBackgammon, not %s!\n",
                            g_type_name(type));
                return FALSE;
        }

        num_points = gsgf_list_of_get_number_of_items(list_of);
        expect_num_points = (sizeof values) / (sizeof *values);
        if (num_points != expect_num_points) {
                g_printerr ("Property 'SQ': Expected %llu points, got %llu!\n",
                            (unsigned long long) expect_num_points,
                            (unsigned long long) num_points);
                return FALSE;
        }

        for (i = 0; i < expect_num_points; ++i) {
                cooked_point = gsgf_list_of_get_nth_item(list_of, i);
                if (!GSGF_IS_POINT_BACKGAMMON (cooked_point)) {
                        g_printerr ("Property 'SQ': Item #%llu is not a"
                                    " GSGFSPointBackgammon!\n",
                                    (unsigned long long) i);
                        return FALSE;
                }
                point = gsgf_point_backgammon_get_point
                                (GSGF_POINT_BACKGAMMON (cooked_point));
                if (point != values[i]) {
                        g_printerr ("Property 'SQ': Item #%llu is not a %d"
                                    " point but a %d point!\n",
                                    (unsigned long long) i, values[i], point);
                        return FALSE;
                }
        }

        return TRUE;
}

static gboolean
test_unique_points_SQ (void)
{
        GError *expect1 = NULL;
        GError *expect2 = NULL;
        gboolean retval = TRUE;

        g_set_error (&expect1, GSGF_ERROR, GSGF_ERROR_SEMANTIC_ERROR,
                     "Property 'SQ': The properties 'SQ' and 'CR' are not"
                     " allowed on the same point within one node");
        g_set_error (&expect2, GSGF_ERROR, GSGF_ERROR_SEMANTIC_ERROR,
                     "Property 'CR': The properties 'CR' and 'SQ' are not"
                     " allowed on the same point within one node");
        if (!expect_errors_from_sgf ("(;GM[6];SQ[a:h]CR[h:m])",
                                     expect1, expect2))
                retval = FALSE;

        expect1 = NULL;
        expect2 = NULL;
        g_set_error (&expect1, GSGF_ERROR, GSGF_ERROR_SEMANTIC_ERROR,
                     "Property 'SQ': The properties 'SQ' and 'MA' are not"
                     " allowed on the same point within one node");
        g_set_error (&expect2, GSGF_ERROR, GSGF_ERROR_SEMANTIC_ERROR,
                     "Property 'MA': The properties 'MA' and 'SQ' are not"
                     " allowed on the same point within one node");
        if (!expect_errors_from_sgf ("(;GM[6];SQ[a:h]MA[h:m])",
                                     expect1, expect2))
                retval = FALSE;

        expect1 = NULL;
        expect2 = NULL;
        g_set_error (&expect1, GSGF_ERROR, GSGF_ERROR_SEMANTIC_ERROR,
                     "Property 'SQ': The properties 'SQ' and 'SL' are not"
                     " allowed on the same point within one node");
        g_set_error (&expect2, GSGF_ERROR, GSGF_ERROR_SEMANTIC_ERROR,
                     "Property 'SL': The properties 'SL' and 'SQ' are not"
                     " allowed on the same point within one node");
        if (!expect_errors_from_sgf ("(;GM[6];SQ[a:h]SL[h:m])",
                                     expect1, expect2))
                retval = FALSE;

        expect1 = NULL;
        expect2 = NULL;
        g_set_error (&expect1, GSGF_ERROR, GSGF_ERROR_SEMANTIC_ERROR,
                     "Property 'SQ': The properties 'SQ' and 'TR' are not"
                     " allowed on the same point within one node");
        g_set_error (&expect2, GSGF_ERROR, GSGF_ERROR_SEMANTIC_ERROR,
                     "Property 'TR': The properties 'TR' and 'SQ' are not"
                     " allowed on the same point within one node");
        if (!expect_errors_from_sgf ("(;GM[6];SQ[a:h]TR[h:m])",
                                     expect1, expect2))
                retval = FALSE;

        return retval;
}


static gboolean
test_prop_TR (const GSGFNode *node)
{
        const GSGFValue *value =
                        gsgf_node_get_property_value (node, "TR");
        const GSGFListOf *list_of;
        GType type;
        GSGFCookedValue *cooked_point;
        gsize num_points;
        guint point;
        gint values[] = { 16, 17, 18, 19 };
        gsize expect_num_points, i;

        if (!value) {
                g_printerr ("No property 'TR'!\n");
                return FALSE;
        }

        if (!GSGF_IS_LIST_OF(value)) {
                g_printerr ("Property 'TR' is not a GSGFListOf!\n");
                return FALSE;
        }

        list_of = GSGF_LIST_OF(value);
        type = gsgf_list_of_get_item_type(list_of);
        if (type != gsgf_point_backgammon_get_type ()) {
                g_printerr ("Property 'TR': Expected GSGFPointBackgammon, not %s!\n",
                            g_type_name(type));
                return FALSE;
        }

        num_points = gsgf_list_of_get_number_of_items(list_of);
        expect_num_points = (sizeof values) / (sizeof *values);
        if (num_points != expect_num_points) {
                g_printerr ("Property 'TR': Expected %llu points, got %llu!\n",
                            (unsigned long long) expect_num_points,
                            (unsigned long long) num_points);
                return FALSE;
        }

        for (i = 0; i < expect_num_points; ++i) {
                cooked_point = gsgf_list_of_get_nth_item(list_of, i);
                if (!GSGF_IS_POINT_BACKGAMMON (cooked_point)) {
                        g_printerr ("Property 'TR': Item #%llu is not a"
                                    " GSGFSPointBackgammon!\n",
                                    (unsigned long long) i);
                        return FALSE;
                }
                point = gsgf_point_backgammon_get_point
                                (GSGF_POINT_BACKGAMMON (cooked_point));
                if (point != values[i]) {
                        g_printerr ("Property 'TR': Item #%llu is not a %d"
                                    " point but a %d point!\n",
                                    (unsigned long long) i, values[i], point);
                        return FALSE;
                }
        }

        return TRUE;
}

static gboolean
test_unique_points_TR (void)
{
        GError *expect1 = NULL;
        GError *expect2 = NULL;
        gboolean retval = TRUE;

        g_set_error (&expect1, GSGF_ERROR, GSGF_ERROR_SEMANTIC_ERROR,
                     "Property 'TR': The properties 'TR' and 'CR' are not"
                     " allowed on the same point within one node");
        g_set_error (&expect2, GSGF_ERROR, GSGF_ERROR_SEMANTIC_ERROR,
                     "Property 'CR': The properties 'CR' and 'TR' are not"
                     " allowed on the same point within one node");
        if (!expect_errors_from_sgf ("(;GM[6];TR[a:h]CR[h:m])",
                                     expect1, expect2))
                retval = FALSE;

        expect1 = NULL;
        expect2 = NULL;
        g_set_error (&expect1, GSGF_ERROR, GSGF_ERROR_SEMANTIC_ERROR,
                     "Property 'TR': The properties 'TR' and 'MA' are not"
                     " allowed on the same point within one node");
        g_set_error (&expect2, GSGF_ERROR, GSGF_ERROR_SEMANTIC_ERROR,
                     "Property 'MA': The properties 'MA' and 'TR' are not"
                     " allowed on the same point within one node");
        if (!expect_errors_from_sgf ("(;GM[6];TR[a:h]MA[h:m])",
                                     expect1, expect2))
                retval = FALSE;

        expect1 = NULL;
        expect2 = NULL;
        g_set_error (&expect1, GSGF_ERROR, GSGF_ERROR_SEMANTIC_ERROR,
                     "Property 'TR': The properties 'TR' and 'SL' are not"
                     " allowed on the same point within one node");
        g_set_error (&expect2, GSGF_ERROR, GSGF_ERROR_SEMANTIC_ERROR,
                     "Property 'SL': The properties 'SL' and 'TR' are not"
                     " allowed on the same point within one node");
        if (!expect_errors_from_sgf ("(;GM[6];TR[a:h]SL[h:m])",
                                     expect1, expect2))
                retval = FALSE;

        expect1 = NULL;
        expect2 = NULL;
        g_set_error (&expect1, GSGF_ERROR, GSGF_ERROR_SEMANTIC_ERROR,
                     "Property 'TR': The properties 'TR' and 'SQ' are not"
                     " allowed on the same point within one node");
        g_set_error (&expect2, GSGF_ERROR, GSGF_ERROR_SEMANTIC_ERROR,
                     "Property 'SQ': The properties 'SQ' and 'TR' are not"
                     " allowed on the same point within one node");
        if (!expect_errors_from_sgf ("(;GM[6];TR[a:h]SQ[h:m])",
                                     expect1, expect2))
                retval = FALSE;

        return retval;
}
