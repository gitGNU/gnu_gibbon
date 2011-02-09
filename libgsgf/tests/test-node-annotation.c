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

char *filename = "node-annotation.sgf";

static gboolean test_prop_C (const GSGFNode *node);
static gboolean test_prop_DM (const GSGFNode *node);
static gboolean test_prop_GB (const GSGFNode *node);
static gboolean test_prop_GW (const GSGFNode *node);
static gboolean test_prop_HO (const GSGFNode *node);
static gboolean test_prop_UC (const GSGFNode *node);
static gboolean test_prop_N (const GSGFNode *node);
static gboolean test_prop_V (const GSGFNode *node);

int 
test_collection (GSGFCollection *collection, GError *error)
{
        GList *game_trees;
        GSGFGameTree *game_tree;
        GList *nodes;
        gpointer item;
        GSGFNode *node;

        if (error) {
                fprintf(stderr, "%s: %s\n", filename, error->message);
                return -1;
        }

        game_trees = gsgf_collection_get_game_trees (collection);
        if (!game_trees) {
                fprintf(stderr, "No game trees found.\n");
                return -1;
        }
        game_tree = GSGF_GAME_TREE (game_trees->data);

        nodes = gsgf_game_tree_get_nodes (game_tree);
        if (!nodes) {
                fprintf(stderr, "No nodes found.\n");
                return -1;
        }

        item = g_list_nth_data (nodes, 1);
        if (!item) {
                fprintf(stderr, "Property #1 not found.\n");
                return -1;
        }
        node = GSGF_NODE (item);
        if (!test_prop_C (node))
                return -1;

        item = g_list_nth_data (nodes, 2);
        if (!item) {
                fprintf(stderr, "Property #2 not found.\n");
                return -1;
        }
        node = GSGF_NODE (item);
        if (!test_prop_DM (node))
                return -1;

        item = g_list_nth_data (nodes, 3);
        if (!item) {
                fprintf(stderr, "Property #3 not found.\n");
                return -1;
        }
        node = GSGF_NODE (item);
        if (!test_prop_GB (node))
                return -1;

        item = g_list_nth_data (nodes, 4);
        if (!item) {
                fprintf(stderr, "Property #4 not found.\n");
                return -1;
        }
        node = GSGF_NODE (item);
        if (!test_prop_GW (node))
                return -1;

        item = g_list_nth_data (nodes, 5);
        if (!item) {
                fprintf(stderr, "Property #5 not found.\n");
                return -1;
        }
        node = GSGF_NODE (item);
        if (!test_prop_HO (node))
                return -1;
        if (!test_prop_N (node))
                return -1;

        item = g_list_nth_data (nodes, 6);
        if (!item) {
                fprintf(stderr, "Property #6 not found.\n");
                return -1;
        }
        node = GSGF_NODE (item);
        if (!test_prop_UC (node))
                return -1;
        if (!test_prop_V (node))
                return -1;

        return expect_error (error, NULL);
}

static gboolean
test_prop_C (const GSGFNode *node)
{
        const GSGFCookedValue *cooked_value =
                        gsgf_node_get_property_cooked (node, "C");
        gchar *value;

        if (!cooked_value) {
                fprintf (stderr, "No property 'C'!\n");
                return FALSE;
        }

        if (!GSGF_IS_TEXT (cooked_value)) {
                fprintf (stderr, "Property 'C' is not a GSGFText!\n");
                return FALSE;
        }

        value = gsgf_text_get_value (GSGF_TEXT (cooked_value));
#define EXPECT "This is the test for node annotation properties."
        if (strcmp (EXPECT, value)) {
                fprintf(stderr, "C: Expected '%s', not '%s'!\n", EXPECT, value);
                return FALSE;
        }

        return TRUE;
}

static gboolean
test_prop_DM (const GSGFNode *node)
{
        const GSGFCookedValue *cooked_value =
                        gsgf_node_get_property_cooked (node, "DM");
        GSGFDoubleEnum value;
        GSGFDoubleEnum expect = 1;

        if (!cooked_value) {
                fprintf (stderr, "No property 'DM'!\n");
                return FALSE;
        }

        if (!GSGF_IS_DOUBLE (cooked_value)) {
                fprintf (stderr, "Property 'DM' is not a GSGFDouble!\n");
                return FALSE;
        }

        value = gsgf_double_get_value (GSGF_DOUBLE (cooked_value));
        if (expect != value) {
                fprintf(stderr, "DM: Expected %d', not '%d'!\n", expect, value);
                return FALSE;
        }

        return TRUE;
}

