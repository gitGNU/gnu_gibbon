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

char *filename = "move-annotation.sgf";

static gboolean test_unique_position_BM (void);
static gboolean test_unique_position_DO (void);
static gboolean test_unique_position_IT (void);
static gboolean test_unique_position_TE (void);
static gboolean test_discard_properties (void);
static gboolean test_prop_BM (const GSGFNode *node);
static gboolean test_prop_DO (const GSGFNode *node);
static gboolean test_prop_IT (const GSGFNode *node);
static gboolean test_prop_TE (const GSGFNode *node);

int 
test_collection (GSGFCollection *collection, GError *error)
{
        GList *game_trees;
        GSGFGameTree *game_tree;
        GList *nodes;
        gpointer item;
        GSGFNode *node;
        gint retval = 0;

        if (!test_unique_position_BM ())
                retval = -1;
        if (!test_unique_position_DO ())
                retval = -1;
        if (!test_unique_position_IT ())
                retval = -1;
        if (!test_unique_position_TE ())
                retval = -1;
        if (!test_discard_properties ())
                retval = -1;

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
        if (!test_prop_BM (node))
                return -1;

        item = g_list_nth_data (nodes, 2);
        if (!item) {
                g_printerr ("Property #2 not found.\n");
                return -1;
        }
        node = GSGF_NODE (item);
        if (!test_prop_DO (node))
                return -1;

        item = g_list_nth_data (nodes, 3);
        if (!item) {
                g_printerr ("Property #3 not found.\n");
                return -1;
        }
        node = GSGF_NODE (item);
        if (!test_prop_IT (node))
                return -1;

        item = g_list_nth_data (nodes, 4);
        if (!item) {
                g_printerr ("Property #4 not found.\n");
                return -1;
        }
        node = GSGF_NODE (item);
        if (!test_prop_TE (node))
                return -1;

        return retval;
}

static gboolean
test_unique_position_BM (void)
{
        GError *expect1 = NULL;
        GError *expect2 = NULL;

        g_set_error (&expect1, GSGF_ERROR, GSGF_ERROR_SEMANTIC_ERROR,
                     "Property 'BM': The properties 'BM', 'DO', 'IT', and 'TE'"
                     " are mutually exclusive within one node");
        g_set_error (&expect2, GSGF_ERROR, GSGF_ERROR_SEMANTIC_ERROR,
                     "Property 'DO': The properties 'BM', 'DO', 'IT', and 'TE'"
                     " are mutually exclusive within one node");
        if (!expect_errors_from_sgf ("(;GM[6];B[31hefe]BM[1]DO[])",
                                     expect1, expect2))
                return FALSE;

        expect1 = NULL;
        expect2 = NULL;
        g_set_error (&expect1, GSGF_ERROR, GSGF_ERROR_SEMANTIC_ERROR,
                     "Property 'BM': The properties 'BM', 'DO', 'IT', and 'TE'"
                     " are mutually exclusive within one node");
        g_set_error (&expect2, GSGF_ERROR, GSGF_ERROR_SEMANTIC_ERROR,
                     "Property 'IT': The properties 'BM', 'DO', 'IT', and 'TE'"
                     " are mutually exclusive within one node");
        if (!expect_errors_from_sgf ("(;GM[6];B[31hefe]BM[1]IT[2])",
                                     expect1, expect2))
                return FALSE;

        expect1 = NULL;
        expect2 = NULL;
        g_set_error (&expect1, GSGF_ERROR, GSGF_ERROR_SEMANTIC_ERROR,
                     "Property 'BM': The properties 'BM', 'DO', 'IT', and 'TE'"
                     " are mutually exclusive within one node");
        g_set_error (&expect2, GSGF_ERROR, GSGF_ERROR_SEMANTIC_ERROR,
                     "Property 'TE': The properties 'BM', 'DO', 'IT', and 'TE'"
                     " are mutually exclusive within one node");
        if (!expect_errors_from_sgf ("(;GM[6];B[31hefe]BM[1]TE[2])",
                                     expect1, expect2))
                return FALSE;

        return TRUE;
}

