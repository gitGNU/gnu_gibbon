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
 * SECTION:gibbon-sgf-writer
 * @short_description: Convert a #GibbonMatch to SGF.
 *
 * Since: 0.1.1
 *
 * A #GibbonMatchWriter for SGF.
 */

#include <glib.h>
#include <glib/gi18n.h>

#include <libgsgf/gsgf.h>

#include "gibbon-sgf-writer.h"
#include "gibbon-game.h"

G_DEFINE_TYPE (GibbonSGFWriter, gibbon_sgf_writer, GIBBON_TYPE_MATCH_WRITER)

static gboolean gibbon_sgf_writer_write_stream (const GibbonMatchWriter *writer,
                                                GOutputStream *out,
                                                const GibbonMatch *match,
                                                GError **error);
static gboolean gibbon_sgf_writer_write_game (const GibbonSGFWriter *self,
                                              GSGFGameTree *game_tree,
                                              const GibbonGame *game,
                                              const GibbonMatch *match,
                                              GError **error);

static void 
gibbon_sgf_writer_init (GibbonSGFWriter *self)
{
}

static void
gibbon_sgf_writer_finalize (GObject *object)
{
        G_OBJECT_CLASS (gibbon_sgf_writer_parent_class)->finalize(object);
}

static void
gibbon_sgf_writer_class_init (GibbonSGFWriterClass *klass)
{
        GObjectClass *object_class = G_OBJECT_CLASS (klass);
        GibbonMatchWriterClass *gibbon_match_writer_class =
                        GIBBON_MATCH_WRITER_CLASS (klass);

        gibbon_match_writer_class->write_stream =
                        gibbon_sgf_writer_write_stream;
        
        object_class->finalize = gibbon_sgf_writer_finalize;
}

/**
 * gibbon_sgf_writer_new:
 *
 * Creates a new #GibbonSGFWriter.
 *
 * Returns: The newly created #GibbonSGFWriter or %NULL in case of failure.
 */
GibbonSGFWriter *
gibbon_sgf_writer_new (void)
{
        GibbonSGFWriter *self = g_object_new (GIBBON_TYPE_SGF_WRITER, NULL);

        return self;
}

static gboolean
gibbon_sgf_writer_write_stream (const GibbonMatchWriter *_self,
                                GOutputStream *out, const GibbonMatch *match,
                                GError **error)
{
        const GibbonSGFWriter *self;
        GibbonGame *game;
        GSGFFlavor *flavor;
        GSGFCollection *collection;
        GSGFGameTree *game_tree;
        gsize bytes_written;
        gsize game_number;

        self = GIBBON_SGF_WRITER (_self);
        g_return_val_if_fail (self != NULL, FALSE);

        game = gibbon_match_get_current_game (match);
        if (!game) {
                g_set_error_literal (error, GIBBON_MATCH_ERROR,
                                     GIBBON_MATCH_ERROR_GENERIC,
                                     _("Empty matches cannot be written as"
                                       "SGF"));
                return FALSE;
        }

        flavor = gsgf_flavor_backgammon_new ();

        collection = gsgf_collection_new ();
        for (game_number = 0; ; ++game_number) {
                game = gibbon_match_get_nth_game (match, game_number);
                if (!game)
                        break;
                game_tree = gsgf_collection_add_game_tree (collection, flavor);
                if (!game_tree) {
                        g_object_unref (collection);
                        return FALSE;
                }
                if (!gibbon_sgf_writer_write_game (self, game_tree, game, match,
                                                   error)) {
                        g_object_unref (collection);
                        return FALSE;
                }
        }

        return gsgf_component_write_stream (GSGF_COMPONENT (collection),
                                            out, &bytes_written, NULL, error);
}

/*
 * Note that we swap black and white on output so that Gibbon's notion of black
 * and white matches that of GNU Backgammon.
 */
static gboolean
gibbon_sgf_writer_write_game (const GibbonSGFWriter *self,
                              GSGFGameTree *game_tree,
                              const GibbonGame *game, const GibbonMatch *match,
                              GError **error)
{
        GSGFNode *root;
        const gchar *text;
        GSGFValue *simple_text;

        if (!gsgf_game_tree_set_application (game_tree,
                                             PACKAGE, VERSION,
                                             error))
                return FALSE;

        root = gsgf_game_tree_add_node (game_tree);

        text = gibbon_match_get_white (match);
        if (!text)
                text = "black";
        simple_text = GSGF_VALUE (gsgf_simple_text_new (text));
        if (!gsgf_node_set_property (root, "PW", simple_text, error)) {
                g_object_unref (simple_text);
                return FALSE;
        }

        text = gibbon_match_get_black (match);
        if (!text)
                text = "white";
        simple_text = GSGF_VALUE (gsgf_simple_text_new (text));
        if (!gsgf_node_set_property (root, "PB", simple_text, error)) {
                g_object_unref (simple_text);
                return FALSE;
        }

        if (gibbon_match_get_crawford (match)) {
                text = gibbon_game_is_crawford (game) ?
                                "Crawford:CrawfordGame" : "Crawford";
                 simple_text = GSGF_VALUE (gsgf_simple_text_new (text));
                 if (!gsgf_node_set_property (root, "PB", simple_text, error)) {
                         g_object_unref (simple_text);
                         return FALSE;
                 }
        }

        return TRUE;
}
