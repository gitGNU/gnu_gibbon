/*
 * This file is part of gibbon.
 * Gibbon is a Gtk+ frontend for the First Internet Backgammon Server FIBS.
 * Copyright (C) 2009-2011 Guido Flohr, http://guido-flohr.net/.
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
 * SECTION:gibbon-game
 * @short_description: Representation of a single game of backgammon in Gibbon!
 *
 * Since: 0.1.0
 *
 * A #GibbonGame represents a single game of backgammon in Gibbon!  It is
 * always associated with a #GSGFGameTree that is used as a backend.
 **/

#include <stdlib.h>

#include <glib.h>
#include <glib/gi18n.h>

#include "gibbon-game.h"
#include "gibbon-position.h"
#include "gibbon-roll.h"
#include "gibbon-move.h"
#include "gibbon-double.h"
#include "gibbon-drop.h"
#include "gibbon-take.h"
#include "gibbon-resign.h"
#include "gibbon-accept.h"
#include "gibbon-reject.h"

typedef struct _GibbonGameSnapshot GibbonGameSnapshot;
struct _GibbonGameSnapshot {
        GibbonGameAction *action;
        GibbonPositionSide side;
        GibbonPosition *resulting_position;
};

typedef struct _GibbonGamePrivate GibbonGamePrivate;
struct _GibbonGamePrivate {
        GibbonMatch *match;
        GSGFGameTree *game_tree;

        GibbonPosition *initial_position;
        gsize num_snapshots;
        GibbonGameSnapshot *snapshots;

        gint score;
};

#define GIBBON_GAME_PRIVATE(obj) (G_TYPE_INSTANCE_GET_PRIVATE ((obj), \
        GIBBON_TYPE_GAME, GibbonGamePrivate))

G_DEFINE_TYPE (GibbonGame, gibbon_game, G_TYPE_OBJECT)

static gboolean gibbon_game_add_roll (GibbonGame *self,
                                      GibbonPositionSide side,
                                      GibbonRoll *roll);
static gboolean gibbon_game_add_move (GibbonGame *self,
                                      GibbonPositionSide side,
                                      GibbonMove *move);
static gboolean gibbon_game_add_double (GibbonGame *self,
                                      GibbonPositionSide side,
                                      GibbonDouble *dbl);
static gboolean gibbon_game_add_drop (GibbonGame *self,
                                      GibbonPositionSide side,
                                      GibbonDrop *drop);
static gboolean gibbon_game_add_take (GibbonGame *self,
                                      GibbonPositionSide side,
                                      GibbonTake *take);
static gboolean gibbon_game_add_resign (GibbonGame *self,
                                        GibbonPositionSide side,
                                        GibbonResign *resign);
static gboolean gibbon_game_add_reject (GibbonGame *self,
                                        GibbonPositionSide side,
                                        GibbonReject *reject);
static gboolean gibbon_game_add_accept (GibbonGame *self,
                                        GibbonPositionSide side,
                                        GibbonAccept *accept);
static gchar gibbon_game_point_to_sgf_char (GibbonPositionSide side,
                                            gint point);
static void gibbon_game_add_snapshot (GibbonGame *self,
                                      GibbonGameAction *action,
                                      GibbonPositionSide side,
                                      GibbonPosition *position);
static const GibbonGameSnapshot *gibbon_game_get_snapshot (const GibbonGame
                                                           *self);

static void 
gibbon_game_init (GibbonGame *self)
{
        self->priv = G_TYPE_INSTANCE_GET_PRIVATE (self,
                GIBBON_TYPE_GAME, GibbonGamePrivate);

        self->priv->game_tree = NULL;
        self->priv->match = NULL;
        self->priv->initial_position = NULL;
        self->priv->snapshots = NULL;
        self->priv->num_snapshots = 0;

        self->priv->score = 0;
}

