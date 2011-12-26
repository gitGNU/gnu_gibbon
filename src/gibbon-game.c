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

typedef struct _GibbonGamePrivate GibbonGamePrivate;
struct _GibbonGamePrivate {
        GibbonMatch *match;
        GSGFGameTree *game_tree;
};

#define GIBBON_GAME_PRIVATE(obj) (G_TYPE_INSTANCE_GET_PRIVATE ((obj), \
        GIBBON_TYPE_GAME, GibbonGamePrivate))

G_DEFINE_TYPE (GibbonGame, gibbon_game, G_TYPE_OBJECT)

static void 
gibbon_game_init (GibbonGame *self)
{
        self->priv = G_TYPE_INSTANCE_GET_PRIVATE (self,
                GIBBON_TYPE_GAME, GibbonGamePrivate);

        self->priv->game_tree = NULL;
}

static void
gibbon_game_finalize (GObject *object)
{
        GibbonGame *self = GIBBON_GAME (object);

        self->priv->game_tree = NULL;

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
        if (!gsgf_node_set_property (root, "PW", simple_text, error)) {
                g_object_unref (simple_text);
                return self;
        }
        simple_text = GSGF_VALUE (gsgf_simple_text_new (black));
        if (!gsgf_node_set_property (root, "PB", simple_text, error)) {
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

        return self;
}

GSGFGameTree *
gibbon_game_get_game_tree (const GibbonGame *self)
{
        g_return_val_if_fail (GIBBON_IS_GAME (self), NULL);

        return self->priv->game_tree;
}
