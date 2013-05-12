/*
 * This file is part of gibbon.
 * Gibbon is a Gtk+ frontend for the First Internet Backgammon Server FIBS.
 * Copyright (C) 2009-2012 Guido Flohr, http://guido-flohr.net/.
 *
 * gibbon is free software: you can redistribute it and/or modify 
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * gibbon is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with gibbon.  If not, see <http://www.gnu.org/licenses/>.
 */

/**
 * SECTION:gibbon-sgf-reader
 * @short_description: Read SGF (Smart Game Format).
 *
 * Since: 0.2.0
 *
 * A #GibbonMatchReader for reading SGF match files.
 */

#include <stdio.h>
#include <string.h>
#include <errno.h>

#include <glib.h>
#include <glib/gi18n.h>
#include <gdk/gdk.h>

#include <libgsgf/gsgf.h>

#include "gibbon-util.h"
#include "gibbon-sgf-reader.h"

#include "gibbon-game.h"
#include "gibbon-game-action.h"
#include "gibbon-game-actions.h"
#include "gibbon-analysis-roll.h"
#include "gibbon-analysis-move.h"
#include "gibbon-variant-list.h"

typedef struct _GibbonSGFReaderPrivate GibbonSGFReaderPrivate;
struct _GibbonSGFReaderPrivate {
        GibbonMatchReaderErrorFunc yyerror;
        gpointer user_data;
        GibbonMatch *match;
        guint scores[2];

        gboolean debug;

        /* Per-instance data.  */
        const gchar *filename;
        gint64 timestamp;
};

GibbonSGFReader *_gibbon_sgf_reader_instance = NULL;

#define GIBBON_SGF_READER_PRIVATE(obj) (G_TYPE_INSTANCE_GET_PRIVATE ((obj), \
        GIBBON_TYPE_SGF_READER, GibbonSGFReaderPrivate))

G_DEFINE_TYPE (GibbonSGFReader, gibbon_sgf_reader, GIBBON_TYPE_MATCH_READER)

static void gibbon_sgf_reader_yyerror (const GibbonSGFReader *reader,
                                       const gchar *msg);
static GibbonMatch *gibbon_sgf_reader_parse (GibbonMatchReader *match_reader,
                                             const gchar *filename);
static gboolean gibbon_sgf_reader_add_action (GibbonSGFReader *self,
                                              GibbonMatch *match,
                                              GibbonPositionSide side,
                                              GibbonGameAction *action,
                                              GibbonAnalysis *analysis,
                                              GError **error);
static gboolean gibbon_sgf_reader_game (GibbonSGFReader *self,
                                        GibbonMatch *match,
                                        GSGFGameTree *game_tree,
                                        GError **error);
static gboolean gibbon_sgf_reader_root_node (GibbonSGFReader *self,
                                             GibbonMatch *match,
                                             GSGFGameTree *game_tree,
                                            GError **error);
static gboolean gibbon_sgf_reader_match_info_item (GibbonSGFReader *self,
                                                   GibbonMatch *match,
                                                   const gchar *kv,
                                                   GError **error);
static gboolean gibbon_sgf_reader_move (GibbonSGFReader *self,
                                        GibbonMatch *match,
                                        const GSGFProperty *prop,
                                        const GSGFNode *node,
                                        GibbonPositionSide side,
                                        GError **error);
static GibbonAnalysis *gibbon_sgf_reader_roll_analysis (const GibbonSGFReader *self,
                                                        const GSGFNode *node,
                                                        GibbonPositionSide side);
static GibbonAnalysis *gibbon_sgf_reader_move_analysis (const GibbonSGFReader *self,
                                                        const GSGFNode *node,
                                                        GibbonPositionSide side,
                                                        guint die1, guint die2,
                                                        gboolean cube_response);
static void gibbon_sgf_reader_doubling_analysis (
                const GibbonSGFReader *self,
                GibbonAnalysisMove *analysis,
                const GSGFNode *node,
                gboolean cube_response);
static void gibbon_sgf_reader_doubling_analysis_eval (
                const GibbonSGFReader *self,
                GibbonAnalysisMove *analysis,
                gchar **tokens);
static void gibbon_sgf_reader_doubling_analysis_rollout (
                const GibbonSGFReader *self,
                GibbonAnalysisMove *analysis,
                gchar **tokens);
gboolean gibbon_sgf_reader_move_variant (const GibbonSGFReader *self,
                                         GtkListStore *store, GtkTreeIter *iter,
                                         gchar **tokens,
                                         const GibbonPosition *pos,
                                         GibbonPositionSide side,
                                         guint die1, guint die2);
gboolean gibbon_sgf_reader_move_variant_eval (const GibbonSGFReader *self,
                                              GtkListStore *store,
                                              GtkTreeIter *iter,
                                              gchar **tokens,
                                              const GibbonPosition *pos,
                                              GibbonPositionSide side,
                                              guint die1, guint die2);
gboolean gibbon_sgf_reader_move_variant_rollout (const GibbonSGFReader *self,
                                                 GtkListStore *store,
                                                 GtkTreeIter *iter,
                                                 gchar **tokens,
                                                 const GibbonPosition *pos,
                                                 GibbonPositionSide side,
                                                 guint die1, guint die2);
static gboolean gibbon_sgf_reader_setup_pre_check (GibbonSGFReader *self,
                                                   const GibbonMatch *match,
                                                   const gchar *prop,
                                                   GError **error);
static gboolean gibbon_sgf_reader_setup_turn (GibbonSGFReader *self,
                                              GibbonMatch *match,
                                              const GSGFProperty *prop,
                                              GError **error);
static gboolean gibbon_sgf_reader_setup_dice (GibbonSGFReader *self,
                                              GibbonMatch *match,
                                              const GSGFProperty *prop,
                                              GError **error);
static gboolean gibbon_sgf_reader_setup_cube (GibbonSGFReader *self,
                                              GibbonMatch *match,
                                              const GSGFProperty *prop,
                                              GError **error);
static gboolean gibbon_sgf_reader_setup_cube_owner (GibbonSGFReader *self,
                                                    GibbonMatch *match,
                                                    const GSGFProperty *prop,
                                                    GError **error);
static gboolean gibbon_sgf_reader_setup_add_empty (GibbonSGFReader *self,
                                                   GibbonMatch *match,
                                                   const GSGFProperty *prop,
                                                   GError **error);
static gboolean gibbon_sgf_reader_setup_add_white (GibbonSGFReader *self,
                                                   GibbonMatch *match,
                                                   const GSGFProperty *prop,
                                                   GError **error);
static gboolean gibbon_sgf_reader_setup_add_black (GibbonSGFReader *self,
                                                   GibbonMatch *match,
                                                   const GSGFProperty *prop,
                                                   GError **error);

static void 
gibbon_sgf_reader_init (GibbonSGFReader *self)
{
        self->priv = G_TYPE_INSTANCE_GET_PRIVATE (self,
                GIBBON_TYPE_SGF_READER, GibbonSGFReaderPrivate);

        self->priv->debug = FALSE;

        self->priv->match = NULL;
        self->priv->scores[0] = 0;
        self->priv->scores[1] = 0;
        self->priv->filename = NULL;
        self->priv->timestamp = G_MININT64;

        self->priv->yyerror = NULL;
        self->priv->user_data = NULL;
}

static void
gibbon_sgf_reader_finalize (GObject *object)
{
        G_OBJECT_CLASS (gibbon_sgf_reader_parent_class)->finalize(object);
}

