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

#include <stdlib.h>

#include "gibbon-gmd-writer.h"
#include "gibbon-match.h"
#include "gibbon-game.h"
#include "gibbon-game-actions.h"
#include "gibbon-util.h"

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
                                        const gchar *timestamp,
                                        GError **error);
static gboolean gibbon_gmd_writer_move (const GibbonGMDWriter *self,
                                        GOutputStream *out,
                                        gchar color,
                                        const GibbonMove *move,
                                        const gchar *timestamp,
                                        GError **error);
static gboolean gibbon_gmd_writer_simple (const GibbonGMDWriter *self,
                                        GOutputStream *out,
                                        gchar color,
                                        const gchar *action,
                                        const gchar *timestamp,
                                        GError **error);
static gboolean gibbon_gmd_writer_resign (const GibbonGMDWriter *self,
                                          GOutputStream *out,
                                          gchar color,
                                          const GibbonResign *resign,
                                          const gchar *timestamp,
                                          GError **error);
static gchar *gibbon_gmd_writer_strescape (const gchar *str);
static gboolean gibbon_gmd_writer_write_setup (const GibbonGMDWriter *self,
                                               GOutputStream *out,
                                               const GibbonMatch *match,
                                               const GibbonGame *game,
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
        gchar *raw;

        self = GIBBON_GMD_WRITER (_self);
        g_return_val_if_fail (self != NULL, FALSE);

        buffer = g_strdup_printf ("GMD-%u # Created by Gibbon version %s\n",
                                  GIBBON_GMD_REVISION, VERSION);
        GIBBON_WRITE_ALL (buffer);

        if (gibbon_match_get_length (match) < 0) {
                buffer = g_strdup_printf ("Length: unlimited\n");
        } else {
                buffer = g_strdup_printf ("Length: %lld\n",
                                          (long long)
                                          gibbon_match_get_length (match));
        }
        GIBBON_WRITE_ALL (buffer);

        raw = gibbon_gmd_writer_strescape (gibbon_match_get_white (match));
        buffer = g_strdup_printf ("Player:W: %s\n", raw);
        g_free (raw);
        GIBBON_WRITE_ALL (buffer);

        raw = gibbon_gmd_writer_strescape (gibbon_match_get_black (match));
        buffer = g_strdup_printf ("Player:B: %s\n", raw);
        g_free (raw);
        GIBBON_WRITE_ALL (buffer);

        if (gibbon_match_get_rank (match, GIBBON_POSITION_SIDE_WHITE)) {
                raw = gibbon_gmd_writer_strescape (gibbon_match_get_rank (
                                match, GIBBON_POSITION_SIDE_WHITE));
                buffer = g_strdup_printf ("Rank:W: %s\n", raw);
                g_free (raw);
                GIBBON_WRITE_ALL (buffer);
        }

        if (gibbon_match_get_rank (match, GIBBON_POSITION_SIDE_WHITE)) {
                raw = gibbon_gmd_writer_strescape (gibbon_match_get_rank (
                                match, GIBBON_POSITION_SIDE_BLACK));
                buffer = g_strdup_printf ("Rank:B: %s\n", raw);
                g_free (raw);
                GIBBON_WRITE_ALL (buffer);
        }

        if (gibbon_match_get_location (match)) {
                raw = gibbon_gmd_writer_strescape (gibbon_match_get_location (
                                match));
                buffer = g_strdup_printf ("Location: %s\n", raw);
                g_free (raw);
                GIBBON_WRITE_ALL (buffer);
        }

        if (gibbon_match_get_crawford (match)) {
                buffer = g_strdup_printf ("Rule: Crawford\n");
                GIBBON_WRITE_ALL (buffer);
        }

        for (game_number = 0; ; ++game_number) {
                game = gibbon_match_get_nth_game (match, game_number);
                if (!game)
                        break;

                if (!game_number
                    && !gibbon_position_is_initial (
                                    gibbon_game_get_initial_position (game))) {
                        if (!gibbon_gmd_writer_write_setup (self, out, match,
                                                            game, error))
                                return FALSE;
                } else {
                        if (!gibbon_gmd_writer_add_game (self, out, error))
                                return FALSE;
                }
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
        const GibbonGameAction *action;
        gint64 timestamp;

        for (action_num = 0; ; ++action_num) {
                action = gibbon_game_get_nth_action (game, action_num, &side);
                if (!action)
                        break;

                timestamp = gibbon_game_get_nth_timestamp (game, action_num);

                if (!gibbon_gmd_writer_write_action (self, out, game, action,
                                                     side, timestamp, error))
                        return FALSE;
        }

        return TRUE;
}

static gboolean
gibbon_gmd_writer_write_setup (const GibbonGMDWriter *self, GOutputStream *out,
                               const GibbonMatch *match, const GibbonGame *game,
                               GError **error)
{
        gchar *buffer;
        const GibbonPosition *pos = gibbon_game_get_initial_position (game);

        buffer = g_strdup_printf ("Game:");
        GIBBON_WRITE_ALL (buffer);

        if (pos->scores[0] || pos->scores[1]) {
                buffer = g_strdup_printf (" Scores{%llu %llu}",
                                          (unsigned long long) pos->scores[0],
                                          (unsigned long long) pos->scores[1]);
                GIBBON_WRITE_ALL (buffer);
        }

        if (memcmp (pos->points, gibbon_position_initial ()->points,
                    sizeof pos->points)) {
                buffer = g_strdup_printf (" Points{%d"
                                " %d %d %d %d %d %d"
                                " %d %d %d %d %d %d"
                                " %d %d %d %d %d %d"
                                " %d %d %d %d %d %d"
                                " %d}",
                                pos->bar[0],
                                pos->points[0], pos->points[1],
                                pos->points[2], pos->points[3],
                                pos->points[4], pos->points[5],
                                pos->points[6], pos->points[7],
                                pos->points[8], pos->points[9],
                                pos->points[10], pos->points[11],
                                pos->points[12], pos->points[13],
                                pos->points[14], pos->points[15],
                                pos->points[16], pos->points[17],
                                pos->points[18], pos->points[19],
                                pos->points[20], pos->points[21],
                                pos->points[22], pos->points[23],
                                pos->bar[1]);
                GIBBON_WRITE_ALL (buffer);
        }

        if (pos->dice[0] || pos->dice[1]) {
                buffer = g_strdup_printf (" Dice{%u %u}",
                                          pos->dice[0],
                                          pos->dice[1]);
                GIBBON_WRITE_ALL (buffer);
        }

        /*
         * FIXME! We have to encode the cube owner.  Otherwise, automatic
         * doubles at the beginning of the game cannot be encoded.  Either
         * blow up the Cube property, or rather a new may-double property.
         */
        if (pos->cube_turned) {
                buffer = g_strdup_printf (" Cube{%llu %d}",
                                          (unsigned long long) pos->cube,
                                          pos->cube_turned);
                GIBBON_WRITE_ALL (buffer);
        } else if (pos->cube > 1) {
                buffer = g_strdup_printf (" Cube{%llu} Turn{%d}",
                                          (unsigned long long) pos->cube,
                                          pos->turn);
                GIBBON_WRITE_ALL (buffer);
        } else {
                buffer = g_strdup_printf (" Turn{%d}",
                                          pos->turn);
                GIBBON_WRITE_ALL (buffer);
        }

        buffer = g_strdup_printf (" May-Double{%u %u}\n",
                                  pos->may_double[0] ? 1 : 0,
                                  pos->may_double[1] ? 1 : 0);
        GIBBON_WRITE_ALL (buffer);

        return TRUE;
}

static gboolean
gibbon_gmd_writer_roll (const GibbonGMDWriter *self, GOutputStream *out,
                        gchar color, const GibbonRoll *roll,
                        const gchar *timestamp, GError **error)
{
        gchar *buffer;

        buffer = g_strdup_printf ("Roll:%c:%s: %d %d\n", color, timestamp,
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
                        gchar color, const GibbonMove *move,
                        const gchar *timestamp, GError **error)
{
        gchar *buffer;
        gsize i;
        GibbonMovement *movement;
        gint from, to;

        buffer = g_strdup_printf ("Move:%c:%s:", color, timestamp);

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
                          gchar color, const gchar *action,
                          const gchar *timestamp, GError **error)
{
        gchar *buffer;

        buffer = g_strdup_printf ("%s:%c:%s\n", action, color, timestamp);

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
                          const gchar *timestamp, GError **error)
{
        gchar *buffer;

        buffer = g_strdup_printf ("Resign:%c:%s: %u\n", color, timestamp,
                                  resign->value);

        if (!g_output_stream_write_all (out, buffer, strlen (buffer),
                                        NULL, NULL, error)) {
                g_free (buffer);
                return FALSE;
        }
        g_free (buffer);

        return TRUE;
}

/*
 * Ths is more or less the same as g_strescape() but it does not accept
 * exceptions, and it does not escape 8-bit characters (we only allow utf-8
 * strings).
 *
 * We can use g_strcompress() for unescaping.
 */
gchar *
gibbon_gmd_writer_strescape (const gchar *source)
{
        const guchar *p;
        gchar *dest;
        gchar *q;

        g_return_val_if_fail (source != NULL, NULL);

        p = (guchar *) source;

        /* Each source byte needs maximally four destination chars (\777) */
        q = dest = g_malloc (strlen (source) * 4 + 1);

        while (*p) {
                switch (*p) {
                case '\b':
                  *q++ = '\\';
                  *q++ = 'b';
                  break;
                case '\f':
                  *q++ = '\\';
                  *q++ = 'f';
                  break;
                case '\n':
                  *q++ = '\\';
                  *q++ = 'n';
                  break;
                case '\r':
                  *q++ = '\\';
                  *q++ = 'r';
                  break;
                case '\t':
                  *q++ = '\\';
                  *q++ = 't';
                  break;
                case '\v':
                  *q++ = '\\';
                  *q++ = 'v';
                  break;
                case '\\':
                  *q++ = '\\';
                  *q++ = '\\';
                  break;
                case '"':
                  *q++ = '\\';
                  *q++ = '"';
                  break;
                default:
                  if ((*p < ' ') || (*p == 0177)) {
                          *q++ = '\\';
                          *q++ = '0' + (((*p) >> 6) & 07);
                          *q++ = '0' + (((*p) >> 3) & 07);
                          *q++ = '0' + ((*p) & 07);
                  } else {
                          *q++ = *p;
                  }
                  break;
                }
                p++;
        }

        *q = 0;

        return dest;
}

gboolean
gibbon_gmd_writer_update_rank (const GibbonGMDWriter *self, GOutputStream *out,
                               const GibbonMatch *match,
                               GibbonPositionSide side, GError **error)
{
        gchar *raw, *buffer;

        gibbon_return_val_if_fail (self != NULL, FALSE, error);
        gibbon_return_val_if_fail (match != NULL, FALSE, error);
        gibbon_return_val_if_fail (side != GIBBON_POSITION_SIDE_NONE,
                                         FALSE, error);
        gibbon_return_val_if_fail (GIBBON_IS_GMD_WRITER (self),
                                         FALSE, error);
        gibbon_return_val_if_fail (GIBBON_IS_MATCH (match),
                                         FALSE, error);
        gibbon_return_val_if_fail (G_IS_OUTPUT_STREAM (out),
                                         FALSE, error);

        if (gibbon_match_get_rank (match, side)) {
                raw = gibbon_gmd_writer_strescape (gibbon_match_get_rank (
                                match, side));
                buffer = g_strdup_printf ("Rank:%c: %s\n",
                                          side < 0 ? 'B' : 'W', raw);
                g_free (raw);
                GIBBON_WRITE_ALL (buffer);
        }

        return TRUE;
}

gboolean
gibbon_gmd_writer_add_game (const GibbonGMDWriter *self, GOutputStream *out,
                            GError **error)
{
        gchar *buffer;

        gibbon_return_val_if_fail (self != NULL, FALSE, error);
        gibbon_return_val_if_fail (GIBBON_IS_GMD_WRITER (self),
                                         FALSE, error);
        gibbon_return_val_if_fail (G_IS_OUTPUT_STREAM (out),
                                         FALSE, error);

        buffer = g_strdup_printf ("Game:\n");
        GIBBON_WRITE_ALL (buffer);

        return TRUE;
}

gboolean
gibbon_gmd_writer_write_action (const GibbonGMDWriter *self, GOutputStream *out,
                                const GibbonGame *game,
                                const GibbonGameAction *action,
                                GibbonPositionSide side, gint64 timestamp,
                                GError **error)
{
        gchar color;
        gchar *buf;

        gibbon_return_val_if_fail (GIBBON_IS_GMD_WRITER (self),
                                         FALSE, error);
        gibbon_return_val_if_fail (G_IS_OUTPUT_STREAM (out),
                                         FALSE, error);
        gibbon_return_val_if_fail (GIBBON_IS_GAME (game),
                                         FALSE, error);
        gibbon_return_val_if_fail (GIBBON_IS_GAME_ACTION (action),
                                         FALSE, error);

        if (side < 0)
                color = 'B';
        else if (side > 0)
                color = 'W';
        else
                color = '-';

        if (timestamp == G_MININT64) {
                buf = g_strdup ("");
        } else {
                buf = g_strdup_printf ("%llu",
                                       (unsigned long long) timestamp);
        }

        if (GIBBON_IS_ROLL (action)) {
                if (!gibbon_gmd_writer_roll (self, out, color,
                                             GIBBON_ROLL (action),
                                             buf, error))
                        return FALSE;
        } else if (GIBBON_IS_MOVE (action)) {
                if (!gibbon_gmd_writer_move (self, out, color,
                                             GIBBON_MOVE (action),
                                             buf, error))
                        return FALSE;
        } else if (GIBBON_IS_DOUBLE (action)) {
                if (!gibbon_gmd_writer_simple (self, out, color,
                                              "Double", buf, error))
                        return FALSE;
        } else if (GIBBON_IS_TAKE (action)) {
                if (!gibbon_gmd_writer_simple (self, out, color,
                                              "Take", buf, error))
                        return FALSE;
        } else if (GIBBON_IS_DROP (action)) {
                if (!gibbon_gmd_writer_simple (self, out, color,
                                               "Drop", buf, error))
                        return FALSE;
        } else if (GIBBON_IS_RESIGN (action)) {
                if (!gibbon_gmd_writer_resign (self, out, color,
                                               GIBBON_RESIGN (action),
                                               buf, error))
                        return FALSE;
        } else if (GIBBON_IS_ACCEPT (action)) {
                if (!gibbon_gmd_writer_simple (self, out, color,
                                               "Accept", buf, error))
                        return FALSE;
        } else if (GIBBON_IS_REJECT (action)) {
                if (!gibbon_gmd_writer_simple (self, out, color,
                                               "Reject", buf, error))
                        return FALSE;
        } else {
                g_free (buf);
                g_set_error (error, GIBBON_ERROR,
                             GIBBON_MATCH_ERROR_GENERIC,
                             _("Action %p is not supported.\n"),
                             action);
                return FALSE;
        }
        g_free (buf);

        return TRUE;
}