static void
gibbon_game_finalize (GObject *object)
{
        GibbonGame *self = GIBBON_GAME (object);
        gsize i;
        GibbonGameSnapshot *snapshot;

        if (self->priv->initial_position)
                gibbon_position_free (self->priv->initial_position);

        for (i = 0; i < self->priv->num_snapshots; ++i) {
                snapshot = &self->priv->snapshots[i];
                g_object_unref (snapshot->action);
                gibbon_position_free (snapshot->resulting_position);
       }
        g_free (self->priv->snapshots);

        G_OBJECT_CLASS (gibbon_game_parent_class)->finalize(object);
}

static void
gibbon_game_class_init (GibbonGameClass *klass)
{
        GObjectClass *object_class = G_OBJECT_CLASS (klass);
        
        g_type_class_add_private (klass, sizeof (GibbonGamePrivate));

        object_class->finalize = gibbon_game_finalize;
}

GibbonGame *
gibbon_game_new (GibbonMatch *match, GSGFGameTree *game_tree,
                 const GibbonPosition *pos,
                 guint game_number,
                 gboolean crawford, gboolean is_crawford)
{
        GibbonGame *self = g_object_new (GIBBON_TYPE_GAME, NULL);
        const GSGFFlavor *flavor;
        GSGFNode *root;
        GSGFValue *simple_text;
        GSGFValue *match_info;
        gchar *str;
        GSGFCookedValue *mi_key, *mi_value;
        GSGFCookedValue *mi_compose;
        GError *error = NULL;
        const gchar *rule;

        self->priv->match = match;
        self->priv->game_tree = game_tree;

        flavor = gsgf_game_tree_get_flavor (game_tree);

        root = gsgf_game_tree_add_node (game_tree);
        if (!gsgf_game_tree_set_application (game_tree,
                                             PACKAGE, VERSION,
                                             &error)) {
                g_warning ("%s", error->message);
                g_error_free (error);
                return self;
        }
        simple_text = GSGF_VALUE (gsgf_simple_text_new (pos->players[0]));
        if (!gsgf_node_set_property (root, "PB", simple_text, &error)) {
                g_object_unref (simple_text);
                g_warning ("%s", error->message);
                g_error_free (error);
                return self;
        }
        simple_text = GSGF_VALUE (gsgf_simple_text_new (pos->players[1]));
        if (!gsgf_node_set_property (root, "PW", simple_text, &error)) {
                g_object_unref (simple_text);
                g_warning ("%s", error->message);
                g_error_free (error);
                return self;
        }
        if (crawford) {
                rule = is_crawford ? "Crawford:CrawfordGame" : "Crawford";
                simple_text = GSGF_VALUE (gsgf_simple_text_new (rule));
                if (!gsgf_node_set_property (root, "RU", simple_text, &error)) {
                        g_object_unref (simple_text);
                        g_warning ("%s", error->message);
                        g_error_free (error);
                        return self;
                }
        }

        match_info = GSGF_VALUE (gsgf_list_of_new (gsgf_compose_get_type (),
                                                   flavor));
        if (!gsgf_node_set_property (root, "MI", match_info, &error)) {
                g_object_unref (match_info);
                g_warning ("%s", error->message);
                g_error_free (error);
                return self;
        }

        mi_key = GSGF_COOKED_VALUE (gsgf_simple_text_new ("length"));
        str = g_strdup_printf ("%u", pos->match_length);
        mi_value = GSGF_COOKED_VALUE (gsgf_simple_text_new (str));
        g_free (str);
        mi_compose = GSGF_COOKED_VALUE (gsgf_compose_new (mi_key, mi_value,
                                                          NULL));
        if (!gsgf_list_of_append (GSGF_LIST_OF (match_info),
                                  mi_compose, &error)) {
                g_object_unref (mi_compose);
                g_warning ("%s", error->message);
                g_error_free (error);
                return self;
        }

        mi_key = GSGF_COOKED_VALUE (gsgf_simple_text_new ("game"));
        str = g_strdup_printf ("%u", game_number);
        mi_value = GSGF_COOKED_VALUE (gsgf_simple_text_new (str));
        g_free (str);
        mi_compose = GSGF_COOKED_VALUE (gsgf_compose_new (mi_key, mi_value,
                                                          NULL));
        if (!gsgf_list_of_append (GSGF_LIST_OF (match_info),
                                  mi_compose, &error)) {
                g_object_unref (mi_compose);
                g_warning ("%s", error->message);
                g_error_free (error);
                return self;
        }

        mi_key = GSGF_COOKED_VALUE (gsgf_simple_text_new ("bs"));
        str = g_strdup_printf ("%u", pos->scores[1]);
        mi_value = GSGF_COOKED_VALUE (gsgf_simple_text_new (str));
        g_free (str);
        mi_compose = GSGF_COOKED_VALUE (gsgf_compose_new (mi_key, mi_value,
                                                          NULL));
        if (!gsgf_list_of_append (GSGF_LIST_OF (match_info),
                                  mi_compose, &error)) {
                g_object_unref (mi_compose);
                g_warning ("%s", error->message);
                g_error_free (error);
                return self;
        }

        mi_key = GSGF_COOKED_VALUE (gsgf_simple_text_new ("ws"));
        str = g_strdup_printf ("%u", pos->scores[0]);
        mi_value = GSGF_COOKED_VALUE (gsgf_simple_text_new (str));
        g_free (str);
        mi_compose = GSGF_COOKED_VALUE (gsgf_compose_new (mi_key, mi_value,
                                                          NULL));
        if (!gsgf_list_of_append (GSGF_LIST_OF (match_info),
                                  mi_compose, &error)) {
                g_object_unref (mi_compose);
                g_warning ("%s", error->message);
                g_error_free (error);
                return self;
        }
        self->priv->initial_position = gibbon_position_copy (pos);

        return self;
}