static void
gibbon_sgf_reader_class_init (GibbonSGFReaderClass *klass)
{
        GObjectClass *object_class = G_OBJECT_CLASS (klass);
        GibbonMatchReaderClass *gibbon_match_reader_class =
                        GIBBON_MATCH_READER_CLASS (klass);

        gibbon_match_reader_class->parse = gibbon_sgf_reader_parse;
        
        g_type_class_add_private (klass, sizeof (GibbonSGFReaderPrivate));

        object_class->finalize = gibbon_sgf_reader_finalize;
}

/**
 * gibbon_sgf_reader_new:
 * @error_func: Error reporting function or %NULL
 * @user_data: Pointer to pass to @error_func or %NULL
 *
 * Creates a new #GibbonSGFReader.
 *
 * Returns: The newly created #GibbonSGFReader.
 */
GibbonSGFReader *
gibbon_sgf_reader_new (GibbonMatchReaderErrorFunc yyerror,
                       gpointer user_data)
{
        GibbonSGFReader *self = g_object_new (GIBBON_TYPE_SGF_READER,
                                                   NULL);

        self->priv->user_data = user_data;
        self->priv->yyerror = yyerror;
        self->priv->debug = gibbon_debug ("sgf-reader");

        return self;
}

static GibbonMatch *
gibbon_sgf_reader_parse (GibbonMatchReader *_self, const gchar *filename)
{
        GibbonSGFReader *self;
        GFile *file;
        GError *error = NULL;
        GSGFCollection *collection;
        GibbonMatch *match;
        GList *iter;
        GSGFGameTree *game_tree;
        const GSGFFlavor *flavor;

        g_return_val_if_fail (GIBBON_IS_SGF_READER (_self), NULL);
        self = GIBBON_SGF_READER (_self);

        self->priv->filename = filename;
        self->priv->timestamp = G_MININT64;

        if (!filename) {
                gibbon_sgf_reader_yyerror (self,
                                           _("SGF cannot be parsed from"
                                             " standard input."));
                return NULL;
        }
        file = g_file_new_for_path (filename);

        collection = gsgf_collection_parse_file (file, NULL, &error);

        g_object_unref (file);

        if (!collection) {
                gibbon_sgf_reader_yyerror (self, error->message);
                return FALSE;
        }

        if (!gsgf_component_cook (GSGF_COMPONENT (collection), NULL, &error)) {
                gibbon_sgf_reader_yyerror (self, error->message);
                g_object_unref (collection);
                return FALSE;
        }

        match = gibbon_match_new (NULL, NULL, 0, FALSE);
        self->priv->match = match;
        self->priv->scores[0] = 0;
        self->priv->scores[1] = 0;

        iter = gsgf_collection_get_game_trees (collection);

        /* Iterate over the game trees.  */
        while (iter) {
                game_tree = GSGF_GAME_TREE (iter->data);

                /*
                 * We ignore all non-backgammon game trees.
                 */
                flavor = gsgf_game_tree_get_flavor (game_tree);
                if (!flavor) {
                        iter = iter->next;
                        continue;
                }

                if (!GSGF_IS_FLAVOR_BACKGAMMON (flavor)) {
                        iter = iter->next;
                        continue;
                }

                /*
                 * SGF stores general match meta information in the the root
                 * node of each game tree.
                 */
                if (!gibbon_sgf_reader_root_node (self, match,
                                                  game_tree, &error)) {
                        gibbon_sgf_reader_yyerror (self, error->message);
                        g_object_unref (match);
                        match = NULL;
                        break;
                }

                if (!gibbon_sgf_reader_game (self, match, game_tree,
                                             &error)) {
                        gibbon_sgf_reader_yyerror (self, error->message);
                        g_object_unref (match);
                        match = NULL;
                        break;
                }

                /* Proceed to next item.  */
                iter = iter->next;
        }

        g_object_unref (collection);

        return match;
}

static gboolean
gibbon_sgf_reader_add_action (GibbonSGFReader *self, GibbonMatch *match,
                              GibbonPositionSide side,
                              GibbonGameAction *action,
                              GibbonAnalysis *analysis, GError **error)
{
        GibbonGame *game;

        game = gibbon_match_get_current_game (match);
        if (!game) {
                g_set_error_literal (error, GIBBON_ERROR, -1,
                                     _("No game in progress!"));
                g_object_unref (action);
                if (analysis)
                        g_object_unref (analysis);
                return FALSE;
        }

        if (!gibbon_game_add_action_with_analysis (game, side, action,
                                                   analysis,
                                                   self->priv->timestamp,
                                                   error)) {
                g_object_unref (action);
                if (analysis)
                        g_object_unref (analysis);
                return FALSE;
        }

        return TRUE;
}

static void
gibbon_sgf_reader_yyerror (const GibbonSGFReader *self,
                           const gchar *msg)
{
        const gchar *location;
        gchar *full_msg;

        if (self->priv->filename)
                location = self->priv->filename;
        else
                location = _("[standard input]");

        full_msg = g_strdup_printf ("%s: %s", location, msg);
        if (self->priv->yyerror)
                self->priv->yyerror (self->priv->user_data, full_msg);
        else
                g_printerr ("%s\n", full_msg);

        g_free (full_msg);
}

static gboolean
gibbon_sgf_reader_game (GibbonSGFReader *self, GibbonMatch *match,
                        GSGFGameTree *game_tree, GError **error)
{
        const GList *iter;
        GibbonPositionSide side;
        const GSGFNode *node;
        const GSGFProperty *prop;
        const GibbonGame *game;
        const GSGFResult *result;
        GibbonResign *resign;
        GibbonGameAction *action;

        if (!gibbon_match_add_game (match, error))
                return FALSE;

        iter = gsgf_game_tree_get_nodes (game_tree);
        if (!iter) return TRUE;

        for (iter = iter->next; iter; iter = iter->next) {
                node = GSGF_NODE (iter->data);
                prop = gsgf_node_get_property (node, "PL");
                if (prop && !gibbon_sgf_reader_setup_turn (self, match, prop,
                                                           error))
                        return FALSE;
                prop = gsgf_node_get_property (node, "DI");
                if (prop && !gibbon_sgf_reader_setup_dice (self, match, prop,
                                                           error))
                        return FALSE;
                prop = gsgf_node_get_property (node, "CV");
                if (prop && !gibbon_sgf_reader_setup_cube (self, match, prop,
                                                           error))
                        return FALSE;
                prop = gsgf_node_get_property (node, "CO");
                if (prop && !gibbon_sgf_reader_setup_cube_owner (self, match,
                                                                 prop, error))
                        return FALSE;
                /*
                 * GNUBG uses the CP property (copyright) instead of CO
                 * (cube owner).  For compatibility, we allow both here.
                 */
                prop = gsgf_node_get_property (node, "CP");
                if (prop && !gibbon_sgf_reader_setup_cube_owner (self, match,
                                                                 prop, error))
                        return FALSE;

                prop = gsgf_node_get_property (node, "AE");
                if (prop && !gibbon_sgf_reader_setup_add_empty (self, match,
                                                                prop, error))
                        return FALSE;
                prop = gsgf_node_get_property (node, "AB");
                if (prop && !gibbon_sgf_reader_setup_add_white (self, match,
                                                                prop, error))
                        return FALSE;
                prop = gsgf_node_get_property (node, "AW");
                if (prop && !gibbon_sgf_reader_setup_add_black (self, match,
                                                                prop, error))
                        return FALSE;

                prop = gsgf_node_get_property (node, "B");
                if (prop) {
                        side = GIBBON_POSITION_SIDE_WHITE;
                } else {
                        prop = gsgf_node_get_property (node, "W");
                        if (!prop)
                                continue;
                        side = GIBBON_POSITION_SIDE_BLACK;
                }
                if (prop && !gibbon_sgf_reader_move (self, match, prop, node,
                                                     side, error))
                                return FALSE;
        }

        game = gibbon_match_get_current_game (match);
        if (!gibbon_game_over (game)) {
                iter = gsgf_game_tree_get_nodes (game_tree);
                node = GSGF_NODE (iter->data);
                prop = gsgf_node_get_property (node, "RE");
                if (!prop)
                        return TRUE;
                result = GSGF_RESULT (gsgf_property_get_value (prop));
                if (GSGF_RESULT_RESIGNATION != gsgf_result_get_cause (result))
                        return TRUE;
                if (GSGF_RESULT_BLACK)
                        side = GIBBON_POSITION_SIDE_BLACK;
                else if (GSGF_RESULT_WHITE)
                        side = GIBBON_POSITION_SIDE_WHITE;
                else
                        return TRUE;

                resign = gibbon_resign_new (gsgf_result_get_score (result));
                action = GIBBON_GAME_ACTION (resign);
                if (!gibbon_sgf_reader_add_action (self, match, side, action,
                                                   NULL, error))
                        return FALSE;
                action = GIBBON_GAME_ACTION (gibbon_accept_new ());
                if (!gibbon_sgf_reader_add_action (self, match, -side, action,
                                                   NULL, error))
                        return FALSE;
        }

        return TRUE;
}

