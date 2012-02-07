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

char *filename = "game-info-properties.sgf";

static gboolean test_prop_AN (const GSGFNode *node);
static gboolean test_prop_BR (const GSGFNode *node);
static gboolean test_prop_BT (const GSGFNode *node);
static gboolean test_prop_CP (const GSGFNode *node);
static gboolean test_prop_DT (const GSGFNode *node);
static gboolean test_prop_EV (const GSGFNode *node);
static gboolean test_prop_GC (const GSGFNode *node);
static gboolean test_prop_GN (const GSGFNode *node);
static gboolean test_prop_ON (const GSGFNode *node);
static gboolean test_prop_OT (const GSGFNode *node);
static gboolean test_prop_PB (const GSGFNode *node);
static gboolean test_prop_PC (const GSGFNode *node);
static gboolean test_prop_PW (const GSGFNode *node);
static gboolean test_prop_RE (const GSGFNode *node);
static gboolean test_prop_RO (const GSGFNode *node);
static gboolean test_prop_RU (const GSGFNode *node);
static gboolean test_prop_SO (const GSGFNode *node);
static gboolean test_prop_TM (const GSGFNode *node);
static gboolean test_prop_US (const GSGFNode *node);
static gboolean test_prop_WR (const GSGFNode *node);
static gboolean test_prop_WT (const GSGFNode *node);

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
        if (!test_prop_AN (node))
                retval = -1;
        if (!test_prop_BR (node))
                retval = -1;
        if (!test_prop_BT (node))
                retval = -1;
        if (!test_prop_CP (node))
                retval = -1;
        if (!test_prop_DT (node))
                retval = -1;
        if (!test_prop_EV (node))
                retval = -1;
        if (!test_prop_GN (node))
                retval = -1;
        if (!test_prop_GC (node))
                retval = -1;
        if (!test_prop_ON (node))
                retval = -1;
        if (!test_prop_OT (node))
                retval = -1;
        if (!test_prop_PB (node))
                retval = -1;
        if (!test_prop_PC (node))
                retval = -1;
        if (!test_prop_PW (node))
                retval = -1;
        if (!test_prop_RE (node))
                retval = -1;
        if (!test_prop_RO (node))
                retval = -1;
        if (!test_prop_RU (node))
                retval = -1;
        if (!test_prop_SO (node))
                retval = -1;
        if (!test_prop_TM (node))
                retval = -1;
        if (!test_prop_US (node))
                retval = -1;
        if (!test_prop_WR (node))
                retval = -1;
        if (!test_prop_WT (node))
                retval = -1;

        return retval;
}

static gboolean
test_prop_AN (const GSGFNode *node)
{
        const GSGFValue *value = gsgf_node_get_property_value (node, "AN");
        const gchar *expect = "Karl Dapp der Abwaschbare";
        const gchar *got;

        if (!value) {
                g_printerr ("No property 'AN'!\n");
                return FALSE;
        }

        if (!GSGF_IS_SIMPLE_TEXT (value)) {
                g_printerr ("Property 'AN' is not a GSGFSimpleText!\n");
                return FALSE;
        }

        got = gsgf_text_get_value (GSGF_TEXT (value));
        if (g_strcmp0 (expect, got)) {
                g_printerr ("Property 'AN': Expected '%s', got '%s'!\n",
                             expect, got);
                return FALSE;
        }

        return TRUE;
}

static gboolean
test_prop_BR (const GSGFNode *node)
{
        const GSGFValue *value = gsgf_node_get_property_value (node, "BR");
        const gchar *expect = "rookie";
        const gchar *got;

        if (!value) {
                g_printerr ("No property 'BR'!\n");
                return FALSE;
        }

        if (!GSGF_IS_SIMPLE_TEXT (value)) {
                g_printerr ("Property 'BR' is not a GSGFSimpleText!\n");
                return FALSE;
        }

        got = gsgf_text_get_value (GSGF_TEXT (value));
        if (g_strcmp0 (expect, got)) {
                g_printerr ("Property 'BR': Expected '%s', got '%s'!\n",
                             expect, got);
                return FALSE;
        }

        return TRUE;
}

static gboolean
test_prop_BT (const GSGFNode *node)
{
        const GSGFValue *value = gsgf_node_get_property_value (node, "BT");
        const gchar *expect = "Martians";
        const gchar *got;

        if (!value) {
                g_printerr ("No property 'BT'!\n");
                return FALSE;
        }

        if (!GSGF_IS_SIMPLE_TEXT (value)) {
                g_printerr ("Property 'BT' is not a GSGFSimpleText!\n");
                return FALSE;
        }

        got = gsgf_text_get_value (GSGF_TEXT (value));
        if (g_strcmp0 (expect, got)) {
                g_printerr ("Property 'BT': Expected '%s', got '%s'!\n",
                             expect, got);
                return FALSE;
        }

        return TRUE;
}