static gboolean
test_unique_position_DO (void)
{
        GError *expect1 = NULL;
        GError *expect2 = NULL;

        g_set_error (&expect1, GSGF_ERROR, GSGF_ERROR_SEMANTIC_ERROR,
                     "Property 'DO': The properties 'BM', 'DO', 'IT', and 'TE'"
                     " are mutually exclusive within one node");
        g_set_error (&expect2, GSGF_ERROR, GSGF_ERROR_SEMANTIC_ERROR,
                     "Property 'BM': The properties 'BM', 'DO', 'IT', and 'TE'"
                     " are mutually exclusive within one node");
        if (!expect_errors_from_sgf ("(;GM[6];B[31hefe]DO[]BM[2])",
                                     expect1, expect2))
                return FALSE;

        expect1 = NULL;
        expect2 = NULL;
        g_set_error (&expect1, GSGF_ERROR, GSGF_ERROR_SEMANTIC_ERROR,
                     "Property 'DO': The properties 'BM', 'DO', 'IT', and 'TE'"
                     " are mutually exclusive within one node");
        g_set_error (&expect2, GSGF_ERROR, GSGF_ERROR_SEMANTIC_ERROR,
                     "Property 'IT': The properties 'BM', 'DO', 'IT', and 'TE'"
                     " are mutually exclusive within one node");
        if (!expect_errors_from_sgf ("(;GM[6];B[31hefe]DO[]IT[2])",
                                     expect1, expect2))
                return FALSE;

        expect1 = NULL;
        expect2 = NULL;
        g_set_error (&expect1, GSGF_ERROR, GSGF_ERROR_SEMANTIC_ERROR,
                     "Property 'DO': The properties 'BM', 'DO', 'IT', and 'TE'"
                     " are mutually exclusive within one node");
        g_set_error (&expect2, GSGF_ERROR, GSGF_ERROR_SEMANTIC_ERROR,
                     "Property 'TE': The properties 'BM', 'DO', 'IT', and 'TE'"
                     " are mutually exclusive within one node");
        if (!expect_errors_from_sgf ("(;GM[6];B[31hefe]DO[]TE[2])",
                                     expect1, expect2))
                return FALSE;

        return TRUE;
}

static gboolean
test_unique_position_IT (void)
{
        GError *expect1 = NULL;
        GError *expect2 = NULL;

        g_set_error (&expect1, GSGF_ERROR, GSGF_ERROR_SEMANTIC_ERROR,
                     "Property 'IT': The properties 'BM', 'DO', 'IT', and 'TE'"
                     " are mutually exclusive within one node");
        g_set_error (&expect2, GSGF_ERROR, GSGF_ERROR_SEMANTIC_ERROR,
                     "Property 'BM': The properties 'BM', 'DO', 'IT', and 'TE'"
                     " are mutually exclusive within one node");
        if (!expect_errors_from_sgf ("(;GM[6];B[31hefe]IT[1]BM[2])",
                                     expect1, expect2))
                return FALSE;

        expect1 = NULL;
        expect2 = NULL;
        g_set_error (&expect1, GSGF_ERROR, GSGF_ERROR_SEMANTIC_ERROR,
                     "Property 'IT': The properties 'BM', 'DO', 'IT', and 'TE'"
                     " are mutually exclusive within one node");
        g_set_error (&expect2, GSGF_ERROR, GSGF_ERROR_SEMANTIC_ERROR,
                     "Property 'DO': The properties 'BM', 'DO', 'IT', and 'TE'"
                     " are mutually exclusive within one node");
        if (!expect_errors_from_sgf ("(;GM[6];B[31hefe]IT[1]DO[])",
                                     expect1, expect2))
                return FALSE;

        expect1 = NULL;
        expect2 = NULL;
        g_set_error (&expect1, GSGF_ERROR, GSGF_ERROR_SEMANTIC_ERROR,
                     "Property 'IT': The properties 'BM', 'DO', 'IT', and 'TE'"
                     " are mutually exclusive within one node");
        g_set_error (&expect2, GSGF_ERROR, GSGF_ERROR_SEMANTIC_ERROR,
                     "Property 'TE': The properties 'BM', 'DO', 'IT', and 'TE'"
                     " are mutually exclusive within one node");
        if (!expect_errors_from_sgf ("(;GM[6];B[31hefe]IT[1]TE[2])",
                                     expect1, expect2))
                return FALSE;

        return TRUE;
}

static gboolean
test_unique_position_TE (void)
{
        GError *expect1 = NULL;
        GError *expect2 = NULL;

        g_set_error (&expect1, GSGF_ERROR, GSGF_ERROR_SEMANTIC_ERROR,
                     "Property 'TE': The properties 'BM', 'DO', 'IT', and 'TE'"
                     " are mutually exclusive within one node");
        g_set_error (&expect2, GSGF_ERROR, GSGF_ERROR_SEMANTIC_ERROR,
                     "Property 'BM': The properties 'BM', 'DO', 'IT', and 'TE'"
                     " are mutually exclusive within one node");
        if (!expect_errors_from_sgf ("(;GM[6];B[31hefe]TE[1]BM[2])",
                                     expect1, expect2))
                return FALSE;

        expect1 = NULL;
        expect2 = NULL;
        g_set_error (&expect1, GSGF_ERROR, GSGF_ERROR_SEMANTIC_ERROR,
                     "Property 'TE': The properties 'BM', 'DO', 'IT', and 'TE'"
                     " are mutually exclusive within one node");
        g_set_error (&expect2, GSGF_ERROR, GSGF_ERROR_SEMANTIC_ERROR,
                     "Property 'DO': The properties 'BM', 'DO', 'IT', and 'TE'"
                     " are mutually exclusive within one node");
        if (!expect_errors_from_sgf ("(;GM[6];B[31hefe]TE[1]DO[])",
                                     expect1, expect2))
                return FALSE;

        expect1 = NULL;
        expect2 = NULL;
        g_set_error (&expect1, GSGF_ERROR, GSGF_ERROR_SEMANTIC_ERROR,
                     "Property 'TE': The properties 'BM', 'DO', 'IT', and 'TE'"
                     " are mutually exclusive within one node");
        g_set_error (&expect2, GSGF_ERROR, GSGF_ERROR_SEMANTIC_ERROR,
                     "Property 'IT': The properties 'BM', 'DO', 'IT', and 'TE'"
                     " are mutually exclusive within one node");
        if (!expect_errors_from_sgf ("(;GM[6];B[31hefe]TE[1]IT[2])",
                                     expect1, expect2))
                return FALSE;

        return TRUE;
}

