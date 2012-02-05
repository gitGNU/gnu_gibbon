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

char *filename = "timing-properties.sgf";

static gboolean test_prop_BL (const GSGFNode *node);
static gboolean test_prop_OB (const GSGFNode *node);
static gboolean test_prop_OW (const GSGFNode *node);
static gboolean test_prop_WL (const GSGFNode *node);

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

        item = g_list_nth_data (nodes, 0);
        if (!item) {
                g_printerr ("Property #0 not found.\n");
                return -1;
        }
        node = GSGF_NODE (item);

        if (!test_prop_BL (node))
                retval = -1;
        if (!test_prop_OB (node))
                retval = -1;
        if (!test_prop_OW (node))
                retval = -1;
        if (!test_prop_WL (node))
                retval = -1;

        return retval;
}

static gboolean
test_prop_BL (const GSGFNode *node)
{
        const GSGFValue *value =
                        gsgf_node_get_property_value (node, "BL");
        gdouble double_value;
        gdouble expect = 1.1;

        if (!value) {
                g_printerr ("No property 'BL'!\n");
                return FALSE;
        }

        if (!GSGF_IS_REAL (value)) {
                g_printerr ("Property 'BL' is not a GSGFReal!\n");
                return FALSE;
        }

        double_value = gsgf_real_get_value (GSGF_REAL (value));
        if (((expect - double_value) < -0.1)
             || ((expect - double_value) > 0.1)) {
                g_printerr ("BL: Expected %f, not %f!\n",
                            expect, double_value);
                return FALSE;
        }

        return TRUE;
}

static gboolean
test_prop_OB (const GSGFNode *node)
{
        const GSGFValue *value =
                        gsgf_node_get_property_value (node, "OB");
        gdouble double_value;
        gdouble expect = 2.2;

        if (!value) {
                g_printerr ("No property 'OB'!\n");
                return FALSE;
        }

        if (!GSGF_IS_REAL (value)) {
                g_printerr ("Property 'OB' is not a GSGFReal!\n");
                return FALSE;
        }

        double_value = gsgf_real_get_value (GSGF_REAL (value));
        if (((expect - double_value) < -0.1)
             || ((expect - double_value) > 0.1)) {
                g_printerr ("OB: Expected %f, not %f!\n",
                            expect, double_value);
                return FALSE;
        }

        return TRUE;
}

static gboolean
test_prop_OW (const GSGFNode *node)
{
        const GSGFValue *value =
                        gsgf_node_get_property_value (node, "OW");
        gdouble double_value;
        gdouble expect = 3.3;

        if (!value) {
                g_printerr ("No property 'OW'!\n");
                return FALSE;
        }

        if (!GSGF_IS_REAL (value)) {
                g_printerr ("Property 'OW' is not a GSGFReal!\n");
                return FALSE;
        }

        double_value = gsgf_real_get_value (GSGF_REAL (value));
        if (((expect - double_value) < -0.1)
             || ((expect - double_value) > 0.1)) {
                g_printerr ("OW: Expected %f, not %f!\n",
                            expect, double_value);
                return FALSE;
        }

        return TRUE;
}

static gboolean
test_prop_WL (const GSGFNode *node)
{
        const GSGFValue *value =
                        gsgf_node_get_property_value (node, "WL");
        gdouble double_value;
        gdouble expect = 4.4;

        if (!value) {
                g_printerr ("No property 'WL'!\n");
                return FALSE;
        }

        if (!GSGF_IS_REAL (value)) {
                g_printerr ("Property 'WL' is not a GSGFReal!\n");
                return FALSE;
        }

        double_value = gsgf_real_get_value (GSGF_REAL (value));
        if (((expect - double_value) < -0.1)
             || ((expect - double_value) > 0.1)) {
                g_printerr ("WL: Expected %f, not %f!\n",
                            expect, double_value);
                return FALSE;
        }

        return TRUE;
}