static gboolean
test_prop_CP (const GSGFNode *node)
{
        const GSGFValue *value = gsgf_node_get_property_value (node, "CP");
        const gchar *expect = "copyleft";
        const gchar *got;

        if (!value) {
                g_printerr ("No property 'CP'!\n");
                return FALSE;
        }

        if (!GSGF_IS_SIMPLE_TEXT (value)) {
                g_printerr ("Property 'CP' is not a GSGFSimpleText!\n");
                return FALSE;
        }

        got = gsgf_text_get_value (GSGF_TEXT (value));
        if (g_strcmp0 (expect, got)) {
                g_printerr ("Property 'CP': Expected '%s', got '%s'!\n",
                             expect, got);
                return FALSE;
        }

        return TRUE;
}

static gboolean
test_prop_DT (const GSGFNode *node)
{
        const GSGFValue *value = gsgf_node_get_property_value (node, "DT");
        const gchar *expect = "2010-01-30,31,02-01";
        const gchar *got;

        if (!value) {
                g_printerr ("No property 'DT'!\n");
                return FALSE;
        }

        if (!GSGF_IS_DATE (value)) {
                g_printerr ("Property 'DT' is not a GSGFDate!\n");
                return FALSE;
        }

        got = gsgf_text_get_value (GSGF_TEXT (value));
        if (g_strcmp0 (expect, got)) {
                g_printerr ("Property 'DT': Expected '%s', got '%s'!\n",
                             expect, got);
                return FALSE;
        }

        return TRUE;
}

static gboolean
test_prop_EV (const GSGFNode *node)
{
        const GSGFValue *value = gsgf_node_get_property_value (node, "EV");
        const gchar *expect = "Walpurgisnacht";
        const gchar *got;

        if (!value) {
                g_printerr ("No property 'EV'!\n");
                return FALSE;
        }

        if (!GSGF_IS_SIMPLE_TEXT (value)) {
                g_printerr ("Property 'EV' is not a GSGFSimpleText!\n");
                return FALSE;
        }

        got = gsgf_text_get_value (GSGF_TEXT (value));
        if (g_strcmp0 (expect, got)) {
                g_printerr ("Property 'EV': Expected '%s', got '%s'!\n",
                             expect, got);
                return FALSE;
        }

        return TRUE;
}

static gboolean
test_prop_GC (const GSGFNode *node)
{
        const GSGFValue *value = gsgf_node_get_property_value (node, "GC");
        const gchar *expect = "This game really shook the world of backgammon!";
        const gchar *got;

        if (!value) {
                g_printerr ("No property 'GC'!\n");
                return FALSE;
        }

        if (!GSGF_IS_TEXT (value)) {
                g_printerr ("Property 'GC' is not a GSGFText!\n");
                return FALSE;
        }

        got = gsgf_text_get_value (GSGF_TEXT (value));
        if (g_strcmp0 (expect, got)) {
                g_printerr ("Property 'GC': Expected '%s', got '%s'!\n",
                             expect, got);
                return FALSE;
        }

        return TRUE;
}

static gboolean
test_prop_GN (const GSGFNode *node)
{
        const GSGFValue *value = gsgf_node_get_property_value (node, "GN");
        const gchar *expect = "Game of the century";
        const gchar *got;

        if (!value) {
                g_printerr ("No property 'GN'!\n");
                return FALSE;
        }

        if (!GSGF_IS_SIMPLE_TEXT (value)) {
                g_printerr ("Property 'GN' is not a GSGFSimpleText!\n");
                return FALSE;
        }

        got = gsgf_text_get_value (GSGF_TEXT (value));
        if (g_strcmp0 (expect, got)) {
                g_printerr ("Property 'GN': Expected '%s', got '%s'!\n",
                             expect, got);
                return FALSE;
        }

        return TRUE;
}

static gboolean
test_prop_ON (const GSGFNode *node)
{
        const GSGFValue *value = gsgf_node_get_property_value (node, "ON");
        const gchar *expect = "filibuster";
        const gchar *got;

        if (!value) {
                g_printerr ("No property 'ON'!\n");
                return FALSE;
        }

        if (!GSGF_IS_SIMPLE_TEXT (value)) {
                g_printerr ("Property 'ON' is not a GSGFSimpleText!\n");
                return FALSE;
        }

        got = gsgf_text_get_value (GSGF_TEXT (value));
        if (g_strcmp0 (expect, got)) {
                g_printerr ("Property 'ON': Expected '%s', got '%s'!\n",
                             expect, got);
                return FALSE;
        }

        return TRUE;
}

