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
 * SECTION:gibbon-gmd-writer
 * @short_description: Writ GMD internal format.
 *
 * Since: 0.1.1
 *
 * A #GibbonMatchWriter for writing match files in the GMD internal
 * format.!
 */

#include <glib.h>
#include <glib/gi18n.h>

#include "gibbon-gmd-writer.h"
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

#define GIBBON_GMD_WRITER_PRIVATE(obj) (G_TYPE_INSTANCE_GET_PRIVATE ((obj), \
        GIBBON_TYPE_GMD_WRITER, GibbonGMDWriterPrivate))

G_DEFINE_TYPE (GibbonGMDWriter, gibbon_gmd_writer, \
               GIBBON_TYPE_MATCH_WRITER)

#define GIBBON_WRITE_ALL(buffer)                                        \
        if (!g_output_stream_write_all (out, buffer, strlen (buffer),   \
               NULL, NULL, error)) {                                    \
               g_free (buffer);                                         \
               return FALSE;                                            \
        }                                                               \
        g_free (buffer);

static gboolean gibbon_gmd_writer_write_stream (const GibbonMatchWriter
                                                *writer,
                                                GOutputStream *out,
                                                const GibbonMatch *match,
                                                GError **error);
static gboolean gibbon_gmd_writer_write_game (const GibbonGMDWriter *self,
                                              GOutputStream *out,
                                              const GibbonGame *game,
                                              GError **error);
static gboolean gibbon_gmd_writer_roll (const GibbonGMDWriter *self,
                                        GOutputStream *out,
                                        gchar color,
                                        const GibbonRoll *roll,
                                        GError **error);
static gboolean gibbon_gmd_writer_move (const GibbonGMDWriter *self,
                                        GOutputStream *out,
                                        gchar color,
                                        const GibbonMove *move,
                                        GError **error);
static gboolean gibbon_gmd_writer_simple (const GibbonGMDWriter *self,
                                        GOutputStream *out,
                                        gchar color,
                                        const gchar *action,
                                        GError **error);
static gboolean gibbon_gmd_writer_resign (const GibbonGMDWriter *self,
                                          GOutputStream *out,
                                          gchar color,
                                          const GibbonResign *resign,
                                          GError **error);

static void 
gibbon_gmd_writer_init (GibbonGMDWriter *self)
{
}

static void
gibbon_gmd_writer_finalize (GObject *object)
{
}

static void
gibbon_gmd_writer_class_init (GibbonGMDWriterClass *klass)
{
        GObjectClass *object_class = G_OBJECT_CLASS (klass);
        GibbonMatchWriterClass *gibbon_match_writer_class = GIBBON_MATCH_WRITER_CLASS (klass);

        gibbon_match_writer_class->write_stream =
                        gibbon_gmd_writer_write_stream;
        
        object_class->finalize = gibbon_gmd_writer_finalize;
}

/**
 * gibbon_gmd_writer_new:
 *
 * Creates a new #GibbonGMDWriter.
 *
 * Returns: The newly created #GibbonGMDWriter or %NULL in case of failure.
 */
GibbonGMDWriter *
gibbon_gmd_writer_new (void)
{
        GibbonGMDWriter *self = g_object_new (GIBBON_TYPE_GMD_WRITER,
                                                   NULL);

        return self;
}

static gboolean
gibbon_gmd_writer_write_stream (const GibbonMatchWriter *_self,
                                      GOutputStream *out,
                                      const GibbonMatch *match,
                                      GError **error)
{
        const GibbonGMDWriter *self;
        gsize game_number;
        GibbonGame *game;
        gchar *buffer;
        gsize match_length;
        gsize num_games;

        self = GIBBON_GMD_WRITER (_self);
        g_return_val_if_fail (self != NULL, FALSE);

        match_length = gibbon_match_get_length (match);

        num_games = gibbon_match_get_number_of_games (match);

        buffer = g_strdup_printf ("GMD-%u # Created by Gibbon version %s\n",
                                  GIBBON_GMD_REVISION, VERSION);
        GIBBON_WRITE_ALL (buffer);

        if (gibbon_match_get_length (match) < 0) {
                buffer = g_strdup_printf ("Length: unlimited\n");
        } else {
                buffer = g_strdup_printf ("Length: %llu\n",
                                          (unsigned long long)
                                          gibbon_match_get_length (match));
        }
        GIBBON_WRITE_ALL (buffer);

        buffer = g_strdup_printf ("White: %s\nBlack: %s\n",
                                  gibbon_match_get_white (match),
                                  gibbon_match_get_black (match));
        GIBBON_WRITE_ALL (buffer);

        if (gibbon_match_get_crawford (match)) {
                buffer = g_strdup_printf ("Rule: Crawford\n");
        }
        GIBBON_WRITE_ALL (buffer);

        for (game_number = 0; ; ++game_number) {
                game = gibbon_match_get_nth_game (match, game_number);
                if (!game)
                        break;
                if (!gibbon_gmd_writer_write_game (self, out, game, error))
                        return FALSE;
        }

        return TRUE;
}

