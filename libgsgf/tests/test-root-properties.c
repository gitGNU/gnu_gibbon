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

char *filename = "root-properties.sgf";

static gboolean test_prop_AP(const GSGFNode *node);
static gboolean test_prop_CA(const GSGFNode *node);
static gboolean test_prop_FF(const GSGFNode *node);
static gboolean test_prop_GM(const GSGFNode *node);
static gboolean test_prop_ST(const GSGFNode *node);
static gboolean test_prop_SZ(const GSGFNode *node);
static gboolean test_prop_XY(const GSGFNode *node);

int 
test_collection(GSGFCollection *collection, GError *error)
{
        GList *game_trees;
        GSGFGameTree *game_tree;
        GList *nodes;
        GSGFNode *root_node;

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
        root_node = GSGF_NODE(nodes->data);

        if (!test_prop_AP(root_node))
                return -1;

        if (!test_prop_CA(root_node))
                return -1;

        if (!test_prop_FF(root_node))
                return -1;

        if (!test_prop_GM(root_node))
                return -1;

        if (!test_prop_ST(root_node))
                return -1;

        if (!test_prop_SZ(root_node))
                return -1;

        if (!test_prop_XY(root_node))
                return -1;

        return expect_error(error, NULL);
}

static gboolean
test_prop_AP(const GSGFNode *node)
{
        const GSGFValue *value = gsgf_node_get_property_value(node, "AP");
        gchar *text_value;
        gsize length;
        const GSGFCookedValue *subvalue;

        if (!value) {
                fprintf(stderr, "No root property 'AP'!\n");
                return FALSE;
        }

        if (!GSGF_IS_COMPOSE(value)) {
                fprintf(stderr, "Root property 'AP' is not a GSGFCompose!\n");
                return FALSE;
        }

        length = gsgf_compose_get_number_of_values(GSGF_COMPOSE(value));
        if (length != 2) {
                fprintf(stderr, "Property 'AP': Expected 2 subvalues, got"
                        " %llu.\n", (unsigned long long) length);
                return FALSE;
        }

        subvalue = gsgf_compose_get_value(GSGF_COMPOSE(value), 0);
        if (!subvalue) {
                fprintf(stderr, "No first 'AP' subvalue!\n");
                return FALSE;
        }
        if (!GSGF_IS_SIMPLE_TEXT(subvalue)) {
                fprintf(stderr, "First 'AP' subvalue is not a GSGFSimpleText!\n");
                return FALSE;
        }
        text_value = gsgf_text_get_value(GSGF_TEXT(subvalue));
        if (strcmp("foobar", text_value)) {
                fprintf(stderr, "Expected 'foobar', not '%s'!\n", text_value);
                return FALSE;
        }

        subvalue = gsgf_compose_get_value(GSGF_COMPOSE(value), 1);
        if (!subvalue) {
                fprintf(stderr, "No second 'AP' subvalue!\n");
                return FALSE;
        }
        if (!GSGF_IS_SIMPLE_TEXT(subvalue)) {
                fprintf(stderr, "Second 'AP' subvalue is not a GSGFSimpleText!\n");
                return FALSE;
        }
        text_value = gsgf_text_get_value(GSGF_TEXT(subvalue));
        if (strcmp("1.2.3", text_value)) {
                fprintf(stderr, "Expected '1.2.3', not '%s'!\n", text_value);
                return FALSE;
        }

        return TRUE;
}

static gboolean
test_prop_CA(const GSGFNode *node)
{
        const GSGFValue *value = gsgf_node_get_property_value(node, "CA");
        gchar *text_value;

        if (!value) {
                fprintf(stderr, "No root property 'CA'!\n");
                return FALSE;
        }

        if (!GSGF_IS_SIMPLE_TEXT(value)) {
                fprintf(stderr, "Root property 'CA' is not a GSGFSimpleText!\n");
                return FALSE;
        }

        text_value = gsgf_text_get_value(GSGF_TEXT(value));
        if (strcmp ("UTF-8", text_value)) {
                fprintf(stderr, "CA: Expected 'UTF-8', got '%s'!\n",
                        text_value);
                return FALSE;
        }

        return TRUE;
}

static gboolean
test_prop_FF(const GSGFNode *node)
{
        const GSGFValue *value = gsgf_node_get_property_value(node, "FF");
        gint int_value;

        if (!value) {
                fprintf(stderr, "No root property 'FF'!\n");
                return FALSE;
        }

        if (!GSGF_IS_NUMBER(value)) {
                fprintf(stderr, "Root property 'FF' is not a GSGFNumber!\n");
                return FALSE;
        }

        int_value = gsgf_number_get_value(GSGF_NUMBER(value));
        if (4 != int_value) {
                fprintf(stderr, "FF: Expected 4, got %d!\n", int_value);
                return FALSE;
        }

        return TRUE;
}

