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
 * SECTION:gibbon-jelly-fish-writer
 * @short_description: Convert GibbonMatch to JellyFish format
 *
 * Since: 0.1.1
 *
 * A #GibbonMatchWriter for the JellyFish format.
 */

#include <glib.h>
#include <glib/gi18n.h>

#include "gibbon-jelly-fish-writer.h"
#include "gibbon-game.h"

G_DEFINE_TYPE (GibbonJellyFishWriter, gibbon_jelly_fish_writer,
               GIBBON_TYPE_MATCH_WRITER)

static gboolean gibbon_jelly_fish_writer_write_stream (const GibbonMatchWriter
                                                       *writer,
                                                       GOutputStream *out,
                                                       const GibbonMatch *match,
                                                       GError **error);
static gboolean gibbon_jelly_fish_writer_write_game (const GibbonJellyFishWriter
                                                     *self,
                                                     GOutputStream *out,
                                                     const GibbonGame *game,
                                                     GError **error);

static void 
gibbon_jelly_fish_writer_init (GibbonJellyFishWriter *self)
{
}

static void
gibbon_jelly_fish_writer_finalize (GObject *object)
{
        G_OBJECT_CLASS (gibbon_jelly_fish_writer_parent_class)->finalize(object);
}

static void
gibbon_jelly_fish_writer_class_init (GibbonJellyFishWriterClass *klass)
{
        GObjectClass *object_class = G_OBJECT_CLASS (klass);
        GibbonMatchWriterClass *gibbon_match_writer_class =
                        GIBBON_MATCH_WRITER_CLASS (klass);

        gibbon_match_writer_class->write_stream =
                        gibbon_jelly_fish_writer_write_stream;

        object_class->finalize = gibbon_jelly_fish_writer_finalize;
}

/**
 * gibbon_jelly_fish_writer_new:
 *
 * Creates a new #GibbonJellyFishWriter.
 *
 * Returns: The newly created #GibbonJellyFishWriter or %NULL in case of failure.
 */
GibbonJellyFishWriter *
gibbon_jelly_fish_writer_new (void)
{
        GibbonJellyFishWriter *self = g_object_new (GIBBON_TYPE_JELLY_FISH_WRITER, NULL);
        return self;
}

static gboolean
gibbon_jelly_fish_writer_write_stream (const GibbonMatchWriter *_self,
                                       GOutputStream *out,
                                       const GibbonMatch *match,
                                       GError **error)
{
        gsize game_number;
        const GibbonGame *game;

        gchar *buffer = g_strdup_printf (" %llu point match\n",
                                         (unsigned long long)
                                         gibbon_match_get_length (match));

        if (!g_output_stream_write_all (out,
                                        buffer, strlen (buffer),
                                        NULL, NULL, error)) {
                g_free (buffer);
                return FALSE;
        }
        g_free (buffer);

        for (game_number = 0; ; ++game_number) {
                game = gibbon_match_get_nth_game (match, game_number);
                if (!game)
                        break;
                buffer = g_strdup_printf ("\n Game %llu\n",
                                          (unsigned long long) game_number);

                if (!g_output_stream_write_all (out,
                                                buffer, strlen (buffer),
                                                NULL, NULL, error)) {
                        g_free (buffer);
                        return FALSE;
                }
                g_free (buffer);
                if (!gibbon_jelly_fish_writer_write_game (
                                GIBBON_JELLY_FISH_WRITER (_self), out, game,
                                error))
                        return FALSE;
        }

        return TRUE;
}

static gboolean
gibbon_jelly_fish_writer_write_game (const GibbonJellyFishWriter *_self,
                                     GOutputStream *out,
                                     const GibbonGame *game,
                                     GError **error)
{
        const GibbonPosition *position =
                        gibbon_game_get_initial_position (game);
        gchar *buffer;
        gchar padding[32];
        glong len, i;
        glong move_num = 0;

        buffer = g_strdup_printf (" %s : %u",
                                  position->players[1],
                                  position->scores[1]);

        if (!g_output_stream_write_all (out,
                                        buffer, strlen (buffer),
                                        NULL, NULL, error)) {
                g_free (buffer);
                return FALSE;
        }
        len = g_utf8_strlen (buffer, -1);
        g_free (buffer);

        padding[0] = 0;
        if (len < 31) {
                for (i = 0; i + len < 31; ++i) {
                        padding[i] = ' ';
                }
                padding[i] = 0;
        }

        buffer = g_strdup_printf ("%s %s : %u\n",
                                  padding,
                                  position->players[0],
                                  position->scores[0]);

        if (!g_output_stream_write_all (out,
                                        buffer, strlen (buffer),
                                        NULL, NULL, error)) {
                g_free (buffer);
                return FALSE;
        }
        g_free (buffer);



        return TRUE;
}