static gboolean
test_discard_property (const gchar *sgf, const gchar *id)
{
        GSGFCollection *collection;
        GError *error;
        GList *game_trees;
        GSGFGameTree *game_tree;
        GList *nodes;
        gpointer item;
        GSGFNode *node;

        error = NULL;
        collection = parse_memory (sgf, &error);
        if (!collection) {
                g_printerr ("Error parsing '%s': %s.\n",
                            sgf, error->message);
                return FALSE;
        }

        game_trees = gsgf_collection_get_game_trees (collection);
        if (!game_trees) {
                g_printerr ("'%s': No game trees found.\n", sgf);
                return FALSE;
        }
        game_tree = GSGF_GAME_TREE (game_trees->data);

        nodes = gsgf_game_tree_get_nodes (game_tree);
        if (!nodes) {
                g_printerr ("'%s': No nodes found.\n", sgf);
                return FALSE;
        }

        item = g_list_nth_data (nodes, 1);
        if (!item) {
                g_printerr ("'%s': Property #1 not found.\n", sgf);
                return FALSE;
        }
        node = GSGF_NODE (item);

        if (gsgf_node_get_property (node, id)) {
                g_printerr ("'%s': Property '%s' was not discarded!\n", sgf, id);
                return FALSE;
        }

        return TRUE;
}

static gboolean
test_discard_properties (void)
{
        gboolean retval = TRUE;

        if (!test_discard_property ("(;GM[6];N[Nonsense]BM[1])", "BM"))
                retval = FALSE;
        if (!test_discard_property ("(;GM[6];N[Nonsense]DO[])", "DO"))
                retval = FALSE;
        if (!test_discard_property ("(;GM[6];N[Nonsense]IT[1])", "IT"))
                retval = FALSE;
        if (!test_discard_property ("(;GM[6];N[Nonsense]TE[1])", "TE"))
                retval = FALSE;

        return retval;
}

static gboolean
test_prop_BM (const GSGFNode *node)
{
        const GSGFValue *value = gsgf_node_get_property_value (node, "BM");
        GSGFDoubleEnum double_value;
        GSGFDoubleEnum expect = 1;

        if (!value) {
                g_printerr ("No property 'BM'!\n");
                return FALSE;
        }

        if (!GSGF_IS_DOUBLE (value)) {
                g_printerr ("Property 'BM' is not a GSGFDouble!\n");
                return FALSE;
        }

        double_value = gsgf_double_get_value (GSGF_DOUBLE (value));
        if (expect != double_value) {
                g_printerr ("BM: Expected %d, not %d!\n", expect, double_value);
                return FALSE;
        }

        return TRUE;
}

static gboolean
test_prop_DO (const GSGFNode *node)
{
        const GSGFValue *value =
                        gsgf_node_get_property_value (node, "DO");

        if (!value) {
                g_printerr ("No property 'DO'!\n");
                return FALSE;
        }

        if (!GSGF_IS_EMPTY (value)) {
                g_printerr ("Property 'DO' is not a GSGFDouble!\n");
                return FALSE;
        }

        return TRUE;
}

static gboolean
test_prop_IT (const GSGFNode *node)
{
        const GSGFValue *value =
                        gsgf_node_get_property_value (node, "IT");
        GSGFDoubleEnum double_value;
        GSGFDoubleEnum expect = 1;

        if (!value) {
                g_printerr ("No property 'IT'!\n");
                return FALSE;
        }

        if (!GSGF_IS_DOUBLE (value)) {
                g_printerr ("Property 'IT' is not a GSGFDouble!\n");
                return FALSE;
        }

        double_value = gsgf_double_get_value (GSGF_DOUBLE (value));
        if (expect != double_value) {
                g_printerr ("IT: Expected %d, not %d!\n", expect, double_value);
                return FALSE;
        }

        return TRUE;
}

static gboolean
test_prop_TE (const GSGFNode *node)
{
        const GSGFValue *value =
                        gsgf_node_get_property_value (node, "TE");
        GSGFDoubleEnum double_value;
        GSGFDoubleEnum expect = 2;

        if (!value) {
                g_printerr ("No property 'TE'!\n");
                return FALSE;
        }

        if (!GSGF_IS_DOUBLE (value)) {
                g_printerr ("Property 'TE' is not a GSGFDouble!\n");
                return FALSE;
        }

        double_value = gsgf_double_get_value (GSGF_DOUBLE (value));
        if (expect != double_value) {
                g_printerr ("TE: Expected %d, not %d!\n", expect, double_value);
                return FALSE;
        }

        return TRUE;
}
