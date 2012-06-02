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

#include <stdio.h>
#include <string.h>

/* Centralized include file.  */
#include <libgsgf/gsgf.h>

static const gchar *program_name;

static gboolean list_sgf();
static gboolean list_collection(const gchar *path,
                                const GSGFCollection *collection);
static gboolean list_game_tree(const gchar *path,
                               const GSGFGameTree *game_tree);
static gboolean list_bg_move(const gchar *path, const GSGFNode *node);
static void print_point(guint point);

int
main (int argc, char *argv[])
{
        int i;
        int status = 0;

        program_name = argv[0];

        if (argc < 2) {
                g_printerr ("Usage: %s SGF-FILES...\n", argv[0]);
                return -1;
        }

        /* Never forget this or your program will crash! */
        g_type_init ();

        for (i = 1; i < argc; ++i) {
                if (!list_sgf(argv[i]))
                        status = -1;
        }

        return status;
}

static gboolean
list_sgf(const gchar *path)
{
        GFile *file;
        GSGFCollection *collection;
        GError *error = NULL;
        gboolean retval = TRUE;

        printf ("\nSGF File: %s\n", path);

        /* We need a GFile for parsing.  */
        file = g_file_new_for_commandline_arg (path);

        /* Parse it.  The second and third arguments are optional.
         * While you can mostly live without passing a GCancellable as
         * the second argument, you will usually want to pass a GError
         * as the third argument, so that you can print out reasonable
         * error messages.
         */
        collection = gsgf_collection_parse_file (file, NULL, &error);

        /* Free the GFile resources.  */
        g_object_unref(file);

        if (!collection || error) {
                g_printerr ("%s: %s\n", path, error->message);
                return FALSE;
        }

        if (!gsgf_component_cook (GSGF_COMPONENT (collection),
                                  NULL, &error)) {
                g_printerr ("%s: %s\n", path, error->message);
                return FALSE;
        }

        /* A GSGFCollection can be thought of as a match consisting of
         * individual games.
         */
        retval = list_collection(path, collection);

        /* Free all resources.  */
        g_object_unref(collection);

        return retval;
}

static gboolean
list_collection (const gchar *path, const GSGFCollection *collection)
{
        gboolean retval = TRUE;
        GSGFGameTree *game_tree;

        /* Get the game trees stored in this collection.  The data is
         * returned as a standard glib list (GList).
         */
        GList *iter = gsgf_collection_get_game_trees(collection);

        /* Iterate over the game trees.  */
        while (iter) {
                /* Get the data for this list item and cast it to
                 * a GSGFGameTree.
                 */
                game_tree = GSGF_GAME_TREE(iter->data);

                if (!list_game_tree(path, game_tree))
                        retval = FALSE;

                /* Proceed to next item.  */
                iter = iter->next;
        }

        return retval;
}

