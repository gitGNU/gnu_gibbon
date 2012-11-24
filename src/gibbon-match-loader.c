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
 * SECTION:gibbon-match-loader
 * @short_description: Load a backgammon match.
 *
 * Since: 0.2.0
 *
 * Load a backgammon match for display in Gibbon.
 */

#include <glib.h>
#include <glib/gi18n.h>
#include <gio/gio.h>

#include "gibbon-match-loader.h"

#include "gibbon-gmd-reader.h"
#include "gibbon-sgf-reader.h"
#include "gibbon-java-fibs-reader.h"
#include "gibbon-jelly-fish-reader.h"
#include "gibbon-util.h"

G_DEFINE_TYPE (GibbonMatchLoader, gibbon_match_loader, G_TYPE_OBJECT)

static GibbonMatchReader *gibbon_match_loader_get_reader (
                const GibbonMatchLoader *self,
                const gchar *filename, GError **error);
static void gibbon_match_loader_yyerror (GError **error, const gchar *msg);

static void 
gibbon_match_loader_init (GibbonMatchLoader *self)
{
}

static void
gibbon_match_loader_finalize (GObject *object)
{
        G_OBJECT_CLASS (gibbon_match_loader_parent_class)->finalize(object);
}

static void
gibbon_match_loader_class_init (GibbonMatchLoaderClass *klass)
{
        GObjectClass *object_class = G_OBJECT_CLASS (klass);
        
        object_class->finalize = gibbon_match_loader_finalize;
}

/**
 * gibbon_match_loader_new:
 *
 * Creates a new #GibbonMatchLoader.
 *
 * Returns: The newly created #GibbonMatchLoader or %NULL in case of failure.
 */
GibbonMatchLoader *
gibbon_match_loader_new (void)
{
        GibbonMatchLoader *self = g_object_new (GIBBON_TYPE_MATCH_LOADER, NULL);

        return self;
}

GibbonMatch *
gibbon_match_loader_read_match (GibbonMatchLoader *self,
                                const gchar *filename,
                                GError **error)
{
        GibbonMatchReader *reader;
        GibbonMatch *match;

        g_return_val_if_fail (GIBBON_IS_MATCH_LOADER (self), NULL);

        reader = gibbon_match_loader_get_reader (self, filename, error);
        if (!reader)
                return NULL;

        match = gibbon_match_reader_parse (reader, filename);
        if (!match)
                return NULL;

        return match;
}

static GibbonMatchReader *
gibbon_match_loader_get_reader (const GibbonMatchLoader *self,
                                const gchar *filename, GError **error)
{
        GFile *file;
        GInputStream *stream;
        gssize read_bytes;
        gchar first;
        GibbonMatchReader *reader;

        /*
         * FIXME! We should figure out how to interface a GInputStream with
         * a flex generated scanner.  Then we could open the input file
         * once, guess the format, rewind it to the start, and parse.
         *
         * Until then we have to live with this ugly solution.
         */

        file = g_file_new_for_path (filename);
        stream = G_INPUT_STREAM (g_file_read (file, NULL, error));
        g_object_unref (file);

        if (!stream) {
                g_object_unref (file);
                return NULL;
        }

        while (1) {
                read_bytes = g_input_stream_read (stream, &first, 1, NULL, error);
                if (read_bytes < 0) {
                        g_object_unref (stream);
                        return NULL;
                }

                if (!read_bytes) {
                        g_set_error_literal (error, GIBBON_ERROR, -1,
                                             _("Premature end of input file!"));
                        g_object_unref (stream);
                        return NULL;
                }

                if (first >= 0x09 && first <= 0x0d)
                        continue;
                if (first != ' ')
                        break;
        }

        switch (first) {
        case 'G':
                reader = GIBBON_MATCH_READER (gibbon_gmd_reader_new (
                                (GibbonMatchReaderErrorFunc)
                                gibbon_match_loader_yyerror,
                                error));
                break;
        case '(':
                reader = GIBBON_MATCH_READER (gibbon_sgf_reader_new (
                                (GibbonMatchReaderErrorFunc)
                                gibbon_match_loader_yyerror,
                                error));
                break;
        case 'J':
                reader = GIBBON_MATCH_READER (gibbon_java_fibs_reader_new (
                                (GibbonMatchReaderErrorFunc)
                                gibbon_match_loader_yyerror,
                                error));
                break;
        default:
                reader = GIBBON_MATCH_READER (gibbon_jelly_fish_reader_new (
                                (GibbonMatchReaderErrorFunc)
                                gibbon_match_loader_yyerror,
                                error));
                break;
        }

        g_object_unref (stream);

        return reader;
}

static void
gibbon_match_loader_yyerror (GError **error, const gchar *msg)
{
        /* We can only report the first error.  */
        if (*error)
                return;

        g_set_error_literal (error, GIBBON_ERROR, -1, msg);
}