static gboolean
gibbon_sgf_reader_root_node (GibbonSGFReader *self, GibbonMatch *match,
                             GSGFGameTree *game_tree, GError **error)
{
        const GList *nodes = gsgf_game_tree_get_nodes (game_tree);
        const GSGFNode *root;
        const GSGFProperty *prop;
        const GSGFListOf *values;
        gsize i, num_items;
        const GSGFText *text;
        const gchar *value;

        if (!nodes) return TRUE;

        root = GSGF_NODE (nodes->data);

        prop = gsgf_node_get_property (root, "MI");
        if (prop) {
                values = GSGF_LIST_OF (gsgf_property_get_value (prop));
                num_items = gsgf_list_of_get_number_of_items (values);
                for (i = 0; i < num_items; ++i) {
                        text = GSGF_TEXT (gsgf_list_of_get_nth_item (values, i));
                        if (!gibbon_sgf_reader_match_info_item (
                                        self, match, gsgf_text_get_value (text),
                                        error))
                                return FALSE;
                }
        }

        /* Colors are swapped!  */
        prop = gsgf_node_get_property (root, "PB");
        if (prop) {
                text = GSGF_TEXT (gsgf_property_get_value (prop));
                if (text)
                        gibbon_match_set_white (match,
                                                gsgf_text_get_value (text));
        }
        prop = gsgf_node_get_property (root, "PW");
        if (prop) {
                text = GSGF_TEXT (gsgf_property_get_value (prop));
                if (text)
                        gibbon_match_set_black (match,
                                                gsgf_text_get_value (text));
        }

        prop = gsgf_node_get_property (root, "RU");
        if (prop) {
                text = GSGF_TEXT (gsgf_property_get_value (prop));
                value = gsgf_text_get_value (text);
                if (text && (0 == g_ascii_strcasecmp ("Crawford", value)
                             || 0 == g_ascii_strcasecmp ("Crawford"
                                                         ":CrawfordGame",
                                                         value))) {
                        gibbon_match_set_crawford (match, TRUE);
                }
        }

        return TRUE;
}

static gboolean
gibbon_sgf_reader_match_info_item (GibbonSGFReader *self, GibbonMatch *match,
                                   const gchar *kv, GError **error)
{
        gchar *key = g_alloca (1 + strlen (kv));
        gchar *string_value;
        guint64 value;
        gchar *endptr;

        strcpy (key, kv);
        string_value = strchr (key, ':');
        if (!string_value)
                return TRUE;

        *string_value++ = 0;
        if (!*string_value)
                return TRUE;

        if (!g_strcmp0 ("length", key)) {
                errno = 0;
                value = g_ascii_strtoull (string_value, &endptr, 010);
                if (errno) {
                        g_set_error (error, GIBBON_ERROR, -1,
                                     _("Invalid match length: %s!"),
                                     strerror (errno));
                        return FALSE;
                }

                if (value > G_MAXSIZE) {
                        g_set_error (error, GIBBON_ERROR, -1,
                                     _("Match length %llu out of range!"),
                                     (unsigned long long) value);
                        return FALSE;
                }

                gibbon_match_set_length (match, value);
        } else if (!g_strcmp0 ("bs", key)) {
                errno = 0;
                value = g_ascii_strtoull (string_value, &endptr, 010);
                if (errno) {
                        g_set_error (error, GIBBON_ERROR, -1,
                                     _("Invalid match length: %s!"),
                                     strerror (errno));
                        return FALSE;
                }
                self->priv->scores[0] = value;
        } else if (!g_strcmp0 ("ws", key)) {
                errno = 0;
                value = g_ascii_strtoull (string_value, &endptr, 010);
                if (errno) {
                        g_set_error (error, GIBBON_ERROR, -1,
                                     _("Invalid match length: %s!"),
                                     strerror (errno));
                        return FALSE;
                }
                self->priv->scores[1] = value;
        }

        return TRUE;
}

