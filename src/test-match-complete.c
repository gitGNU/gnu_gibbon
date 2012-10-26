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

#include <errno.h>
#include <glib.h>
#include <gio/gio.h>

#include <string.h>

#include "gibbon-gmd-reader.h"
#include "gibbon-gmd-writer.h"
#include "gibbon-match.h"
#include "gibbon-match-play.h"

int
main(int argc, char *argv[])
{
        GibbonMatchReader *reader;
        GibbonMatch *from, *to;
        const GibbonPosition *current_pos;
        const GibbonPosition *target_pos;
        guint64 replay = 0;
        GSList *iter;
        GibbonMatchPlay *play;
        GibbonGameAction *action;
        GibbonPositionSide side;
        GError *error = NULL;
        GOutputStream *out;
        GibbonMatchWriter *writer;

        g_type_init ();

        if (argc < 3 || argc > 4) {
                g_printerr ("Usage: %s FROM TO [SKIP]\n", argv[0]);
                return -1;
        }

        reader = GIBBON_MATCH_READER (gibbon_gmd_reader_new (NULL, NULL));
        g_return_val_if_fail (reader != NULL, -1);

        from = gibbon_match_reader_parse (reader, argv[1]);
        if (!from) {
                g_object_unref (reader);
                return -1;
        }

        g_object_unref (reader);
        reader = GIBBON_MATCH_READER (gibbon_gmd_reader_new (NULL, NULL));
        g_return_val_if_fail (reader != NULL, -1);

        to = gibbon_match_reader_parse (reader, argv[2]);
        if (!to) {
                g_object_unref (from);
                g_object_unref (reader);
                return -1;
        }

        g_object_unref (reader);

        if (argc == 4)
                replay = g_ascii_strtoull (argv[3], NULL, 10);

        target_pos = gibbon_match_get_current_position (to);

        if (!gibbon_match_get_missing_actions (from, target_pos, &iter)) {
                g_printerr ("Cannot deduce actions leading from `%s' to"
                            " `%s'!\n", argv[1], argv[2]);
                g_object_unref (from);
                g_object_unref (to);
                return -1;
        }

        while (iter) {
                play = (GibbonMatchPlay *) iter->data;
                action = play->action;
                side = play->side;
                if (!gibbon_match_add_action (from, side, action, G_MININT64,
                                              &error)) {
                        g_printerr ("Completing `%s' to `%s':\n",
                                    argv[1], argv[2]);
                        g_printerr ("Error applying match action %s for %s: %s\n",
                                    G_OBJECT_TYPE_NAME (action),
                                    side < 0 ? "black" : side > 0 ? "white"
                                                    : "none",
                                    error->message);
                        g_printerr ("Completed match so far:\n");
                        out = G_OUTPUT_STREAM (g_memory_output_stream_new (NULL, 0,
                                                                           g_realloc,
                                                                           g_free));
                        writer = GIBBON_MATCH_WRITER (gibbon_gmd_writer_new ());
                        gibbon_match_writer_write_stream (writer, out, from, NULL);
                        g_printerr ("%s",
                                    (gchar *) g_memory_output_stream_get_data  (
                                                    G_MEMORY_OUTPUT_STREAM (out)));
                        g_object_unref (out);
                        g_object_unref (from);
                        g_object_unref (to);
                        g_slist_free_full (iter,
                                (GDestroyNotify) gibbon_match_play_free);
                        return -1;
                }

                iter = iter->next;
        }
        g_slist_free_full (iter, (GDestroyNotify) gibbon_match_play_free);

        current_pos = gibbon_match_get_current_position (from);
        target_pos = gibbon_match_get_current_position (to);
        if (!gibbon_position_equals_technically (current_pos, target_pos)) {
                g_printerr ("Match positions differ after replaying"
                            " missed actions.  Wanted contents of `%s', got:\n",
                            argv[2]);
                out = G_OUTPUT_STREAM (g_memory_output_stream_new (NULL, 0,
                                                                   g_realloc,
                                                                   g_free));
                writer = GIBBON_MATCH_WRITER (gibbon_gmd_writer_new ());
                gibbon_match_writer_write_stream (writer, out, from, NULL);
                g_printerr ("%s",
                            (gchar *) g_memory_output_stream_get_data  (
                                            G_MEMORY_OUTPUT_STREAM (out)));
                g_object_unref (out);
                g_object_unref (from);
                g_object_unref (to);
                return -1;
        }

        g_object_unref (from);
        g_object_unref (to);

        return 0;
}
