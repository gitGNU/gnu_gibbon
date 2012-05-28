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
 * SECTION:gibbon-java-fibs-writer
 * @short_description: Writ JavaFIBS internal format.
 *
 * Since: 0.1.1
 *
 * A #GibbonMatchWriter for writing match files in the JavaFIBS internal
 * format.!
 */

#include <glib.h>
#include <glib/gi18n.h>

#include "gibbon-java-fibs-writer.h"
#include "gibbon-match.h"
#include "gibbon-game.h"

#include "gibbon-roll.h"
#include "gibbon-move.h"
#include "gibbon-double.h"
#include "gibbon-drop.h"
#include "gibbon-take.h"
#include "gibbon-resign.h"
#include "gibbon-accept.h"
#include "gibbon-reject.h"

#define GIBBON_JAVA_FIBS_WRITER_PRIVATE(obj) (G_TYPE_INSTANCE_GET_PRIVATE ((obj), \
        GIBBON_TYPE_JAVA_FIBS_WRITER, GibbonJavaFIBSWriterPrivate))

#define GIBBON_JAVA_FIBS_PROLOG "JavaFIBS internal match representation v1.1"

G_DEFINE_TYPE (GibbonJavaFIBSWriter, gibbon_java_fibs_writer, \
               GIBBON_TYPE_MATCH_WRITER)

static gboolean gibbon_java_fibs_writer_write_stream (const GibbonMatchWriter
                                                      *writer,
                                                      GOutputStream *out,
                                                      const GibbonMatch *match,
                                                      GError **error);
static gboolean gibbon_java_fibs_writer_write_game (const GibbonJavaFIBSWriter
                                                    *self,
                                                    GOutputStream *out,
                                                    const GibbonGame *game,
                                                    guint game_number,
                                                    const GibbonMatch *match,
                                                    GError **error);
static gboolean gibbon_java_fibs_writer_roll (const GibbonJavaFIBSWriter *self,
                                              GOutputStream *out,
                                              GibbonPositionSide side,
                                              GibbonRoll *roll,
                                              const GibbonMatch *match,
                                              gboolean is_opening,
                                              GError **error);

static void 
gibbon_java_fibs_writer_init (GibbonJavaFIBSWriter *self)
{
}

static void
gibbon_java_fibs_writer_finalize (GObject *object)
{
}

static void
gibbon_java_fibs_writer_class_init (GibbonJavaFIBSWriterClass *klass)
{
        GObjectClass *object_class = G_OBJECT_CLASS (klass);
        GibbonMatchWriterClass *gibbon_match_writer_class = GIBBON_MATCH_WRITER_CLASS (klass);

        gibbon_match_writer_class->write_stream =
                        gibbon_java_fibs_writer_write_stream;
        
        object_class->finalize = gibbon_java_fibs_writer_finalize;
}

/**
 * gibbon_java_fibs_writer_new:
 *
 * Creates a new #GibbonJavaFIBSWriter.
 *
 * Returns: The newly created #GibbonJavaFIBSWriter or %NULL in case of failure.
 */
GibbonJavaFIBSWriter *
gibbon_java_fibs_writer_new (void)
{
        GibbonJavaFIBSWriter *self = g_object_new (GIBBON_TYPE_JAVA_FIBS_WRITER,
                                                   NULL);

        return self;
}


static gboolean
gibbon_java_fibs_writer_write_stream (const GibbonMatchWriter *_self,
                                      GOutputStream *out,
                                      const GibbonMatch *match,
                                      GError **error)
{
        const GibbonJavaFIBSWriter *self;
        gsize game_number;
        GibbonGame *game;
        gchar *prolog;

        self = GIBBON_JAVA_FIBS_WRITER (_self);
        g_return_val_if_fail (self != NULL, FALSE);

        prolog = g_strdup_printf ("%s\n8:%s:%llu\n",
                                  GIBBON_JAVA_FIBS_PROLOG,
                                  gibbon_match_get_black (match),
                                  (unsigned long long)
                                  gibbon_match_get_length (match));
        if (!g_output_stream_write_all (out, prolog, strlen (prolog),
                                        NULL, NULL, error)) {
                g_free (prolog);
                return FALSE;
        }
        g_free (prolog);

        for (game_number = 0; ; ++game_number) {
                game = gibbon_match_get_nth_game (match, game_number);
                if (!game)
                        break;
                if (!gibbon_java_fibs_writer_write_game (self, out, game,
                                                         game_number, match,
                                                         error))
                        return FALSE;
        }

        return TRUE;
}

static gboolean
gibbon_java_fibs_writer_write_game (const GibbonJavaFIBSWriter *self,
                                    GOutputStream *out,
                                    const GibbonGame *game,
                                    guint game_number,
                                    const GibbonMatch *match,
                                    GError **error)
{
        gchar *prolog;
        glong action_num;
        GibbonPositionSide side;
        const GibbonGameAction *action = NULL;
        gboolean opening = TRUE;

        prolog = g_strdup_printf ("13:%s:\n6:You:%s\n",
                                  gibbon_match_get_black (match),
                                  gibbon_match_get_black (match));
        if (!g_output_stream_write_all (out, prolog, strlen (prolog),
                                        NULL, NULL, error)) {
                g_free (prolog);
                return FALSE;
        }
        g_free (prolog);

        for (action_num = 0; ; ++action_num) {
                action = gibbon_game_get_nth_action (game, action_num, &side);
                if (!action)
                        break;
                if (GIBBON_IS_ROLL (action)) {
                        if (!side)
                                continue;
                        if (!gibbon_java_fibs_writer_roll (self, out, side,
                                                           GIBBON_ROLL (action),
                                                           match,
                                                           opening, error))
                                return FALSE;
                        opening = FALSE;
                }
        }

        return TRUE;
}

static gboolean
gibbon_java_fibs_writer_roll (const GibbonJavaFIBSWriter *self,
                              GOutputStream *out,
                              GibbonPositionSide side, GibbonRoll *roll,
                              const GibbonMatch *match, gboolean is_opening,
                              GError **error)
{
        gchar *buffer;
        guint opcode = is_opening ? 11 : 0;

        buffer = g_strdup_printf ("%u:%s:%u %u\n",
                                  opcode,
                                  side == GIBBON_POSITION_SIDE_WHITE ?
                                  "You" : gibbon_match_get_black (match),
                                  roll->die1, roll->die2);

        if (!g_output_stream_write_all (out, buffer, strlen (buffer),
                                        NULL, NULL, error)) {
                g_free (buffer);
                return FALSE;
        }
        g_free (buffer);

        return TRUE;
}
