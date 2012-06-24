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

#include "gibbon-sgf-reader.h"

#include "gibbon-game.h"
#include "gibbon-game-action.h"
#include "gibbon-roll.h"
#include "gibbon-move.h"
#include "gibbon-double.h"
#include "gibbon-drop.h"
#include "gibbon-take.h"
#include "gibbon-resign.h"
#include "gibbon-reject.h"
#include "gibbon-accept.h"

#include "gibbon-analysis-roll.h"
#include "gibbon-analysis-move.h"

typedef struct _GibbonSGFReaderPrivate GibbonSGFReaderPrivate;
struct _GibbonSGFReaderPrivate {
        GibbonMatchReaderErrorFunc yyerror;
        gpointer user_data;

        /* Per-instance data.  */
        const gchar *filename;
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
                                                        const GSGFNode *node);

static void 
gibbon_sgf_reader_init (GibbonSGFReader *self)
{
        self->priv = G_TYPE_INSTANCE_GET_PRIVATE (self,
                GIBBON_TYPE_SGF_READER, GibbonSGFReaderPrivate);

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
                g_set_error_literal (error, 0, -1, _("No game in progress!"));
                g_object_unref (action);
                if (analysis)
                        g_object_unref (analysis);
                return FALSE;
        }

        if (!gibbon_game_add_action_with_analysis (game, side, action,
                                                   analysis, error)) {
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
                prop = gsgf_node_get_property (node, "B");
                if (prop) {
                        side = GIBBON_POSITION_SIDE_WHITE;
                } else {
                        prop = gsgf_node_get_property (node, "W");
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

        /* The only property we are currently interested in is the length.  */
        if (g_strcmp0 ("length", key))
                return TRUE;

        errno = 0;
        value = g_ascii_strtoull (string_value, &endptr, 010);
        if (errno) {
                g_set_error (error, 0, -1,
                             _("Invalid match length: %s!"),
                             strerror (errno));
                return FALSE;
        }

        if (value > G_MAXSIZE) {
                g_set_error (error, 0, -1,
                             _("Match length %llu out of range!"),
                             (unsigned long long) value);
                return FALSE;
        }

        gibbon_match_set_length (match, value);

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
                action = GIBBON_GAME_ACTION (move);
                analysis = gibbon_sgf_reader_move_analysis (self, node);
                if (!gibbon_sgf_reader_add_action (self, match, side, action,
                                                   analysis, error))
                        return FALSE;
        } else if (gsgf_move_backgammon_is_double (gsgf_move)) {
                action = GIBBON_GAME_ACTION (gibbon_double_new ());
                analysis = gibbon_sgf_reader_move_analysis (self, node);
                if (!gibbon_sgf_reader_add_action (self, match, side, action,
                                                   analysis, error))
                        return FALSE;
        } else if (gsgf_move_backgammon_is_drop (gsgf_move)) {
                action = GIBBON_GAME_ACTION (gibbon_drop_new ());
                analysis = gibbon_sgf_reader_move_analysis (self, node);
                if (!gibbon_sgf_reader_add_action (self, match, side, action,
                                                   analysis, error))
                        return FALSE;
        } else if (gsgf_move_backgammon_is_take (gsgf_move)) {
                action = GIBBON_GAME_ACTION (gibbon_take_new ());
                analysis = gibbon_sgf_reader_move_analysis (self, node);
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
                                 const GSGFNode *node)
{
        gboolean valid = FALSE;
        GibbonAnalysisMove *a = gibbon_analysis_move_new ();

        if (!valid) {
                g_object_unref (a);
                return NULL;
        }

        return GIBBON_ANALYSIS (a);
}