static gboolean
test_prop_OT (const GSGFNode *node)
{
        const GSGFValue *value = gsgf_node_get_property_value (node, "OT");
        const gchar *expect = "unlimited";
        const gchar *got;

        if (!value) {
                g_printerr ("No property 'RO'!\n");
                return FALSE;
        }

        if (!GSGF_IS_SIMPLE_TEXT (value)) {
                g_printerr ("Property 'OT' is not a GSGFSimpleText!\n");
                return FALSE;
        }

        got = gsgf_text_get_value (GSGF_TEXT (value));
        if (g_strcmp0 (expect, got)) {
                g_printerr ("Property 'OT': Expected '%s', got '%s'!\n",
                             expect, got);
                return FALSE;
        }

        return TRUE;
}

static gboolean
test_prop_PB (const GSGFNode *node)
{
        const GSGFValue *value = gsgf_node_get_property_value (node, "PB");
        const gchar *expect = "Tuennes";
        const gchar *got;

        if (!value) {
                g_printerr ("No property 'PB'!\n");
                return FALSE;
        }

        if (!GSGF_IS_SIMPLE_TEXT (value)) {
                g_printerr ("Property 'PB' is not a GSGFSimpleText!\n");
                return FALSE;
        }

        got = gsgf_text_get_value (GSGF_TEXT (value));
        if (g_strcmp0 (expect, got)) {
                g_printerr ("Property 'PB': Expected '%s', got '%s'!\n",
                             expect, got);
                return FALSE;
        }

        return TRUE;
}

static gboolean
test_prop_PC (const GSGFNode *node)
{
        const GSGFValue *value = gsgf_node_get_property_value (node, "PC");
        const gchar *expect = "Venus";
        const gchar *got;

        if (!value) {
                g_printerr ("No property 'PC'!\n");
                return FALSE;
        }

        if (!GSGF_IS_SIMPLE_TEXT (value)) {
                g_printerr ("Property 'PC' is not a GSGFSimpleText!\n");
                return FALSE;
        }

        got = gsgf_text_get_value (GSGF_TEXT (value));
        if (g_strcmp0 (expect, got)) {
                g_printerr ("Property 'PC': Expected '%s', got '%s'!\n",
                             expect, got);
                return FALSE;
        }

        return TRUE;
}

static gboolean
test_prop_PW (const GSGFNode *node)
{
        const GSGFValue *value = gsgf_node_get_property_value (node, "PW");
        const gchar *expect = "Schaeael";
        const gchar *got;

        if (!value) {
                g_printerr ("No property 'PW'!\n");
                return FALSE;
        }

        if (!GSGF_IS_SIMPLE_TEXT (value)) {
                g_printerr ("Property 'PW' is not a GSGFSimpleText!\n");
                return FALSE;
        }

        got = gsgf_text_get_value (GSGF_TEXT (value));
        if (g_strcmp0 (expect, got)) {
                g_printerr ("Property 'PW': Expected '%s', got '%s'!\n",
                             expect, got);
                return FALSE;
        }

        return TRUE;
}

static gboolean
test_prop_RE (const GSGFNode *node)
{
        const GSGFValue *value = gsgf_node_get_property_value (node, "RE");
        const gchar *expect = "B+32";
        const gchar *got;

        if (!value) {
                g_printerr ("No property 'RE'!\n");
                return FALSE;
        }

        if (!GSGF_IS_RESULT (value)) {
                g_printerr ("Property 'RE' is not a GSGFResult!\n");
                return FALSE;
        }

        got = gsgf_text_get_value (GSGF_TEXT (value));
        if (g_strcmp0 (expect, got)) {
                g_printerr ("Property 'RE': Expected '%s', got '%s'!\n",
                             expect, got);
                return FALSE;
        }

        return TRUE;
}

static gboolean
test_prop_RO (const GSGFNode *node)
{
        const GSGFValue *value = gsgf_node_get_property_value (node, "RO");
        const gchar *expect = "finals";
        const gchar *got;

        if (!value) {
                g_printerr ("No property 'RO'!\n");
                return FALSE;
        }

        if (!GSGF_IS_SIMPLE_TEXT (value)) {
                g_printerr ("Property 'RO' is not a GSGFSimpleText!\n");
                return FALSE;
        }

        got = gsgf_text_get_value (GSGF_TEXT (value));
        if (g_strcmp0 (expect, got)) {
                g_printerr ("Property 'RO': Expected '%s', got '%s'!\n",
                             expect, got);
                return FALSE;
        }

        return TRUE;
}