GSGFGameTree *
gibbon_game_get_game_tree (const GibbonGame *self)
{
        g_return_val_if_fail (GIBBON_IS_GAME (self), NULL);

        return self->priv->game_tree;
}

gboolean
gibbon_game_add_action (GibbonGame *self, GibbonPositionSide side,
                        GibbonGameAction *action)
{
        g_return_val_if_fail (GIBBON_IS_GAME (self), FALSE);
        g_return_val_if_fail (GIBBON_IS_GAME_ACTION (action), FALSE);
        g_return_val_if_fail (side == GIBBON_POSITION_SIDE_WHITE
                              || GIBBON_POSITION_SIDE_BLACK,
                              FALSE);

        if (GIBBON_IS_ROLL (action)) {
                return gibbon_game_add_roll (self, side, GIBBON_ROLL (action));
        } else if (GIBBON_IS_MOVE (action)) {
                return gibbon_game_add_move (self, side, GIBBON_MOVE (action));
        } else if (GIBBON_IS_DOUBLE (action)) {
                return gibbon_game_add_double (self, side,
                                               GIBBON_DOUBLE (action));
        } else if (GIBBON_IS_DROP (action)) {
                return gibbon_game_add_drop (self, side,
                                             GIBBON_DROP (action));
        } else if (GIBBON_IS_TAKE (action)) {
                return gibbon_game_add_take (self, side,
                                             GIBBON_TAKE (action));
        } else if (GIBBON_IS_RESIGN (action)) {
                return gibbon_game_add_resign (self, side,
                                               GIBBON_RESIGN (action));
        } else if (GIBBON_IS_REJECT (action)) {
                return gibbon_game_add_reject (self, side,
                                               GIBBON_REJECT (action));
        } else if (GIBBON_IS_ACCEPT (action)) {
                return gibbon_game_add_accept (self, side,
                                               GIBBON_ACCEPT (action));
        } else {
                g_critical ("gibbon_game_add_action: unsupported action type"
                            " %s!", G_OBJECT_TYPE_NAME (action));
                return FALSE;
        }

        return TRUE;
}