static gboolean
gibbon_sgf_reader_move (GibbonSGFReader *self, GibbonMatch *match,
                        const GSGFProperty *prop, const GSGFNode *node,
                        GibbonPositionSide side, GError **error)
{
        GSGFMoveBackgammon *gsgf_move;
        guint dice[2];
        GibbonGameAction *action;
        GibbonMove *move;
        gsize num_movements, i;
        guint from, to;
        GibbonMovement *movement;
        GibbonAnalysis *analysis = NULL;
        GibbonAnalysisMove *ma;

        gsgf_move = GSGF_MOVE_BACKGAMMON (gsgf_property_get_value (prop));

        if (gsgf_move_backgammon_is_regular (gsgf_move)) {
                dice[0] = gsgf_move_backgammon_get_die (gsgf_move, 0);
                dice[1] = gsgf_move_backgammon_get_die (gsgf_move, 1);
                action = GIBBON_GAME_ACTION (gibbon_roll_new (dice[0],
                                                              dice[1]));
                analysis = gibbon_sgf_reader_roll_analysis (self, node, side);
                if (!gibbon_sgf_reader_add_action (self, match, side, action,
                                                   analysis, error))
                        return FALSE;
                num_movements = gsgf_move_backgammon_get_num_moves (gsgf_move);
                move = gibbon_move_new (dice[0], dice[1], num_movements);
                move->number = num_movements;
                for (i = 0; i < num_movements; ++i) {
                        movement = move->movements + i;
                        from = gsgf_move_backgammon_get_from (gsgf_move, i);
                        to = gsgf_move_backgammon_get_to (gsgf_move, i);
                        if (from == 24) {
                                if (to < 6)
                                        from = 0;
                                else
                                        from = 25;
                        } else {
                                ++from;
                        }
                        if (to == 25) {
                                if (from <= 6)
                                        to = 0;
                        } else {
                                ++to;
                        }
                        from = 25 - from;
                        to = 25 - to;
                        movement->from = from;
                        movement->to = to;
                }
                gibbon_move_sort (move);

                action = GIBBON_GAME_ACTION (move);
                analysis = gibbon_sgf_reader_move_analysis (self, node, side,
                                                            dice[0], dice[1],
                                                            FALSE);
                if (!gibbon_sgf_reader_add_action (self, match, side, action,
                                                   analysis, error))
                        return FALSE;
        } else if (gsgf_move_backgammon_is_double (gsgf_move)) {
                action = GIBBON_GAME_ACTION (gibbon_double_new ());
                analysis = gibbon_sgf_reader_move_analysis (self, node, side,
                                                            0, 0, FALSE);
                if (!gibbon_sgf_reader_add_action (self, match, side, action,
                                                   analysis, error))
                        return FALSE;
        } else if (gsgf_move_backgammon_is_drop (gsgf_move)) {
                action = GIBBON_GAME_ACTION (gibbon_drop_new ());
                analysis = gibbon_sgf_reader_move_analysis (self, node, side,
                                                            0, 0, TRUE);
                ma = GIBBON_ANALYSIS_MOVE (analysis);
                /*
                 * Make sure that the may double flag is set.  Otherwise,
                 * the cube decision would return "cube not available" for
                 * redoubles.
                 */
                ma->may_double = TRUE;
                ma->opp_may_double = TRUE;
                ma->da_take_analysis = TRUE;
                if (!gibbon_sgf_reader_add_action (self, match, side, action,
                                                   analysis, error))
                        return FALSE;
        } else if (gsgf_move_backgammon_is_take (gsgf_move)) {
                action = GIBBON_GAME_ACTION (gibbon_take_new ());
                analysis = gibbon_sgf_reader_move_analysis (self, node, side,
                                                            0, 0, TRUE);
                ma = GIBBON_ANALYSIS_MOVE (analysis);
                /*
                 * Make sure that the may double flag is set.  Otherwise,
                 * the cube decision would return "cube not available" for
                 * redoubles.
                 */
                ma->may_double = TRUE;
                ma->opp_may_double = TRUE;
                ma->da_take_analysis = TRUE;
                if (!gibbon_sgf_reader_add_action (self, match, side, action,
                                                   analysis, error))
                        return FALSE;
        } else if (gsgf_move_backgammon_is_resign (gsgf_move)) {
                action = GIBBON_GAME_ACTION (gibbon_resign_new (
                                gsgf_move_backgammon_is_resign (gsgf_move)));
                if (!gibbon_sgf_reader_add_action (self, match, side, action,
                                                   NULL, error))
                        return FALSE;
        } else if (gsgf_move_backgammon_is_accept (gsgf_move)) {
                action = GIBBON_GAME_ACTION (gibbon_accept_new ());
                if (!gibbon_sgf_reader_add_action (self, match, side, action,
                                                   NULL, error))
                        return FALSE;
        } else if (gsgf_move_backgammon_is_reject (gsgf_move)) {
                action = GIBBON_GAME_ACTION (gibbon_reject_new ());
                if (!gibbon_sgf_reader_add_action (self, match, side, action,
                                                   NULL, error))
                        return FALSE;
        }

        return TRUE;
}

static GibbonAnalysis *
gibbon_sgf_reader_roll_analysis (const GibbonSGFReader *self,
                                 const GSGFNode *node,
                                 GibbonPositionSide side)
{
        GSGFProperty *prop;
        GSGFValue *value;
        GibbonAnalysisRollLuck type = GIBBON_ANALYSIS_ROLL_LUCK_UNKNOWN;
        gdouble luck = 0.0;
        gboolean valid = FALSE;
        gboolean reverse = side == GIBBON_POSITION_SIDE_BLACK;

        prop = gsgf_node_get_property (node, "LU");
        if (prop) {
                valid = TRUE;
                value = gsgf_property_get_value (prop);
                luck = gsgf_real_get_value (GSGF_REAL (value));
        }

        prop = gsgf_node_get_property (node, "GW");
        if (!prop) {
                reverse = !reverse;
                prop = gsgf_node_get_property (node, "GB");
        }
        if (prop) {
                valid = TRUE;
                value = gsgf_property_get_value (prop);
                switch (gsgf_double_get_value (GSGF_DOUBLE (value))) {
                case GSGF_DOUBLE_NORMAL:
                        if (reverse)
                                type = GIBBON_ANALYSIS_ROLL_LUCK_LUCKY;
                        else
                                type = GIBBON_ANALYSIS_ROLL_LUCK_UNLUCKY;
                        break;
                case GSGF_DOUBLE_VERY:
                        if (reverse)
                                type = GIBBON_ANALYSIS_ROLL_LUCK_VERY_LUCKY;
                        else
                                type = GIBBON_ANALYSIS_ROLL_LUCK_VERY_UNLUCKY;
                        break;
                }
        }

        if (!valid)
                return NULL;

        if (!type)
                type = GIBBON_ANALYSIS_ROLL_LUCK_NONE;

        return GIBBON_ANALYSIS (gibbon_analysis_roll_new (type, luck));
}

static GibbonAnalysis *
gibbon_sgf_reader_move_analysis (const GibbonSGFReader *self,
                                 const GSGFNode *node, GibbonPositionSide side,
                                 guint die1, guint die2,
                                 gboolean cube_response)
{
        GibbonAnalysisMove *a = gibbon_analysis_move_new ();
        GSGFProperty *prop;
        GSGFDouble *gsgf_double;
        const GibbonGame *game;
        gsize num_actions;
        const GibbonPosition *pos;
        GSGFListOf *list;
        gsize num_items;
        GSGFText *text;
        const gchar *str_value;
        gchar *endptr;
        gsize i;
        gchar **tokens;
        GtkTreeIter iter;
        GtkListStore *store;
        PangoWeight weight;

        game = gibbon_match_get_current_game (self->priv->match);
        num_actions = gibbon_game_get_num_actions (game);
        if (num_actions)
                pos = gibbon_game_get_nth_position (game, num_actions - 1);
        else
                pos = gibbon_game_get_initial_position (game);

        a->match_length = gibbon_match_get_length (self->priv->match);
        a->cube = pos->cube;
        a->my_score = side > 0 ? pos->scores[0] : pos->scores[1];
        a->opp_score = side > 0 ? pos->scores[1] : pos->scores[0];
        a->crawford = gibbon_match_get_crawford (self->priv->match);
        a->is_crawford = gibbon_game_is_crawford (game);
        if (a->match_length > 0 && a->crawford && !a->is_crawford
            && (a->my_score == a->match_length - 1
                || a->opp_score == a->match_length - 1))
                a->post_crawford = TRUE;
        else
                a->post_crawford = FALSE;
        a->may_double = side > 0 ? pos->may_double[0] : pos->may_double[1];
        a->opp_may_double = side > 0 ? pos->may_double[1] : pos->may_double[0];

        /*
         * Beavers are currently not supported by Gibbon.  Jacoby is
         * currently not supported by FIBS itself.
         */
        a->beavers = FALSE;
        a->jacoby = FALSE;

        gibbon_sgf_reader_doubling_analysis (self, a, node, cube_response);

        prop = gsgf_node_get_property (node, "DO");
        if (prop && !cube_response) {
                a->ma_bad = 1;
        } else {
                prop = gsgf_node_get_property (node, "BM");
                if (prop) {
                        gsgf_double = GSGF_DOUBLE (gsgf_property_get_value (prop));
                        a->ma_bad = 1 + gsgf_double_get_value (gsgf_double);
                }
        }

        prop = gsgf_node_get_property (node, "A");
        if (!prop)
                return GIBBON_ANALYSIS (a);

        list = GSGF_LIST_OF (gsgf_property_get_value (prop));
        num_items = gsgf_list_of_get_number_of_items (list);
        if (num_items < 2)
                return GIBBON_ANALYSIS (a);

        /*
         * The first element is the index of the actual move made into the
         * following move list.
         */
        text = GSGF_TEXT (gsgf_list_of_get_nth_item (list, 0));
        str_value = gsgf_text_get_value (text);

        errno = 0;
        a->ma_imove = g_ascii_strtoull (str_value, &endptr, 10);
        if (errno || !endptr || *endptr)
                return GIBBON_ANALYSIS (a);

        a->ma_variants = gibbon_variant_list_new ();
        store = gibbon_variant_list_get_store (a->ma_variants);

        for (i = 1; i < num_items; ++i) {
                text = GSGF_TEXT (gsgf_list_of_get_nth_item (list, i));
                str_value = gsgf_text_get_value (text);
                tokens = g_strsplit_set (str_value, " \t\r\n\f\v", 0);

                if (!tokens || !tokens[0]) {
                        return GIBBON_ANALYSIS (a);
                }
                gtk_list_store_append (store, &iter);

                weight = (i == a->ma_imove + 1) ?
                                PANGO_WEIGHT_BOLD : PANGO_WEIGHT_NORMAL;

                gtk_list_store_set (store, &iter,
                                    GIBBON_VARIANT_LIST_COL_NUMBER, i,
                                    GIBBON_VARIANT_LIST_COL_WEIGHT, weight,
                                    -1);

                if (!gibbon_sgf_reader_move_variant (self, store, &iter, tokens,
                                                     pos, side,
                                                     die1, die2)) {
                        g_strfreev (tokens);
                        return GIBBON_ANALYSIS (a);
                }
                g_strfreev (tokens);
        }

        a->ma = TRUE;

        return GIBBON_ANALYSIS (a);
}

