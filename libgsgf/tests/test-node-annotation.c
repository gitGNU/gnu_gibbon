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

char *filename = "node-annotation.sgf";

static gboolean test_unique_position_DM (void);
static gboolean test_unique_position_GB (void);
static gboolean test_unique_position_GW (void);
static gboolean test_unique_position_UC (void);
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

        if (!test_unique_position_DM ())
                return -1;
        if (!test_unique_position_GB ())
                return -1;
        if (!test_unique_position_GW ())
                return -1;
        if (!test_unique_position_UC ())
                return -1;

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

        item = g_list_nth_data (nodes, 0);
        if (!item) {
                fprintf(stderr, "Property #0 not found.\n");
                return -1;
        }
        node = GSGF_NODE (item);
        if (!test_prop_C (node))
                return -1;

        item = g_list_nth_data (nodes, 1);
        if (!item) {
                fprintf(stderr, "Property #1 not found.\n");
                return -1;
        }
        node = GSGF_NODE (item);
        if (!test_prop_DM (node))
                return -1;

        item = g_list_nth_data (nodes, 2);
        if (!item) {
                fprintf(stderr, "Property #2 not found.\n");
                return -1;
        }
        node = GSGF_NODE (item);
        if (!test_prop_GB (node))
                return -1;

        item = g_list_nth_data (nodes, 3);
        if (!item) {
                fprintf(stderr, "Property #3 not found.\n");
                return -1;
        }
        node = GSGF_NODE (item);
        if (!test_prop_GW (node))
                return -1;

        item = g_list_nth_data (nodes, 4);
        if (!item) {
                fprintf(stderr, "Property #4 not found.\n");
                return -1;
        }
        node = GSGF_NODE (item);
        if (!test_prop_HO (node))
                return -1;
        if (!test_prop_N (node))
                return -1;

        item = g_list_nth_data (nodes, 5);
        if (!item) {
                fprintf(stderr, "Property #5 not found.\n");
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
test_unique_position_DM (void)
{
        GError *expect1 = NULL;
        GError *expect2 = NULL;

        g_set_error (&expect1, GSGF_ERROR, GSGF_ERROR_SEMANTIC_ERROR,
                     "Property 'DM': The properties 'DM', 'GB', 'GW', and 'UC'"
                     " are mutually exclusive within one node");
        g_set_error (&expect2, GSGF_ERROR, GSGF_ERROR_SEMANTIC_ERROR,
                     "Property 'GB': The properties 'DM', 'GB', 'GW', and 'UC'"
                     " are mutually exclusive within one node");
        if (!expect_errors_from_sgf ("(;DM[1]GB[2])",
                                     expect1, expect2))
                return FALSE;

        expect1 = NULL;
        expect2 = NULL;
        g_set_error (&expect1, GSGF_ERROR, GSGF_ERROR_SEMANTIC_ERROR,
                     "Property 'DM': The properties 'DM', 'GB', 'GW', and 'UC'"
                     " are mutually exclusive within one node");
        g_set_error (&expect2, GSGF_ERROR, GSGF_ERROR_SEMANTIC_ERROR,
                     "Property 'GW': The properties 'DM', 'GB', 'GW', and 'UC'"
                     " are mutually exclusive within one node");
        if (!expect_errors_from_sgf ("(;DM[1]GW[2])",
                                     expect1, expect2))
                return FALSE;

        expect1 = NULL;
        expect2 = NULL;
        g_set_error (&expect1, GSGF_ERROR, GSGF_ERROR_SEMANTIC_ERROR,
                     "Property 'DM': The properties 'DM', 'GB', 'GW', and 'UC'"
                     " are mutually exclusive within one node");
        g_set_error (&expect2, GSGF_ERROR, GSGF_ERROR_SEMANTIC_ERROR,
                     "Property 'UC': The properties 'DM', 'GB', 'GW', and 'UC'"
                     " are mutually exclusive within one node");
        if (!expect_errors_from_sgf ("(;DM[1]UC[2])",
                                     expect1, expect2))
                return FALSE;

        return TRUE;
}

static gboolean
test_unique_position_GB (void)
{
        GError *expect1 = NULL;
        GError *expect2 = NULL;

        g_set_error (&expect1, GSGF_ERROR, GSGF_ERROR_SEMANTIC_ERROR,
                     "Property 'GB': The properties 'DM', 'GB', 'GW', and 'UC'"
                     " are mutually exclusive within one node");
        g_set_error (&expect2, GSGF_ERROR, GSGF_ERROR_SEMANTIC_ERROR,
                     "Property 'DM': The properties 'DM', 'GB', 'GW', and 'UC'"
                     " are mutually exclusive within one node");
        if (!expect_errors_from_sgf ("(;GB[1]DM[2])",
                                     expect1, expect2))
                return FALSE;

        expect1 = NULL;
        expect2 = NULL;
        g_set_error (&expect1, GSGF_ERROR, GSGF_ERROR_SEMANTIC_ERROR,
                     "Property 'GB': The properties 'DM', 'GB', 'GW', and 'UC'"
                     " are mutually exclusive within one node");
        g_set_error (&expect2, GSGF_ERROR, GSGF_ERROR_SEMANTIC_ERROR,
                     "Property 'GW': The properties 'DM', 'GB', 'GW', and 'UC'"
                     " are mutually exclusive within one node");
        if (!expect_errors_from_sgf ("(;GB[1]GW[2])",
                                     expect1, expect2))
                return FALSE;

        expect1 = NULL;
        expect2 = NULL;
        g_set_error (&expect1, GSGF_ERROR, GSGF_ERROR_SEMANTIC_ERROR,
                     "Property 'GB': The properties 'DM', 'GB', 'GW', and 'UC'"
                     " are mutually exclusive within one node");
        g_set_error (&expect2, GSGF_ERROR, GSGF_ERROR_SEMANTIC_ERROR,
                     "Property 'UC': The properties 'DM', 'GB', 'GW', and 'UC'"
                     " are mutually exclusive within one node");
        if (!expect_errors_from_sgf ("(;GB[1]UC[2])",
                                     expect1, expect2))
                return FALSE;

        return TRUE;
}

static gboolean
test_unique_position_GW (void)
{
        GError *expect1 = NULL;
        GError *expect2 = NULL;

        g_set_error (&expect1, GSGF_ERROR, GSGF_ERROR_SEMANTIC_ERROR,
                     "Property 'GW': The properties 'DM', 'GB', 'GW', and 'UC'"
                     " are mutually exclusive within one node");
        g_set_error (&expect2, GSGF_ERROR, GSGF_ERROR_SEMANTIC_ERROR,
                     "Property 'DM': The properties 'DM', 'GB', 'GW', and 'UC'"
                     " are mutually exclusive within one node");
        if (!expect_errors_from_sgf ("(;GW[1]DM[2])",
                                     expect1, expect2))
                return FALSE;

        expect1 = NULL;
        expect2 = NULL;
        g_set_error (&expect1, GSGF_ERROR, GSGF_ERROR_SEMANTIC_ERROR,
                     "Property 'GW': The properties 'DM', 'GB', 'GW', and 'UC'"
                     " are mutually exclusive within one node");
        g_set_error (&expect2, GSGF_ERROR, GSGF_ERROR_SEMANTIC_ERROR,
                     "Property 'GB': The properties 'DM', 'GB', 'GW', and 'UC'"
                     " are mutually exclusive within one node");
        if (!expect_errors_from_sgf ("(;GW[1]GB[2])",
                                     expect1, expect2))
                return FALSE;

        expect1 = NULL;
        expect2 = NULL;
        g_set_error (&expect1, GSGF_ERROR, GSGF_ERROR_SEMANTIC_ERROR,
                     "Property 'GW': The properties 'DM', 'GB', 'GW', and 'UC'"
                     " are mutually exclusive within one node");
        g_set_error (&expect2, GSGF_ERROR, GSGF_ERROR_SEMANTIC_ERROR,
                     "Property 'UC': The properties 'DM', 'GB', 'GW', and 'UC'"
                     " are mutually exclusive within one node");
        if (!expect_errors_from_sgf ("(;GW[1]UC[2])",
                                     expect1, expect2))
                return FALSE;

        return TRUE;
}

static gboolean
test_unique_position_UC (void)
{
        GError *expect1 = NULL;
        GError *expect2 = NULL;

        g_set_error (&expect1, GSGF_ERROR, GSGF_ERROR_SEMANTIC_ERROR,
                     "Property 'UC': The properties 'DM', 'GB', 'GW', and 'UC'"
                     " are mutually exclusive within one node");
        g_set_error (&expect2, GSGF_ERROR, GSGF_ERROR_SEMANTIC_ERROR,
                     "Property 'DM': The properties 'DM', 'GB', 'GW', and 'UC'"
                     " are mutually exclusive within one node");
        if (!expect_errors_from_sgf ("(;UC[1]DM[2])",
                                     expect1, expect2))
                return FALSE;

        expect1 = NULL;
        expect2 = NULL;
        g_set_error (&expect1, GSGF_ERROR, GSGF_ERROR_SEMANTIC_ERROR,
                     "Property 'UC': The properties 'DM', 'GB', 'GW', and 'UC'"
                     " are mutually exclusive within one node");
        g_set_error (&expect2, GSGF_ERROR, GSGF_ERROR_SEMANTIC_ERROR,
                     "Property 'GB': The properties 'DM', 'GB', 'GW', and 'UC'"
                     " are mutually exclusive within one node");
        if (!expect_errors_from_sgf ("(;UC[1]GB[2])",
                                     expect1, expect2))
                return FALSE;

        expect1 = NULL;
        expect2 = NULL;
        g_set_error (&expect1, GSGF_ERROR, GSGF_ERROR_SEMANTIC_ERROR,
                     "Property 'UC': The properties 'DM', 'GB', 'GW', and 'UC'"
                     " are mutually exclusive within one node");
        g_set_error (&expect2, GSGF_ERROR, GSGF_ERROR_SEMANTIC_ERROR,
                     "Property 'GW': The properties 'DM', 'GB', 'GW', and 'UC'"
                     " are mutually exclusive within one node");
        if (!expect_errors_from_sgf ("(;UC[1]GW[2])",
                                     expect1, expect2))
                return FALSE;

        return TRUE;
}

static gboolean
test_prop_C (const GSGFNode *node)
{
        const GSGFValue *value =
                        gsgf_node_get_property_value (node, "C");
        gchar *text_value;

        if (!value) {
                fprintf (stderr, "No property 'C'!\n");
                return FALSE;
        }

        if (!GSGF_IS_TEXT (value)) {
                fprintf (stderr, "Property 'C' is not a GSGFText!\n");
                return FALSE;
        }

        text_value = gsgf_text_get_value (GSGF_TEXT (value));
#define EXPECT1 "This is the test for node annotation properties."
        if (strcmp (EXPECT1, text_value)) {
                fprintf(stderr, "C: Expected '%s', not '%s'!\n",
                        EXPECT1, text_value);
                return FALSE;
        }

        return TRUE;
}

static gboolean
test_prop_DM (const GSGFNode *node)
{
        const GSGFValue *value =
                        gsgf_node_get_property_value (node, "DM");
        GSGFDoubleEnum double_value;
        GSGFDoubleEnum expect = 1;

        if (!value) {
                fprintf (stderr, "No property 'DM'!\n");
                return FALSE;
        }

        if (!GSGF_IS_DOUBLE (value)) {
                fprintf (stderr, "Property 'DM' is not a GSGFDouble!\n");
                return FALSE;
        }

        double_value = gsgf_double_get_value (GSGF_DOUBLE (value));
        if (expect != double_value) {
                fprintf(stderr, "DM: Expected %d, not %d!\n",
                        expect, double_value);
                return FALSE;
        }

        return TRUE;
}

static gboolean
test_prop_GB (const GSGFNode *node)
{
        const GSGFValue *value =
                        gsgf_node_get_property_value (node, "GB");
        GSGFDoubleEnum double_value;
        GSGFDoubleEnum expect = 2;

        if (!value) {
                fprintf (stderr, "No property 'GB'!\n");
                return FALSE;
        }

        if (!GSGF_IS_DOUBLE (value)) {
                fprintf (stderr, "Property 'GB' is not a GSGFDouble!\n");
                return FALSE;
        }

        double_value = gsgf_double_get_value (GSGF_DOUBLE (value));
        if (expect != double_value) {
                fprintf(stderr, "GB: Expected %d, not %d!\n",
                        expect, double_value);
                return FALSE;
        }

        return TRUE;
}

static gboolean
test_prop_GW (const GSGFNode *node)
{
        const GSGFValue *value =
                        gsgf_node_get_property_value (node, "GW");
        GSGFDoubleEnum double_value;
        GSGFDoubleEnum expect = 1;

        if (!value) {
                fprintf (stderr, "No property 'GW'!\n");
                return FALSE;
        }

        if (!GSGF_IS_DOUBLE (value)) {
                fprintf (stderr, "Property 'GW' is not a GSGFDouble!\n");
                return FALSE;
        }

        double_value = gsgf_double_get_value (GSGF_DOUBLE (value));
        if (expect != double_value) {
                fprintf(stderr, "GW: Expected %d, not %d!\n",
                        expect, double_value);
                return FALSE;
        }

        return TRUE;
}

static gboolean
test_prop_HO (const GSGFNode *node)
{
        const GSGFValue *value = gsgf_node_get_property_value (node, "HO");
        GSGFDoubleEnum double_value;
        GSGFDoubleEnum expect = 2;

        if (!value) {
                fprintf (stderr, "No property 'HO'!\n");
                return FALSE;
        }

        if (!GSGF_IS_DOUBLE (value)) {
                fprintf (stderr, "Property 'HO' is not a GSGFDouble!\n");
                return FALSE;
        }

        double_value = gsgf_double_get_value (GSGF_DOUBLE (value));
        if (expect != double_value) {
                fprintf(stderr, "HO: Expected %d, not %d!\n",
                        expect, double_value);
                return FALSE;
        }

        return TRUE;
}

static gboolean
test_prop_N (const GSGFNode *node)
{
        const GSGFValue *value =
                        gsgf_node_get_property_value (node, "N");
        gchar *text_value;

        if (!value) {
                fprintf (stderr, "No property 'N'!\n");
                return FALSE;
        }

        if (!GSGF_IS_SIMPLE_TEXT (value)) {
                fprintf (stderr, "Property 'N' is not a GSGFSimpleText!\n");
                return FALSE;
        }

        text_value = gsgf_text_get_value (GSGF_TEXT (value));
#define EXPECT2 "Node annotation properties"
        if (strcmp (EXPECT2, text_value)) {
                fprintf(stderr, "C: Expected '%s', not '%s'!\n",
                        EXPECT2, text_value);
                return FALSE;
        }

        return TRUE;
}

static gboolean
test_prop_UC (const GSGFNode *node)
{
        const GSGFValue *value =
                        gsgf_node_get_property_value (node, "UC");
        GSGFDoubleEnum double_value;
        GSGFDoubleEnum expect = 1;

        if (!value) {
                fprintf (stderr, "No property 'UC'!\n");
                return FALSE;
        }

        if (!GSGF_IS_DOUBLE (value)) {
                fprintf (stderr, "Property 'UC' is not a GSGFDouble!\n");
                return FALSE;
        }

        double_value = gsgf_double_get_value (GSGF_DOUBLE (value));
        if (expect != double_value) {
                fprintf(stderr, "UC: Expected %d, not %d!\n", expect,
                        double_value);
                return FALSE;
        }

        return TRUE;
}

static gboolean
test_prop_V (const GSGFNode *node)
{
        const GSGFValue *value =
                        gsgf_node_get_property_value (node, "V");
        gdouble double_value;

        if (!value) {
                fprintf (stderr, "No property 'V'!\n");
                return FALSE;
        }

        if (!GSGF_IS_REAL (value)) {
                fprintf (stderr, "Property 'V' is not a GSGFReal!\n");
                return FALSE;
        }

        double_value = gsgf_real_get_value (GSGF_REAL (value));
        if (double_value < 1.733 || double_value > 1.735) {
                fprintf(stderr, "V: Expected 1.734, not '%g'!\n", double_value);
                return FALSE;
        }

        return TRUE;
}