static gboolean
gibbon_game_add_roll (GibbonGame *self, GibbonPositionSide side,
                      GibbonRoll *roll)
{
        GSGFNode *node;
        GSGFProperty *property;
        GError *error = NULL;
        gchar raw_string[3];
        GSGFRaw *raw;
        GibbonPosition *pos;
        GibbonGameSnapshot *snapshot = NULL;

        g_return_val_if_fail (self->priv->score == 0, FALSE);

        if (self->priv->snapshots && self->priv->num_snapshots) {
                snapshot = self->priv->snapshots
                                + self->priv->num_snapshots - 1;
                pos = gibbon_position_copy (snapshot->resulting_position);
        } else {
                pos = gibbon_position_copy (self->priv->initial_position);
        }

        pos->dice[0] = roll->die1;
        pos->dice[1] = roll->die2;

        gibbon_game_add_snapshot (self, GIBBON_GAME_ACTION (roll), side, pos);

        raw_string[0] = '0' + abs (pos->dice[0]);
        raw_string[1] = '0' + abs (pos->dice[1]);
        raw_string[2] = 0;

        node = gsgf_game_tree_add_node (self->priv->game_tree);
        property = gsgf_node_add_property (node, "DI", &error);
        if (!property) {
                g_warning ("gibbon_game_add_roll: %s!",
                            error->message);
                g_error_free (error);
                return FALSE;
        }

        raw = gsgf_raw_new (raw_string);
        if (!gsgf_property_set_value (property, GSGF_VALUE (raw), &error)) {
                g_warning ("gibbon_game_add_roll: %s!",
                            error->message);
                g_error_free (error);
                return FALSE;
        }

        if (side != GIBBON_POSITION_SIDE_NONE) {
                property = gsgf_node_add_property (node, "PL", &error);
                if (!property) {
                        g_warning ("gibbon_game_add_roll: %s!",
                                    error->message);
                        g_error_free (error);
                        return FALSE;
                }

                raw = gsgf_raw_new (side == GIBBON_POSITION_SIDE_WHITE
                                    ? "B" : "W");
                if (!gsgf_property_set_value (property, GSGF_VALUE (raw),
                                              &error)) {
                        g_warning ("gibbon_game_add_roll: %s!",
                                    error->message);
                        g_error_free (error);
                        return FALSE;
                }
        }

        return TRUE;
}