static gboolean
list_game_tree (const gchar *path, const GSGFGameTree *game_tree)
{
        GList *nodes;
        GSGFNode *root;
        GSGFNumber *number;
        GSGFValue *value;
        GSGFCookedValue *cooked;
        GSGFCompose *AP;
        GSGFSimpleText *application;
        GSGFSimpleText *version;
        GList *ids;
        GList *iter;

        /* The nodes of this game, again as a standard glib
         * list (GList).
         */
        nodes = gsgf_game_tree_get_nodes(game_tree);
        if (!nodes) {
                /* An empty game tree is not considered an error.  */
                printf("%s: empty game tree!\n", path);
                return TRUE;
        }

        /* The root node of the game tree is the head of the list
         * and stores some meta data of the game.
         */
        root = GSGF_NODE(g_list_nth_data(nodes, 0));

        /* A named property is returned as a GSGFCookedValue.  You have to
         * consult the documenation to find out the exact type of the
         * property, and you should cast the cooked value to that type.
         *
         * The GM property defines the type of game.  We are only interested
         * in backgammon games (type 6).
         */
        number = GSGF_NUMBER(gsgf_node_get_property_value (root, "GM"));
        if (6 != gsgf_number_get_value(number)) {
               printf("%s: not backgammon but type %lld!\n",
                      path, (long long int) gsgf_number_get_value(number));
               return TRUE;
        }

        /* The AP property gives information about the application that
         * created the SGF file.  It is not necessarily present.
         */
        cooked = GSGF_COOKED_VALUE (gsgf_node_get_property_value (root, "AP"));
        if (cooked) {
                /* The AP property is a property composed of two
                 * GSGFSimpleText values.
                 */
                AP = GSGF_COMPOSE(cooked);

                /* Now get the two subvalues.  Again they are returned as
                 * abstract GSGFCookedValue objects and have to be cast
                 * to the correct type.  You can find out the type to
                 * expect from the documentation.
                 */
                application = GSGF_SIMPLE_TEXT(gsgf_compose_get_value(AP, 0));
                version = GSGF_SIMPLE_TEXT(gsgf_compose_get_value(AP, 1));

                /* GSGFSimpleText inherits from GSGFText.  The get_value()
                 * method is inerited from GSGFText.  We therefore have
                 * to cast again.
                 */
                printf("Application: %s (version %s)\n",
                       gsgf_text_get_value(GSGF_TEXT(application)),
                       gsgf_text_get_value(GSGF_TEXT(version)));
        }

        /* The only things you should do with node properties that your
         * application does not know is leave them alone or delete
         * them.  As an example, we iterate over all properties of
         * the root node, and delete everything that we don't know.
         *
         * The list of property ids is returned as a standard glib
         * list object (GList).  The returned object is owned by
         * libgsgf and should not be manipulated.  Especially, if
         * you insert or delete properties of a node, you invalidate
         * the list.  We therefore need a shallow copy of it.
         */
        ids = g_list_copy(gsgf_node_get_property_ids(root));

        /* Iterate over all properties.  */
        iter = ids;
        while (iter) {
                if (strcmp("GM", (gchar *) iter->data)
                    && strcmp("AP", (gchar *) iter->data)) {
                        value = GSGF_VALUE (
                                gsgf_node_get_property_value (root,
                                                             (gchar *)
                                                              iter->data));

                        /* Print the type of this object.  If the
                         * object in question is not known to
                         * libgsgf, you will get a GSGFRaw here.
                         */
                        printf("Property %s: %s\n",
                               (gchar *) iter->data,
                               G_OBJECT_TYPE_NAME(G_OBJECT(value)));
                }

                /* And remove the property.  */
                gsgf_node_remove_property(root, (gchar *) iter->data);
                iter = iter->next;
        }
        g_list_free(ids);

        nodes = gsgf_game_tree_get_nodes(game_tree);
        iter = nodes;

        /* The first node was the root node.  Proceed to the next.  */
        if (iter)
                iter = iter->next;

        while (iter) {
                /* Display the move.  */
                if (!list_bg_move(path, GSGF_NODE(iter->data)))
                        return FALSE;

                /* Proceed to next node.  */
                iter = iter->next;
        }

        g_print("\n");

        return TRUE;
}

static gboolean
list_bg_move (const gchar *path, const GSGFNode *node)
{
        char prop_id[2] = { 'W', 0 };
        const GSGFCookedValue *cooked_value =
                GSGF_COOKED_VALUE (gsgf_node_get_property_value (node,
                                                                 prop_id));
        const GSGFMoveBackgammon *move;
        gint i;

        if (!cooked_value) {
                /* Not a W property, try B.  */
                prop_id[0] = 'B';
                cooked_value =
                        GSGF_COOKED_VALUE (gsgf_node_get_property_value (node,
                                                                         prop_id));
        }

        if (!cooked_value) {
                /* Some other node, for example result.  */
                return TRUE;
        }

        /* Type-check and cast.  */
        if (!GSGF_IS_MOVE_BACKGAMMON(cooked_value)) {
                g_printerr ("%s: property '%s' is not a backgammon move but a '%s'.\n",
                            path, prop_id,
                            G_OBJECT_TYPE_NAME(G_OBJECT(cooked_value)));
                return FALSE;
        }
        move = GSGF_MOVE_BACKGAMMON(cooked_value);

        if (gsgf_move_backgammon_is_regular(move)) {
                g_print("%d%d: ",
                                gsgf_move_backgammon_get_die(move, 0),
                                gsgf_move_backgammon_get_die(move, 1)
                        );

                for (i = 0; i < gsgf_move_backgammon_get_num_moves(move); ++i) {
                        print_point(gsgf_move_backgammon_get_from(move, i));
                        g_print("/");
                        print_point(gsgf_move_backgammon_get_to(move, i));
                        g_print(" ");
                }
        } else if (gsgf_move_backgammon_is_double(move)) {
                g_print("Double ");
        } else if (gsgf_move_backgammon_is_take(move)) {
                g_print("Take ");
        } else if (gsgf_move_backgammon_is_drop(move)) {
                g_print("Drop ");
        } else {
                g_print("???????? ");
        }

        if ('B' == prop_id[0])
                g_print("\n");
        else
                g_print("| ");

        return TRUE;
}

static void
print_point(guint point)
{
        if (point < 24)
                g_print("%d", point + 1);
        else if (point == 24)
                g_print("bar");
        else if (point == 25)
                g_print("off");
        else
                g_print("??");
}
