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
                                              GibbonGameAction *action);
static gboolean gibbon_sgf_reader_game (GibbonSGFReader *self,
                                        GibbonMatch *match,
                                        GSGFGameTree *game_tree,
                                        GError **error);
static gboolean gibbon_sgf_reader_match_info (GibbonSGFReader *self,
                                              GibbonMatch *match,
                                              GSGFGameTree *game_tree,
                                              GError **error);
static gboolean gibbon_sgf_reader_match_info_item (GibbonSGFReader *self,
                                                   GibbonMatch *match,
                                                   const gchar *kv,
                                                   GError **error);

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
        gboolean match_info_read = FALSE;
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
                 * node of each game tree.  We read it only for the first
                 * game tree and rely on it being constant over the rest
                 * of the file.
                 */
                if (!match_info_read) {
                        if (!gibbon_sgf_reader_match_info (self, match,
                                                           game_tree, &error)) {
                                gibbon_sgf_reader_yyerror (self, error->message);
                                g_object_unref (match);
                                match = NULL;
                                break;
                        }
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
                              GibbonGameAction *action)
{
        GibbonGame *game;
        GError *error = NULL;

        game = gibbon_match_get_current_game (match);
        if (!game) {
                gibbon_sgf_reader_yyerror (self, _("No game in progress!"));
                g_object_unref (action);
                return FALSE;
        }

        if (!gibbon_game_add_action (game, side, action, &error)) {
                gibbon_sgf_reader_yyerror (self, error->message);
                g_object_unref (action);
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
                g_printerr ("%s: %s\n", location, full_msg);

        g_free (full_msg);
}

static gboolean
gibbon_sgf_reader_game (GibbonSGFReader *self, GibbonMatch *match,
                        GSGFGameTree *game_tree, GError **error)
{
        if (!gibbon_match_add_game (match, error))
                return FALSE;

        return TRUE;
}

static gboolean
gibbon_sgf_reader_match_info (GibbonSGFReader *self, GibbonMatch *match,
                              GSGFGameTree *game_tree, GError **error)
{
        const GList *nodes = gsgf_game_tree_get_nodes (game_tree);
        const GSGFNode *root;
        const GSGFProperty *mi;
        const GSGFListOf *values;
        gsize i, num_items;
        const GSGFText *value;

        if (!nodes) return TRUE;

        root = GSGF_NODE (nodes->data);

        mi = gsgf_node_get_property (root, "MI");
        if (!mi) return TRUE;

        values = GSGF_LIST_OF (gsgf_property_get_value (mi));
        num_items = gsgf_list_of_get_number_of_items (values);
        for (i = 0; i < num_items; ++i) {
                value = GSGF_TEXT (gsgf_list_of_get_nth_item (values, i));
                if (!gibbon_sgf_reader_match_info_item (
                                self, match, gsgf_text_get_value (value),
                                error))
                        return FALSE;
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