static gboolean
gibbon_game_add_move (GibbonGame *self, GibbonPositionSide side,
                      GibbonMove *move)
{
        gchar move_string[11];
        GSGFNode *node = NULL;
        GSGFProperty *property;
        GError *error = NULL;
        const gchar *id;
        GSGFRaw *raw;
        gsize i, j;
        GibbonMovement *movement;
        GibbonPosition *pos;
        const GibbonGameSnapshot *snapshot = NULL;
        gboolean delete_roll = FALSE;
        GList *nodes;
        gchar *pretty_move;

        g_return_val_if_fail (move->number <= 4, FALSE);
        g_return_val_if_fail (self->priv->score == 0, FALSE);

        snapshot = gibbon_game_get_snapshot (self);
        pos = gibbon_position_copy (gibbon_game_get_position (self));
        if (snapshot && GIBBON_IS_ROLL (snapshot->action)
            && side == snapshot->side)
                delete_roll = TRUE;

        if (!gibbon_position_apply_move (pos, move, side, FALSE)) {
                gibbon_position_free (pos);
                return FALSE;
        }

        g_free (pos->status);
        if (move->movements) {
                pretty_move = gibbon_position_format_move (pos, move, side,
                                                           FALSE);
                pos->status = g_strdup_printf (_("%d%d: %s has moved %s."),
                                               move->die1, move->die2,
                                               side
                                               == GIBBON_POSITION_SIDE_BLACK
                                               ? pos->players[1]
                                               : pos->players[0],
                                               pretty_move);
        } else {
                pos->status = g_strdup_printf (_("%d%d: %s cannot move!"),
                                               move->die1, move->die2,
                                               side
                                               == GIBBON_POSITION_SIDE_BLACK
                                               ? pos->players[1]
                                               : pos->players[0]);
        }

        gibbon_game_add_snapshot (self, GIBBON_GAME_ACTION (move), side, pos);

        /*
         * When exporting to SGF, we swap sides in order to match GNU
         * backgammon's notion of colors and directions.
         */
        id = side == GIBBON_POSITION_SIDE_BLACK ? "W" : "B";

        move_string[0] = move->die1 + '0';
        move_string[1] = move->die2 + '0';
        j = 1;
        for (i = 0; i < move->number; ++i) {
                movement = move->movements + i;
                move_string[++j] =
                        gibbon_game_point_to_sgf_char (side,
                                                       move->movements[i].from);
                move_string[++j] =
                        gibbon_game_point_to_sgf_char (side,
                                                       move->movements[i].to);
        }
        move_string[++j] = 0;

        if (delete_roll) {
                nodes = gsgf_game_tree_get_nodes (self->priv->game_tree);
                nodes = g_list_last (nodes);
                if (nodes) {
                        node = nodes->data;
                        gsgf_node_remove_property (node, "DI");
                        gsgf_node_remove_property (node, "PL");
                }
        }

        if (!node)
                node = gsgf_game_tree_add_node (self->priv->game_tree);
        property = gsgf_node_add_property (node, id, &error);
        if (!property) {
                g_warning ("gibbon_game_add_move: %s!",
                            error->message);
                g_error_free (error);
                return FALSE;
        }

        raw = gsgf_raw_new (move_string);
        if (!gsgf_property_set_value (property, GSGF_VALUE (raw), &error)) {
                g_warning ("gibbon_game_add_move: %s!",
                            error->message);
                g_error_free (error);
                return FALSE;
        }

        return TRUE;
}

static gboolean
gibbon_game_add_double (GibbonGame *self, GibbonPositionSide side,
                        GibbonDouble *dbl)
{
        const gchar *id;
        GSGFNode *node;
        GSGFProperty *property;
        GError *error = NULL;
        GSGFRaw *raw;
        GibbonPosition *pos;

        g_return_val_if_fail (self->priv->score == 0, FALSE);

        pos = gibbon_position_copy (gibbon_game_get_position (self));

        g_free (pos->status);
        if (side == GIBBON_POSITION_SIDE_WHITE) {
                pos->cube_turned = GIBBON_POSITION_SIDE_BLACK;
                pos->status = g_strdup_printf (_("%s offers a double."),
                                               pos->players[0]);
        } else {
                pos->cube_turned = GIBBON_POSITION_SIDE_WHITE;
                pos->status = g_strdup_printf (_("%s offers a double."),
                                               pos->players[1]);
        }

        gibbon_game_add_snapshot (self, GIBBON_GAME_ACTION (dbl), side, pos);

        /*
         * When exporting to SGF, we swap sides in order to match GNU
         * backgammon's notion of colors and directions.
         */
        id = side == GIBBON_POSITION_SIDE_BLACK ? "W" : "B";

        node = gsgf_game_tree_add_node (self->priv->game_tree);
        property = gsgf_node_add_property (node, id, &error);
        if (!property) {
                g_warning ("gibbon_game_add_double: %s!",
                            error->message);
                g_error_free (error);
                return FALSE;
        }

        raw = gsgf_raw_new ("double");
        if (!gsgf_property_set_value (property, GSGF_VALUE (raw), &error)) {
                g_warning ("gibbon_game_add_move: %s!",
                            error->message);
                g_error_free (error);
                return FALSE;
        }

        return TRUE;
}

