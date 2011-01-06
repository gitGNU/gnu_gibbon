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

char *filename = "move-properties.sgf";

static gboolean test_regular_white_move(const GSGFNode *node);
static gboolean test_regular_black_move(const GSGFNode *node);
static gboolean test_double(const GSGFNode *node);

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
                fprintf(stderr, "Property #1 not found.\n");
                return -1;
        }
        node = GSGF_NODE(item);
        if (!test_regular_white_move(node))
                return -1;

        item = g_list_nth_data(nodes, 2);
        if (!item) {
                fprintf(stderr, "Property #2 not found.\n");
                return -1;
        }
        node = GSGF_NODE(item);
        if (!test_regular_black_move(node))
                return -1;

        item = g_list_nth_data(nodes, 3);
        if (!item) {
                fprintf(stderr, "Property #3 not found.\n");
                return -1;
        }
        node = GSGF_NODE(item);
        if (!test_double(node))
                return -1;

        return expect_error(error, NULL);
}

static gboolean
test_regular_white_move(const GSGFNode *node)
{
        const GSGFCookedValue *cooked_value = gsgf_node_get_property_cooked(node, "W");
        const GSGFMoveBackgammon *move;
        gint point, die;

        if (!cooked_value) {
                fprintf(stderr, "No property 'W'!\n");
                return FALSE;
        }

        if (!GSGF_IS_MOVE(cooked_value)) {
                fprintf(stderr, "Property 'W' is not a GSGFMove!\n");
                return FALSE;
        }

        if (!GSGF_IS_MOVE_BACKGAMMON(cooked_value)) {
                fprintf(stderr, "Property 'W' is not a GSGFBackgammonMove!\n");
                return FALSE;
        }
        move = GSGF_MOVE_BACKGAMMON(cooked_value);

        if (!gsgf_move_backgammon_is_regular(move)) {
                fprintf(stderr, "Property 'W' is not a regular backgammon move!\n");
                return FALSE;
        }

        if (2 != gsgf_move_backgammon_get_num_moves(move)) {
                fprintf(stderr, "Expected two checker moves, got %d!\n",
                                gsgf_move_backgammon_get_num_moves(move));
                return FALSE;
        }

        die = gsgf_move_backgammon_get_die(move, 0);
        if (3 != die) {
                fprintf(stderr, "Expected 3, got %d!\n", die);
                return FALSE;
        }

        die = gsgf_move_backgammon_get_die(move, 1);
        if (1 != die) {
                fprintf(stderr, "Expected 1, got %d!\n", die);
                return FALSE;
        }

        point = gsgf_move_backgammon_get_from(move, 0);
        if (7 != point) {
                fprintf(stderr, "Expected 7, got %d!\n", point);
                return FALSE;
        }

        point = gsgf_move_backgammon_get_to(move, 0);
        if (4 != point) {
                fprintf(stderr, "Expected 4, got %d!\n", point);
                return FALSE;
        }

        point = gsgf_move_backgammon_get_from(move, 1);
        if (5 != point) {
                fprintf(stderr, "Expected 5, got %d!\n", point);
                return FALSE;
        }

        point = gsgf_move_backgammon_get_to(move, 1);
        if (4 != point) {
                fprintf(stderr, "Expected 4, go %d!\n", point);
                return FALSE;
        }

        return TRUE;
}

static gboolean
test_regular_black_move(const GSGFNode *node)
{
        const GSGFCookedValue *cooked_value = gsgf_node_get_property_cooked(node, "B");
        const GSGFMoveBackgammon *move;
        gint point, die;

        if (!cooked_value) {
                fprintf(stderr, "No property 'B'!\n");
                return FALSE;
        }

        if (!GSGF_IS_MOVE(cooked_value)) {
                fprintf(stderr, "Property 'B' is not a GSGFMove!\n");
                return FALSE;
        }

        if (!GSGF_IS_MOVE_BACKGAMMON(cooked_value)) {
                fprintf(stderr, "Property 'B' is not a GSGFBackgammonMove!\n");
                return FALSE;
        }
        move = GSGF_MOVE_BACKGAMMON(cooked_value);

        if (!gsgf_move_backgammon_is_regular(move)) {
                fprintf(stderr, "Property 'B' is not a regular backgammon move!\n");
                return FALSE;
        }

        if (4 != gsgf_move_backgammon_get_num_moves(move)) {
                fprintf(stderr, "Expected four checker moves, got %d!\n",
                                gsgf_move_backgammon_get_num_moves(move));
                return FALSE;
        }

        die = gsgf_move_backgammon_get_die(move, 0);
        if (6 != die) {
                fprintf(stderr, "Expected 6, got %d!\n", die);
                return FALSE;
        }

        die = gsgf_move_backgammon_get_die(move, 1);
        if (6 != die) {
                fprintf(stderr, "Expected 6, got %d!\n", die);
                return FALSE;
        }

        point = gsgf_move_backgammon_get_from(move, 0);
        if (0 != point) {
                fprintf(stderr, "Expected 0, got %d!\n", point);
                return FALSE;
        }

        point = gsgf_move_backgammon_get_to(move, 0);
        if (6 != point) {
                fprintf(stderr, "Expected 6, got %d!\n", point);
                return FALSE;
        }

        point = gsgf_move_backgammon_get_from(move, 1);
        if (0 != point) {
                fprintf(stderr, "Expected 0, got %d!\n", point);
                return FALSE;
        }

        point = gsgf_move_backgammon_get_to(move, 1);
        if (6 != point) {
                fprintf(stderr, "Expected 6, go %d!\n", point);
                return FALSE;
        }

        point = gsgf_move_backgammon_get_from(move, 2);
        if (11 != point) {
                fprintf(stderr, "Expected 11, got %d!\n", point);
                return FALSE;
        }

        point = gsgf_move_backgammon_get_to(move, 2);
        if (17 != point) {
                fprintf(stderr, "Expected 17, go %d!\n", point);
                return FALSE;
        }

        point = gsgf_move_backgammon_get_from(move, 3);
        if (11 != point) {
                fprintf(stderr, "Expected 11, got %d!\n", point);
                return FALSE;
        }

        point = gsgf_move_backgammon_get_to(move, 3);
        if (17 != point) {
                fprintf(stderr, "Expected 17, go %d!\n", point);
                return FALSE;
        }

        return TRUE;
}

static gboolean
test_double(const GSGFNode *node)
{
        const GSGFCookedValue *cooked_value = gsgf_node_get_property_cooked(node, "W");
        const GSGFMoveBackgammon *move;

        if (!cooked_value) {
                fprintf(stderr, "No property 'W'!\n");
                return FALSE;
        }

        if (!GSGF_IS_MOVE(cooked_value)) {
                fprintf(stderr, "Property 'W' is not a GSGFMove!\n");
                return FALSE;
        }

        if (!GSGF_IS_MOVE_BACKGAMMON(cooked_value)) {
                fprintf(stderr, "Property 'W' is not a GSGFBackgammonMove!\n");
                return FALSE;
        }
        move = GSGF_MOVE_BACKGAMMON(cooked_value);

        if (!gsgf_move_backgammon_is_double(move)) {
                fprintf(stderr, "Property 'W' is not a backgammon double!\n");
                return FALSE;
        }

        return TRUE;
}