static gboolean
test_prop_GB (const GSGFNode *node)
{
        const GSGFCookedValue *cooked_value =
                        gsgf_node_get_property_cooked (node, "GB");
        GSGFDoubleEnum value;
        GSGFDoubleEnum expect = 2;

        if (!cooked_value) {
                fprintf (stderr, "No property 'GB'!\n");
                return FALSE;
        }

        if (!GSGF_IS_DOUBLE (cooked_value)) {
                fprintf (stderr, "Property 'GB' is not a GSGFDouble!\n");
                return FALSE;
        }

        value = gsgf_double_get_value (GSGF_DOUBLE (cooked_value));
        if (expect != value) {
                fprintf(stderr, "GB: Expected %d', not '%d'!\n", expect, value);
                return FALSE;
        }

        return TRUE;
}

static gboolean
test_prop_GW (const GSGFNode *node)
{
        const GSGFCookedValue *cooked_value =
                        gsgf_node_get_property_cooked (node, "GW");
        GSGFDoubleEnum value;
        GSGFDoubleEnum expect = 1;

        if (!cooked_value) {
                fprintf (stderr, "No property 'GW'!\n");
                return FALSE;
        }

        if (!GSGF_IS_DOUBLE (cooked_value)) {
                fprintf (stderr, "Property 'GW' is not a GSGFDouble!\n");
                return FALSE;
        }

        value = gsgf_double_get_value (GSGF_DOUBLE (cooked_value));
        if (expect != value) {
                fprintf(stderr, "GW: Expected %d', not '%d'!\n", expect, value);
                return FALSE;
        }

        return TRUE;
}

static gboolean
test_prop_HO (const GSGFNode *node)
{
        const GSGFCookedValue *cooked_value =
                        gsgf_node_get_property_cooked (node, "HO");
        GSGFDoubleEnum value;
        GSGFDoubleEnum expect = 2;

        if (!cooked_value) {
                fprintf (stderr, "No property 'HO'!\n");
                return FALSE;
        }

        if (!GSGF_IS_DOUBLE (cooked_value)) {
                fprintf (stderr, "Property 'HO' is not a GSGFDouble!\n");
                return FALSE;
        }

        value = gsgf_double_get_value (GSGF_DOUBLE (cooked_value));
        if (expect != value) {
                fprintf(stderr, "HO: Expected %d', not '%d'!\n", expect, value);
                return FALSE;
        }

        return TRUE;
}

static gboolean
test_prop_N (const GSGFNode *node)
{
        const GSGFCookedValue *cooked_value =
                        gsgf_node_get_property_cooked (node, "N");
        gchar *value;

        if (!cooked_value) {
                fprintf (stderr, "No property 'N'!\n");
                return FALSE;
        }

        if (!GSGF_IS_SIMPLE_TEXT (cooked_value)) {
                fprintf (stderr, "Property 'N' is not a GSGFSimpleText!\n");
                return FALSE;
        }

        value = gsgf_text_get_value (GSGF_SIMPLE_TEXT (cooked_value));
#define EXPECT "This is the test for node annotation properties."
        if (strcmp (EXPECT, value)) {
                fprintf(stderr, "C: Expected '%s', not '%s'!\n", EXPECT, value);
                return FALSE;
        }

        return TRUE;
}

static gboolean
test_prop_UC (const GSGFNode *node)
{
        const GSGFCookedValue *cooked_value =
                        gsgf_node_get_property_cooked (node, "UC");
        GSGFDoubleEnum value;
        GSGFDoubleEnum expect = 1;

        if (!cooked_value) {
                fprintf (stderr, "No property 'UC'!\n");
                return FALSE;
        }

        if (!GSGF_IS_DOUBLE (cooked_value)) {
                fprintf (stderr, "Property 'UC' is not a GSGFDouble!\n");
                return FALSE;
        }

        value = gsgf_double_get_value (GSGF_DOUBLE (cooked_value));
        if (expect != value) {
                fprintf(stderr, "UC: Expected %d', not '%d'!\n", expect, value);
                return FALSE;
        }

        return TRUE;
}

static gboolean
test_prop_V (const GSGFNode *node)
{
        const GSGFCookedValue *cooked_value =
                        gsgf_node_get_property_cooked (node, "V");
        gdouble value;

        if (!cooked_value) {
                fprintf (stderr, "No property 'V'!\n");
                return FALSE;
        }

        if (!GSGF_IS_REAL (cooked_value)) {
                fprintf (stderr, "Property 'V' is not a GSGFReal!\n");
                return FALSE;
        }

        value = gsgf_real_get_value (GSGF_REAL (cooked_value));
        if (value < 1.733 || value > 1.735) {
                fprintf(stderr, "V: Expected 1.734, not '%g'!\n", value);
                return FALSE;
        }

        return TRUE;
}