static gboolean
gibbon_game_add_drop (GibbonGame *self, GibbonPositionSide side,
                      GibbonDrop *drop)
{
        const gchar *id1;
        const gchar *id2;
        GSGFNode *node;
        GSGFProperty *property;
        GError *error = NULL;
        GSGFRaw *raw;
        GibbonPosition *pos;
        gchar *raw_string;

        g_return_val_if_fail (self->priv->score == 0, FALSE);

        pos = gibbon_position_copy (gibbon_game_get_position (self));

        /*
         * When exporting to SGF, we swap sides in order to match GNU
         * backgammon's notion of colors and directions.
         */
        g_free (pos->status);
        if (side == GIBBON_POSITION_SIDE_WHITE) {
                id1 = "B";
                id2 = "W";
                self->priv->score = -pos->cube;
                pos->scores[1] += pos->cube;
                pos->status = g_strdup_printf (_("%s refuses the cube."),
                                               pos->players[0]);

        } else {
                id1 = "W";
                id2 = "B";
                self->priv->score = pos->cube;
                pos->scores[0] += pos->cube;
                pos->status = g_strdup_printf (_("%s refuses the cube."),
                                               pos->players[0]);
        }

        gibbon_game_add_snapshot (self, GIBBON_GAME_ACTION (drop), side, pos);

        node = gsgf_game_tree_add_node (self->priv->game_tree);
        property = gsgf_node_add_property (node, id1, &error);
        if (!property) {
                g_warning ("gibbon_game_add_double: %s!",
                            error->message);
                g_error_free (error);
                return FALSE;
        }

        raw = gsgf_raw_new ("drop");
        if (!gsgf_property_set_value (property, GSGF_VALUE (raw), &error)) {
                g_warning ("gibbon_game_add_move: %s!",
                            error->message);
                g_error_free (error);
                return FALSE;
        }

        property = gsgf_node_add_property (node, "RE", &error);
        if (!property) {
                g_warning ("gibbon_game_add_double: %s!",
                            error->message);
                g_error_free (error);
                return FALSE;
        }

        raw_string = g_strdup_printf ("%s+%d", id2, pos->cube);
        raw = gsgf_raw_new (raw_string);
        g_free (raw_string);
        if (!gsgf_property_set_value (property, GSGF_VALUE (raw), &error)) {
                g_warning ("gibbon_game_add_move: %s!",
                            error->message);
                g_error_free (error);
                return FALSE;
        }

        return TRUE;
}

static gboolean
gibbon_game_add_take (GibbonGame *self, GibbonPositionSide side,
                      GibbonTake *take)
{
        const gchar *id;
        GSGFNode *node;
        GSGFProperty *property;
        GError *error = NULL;
        GSGFRaw *raw;
        GibbonPosition *pos;

        g_return_val_if_fail (self->priv->score == 0, FALSE);

        pos = gibbon_position_copy (gibbon_game_get_position (self));

        /*
         * When exporting to SGF, we swap sides in order to match GNU
         * backgammon's notion of colors and directions.
         */
        if (side == GIBBON_POSITION_SIDE_BLACK) {
                id = "W";
        } else {
                id = "B";
        }
        pos->cube <<= 1;

        gibbon_game_add_snapshot (self, GIBBON_GAME_ACTION (take), side, pos);

        node = gsgf_game_tree_add_node (self->priv->game_tree);
        property = gsgf_node_add_property (node, id, &error);
        if (!property) {
                g_warning ("gibbon_game_add_take: %s!",
                            error->message);
                g_error_free (error);
                return FALSE;
        }

        raw = gsgf_raw_new ("take");
        if (!gsgf_property_set_value (property, GSGF_VALUE (raw), &error)) {
                g_warning ("gibbon_game_add_take: %s!",
                            error->message);
                g_error_free (error);
                return FALSE;
        }

        return TRUE;
}