static void
gibbon_sgf_reader_doubling_analysis (const GibbonSGFReader *self,
                                     GibbonAnalysisMove *a,
                                     const GSGFNode *node,
                                     gboolean cube_response)
{
        GSGFProperty *prop;
        GSGFText *text;
        const gchar *str_value;
        gchar **tokens;
        GSGFDouble *gsgf_double;
        const gchar *doubtful_propname;
        const gchar *bad_propname;

        prop = gsgf_node_get_property (node, "DA");
        if (!prop)
                return;

        text = GSGF_TEXT (gsgf_property_get_value (prop));
        str_value = gsgf_text_get_value (text);
        tokens = g_strsplit_set (str_value, " \t\r\n\f\v", 0);

        if (!tokens)
                return;
        if (!tokens[0])
                return;
        if (tokens[0][1])
                return;

        switch (tokens[0][0]) {
        case 'E':
                gibbon_sgf_reader_doubling_analysis_eval (self, a, tokens);
                break;
        case 'X':
                gibbon_sgf_reader_doubling_analysis_rollout (self, a, tokens);
                break;
        }

        if (cube_response) {
                doubtful_propname = "DO";
                bad_propname = "BC";
        } else {
                doubtful_propname = "DC";
                bad_propname = "BC";
        }

        prop = gsgf_node_get_property (node, doubtful_propname);
        if (prop) {
                a->da_bad = 1;
        } else {
                prop = gsgf_node_get_property (node, bad_propname);
                if (prop) {
                        gsgf_double = GSGF_DOUBLE (gsgf_property_get_value (prop));
                        a->da_bad = 1 + gsgf_double_get_value (gsgf_double);
                }
        }

        g_strfreev (tokens);
}

static void
gibbon_sgf_reader_doubling_analysis_eval (const GibbonSGFReader *self,
                                          GibbonAnalysisMove *a,
                                          gchar **tokens)
{
        gchar *endptr;
        guint i, j;

        if (21 != g_strv_length (tokens))
                return;

        if (g_strcmp0 ("ver", tokens[1]))
                return;
        if (!gibbon_chareq ("3", tokens[2])) {
                g_message (_("Unsupported version %s for DA property."),
                           tokens[2]);
                return;
        }

        errno = 0;
        a->da_plies = g_ascii_strtoull (tokens[3], &endptr, 10);
        if (errno || !endptr)
                return;
        if (!*endptr)
                a->da_cubeful = FALSE;
        else if (gibbon_chareq ("C", endptr))
                a->da_cubeful = TRUE;
        else
                return;

        if ('1' == tokens[4][0])
                a->da_deterministic = TRUE;
        else if ('0' == tokens[4][0])
                a->da_deterministic = FALSE;
        if (tokens[4][1])
                return;

        a->da_noise = g_ascii_strtod (tokens[5], &endptr);
        if (errno || !endptr || *endptr)
                return;

        if ('1' == tokens[6][0])
                a->da_use_prune = TRUE;
        else if ('0' == tokens[6][0])
                a->da_use_prune = FALSE;
        if (tokens[6][1])
                return;

        for (i = 0; i < 2; ++i) {
                for (j = 0; j < 7; ++j) {
                        a->da_p[i][j] = g_ascii_strtod (tokens[7 + 7 * i + j],
                                                        &endptr);
                        if (errno || !endptr || *endptr)
                                return;
                }
        }

        a->da = TRUE;
        a->da_rollout = FALSE;
}

static void
gibbon_sgf_reader_doubling_analysis_rollout (const GibbonSGFReader *self,
                                             GibbonAnalysisMove *a,
                                             gchar **tokens)
{
        gchar *endptr;
        guint i, j;

        if (g_strcmp0 ("ver", tokens[1])) {
#if GIBBON_SGF_READER_DEBUG
                g_message ("Expected 'ver', not '%s'.", tokens[1]);
#endif
                return;
        }

        if (!gibbon_chareq ("3", tokens[2])) {
                g_message (_("Unsupported version %s for DA property."),
                           tokens[2]);
                return;
        }

        if (g_strcmp0 ("Eq", tokens[3])) {
#if GIBBON_SGF_READER_DEBUG
                g_message ("Expected 'Eq', not '%s'.", tokens[3]);
#endif
                return;
        }

        if (g_strcmp0 ("Trials", tokens[4])) {
#if GIBBON_SGF_READER_DEBUG
                g_message ("Expected 'Trials', not '%s'.", tokens[4]);
#endif
                return;
        }

        if (!tokens[5]) {
#if GIBBON_SGF_READER_DEBUG
                g_message ("Number of trials missing.");
#endif
                return;
        }

        errno = 0;
        a->da_trials = g_ascii_strtoull (tokens[5], &endptr, 10);
        if (errno || !endptr || *endptr) {
                if (self->priv->debug)
                        g_message ("Invalid number of trials '%s': %s.",
                                        tokens[5], strerror (errno));
        }

        for (i = 0; i < 2; ++i) {
                if (i == 0) {
                        if (g_strcmp0 ("NoDouble", tokens[6 + 17 * i])) {
                                if (self->priv->debug)
                                        g_message ("Expected 'NoDouble', not '%s'.",
                                                        tokens[6 + 16 * i]);
                                return;
                        }
                } else {
                        if (g_strcmp0 ("DoubleTake", tokens[6 + 17 * i])) {
                                if (self->priv->debug)
                                        g_message ("Expected 'DoubleTake', not '%s'.",
                                                        tokens[6 + 16 * i]);
                                return;
                        }
                }
                if (g_strcmp0 ("Output", tokens[7 + 17 * i])) {
                        if (self->priv->debug)
                                g_message ("Expected 'Output', not '%s'.",
                                                tokens[7 + 16 * i]);
                        return;
                }

                for (j = 0; j < 7; ++j) {
                        a->da_p[i][j] = g_ascii_strtod (tokens[8 + 17 * i + j],
                                                        &endptr);
                        if (errno || !endptr || *endptr)
                                return;
                }

                /* Discard standard deviations.  We do not display them.  */
        }

        /* Discard the rollout context.  */

        a->da = TRUE;
        a->da_rollout = TRUE;
}

