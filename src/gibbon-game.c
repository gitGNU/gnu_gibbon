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

#include <glib.h>
#include <glib/gi18n.h>

#include "gibbon-game.h"
#include "gibbon-move.h"
#include "gibbon-position.h"

typedef struct _GibbonGameSnapshot GibbonGameSnapshot;
struct _GibbonGameSnapshot {
        GibbonGameAction *action;
        GibbonPosition *resulting_position;
};

typedef struct _GibbonGamePrivate GibbonGamePrivate;
struct _GibbonGamePrivate {
        GibbonMatch *match;
        GSGFGameTree *game_tree;

        GibbonPosition *initial_position;
        gsize num_half_moves;
        GibbonGameSnapshot *snapshots;
};

#define GIBBON_GAME_PRIVATE(obj) (G_TYPE_INSTANCE_GET_PRIVATE ((obj), \
        GIBBON_TYPE_GAME, GibbonGamePrivate))

G_DEFINE_TYPE (GibbonGame, gibbon_game, G_TYPE_OBJECT)

static gboolean gibbon_game_add_move (GibbonGame *self,
                                      GibbonPositionSide side,
                                      GibbonMove *move);
static gchar gibbon_game_point_to_sgf_char (GibbonPositionSide side,
                                            gint point);

static void 
gibbon_game_init (GibbonGame *self)
{
        self->priv = G_TYPE_INSTANCE_GET_PRIVATE (self,
                GIBBON_TYPE_GAME, GibbonGamePrivate);

        self->priv->game_tree = NULL;
        self->priv->match = NULL;
        self->priv->initial_position = NULL;
        self->priv->snapshots = NULL;
        self->priv->num_half_moves = 0;
}

static void
gibbon_game_finalize (GObject *object)
{
        GibbonGame *self = GIBBON_GAME (object);
        gsize i;
        GibbonGameSnapshot *snapshot;

        if (self->priv->initial_position)
                gibbon_position_free (self->priv->initial_position);

        for (i = 0; i < self->priv->num_half_moves; ++i) {
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
                const gchar *white, const gchar *black,
                guint match_length, guint game_number,
                guint white_score, guint black_score,
                gboolean crawford, gboolean is_crawford,
                GError **error)
{
        GibbonGame *self = g_object_new (GIBBON_TYPE_GAME, NULL);
        const GSGFFlavor *flavor;
        GSGFNode *root;
        GSGFValue *simple_text;
        GSGFValue *match_info;
        gchar *str;
        GSGFCookedValue *mi_key, *mi_value;
        GSGFCookedValue *mi_compose;

        self->priv->match = match;
        self->priv->game_tree = game_tree;

        flavor = gsgf_game_tree_get_flavor (game_tree);

        root = gsgf_game_tree_add_node (game_tree);
        if (!gsgf_game_tree_set_application (game_tree,
                                             PACKAGE, VERSION,
                                             error))
                return self;
        simple_text = GSGF_VALUE (gsgf_simple_text_new (white));
        if (!gsgf_node_set_property (root, "PB", simple_text, error)) {
                g_object_unref (simple_text);
                return self;
        }
        simple_text = GSGF_VALUE (gsgf_simple_text_new (black));
        if (!gsgf_node_set_property (root, "PW", simple_text, error)) {
                g_object_unref (simple_text);
                return self;
        }
        if (crawford) {
                simple_text = GSGF_VALUE (gsgf_simple_text_new ("Crawford"));
                if (!gsgf_node_set_property (root, "RU", simple_text, error)) {
                        g_object_unref (simple_text);
                        return self;
                }
        }

        match_info = GSGF_VALUE (gsgf_list_of_new (gsgf_compose_get_type (),
                                                   flavor));
        if (!gsgf_node_set_property (root, "MI", match_info, error)) {
                g_object_unref (match_info);
                return self;
        }

        mi_key = GSGF_COOKED_VALUE (gsgf_simple_text_new ("length"));
        str = g_strdup_printf ("%u", match_length);
        mi_value = GSGF_COOKED_VALUE (gsgf_simple_text_new (str));
        g_free (str);
        mi_compose = GSGF_COOKED_VALUE (gsgf_compose_new (mi_key, mi_value,
                                                          NULL));
        if (!gsgf_list_of_append (GSGF_LIST_OF (match_info),
                                  mi_compose, error)) {
                g_object_unref (mi_compose);
                return self;
        }

        mi_key = GSGF_COOKED_VALUE (gsgf_simple_text_new ("game"));
        str = g_strdup_printf ("%u", game_number);
        mi_value = GSGF_COOKED_VALUE (gsgf_simple_text_new (str));
        g_free (str);
        mi_compose = GSGF_COOKED_VALUE (gsgf_compose_new (mi_key, mi_value,
                                                          NULL));
        if (!gsgf_list_of_append (GSGF_LIST_OF (match_info),
                                  mi_compose, error)) {
                g_object_unref (mi_compose);
                return self;
        }

        mi_key = GSGF_COOKED_VALUE (gsgf_simple_text_new ("bs"));
        str = g_strdup_printf ("%u", black_score);
        mi_value = GSGF_COOKED_VALUE (gsgf_simple_text_new (str));
        g_free (str);
        mi_compose = GSGF_COOKED_VALUE (gsgf_compose_new (mi_key, mi_value,
                                                          NULL));
        if (!gsgf_list_of_append (GSGF_LIST_OF (match_info),
                                  mi_compose, error)) {
                g_object_unref (mi_compose);
                return self;
        }

        mi_key = GSGF_COOKED_VALUE (gsgf_simple_text_new ("ws"));
        str = g_strdup_printf ("%u", white_score);
        mi_value = GSGF_COOKED_VALUE (gsgf_simple_text_new (str));
        g_free (str);
        mi_compose = GSGF_COOKED_VALUE (gsgf_compose_new (mi_key, mi_value,
                                                          NULL));
        if (!gsgf_list_of_append (GSGF_LIST_OF (match_info),
                                  mi_compose, error)) {
                g_object_unref (mi_compose);
                return self;
        }

        self->priv->initial_position = gibbon_position_new ();

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

        if (GIBBON_IS_MOVE (action)) {
                return gibbon_game_add_move (self, side, GIBBON_MOVE (action));
        } else {
                g_critical ("gibbon_game_add_action: unsupported action type"
                            " %s!", G_OBJECT_TYPE_NAME (action));
                return FALSE;
        }

        return TRUE;
}

static gboolean
gibbon_game_add_move (GibbonGame *self, GibbonPositionSide side,
                      GibbonMove *move)
{
        gchar move_string[11];
        GSGFNode *node;
        GSGFProperty *property;
        GError *error = NULL;
        const gchar *id;
        GSGFRaw *raw;
        gsize i, j;
        GibbonMovement *movement;

        g_return_val_if_fail (move->number <= 4, FALSE);

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

        node = gsgf_game_tree_add_node (self->priv->game_tree);
        property = gsgf_node_add_property (node, id, &error);
        if (!property) {
                g_critical ("gibbon_game_add_move: %s!",
                            error->message);
                g_error_free (error);
                return FALSE;
        }

        raw = gsgf_raw_new (move_string);
        if (!gsgf_property_set_value (property, GSGF_VALUE (raw), &error)) {
                g_critical ("gibbon_game_add_move: %s!",
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
                        return 'z';
                else if (point >= 25)
                        return 'y';
                else
                        return 'x' + 1 - point;
        } else {
                if (point <= 0)
                        return 'y';
                else if (point >= 25)
                        return 'z';
                else
                        return 'x' + 1 - point;
        }
}