static gboolean
gibbon_game_add_resign (GibbonGame *self, GibbonPositionSide side,
                        GibbonResign *resign)
{
        const gchar *color;
        GSGFNode *node;
        GSGFProperty *property;
        GError *error = NULL;
        GSGFRaw *raw;
        GibbonPosition *pos;
        gchar *raw_string;

        g_return_val_if_fail (self->priv->score == 0, FALSE);

        pos = gibbon_position_copy (gibbon_game_get_position (self));

        if (side == GIBBON_POSITION_SIDE_BLACK) {
                color = "B";
        } else {
                color = "B";
        }

        gibbon_game_add_snapshot (self, GIBBON_GAME_ACTION (resign), side, pos);

        node = gsgf_game_tree_add_node (self->priv->game_tree);
        property = gsgf_node_add_property (node, "OR", &error);
        if (!property) {
                g_warning ("gibbon_game_add_resign: %s!",
                            error->message);
                g_error_free (error);
                return FALSE;
        }

        raw_string = g_strdup_printf ("%s-%u",
                                      color, pos->cube * resign->value);
        raw = gsgf_raw_new (raw_string);
        g_free (raw_string);
        if (!gsgf_property_set_value (property, GSGF_VALUE (raw), &error)) {
                g_warning ("gibbon_game_add_resign: %s!",
                            error->message);
                g_error_free (error);
                return FALSE;
        }

        return TRUE;
}

static gboolean
gibbon_game_add_reject (GibbonGame *self, GibbonPositionSide side,
                        GibbonReject *reject)
{
        const gchar *color;
        GSGFNode *node;
        GSGFProperty *property;
        GError *error = NULL;
        GSGFRaw *raw;
        GibbonPosition *pos;
        gchar *player;

        g_return_val_if_fail (self->priv->score == 0, FALSE);

        pos = gibbon_position_copy (gibbon_game_get_position (self));

        if (side == GIBBON_POSITION_SIDE_BLACK) {
                color = "W";
                player = pos->players[1];
        } else {
                color = "B";
                player = pos->players[0];
        }

        g_free (pos->status);
        pos->status = g_strdup_printf (_("%s rejects."), player);

        gibbon_game_add_snapshot (self, GIBBON_GAME_ACTION (reject), side, pos);

        node = gsgf_game_tree_add_node (self->priv->game_tree);
        property = gsgf_node_add_property (node, "RR", &error);
        if (!property) {
                g_warning ("gibbon_game_add_reject: %s!",
                            error->message);
                g_error_free (error);
                return FALSE;
        }

        raw = gsgf_raw_new (color);
        if (!gsgf_property_set_value (property, GSGF_VALUE (raw), &error)) {
                g_warning ("gibbon_game_add_reject: %s!",
                            error->message);
                g_error_free (error);
                return FALSE;
        }

        return TRUE;
}