gboolean
gibbon_sgf_reader_move_variant (const GibbonSGFReader *self,
                                GtkListStore *store, GtkTreeIter *iter,
                                gchar **tokens,
                                const GibbonPosition *pos,
                                GibbonPositionSide side,
                                guint die1, guint die2)
{
        guint num_tokens;

        num_tokens = g_strv_length (tokens);

        if (15 > num_tokens) {
                if (self->priv->debug)
                        g_message ("Invalid number of tokens %u in A record.",
                                        num_tokens);
                return FALSE;
        }

        if (tokens[1][1]) {
                g_message (_("Unsupported move analysis type '%s'."),
                           tokens[1]);
                return FALSE;
        } else if ('E' == tokens[1][0]) {
                return gibbon_sgf_reader_move_variant_eval (self, store,
                                                            iter, tokens,
                                                            pos, side,
                                                            die1, die1);
        } else if ('X' == tokens[1][0]) {
                return gibbon_sgf_reader_move_variant_rollout (self, store,
                                                               iter, tokens,
                                                               pos, side,
                                                               die1, die1);
        } else {
                g_message (_("Unsupported move analysis type '%s'."),
                           tokens[1]);
                return FALSE;
        }
}

gboolean
gibbon_sgf_reader_move_variant_eval (const GibbonSGFReader *self,
                                     GtkListStore *store, GtkTreeIter *iter,
                                     gchar **tokens,
                                     const GibbonPosition *pos,
                                     GibbonPositionSide side,
                                     guint die1, guint die2)
{
        gchar *endptr;
        guint i;
        guint64 plies;
        gboolean cubeful;
        gchar *encoded_move;
        gdouble p[6];
        gsize l, num_movements;
        GibbonMove *move;
        GibbonMovement *movement;
        gint from, to;
        gchar *analysis_type;
        gchar *formatted_move;
        gdouble equity;
        guint scores[2];
        GibbonPosition *new_pos;

        encoded_move = tokens[0];
        l = strlen (encoded_move);
        if (!l || (l & 1) || l > 8) {
                if (self->priv->debug)
                        g_message ("Invalid encoded move '%s' in A record.",
                                        encoded_move);
                return FALSE;
        }
        for (i = 0; i < l; ++i) {
                if (encoded_move[i] < 'a' || encoded_move[i] > 'z') {
                        if (self->priv->debug)
                                g_message ("Invalid encoded move '%s' in A record.",
                                        encoded_move);
                        return FALSE;
                }
        }

        if (g_strcmp0 ("ver", tokens[2])) {
                if (self->priv->debug)
                        g_message ("Expected 'ver' not '%s' in A record.",
                                        tokens[2]);
                return FALSE;
        }
        if (!gibbon_chareq ("3", tokens[3])) {
                g_message (_("Unsupported version %s for A property."),
                           tokens[3]);
                return FALSE;
        }

        errno = 0;
        for (i = 0; i < 6; ++i) {
                p[i] = g_ascii_strtod (tokens[4 + i], &endptr);
                        if (errno || !endptr || *endptr) {
                                if (self->priv->debug)
                                        g_message ("Garbage number in A record: %s.",
                                                        tokens[4 + i]);
                                return FALSE;
                        }
        }

        errno = 0;
        plies = g_ascii_strtoull (tokens[10], &endptr, 10);
        if (errno || !endptr)
                return FALSE;
        if (!*endptr)
                cubeful = FALSE;
        else if (gibbon_chareq ("C", endptr))
                cubeful = TRUE;
        else
                return FALSE;

        /* Token #11 is always 0.  Ignore it.  */

        if ('1' == tokens[12][0])
                ;
        else if ('0' == tokens[12][0])
                ;
        else {
                if (self->priv->debug)
                        g_message ("Expected [01] in det. flag for A not '%s'.",
                                        tokens[12]);
        }
        if (tokens[12][1]) {
                if (self->priv->debug)
                        g_message ("Expected [01] in det. flag for A not '%s'.",
                                        tokens[12]);
                return FALSE;
        }

        errno = 0;
        /* noise = */
        (void) g_ascii_strtod (tokens[13], &endptr);
        if (errno || !endptr || *endptr) {
                if (self->priv->debug)
                        g_message ("Garbage number in A record: %s.",
                                        tokens[13]);
                return FALSE;
        }

        if ('1' == tokens[14][0])
                ;
        else if ('0' == tokens[6][0])
                ;
        else {
                if (self->priv->debug)
                        g_message ("Expected [01] in prune flag for A not '%s'.",
                                        tokens[14]);
        }
        if (tokens[14][1]) {
                if (self->priv->debug)
                        g_message ("Expected [01] in prune flag for A not '%s'.",
                                        tokens[14]);
                return FALSE;
        }

        num_movements = l >> 1;
        move = gibbon_move_new (die1, die2, num_movements);
        move->number = num_movements;
        for (i = 0; i < num_movements; ++i) {
                movement = move->movements + i;
                from = encoded_move[2 * i] - 'a';
                to = encoded_move[2 * i + 1] - 'a';
                if (from == 24) {
                        if (to < 6)
                                from = 0;
                        else
                                from = 25;
                } else {
                        ++from;
                }
                if (to == 25) {
                        if (from <= 6)
                                to = 0;
                } else {
                        ++to;
                }
                from = 25 - from;
                to = 25 - to;
                movement->from = from;
                movement->to = to;
        }
        gibbon_move_sort (move);

        if (cubeful)
                analysis_type = g_strdup_printf (_("Cubeful %llu-ply"),
                                (unsigned long long) plies);
        else
                analysis_type = g_strdup_printf (_("Cubeless %llu-ply"),
                                (unsigned long long) plies);

        formatted_move = gibbon_position_format_move (pos, move, side, FALSE);

        new_pos = gibbon_position_copy (pos);
        gibbon_position_apply_move (new_pos, move, side, FALSE);

        errno = 0;
        equity = p[5];

        if (side > 0) {
                scores[0] = pos->scores[0];
                scores[1] = pos->scores[1];
        } else {
                scores[0] = pos->scores[1];
                scores[1] = pos->scores[0];
        }
        gtk_list_store_set (store, iter,
                            GIBBON_VARIANT_LIST_COL_ANALYSIS_TYPE,
                            analysis_type,
                            GIBBON_VARIANT_LIST_COL_MOVE, formatted_move,
                            GIBBON_VARIANT_LIST_COL_EQUITY, equity,
                            GIBBON_VARIANT_LIST_COL_MATCH_LENGTH,
                            pos->match_length,
                            GIBBON_VARIANT_LIST_COL_CUBE, pos->cube,
                            GIBBON_VARIANT_LIST_COL_MY_SCORE, scores[0],
                            GIBBON_VARIANT_LIST_COL_OPP_SCORE, scores[1],
                            GIBBON_VARIANT_LIST_COL_PWIN, p[0],
                            GIBBON_VARIANT_LIST_COL_PWIN_G, p[1],
                            GIBBON_VARIANT_LIST_COL_PWIN_BG, p[2],
                            GIBBON_VARIANT_LIST_COL_PLOSE, 1.0f - p[0],
                            GIBBON_VARIANT_LIST_COL_PLOSE_G, p[3],
                            GIBBON_VARIANT_LIST_COL_PLOSE_BG, p[4],
                            GIBBON_VARIANT_LIST_COL_POSITION, new_pos,
                            -1);
        g_free (analysis_type);
        g_free (formatted_move);
        gibbon_position_free (new_pos);

        return TRUE;
}