static gboolean
test_prop_GM(const GSGFNode *node)
{
        const GSGFValue *value = gsgf_node_get_property_value(node, "GM");
        gint int_value;

        if (!value) {
                fprintf(stderr, "No root property 'GM'!\n");
                return FALSE;
        }

        if (!GSGF_IS_NUMBER(value)) {
                fprintf(stderr, "Root property 'GM' is not a GSGFNumber!\n");
                return FALSE;
        }

        int_value = gsgf_number_get_value(GSGF_NUMBER(value));
        if (6 != int_value) {
                fprintf(stderr, "GM: Expected 6, got %d!\n", int_value);
                return FALSE;
        }

        return TRUE;
}

static gboolean
test_prop_ST(const GSGFNode *node)
{
        const GSGFValue *value = gsgf_node_get_property_value(node, "ST");
        gint int_value;

        if (!value) {
                fprintf(stderr, "No root property 'ST'!\n");
                return FALSE;
        }

        if (!GSGF_IS_NUMBER(value)) {
                fprintf(stderr, "Root property 'ST' is not a GSGFNumber!\n");
                return FALSE;
        }

        int_value = gsgf_number_get_value(GSGF_NUMBER(value));
        if (3 != int_value) {
                fprintf(stderr, "GM: Expected 3, got %d!\n", int_value);
                return FALSE;
        }

        return TRUE;
}

static gboolean
test_prop_SZ(const GSGFNode *node)
{
        const GSGFValue *value = gsgf_node_get_property_value(node, "SZ");
        gint64 int_value;
        gsize length;
        const GSGFCookedValue *subvalue;

        if (!value) {
                fprintf(stderr, "No root property 'SZ'!\n");
                return FALSE;
        }

        if (!GSGF_IS_COMPOSE(value)) {
                fprintf(stderr, "Root property 'SZ' is not a GSGFCompose ");
                fprintf(stderr, "but a '%s'.\n", G_OBJECT_TYPE_NAME (value));
                return FALSE;
        }

        length = gsgf_compose_get_number_of_values(GSGF_COMPOSE(value));
        if (length != 2) {
                fprintf(stderr, "Property 'SZ': Expected 2 subvalues, got"
                       " %llu.\n", (unsigned long long) length);
                return FALSE;
        }

        subvalue = gsgf_compose_get_value(GSGF_COMPOSE(value), 0);
        if (!subvalue) {
                fprintf(stderr, "No first 'SZ' subvalue!\n");
                return FALSE;
        }
        if (!GSGF_IS_NUMBER(subvalue)) {
                fprintf(stderr, "First 'SZ' subvalue is not a GSGFNumber!\n");
                return FALSE;
        }
        int_value = gsgf_number_get_value(GSGF_NUMBER(subvalue));
        if (24 != int_value) {
                fprintf(stderr, "Expected 24, not %lld!\n",
                        (long long) int_value);
                return FALSE;
        }

        subvalue = gsgf_compose_get_value(GSGF_COMPOSE(value), 1);
        if (!subvalue) {
                fprintf(stderr, "No second 'SZ' subvalue!\n");
                return FALSE;
        }
        if (!GSGF_IS_NUMBER(subvalue)) {
                fprintf(stderr, "Second 'SZ' subvalue is not a GSGFNumber!\n");
                return FALSE;
        }
        int_value = gsgf_number_get_value(GSGF_NUMBER(subvalue));
        if (1 != int_value) {
                fprintf(stderr, "Expected 1, not %lld!\n",
                        (long long) int_value);
                return FALSE;
        }

        return TRUE;
}

static gboolean
test_prop_XY(const GSGFNode *node)
{
        const GSGFValue *value = gsgf_node_get_property_value(node, "XY");
        const gchar *raw_value;
        gsize length;

        if (!value) {
                fprintf(stderr, "No root property 'XY'!\n");
                return FALSE;
        }

        if (!GSGF_IS_RAW(value)) {
                fprintf(stderr, "Root property 'XY' is not a GSGFRaw!\n");
                return FALSE;
        }

        length = gsgf_raw_get_number_of_values(GSGF_RAW(value));
        if (length != 2) {
                fprintf(stderr, "Expected 2 values for 'XY', got %llu.\n",
                        (unsigned long long) length);
                return FALSE;
        }

        raw_value = gsgf_raw_get_value(GSGF_RAW(value), 0);
        if (strcmp("Proprietary property #1", raw_value)) {
                fprintf(stderr, "Expected 'Proprietary property #1', got '%s'!\n",
                        raw_value);
                return FALSE;
        }

        return TRUE;
}