static gboolean
gibbon_game_add_accept (GibbonGame *self, GibbonPositionSide side,
                        GibbonAccept *accept)
{
        GSGFNode *node;
        GSGFProperty *property;
        GSGFRaw *raw;
        GError *error = NULL;
        GibbonPosition *pos;
        const GibbonGameSnapshot *snapshot = NULL;
        GList *nodes;
        GibbonPositionSide other;
        GibbonResign* resign;
        gchar color;
        gchar *raw_string;
        gint value;

        g_return_val_if_fail (self->priv->score == 0, FALSE);

        other = side == GIBBON_POSITION_SIDE_BLACK ?
                        GIBBON_POSITION_SIDE_WHITE : GIBBON_POSITION_SIDE_BLACK;

        snapshot = gibbon_game_get_snapshot (self);
        if (!snapshot)
                g_printerr ("No snapshot!\n");
        if (!GIBBON_IS_RESIGN (snapshot->action))
                g_printerr ("Not a resignation!\n");
        if (other != snapshot->side)
                g_printerr ("wrong side!\n");
        if (!snapshot || !GIBBON_IS_RESIGN (snapshot->action)
            || other != snapshot->side) {
                g_warning (_("Accept without resignation!"));
                return FALSE;
        }
        resign = GIBBON_RESIGN (snapshot->action);

        pos = gibbon_position_copy (gibbon_game_get_position (self));

        value = resign->value * pos->cube;

        g_free (pos->status);
        pos->status = g_strdup_printf (
                        g_dngettext (GETTEXT_PACKAGE,
                                     "%s resigns and gives up one point.",
                                     "%s resigns and gives up %d points.",
                                     pos->cube),
                      other == GIBBON_POSITION_SIDE_BLACK ?
                                      pos->players[0] : pos->players[1],
                      pos->cube);

        if (side == GIBBON_POSITION_SIDE_BLACK) {
                pos->scores[1] += value;
                self->priv->score = -value;
        } else {
                pos->scores[0] += value;
                self->priv->score = value;
        }

        gibbon_game_add_snapshot (self, GIBBON_GAME_ACTION (accept), side, pos);

        nodes = gsgf_game_tree_get_nodes (self->priv->game_tree);
        nodes = g_list_last (nodes);
        g_return_val_if_fail (nodes != NULL, FALSE);

        node = nodes->data;
        gsgf_node_remove_property (node, "OR");
        g_return_val_if_fail (node != NULL, FALSE);
        property = gsgf_node_add_property (node, "RE", &error);
        if (!property) {
                g_warning ("gibbon_game_add_move: %s!",
                            error->message);
                g_error_free (error);
                return FALSE;
        }

        /*
         * When exporting to SGF, we swap sides in order to match GNU
         * backgammon's notion of colors and directions.
         */
        color = side == GIBBON_POSITION_SIDE_BLACK ? 'W' : 'B';
        raw_string = g_strdup_printf ("%c+%d", color, value);
        raw = gsgf_raw_new (raw_string);
        if (!gsgf_property_set_value (property, GSGF_VALUE (raw), &error)) {
                g_warning ("gibbon_game_add_move: %s!",
                            error->message);
                g_error_free (error);
                return FALSE;
        }

        return TRUE;
}

static gchar
gibbon_game_point_to_sgf_char (GibbonPositionSide side, gint point)
{
        if (side == GIBBON_POSITION_SIDE_BLACK) {
                if (point <= 0)
                        return 'y';
                else if (point >= 25)
                        return 'z';
                else
                        return 'x' + 1 - point;
        } else {
                if (point <= 0)
                        return 'z';
                else if (point >= 25)
                        return 'y';
                else
                        return 'x' + 1 - point;
        }
}

gint
gibbon_game_over (const GibbonGame *self)
{
        g_return_val_if_fail (GIBBON_IS_GAME (self),
                              GIBBON_POSITION_SIDE_NONE);

        return self->priv->score;
}

const GibbonPosition *
gibbon_game_get_position (const GibbonGame *self)
{
        const GibbonGameSnapshot *snapshot;

        g_return_val_if_fail (GIBBON_IS_GAME (self), NULL);

        snapshot = gibbon_game_get_snapshot (self);
        if (snapshot)
                return snapshot->resulting_position;
        else
                return self->priv->initial_position;
}

static void
gibbon_game_add_snapshot (GibbonGame *self, GibbonGameAction *action,
                          GibbonPositionSide side, GibbonPosition *position)
{
        GibbonGameSnapshot *snapshot;

        self->priv->snapshots = g_realloc (self->priv->snapshots,
                                           ++self->priv->num_snapshots
                                           * sizeof *self->priv->snapshots);

        snapshot = self->priv->snapshots + self->priv->num_snapshots - 1;

        snapshot->action = action;
        snapshot->side = side;
        snapshot->resulting_position = position;
}

static const GibbonGameSnapshot *
gibbon_game_get_snapshot (const GibbonGame *self)
{
        if (!self->priv->snapshots)
                return NULL;

        return self->priv->snapshots + self->priv->num_snapshots - 1;
}