gboolean
gibbon_sgf_reader_move_variant_rollout (const GibbonSGFReader *self,
                                     GtkListStore *store, GtkTreeIter *iter,
                                     gchar **tokens,
                                     const GibbonPosition *pos,
                                     GibbonPositionSide side,
                                     guint die1, guint die2)
{
        gchar *endptr;
        guint i;
        guint64 trials;
        gchar *encoded_move;
        gdouble p[6];
        gsize l, num_movements;
        GibbonMove *move;
        GibbonMovement *movement;
        gint from, to;
        gchar *analysis_type;
        gchar *formatted_move;
        gdouble equity;
        guint scores[2];
        guint num_tokens;
        GibbonPosition* new_pos;

        num_tokens = g_strv_length (tokens);

        if (17 > num_tokens) {
                if (self->priv->debug)
                        g_message ("Invalid number of tokens %u in A record.",
                                        num_tokens);
                return FALSE;
        }

        encoded_move = tokens[0];
        l = strlen (encoded_move);
        if (!l || (l & 1) || l > 8) {
                if (self->priv->debug)
                        g_message ("Invalid encoded move '%s' in A record.",
                                        encoded_move);
                return FALSE;
        }

        for (i = 0; i < l; ++i) {
                if (encoded_move[i] < 'a' || encoded_move[i] > 'z') {
                        if (self->priv->debug)
                                g_message ("Invalid encoded move '%s' in A record.",
                                                encoded_move);
                        return FALSE;
                }
        }

        if (g_strcmp0 ("ver", tokens[2])) {
                if (self->priv->debug)
                        g_message ("Expected 'ver' not '%s' in A record.",
                                        tokens[2]);
                return FALSE;
        }
        if (!gibbon_chareq ("3", tokens[3])) {
                g_message (_("Unsupported version %s for A property."),
                           tokens[3]);
                return FALSE;
        }

        if (g_strcmp0 ("Trials", tokens[7])) {
                if (self->priv->debug)
                        g_message ("Expected 'Trials' not '%s' in A record.",
                                        tokens[4]);
                return FALSE;
        }

        errno = 0;
        trials = g_ascii_strtoull (tokens[8], &endptr, 10);
        if (errno || !endptr || *endptr) {
                if (self->priv->debug)
                        g_message ("Invalid number of trials in A record: %s.",
                                        tokens[8]);
                return FALSE;
        }

        if (g_strcmp0 ("Output", tokens[9])) {
                if (self->priv->debug)
                        g_message ("Expected 'Output' not '%s' in A record.",
                                        tokens[4]);
                return FALSE;
        }

        errno = 0;
        for (i = 0; i < 6; ++i) {
                p[i] = g_ascii_strtod (tokens[10 + i], &endptr);
                        if (errno || !endptr || *endptr) {
                                if (self->priv->debug)
                                        g_message ("Garbage number in A record: %s.",
                                                        tokens[4 + i]);
                                return FALSE;
                        }
        }

        num_movements = l >> 1;
        move = gibbon_move_new (die1, die2, num_movements);
        move->number = num_movements;
        for (i = 0; i < num_movements; ++i) {
                movement = move->movements + i;
                from = encoded_move[2 * i] - 'a';
                to = encoded_move[2 * i + 1] - 'a';
                if (from == 24) {
                        if (to < 6)
                                from = 0;
                        else
                                from = 25;
                } else {
                        ++from;
                }
                if (to == 25) {
                        if (from <= 6)
                                to = 0;
                } else {
                        ++to;
                }
                from = 25 - from;
                to = 25 - to;
                movement->from = from;
                movement->to = to;
        }
        gibbon_move_sort (move);

        analysis_type = g_strdup_printf (_("Rollout (%llu trials)"),
                                         (unsigned long long) trials);

        formatted_move = gibbon_position_format_move (pos, move, side, FALSE);

        new_pos = gibbon_position_copy (pos);
        gibbon_position_apply_move (new_pos, move, side, FALSE);

        equity = p[5];

        if (side > 0) {
                scores[0] = pos->scores[0];
                scores[1] = pos->scores[1];
        } else {
                scores[0] = pos->scores[1];
                scores[1] = pos->scores[0];
        }
        gtk_list_store_set (store, iter,
                            GIBBON_VARIANT_LIST_COL_ANALYSIS_TYPE,
                            analysis_type,
                            GIBBON_VARIANT_LIST_COL_MOVE, formatted_move,
                            GIBBON_VARIANT_LIST_COL_EQUITY, equity,
                            GIBBON_VARIANT_LIST_COL_MATCH_LENGTH,
                            pos->match_length,
                            GIBBON_VARIANT_LIST_COL_CUBE, pos->cube,
                            GIBBON_VARIANT_LIST_COL_MY_SCORE, scores[0],
                            GIBBON_VARIANT_LIST_COL_OPP_SCORE, scores[1],
                            GIBBON_VARIANT_LIST_COL_PWIN, p[0],
                            GIBBON_VARIANT_LIST_COL_PWIN_G, p[1],
                            GIBBON_VARIANT_LIST_COL_PWIN_BG, p[2],
                            GIBBON_VARIANT_LIST_COL_PLOSE, 1.0f - p[0],
                            GIBBON_VARIANT_LIST_COL_PLOSE_G, p[3],
                            GIBBON_VARIANT_LIST_COL_PLOSE_BG, p[4],
                            GIBBON_VARIANT_LIST_COL_POSITION, new_pos,
                            -1);
        g_free (analysis_type);
        g_free (formatted_move);
        gibbon_position_free (new_pos);

        return TRUE;
}

static gboolean
gibbon_sgf_reader_setup_pre_check (GibbonSGFReader *self,
                                   const GibbonMatch *match,
                                   const gchar *prop,
                                   GError **error)
{
        GibbonGame *game;
        GibbonPosition *pos;

        if (1 != gibbon_match_get_number_of_games (match)) {
                g_set_error (error, GIBBON_ERROR, -1,
                             _("SGF setup property `%s' only allowed in"
                               " first game!"), prop);
                return FALSE;
        }

        game = gibbon_match_get_current_game (match);
        if (gibbon_game_get_num_actions (game)) {
                g_set_error (error, GIBBON_ERROR, -1,
                             _("SGF setup property `%s' only allowed before"
                               " first regular game action!"), prop);
                return FALSE;
        }

        pos = gibbon_game_get_initial_position_editable (game);
        pos->scores[0] = self->priv->scores[0];
        pos->scores[1] = self->priv->scores[1];

        return TRUE;
}

static gboolean
gibbon_sgf_reader_setup_turn (GibbonSGFReader *self, GibbonMatch *match,
                              const GSGFProperty *prop, GError **error)
{
        GSGFColor *color;
        GibbonGame *game;
        GibbonPosition *pos;

        if (!gibbon_sgf_reader_setup_pre_check (self, match, "PL", error))
                return FALSE;

        color = GSGF_COLOR (gsgf_property_get_value (prop));
        game = gibbon_match_get_current_game (match);
        pos = gibbon_game_get_initial_position_editable (game);
        switch (gsgf_color_get_color (color)) {
        case GSGF_COLOR_WHITE:
                pos->turn = GIBBON_POSITION_SIDE_BLACK;
                break;
        case GSGF_COLOR_BLACK:
                pos->turn = GIBBON_POSITION_SIDE_WHITE;
                break;
        };
        return TRUE;
}