static gboolean
test_prop_RU (const GSGFNode *node)
{
        const GSGFValue *value = gsgf_node_get_property_value (node, "RU");
        const gchar *expect = "Jacoby";
        const gchar *got;

        if (!value) {
                g_printerr ("No property 'RU'!\n");
                return FALSE;
        }

        if (!GSGF_IS_SIMPLE_TEXT (value)) {
                g_printerr ("Property 'RU' is not a GSGFSimpleText!\n");
                return FALSE;
        }

        got = gsgf_text_get_value (GSGF_TEXT (value));
        if (g_strcmp0 (expect, got)) {
                g_printerr ("Property 'RU': Expected '%s', got '%s'!\n",
                             expect, got);
                return FALSE;
        }

        return TRUE;
}

static gboolean
test_prop_SO (const GSGFNode *node)
{
        const GSGFValue *value = gsgf_node_get_property_value (node, "SO");
        const gchar *expect = "imagination";
        const gchar *got;

        if (!value) {
                g_printerr ("No property 'SO'!\n");
                return FALSE;
        }

        if (!GSGF_IS_SIMPLE_TEXT (value)) {
                g_printerr ("Property 'SO' is not a GSGFSimpleText!\n");
                return FALSE;
        }

        got = gsgf_text_get_value (GSGF_TEXT (value));
        if (g_strcmp0 (expect, got)) {
                g_printerr ("Property 'SO': Expected '%s', got '%s'!\n",
                             expect, got);
                return FALSE;
        }

        return TRUE;
}

static gboolean
test_prop_TM (const GSGFNode *node)
{
        const GSGFValue *value = gsgf_node_get_property_value (node, "TM");
        gdouble expect = 123;
        gdouble got;

        if (!value) {
                g_printerr ("No property 'TM'!\n");
                return FALSE;
        }

        if (!GSGF_IS_REAL (value)) {
                g_printerr ("Property 'TM' is not a GSGFReal!\n");
                return FALSE;
        }

        got = gsgf_real_get_value (GSGF_REAL (value));
        if (expect != got) {
                g_printerr ("Property 'TM': Expected '%g', got '%g'!\n",
                             expect, got);
                return FALSE;
        }

        return TRUE;
}

static gboolean
test_prop_US (const GSGFNode *node)
{
        const GSGFValue *value = gsgf_node_get_property_value (node, "US");
        const gchar *expect = "Gibbon";
        const gchar *got;

        if (!value) {
                g_printerr ("No property 'US'!\n");
                return FALSE;
        }

        if (!GSGF_IS_SIMPLE_TEXT (value)) {
                g_printerr ("Property 'US' is not a GSGFSimpleText!\n");
                return FALSE;
        }

        got = gsgf_text_get_value (GSGF_TEXT (value));
        if (g_strcmp0 (expect, got)) {
                g_printerr ("Property 'US': Expected '%s', got '%s'!\n",
                             expect, got);
                return FALSE;
        }

        return TRUE;
}

static gboolean
test_prop_WR (const GSGFNode *node)
{
        const GSGFValue *value = gsgf_node_get_property_value (node, "WR");
        const gchar *expect = "World champion";
        const gchar *got;

        if (!value) {
                g_printerr ("No property 'WR'!\n");
                return FALSE;
        }

        if (!GSGF_IS_SIMPLE_TEXT (value)) {
                g_printerr ("Property 'WR' is not a GSGFSimpleText!\n");
                return FALSE;
        }

        got = gsgf_text_get_value (GSGF_TEXT (value));
        if (g_strcmp0 (expect, got)) {
                g_printerr ("Property 'WR': Expected '%s', got '%s'!\n",
                             expect, got);
                return FALSE;
        }

        return TRUE;
}

static gboolean
test_prop_WT (const GSGFNode *node)
{
        const GSGFValue *value = gsgf_node_get_property_value (node, "WT");
        const gchar *expect = "Klingons";
        const gchar *got;

        if (!value) {
                g_printerr ("No property 'WT'!\n");
                return FALSE;
        }

        if (!GSGF_IS_SIMPLE_TEXT (value)) {
                g_printerr ("Property 'WT' is not a GSGFSimpleText!\n");
                return FALSE;
        }

        got = gsgf_text_get_value (GSGF_TEXT (value));
        if (g_strcmp0 (expect, got)) {
                g_printerr ("Property 'WT': Expected '%s', got '%s'!\n",
                             expect, got);
                return FALSE;
        }

        return TRUE;
}