static gboolean
gibbon_gmd_writer_write_game (const GibbonGMDWriter *self, GOutputStream *out,
                              const GibbonGame *game, GError **error)
{
        glong action_num;
        GibbonPositionSide side;
        const GibbonGameAction *action = NULL;
        gchar color;

        for (action_num = 0; ; ++action_num) {
                action = gibbon_game_get_nth_action (game, action_num, &side);
                if (!action)
                        break;
                if (side < 0)
                        color = 'B';
                else if (color > 0)
                        color = 'W';
                else
                        color = '0';

                if (GIBBON_IS_ROLL (action)) {
                        if (!gibbon_gmd_writer_roll (self, out, color,
                                                     GIBBON_ROLL (action),
                                                     error))
                                return FALSE;
                } else if (GIBBON_IS_MOVE (action)) {
                        if (!gibbon_gmd_writer_move (self, out, color,
                                                     GIBBON_MOVE (action),
                                                     error))
                                return FALSE;
                } else if (GIBBON_IS_DOUBLE (action)) {
                        if (!gibbon_gmd_writer_simple (self, out, color,
                                                      "Double", error))
                                return FALSE;
                } else if (GIBBON_IS_TAKE (action)) {
                        if (!gibbon_gmd_writer_simple (self, out, color,
                                                      "Take", error))
                                return FALSE;
                } else if (GIBBON_IS_DROP (action)) {
                        if (!gibbon_gmd_writer_simple (self, out, color,
                                                       "Drop", error))
                                return FALSE;
                } else if (GIBBON_IS_RESIGN (action)) {
                        if (!gibbon_gmd_writer_resign (self, out, color,
                                                       GIBBON_RESIGN (action),
                                                       error))
                                return FALSE;
                } else if (GIBBON_IS_ACCEPT (action)) {
                        if (!gibbon_gmd_writer_simple (self, out, color,
                                                       "Accept", error))
                                return FALSE;
                } else if (GIBBON_IS_REJECT (action)) {
                        if (!gibbon_gmd_writer_simple (self, out, color,
                                                       "Reject", error))
                                return FALSE;
                } else {
                        g_printerr ("Action %p is not supported.\n", action);
                }
        }

        return TRUE;
}

static gboolean
gibbon_gmd_writer_roll (const GibbonGMDWriter *self, GOutputStream *out,
                        gchar color, const GibbonRoll *roll, GError **error)
{
        gchar *buffer;

        buffer = g_strdup_printf ("Roll:%c: %d %d\n", color,
                                  roll->die1, roll->die2);

        if (!g_output_stream_write_all (out, buffer, strlen (buffer),
                                        NULL, NULL, error)) {
                g_free (buffer);
                return FALSE;
        }
        g_free (buffer);

        return TRUE;
}

static gboolean
gibbon_gmd_writer_move (const GibbonGMDWriter *self, GOutputStream *out,
                        gchar color, const GibbonMove *move, GError **error)
{
        gchar *buffer;
        gsize i;
        GibbonMovement *movement;
        gint from, to;

        buffer = g_strdup_printf ("Move:%c:", color);

        if (!g_output_stream_write_all (out, buffer, strlen (buffer),
                                        NULL, NULL, error)) {
                g_free (buffer);
                return FALSE;
        }
        g_free (buffer);

        for (i = 0; i < move->number; ++i) {
                movement = move->movements + i;
                from = movement->from;
                to = movement->to;
                if (from == 25 || from == 0)
                        buffer = g_strdup_printf (" bar/%u", to);
                else if (to == 25 || to == 0)
                        buffer = g_strdup_printf (" %u/off", from);
                else
                        buffer = g_strdup_printf (" %u/%u", from, to);
                if (!g_output_stream_write_all (out, buffer, strlen (buffer),
                                                NULL, NULL, error)) {
                        g_free (buffer);
                        return FALSE;
                }
                g_free (buffer);
        }

        if (!g_output_stream_write_all (out, "\n", strlen ("\n"),
                                        NULL, NULL, error))
                return FALSE;

        return TRUE;
}

static gboolean
gibbon_gmd_writer_simple (const GibbonGMDWriter *self, GOutputStream *out,
                          gchar color, const gchar *action, GError **error)
{
        gchar *buffer;

        buffer = g_strdup_printf ("%s:%c\n", action, color);

        if (!g_output_stream_write_all (out, buffer, strlen (buffer),
                                        NULL, NULL, error)) {
                g_free (buffer);
                return FALSE;
        }
        g_free (buffer);

        return TRUE;
}

static gboolean
gibbon_gmd_writer_resign (const GibbonGMDWriter *self, GOutputStream *out,
                          gchar color, const GibbonResign *resign,
                          GError **error)
{
        gchar *buffer;

        buffer = g_strdup_printf ("Double:%c:%u\n", color, resign->value);

        if (!g_output_stream_write_all (out, buffer, strlen (buffer),
                                        NULL, NULL, error)) {
                g_free (buffer);
                return FALSE;
        }
        g_free (buffer);

        return TRUE;
}