static gboolean
gibbon_sgf_reader_setup_dice (GibbonSGFReader *self, GibbonMatch *match,
                              const GSGFProperty *prop, GError **error)
{
        GSGFNumber *number;
        GibbonGame *game;
        GibbonPosition *pos;
        gint64 encoded;

        if (!gibbon_sgf_reader_setup_pre_check (self, match, "DI", error))
                return FALSE;

        number = GSGF_NUMBER (gsgf_property_get_value (prop));
        encoded = gsgf_number_get_value (number);
        if (encoded > 66 || encoded < 11 || !encoded % 10
            || encoded % 10 > 6) {
                g_set_error (error, GIBBON_ERROR, -1,
                             _("Invalid dice value `%lld' in SGF setup"
                               " property DI!"),
                             (long long) encoded);
                return FALSE;
        }
        game = gibbon_match_get_current_game (match);
        pos = gibbon_game_get_initial_position_editable (game);
        pos->dice[0] = encoded / 10;
        pos->dice[1] = encoded % 10;

        return TRUE;
}

static gboolean
gibbon_sgf_reader_setup_cube (GibbonSGFReader *self, GibbonMatch *match,
                              const GSGFProperty *prop, GError **error)
{
        GSGFNumber *number;
        GibbonGame *game;
        GibbonPosition *pos;
        gint64 cube;

        if (!gibbon_sgf_reader_setup_pre_check (self, match, "CV", error))
                return FALSE;

        number = GSGF_NUMBER (gsgf_property_get_value (prop));
        cube = gsgf_number_get_value (number);
        if (cube < 0) {
                g_set_error (error, GIBBON_ERROR, -1,
                             _("Invalid cube value `%lld' in SGF setup"
                               " property CV!"),
                             (long long) cube);
                return FALSE;
        }
        if (!cube)
                cube = 1;

        game = gibbon_match_get_current_game (match);
        pos = gibbon_game_get_initial_position_editable (game);
        pos->cube = cube;

        return TRUE;
}

static gboolean
gibbon_sgf_reader_setup_cube_owner (GibbonSGFReader *self, GibbonMatch *match,
                                    const GSGFProperty *prop, GError **error)
{
        GSGFText *text;
        GibbonGame *game;
        GibbonPosition *pos;
        gchar *owner;

        if (!gibbon_sgf_reader_setup_pre_check (self, match,
                                                gsgf_property_get_id (prop),
                                                error))
                return FALSE;

        text = GSGF_TEXT (gsgf_property_get_value (prop));
        owner = gsgf_text_get_value (text);

        game = gibbon_match_get_current_game (match);
        pos = gibbon_game_get_initial_position_editable (game);

        if (!g_ascii_strcasecmp ("b", owner)
            || !g_ascii_strcasecmp ("black", owner)) {
                pos->may_double[0] = TRUE;
                pos->may_double[1] = FALSE;
        } else if (!g_ascii_strcasecmp ("w", owner)
                   || !g_ascii_strcasecmp ("white", owner)) {
                        pos->may_double[0] = FALSE;
                        pos->may_double[1] = TRUE;
        } else if (!g_ascii_strcasecmp ("n", owner)
                   || !g_ascii_strcasecmp ("none", owner)
                   || !g_ascii_strcasecmp ("nobody", owner)
                   || !g_ascii_strcasecmp ("noone", owner)) {
                        pos->may_double[0] = FALSE;
                        pos->may_double[1] = FALSE;
        } else if (!g_ascii_strcasecmp ("c", owner)
                   || !g_ascii_strcasecmp ("centered", owner)
                   || !g_ascii_strcasecmp ("centred", owner)
                   || !g_ascii_strcasecmp ("center", owner)
                   || !g_ascii_strcasecmp ("centre", owner)) {
                        pos->may_double[0] = FALSE;
                        pos->may_double[1] = FALSE;
        } else {
                g_set_error (error, GIBBON_ERROR, -1,
                             _("Invalid cube owner `%s' in SGF setup"
                               " property %s!"),
                             owner, gsgf_property_get_id (prop));
                return FALSE;
        }

        return TRUE;
}

static gboolean
gibbon_sgf_reader_setup_add_empty (GibbonSGFReader *self, GibbonMatch *match,
                                   const GSGFProperty *prop, GError **error)
{
        GSGFListOf *values;
        GibbonGame *game;
        GibbonPosition *pos;
        gsize num_items, i;
        GSGFPointBackgammon *gsgf_point;
        gint point;

        if (!gibbon_sgf_reader_setup_pre_check (self, match, "AE", error))
                return FALSE;

        game = gibbon_match_get_current_game (match);
        pos = gibbon_game_get_initial_position_editable (game);

        values = GSGF_LIST_OF (gsgf_property_get_value (prop));
        num_items = gsgf_list_of_get_number_of_items (values);
        for (i = 0; i < num_items; ++i) {
                gsgf_point = GSGF_POINT_BACKGAMMON (
                                gsgf_list_of_get_nth_item (values, i));
                point = gsgf_point_backgammon_get_point (gsgf_point);
                /*
                 * GNUBG used to clear the bar as well.  But that makes no
                 * sense because it would clear both bars.
                 */
                if (point >= 0 && point < 24) {
                        pos->points[23 - point] = 0;
                }
        }

        return TRUE;
}

static gboolean
gibbon_sgf_reader_setup_add_white (GibbonSGFReader *self, GibbonMatch *match,
                                   const GSGFProperty *prop, GError **error)
{
        GSGFListOf *values;
        GibbonGame *game;
        GibbonPosition *pos;
        gsize num_items, i;
        GSGFStoneBackgammon *gsgf_stone;
        gint point;

        if (!gibbon_sgf_reader_setup_pre_check (self, match, "AB", error))
                return FALSE;

        game = gibbon_match_get_current_game (match);
        pos = gibbon_game_get_initial_position_editable (game);

        values = GSGF_LIST_OF (gsgf_property_get_value (prop));
        num_items = gsgf_list_of_get_number_of_items (values);
        for (i = 0; i < num_items; ++i) {
                gsgf_stone = GSGF_STONE_BACKGAMMON (
                                gsgf_list_of_get_nth_item (values, i));
                point = gsgf_stone_backgammon_get_stone (gsgf_stone);

                if (point >= 0 && point < 24) {
                        ++pos->points[23 - point];
                } else if (point == 24) {
                        ++pos->bar[0];
                }
        }

        return TRUE;
}

static gboolean
gibbon_sgf_reader_setup_add_black (GibbonSGFReader *self, GibbonMatch *match,
                                   const GSGFProperty *prop, GError **error)
{
        GSGFListOf *values;
        GibbonGame *game;
        GibbonPosition *pos;
        gsize num_items, i;
        GSGFStoneBackgammon *gsgf_stone;
        gint point;

        if (!gibbon_sgf_reader_setup_pre_check (self, match, "AW", error))
                return FALSE;

        game = gibbon_match_get_current_game (match);
        pos = gibbon_game_get_initial_position_editable (game);

        values = GSGF_LIST_OF (gsgf_property_get_value (prop));
        num_items = gsgf_list_of_get_number_of_items (values);
        for (i = 0; i < num_items; ++i) {
                gsgf_stone = GSGF_STONE_BACKGAMMON (
                                gsgf_list_of_get_nth_item (values, i));
                point = gsgf_stone_backgammon_get_stone (gsgf_stone);

                if (point >= 0 && point < 24) {
                        --pos->points[23 - point];
                } else if (point == 24) {
                        --pos->bar[1];
                }
        }

        return TRUE;
}
