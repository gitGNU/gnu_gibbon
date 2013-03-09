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
 * @short_description: Convert a #GibbonMatch to JellyFish format
 *
 * Since: 0.1.1
 *
 * A #GibbonMatchWriter for the JellyFish format.
 */

#include <glib.h>
#include <glib/gi18n.h>
#include <stdlib.h>

#include "gibbon-jelly-fish-writer.h"
#include "gibbon-game.h"

#include "gibbon-roll.h"
#include "gibbon-move.h"
#include "gibbon-double.h"
#include "gibbon-drop.h"
#include "gibbon-take.h"

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
static gchar *gibbon_jelly_fish_writer_roll (const GibbonJellyFishWriter *self,
                                             const GibbonRoll *roll);
static gchar *gibbon_jelly_fish_writer_move (const GibbonJellyFishWriter *self,
                                             const GibbonPosition *position,
                                             GibbonPositionSide side,
                                             const GibbonMove *move);
static gchar *gibbon_jelly_fish_writer_double (const GibbonJellyFishWriter *self,
                                               const GibbonPosition *position);
static gchar *gibbon_jelly_fish_writer_take (const GibbonJellyFishWriter *self);
static gchar *gibbon_jelly_fish_writer_drop (const GibbonJellyFishWriter *self);

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

/*
 * JellyFish is an MS-DOS file format.  We therefore use MS-DOS line ending
 * conventions which!
 */
static gboolean
gibbon_jelly_fish_writer_write_stream (const GibbonMatchWriter *_self,
                                       GOutputStream *out,
                                       const GibbonMatch *match,
                                       GError **error)
{
        gsize game_number;
        const GibbonGame *game;
        gchar *buffer = g_strdup_printf (" %llu point match\015\012",
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
                buffer = g_strdup_printf ("\015\012 Game %llu\015\012",
                                          (unsigned long long) game_number + 1);

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
gibbon_jelly_fish_writer_write_game (const GibbonJellyFishWriter *self,
                                     GOutputStream *out,
                                     const GibbonGame *game,
                                     GError **error)
{
        const GibbonPosition *position =
                        gibbon_game_get_initial_position (game);
        gchar *buffer;
        gchar padding[33];
        glong len, i;
        glong move_num = 0;
        glong action_num;
        const GibbonGameAction *action = NULL;
        const GibbonGameAction *last_action = NULL;
        GibbonPositionSide side;
        gsize column = 0;
        gchar last_char = 0;
        gint score;

        buffer = g_strdup_printf (" %s : %llu",
                                  position->players[1],
                                  (unsigned long long) position->scores[1]);

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

        buffer = g_strdup_printf ("%s %s : %llu",
                                  padding,
                                  position->players[0],
                                  (unsigned long long) position->scores[0]);

        if (!g_output_stream_write_all (out, buffer, strlen (buffer),
                                        NULL, NULL, error)) {
                g_free (buffer);
                return FALSE;
        }
        g_free (buffer);

        last_char = 'x';

#define write_buffer()                                                        \
        if (buffer) {                                                         \
                if (!g_output_stream_write_all (out, buffer, strlen (buffer), \
                                                NULL, NULL, error)) {         \
                        g_free (buffer);                                      \
                        return FALSE;                                         \
                }                                                             \
                column += g_utf8_strlen (buffer, -1);                         \
                last_char = buffer[strlen(buffer) - 1];                       \
                g_free (buffer);                                              \
        }

#define pad_white_action(tab)                                                 \
        padding[0] = 0;                                                       \
        if (column < tab) {                                                   \
                for (i = 0; i + column < tab; ++i) {                          \
                        padding[i] = ' ';                                     \
                }                                                             \
                if (!g_output_stream_write_all (out, padding, i,              \
                                                NULL, NULL, error)) {         \
                        return FALSE;                                         \
                }                                                             \
                last_char = ' ';                                              \
        }

#define new_half_move()                                                       \
        if (!move_num || side == GIBBON_POSITION_SIDE_BLACK) {                \
                column = 0;                                                   \
                buffer = g_strdup_printf ("\015\012%3ld)", ++move_num);       \
                write_buffer ();                                              \
                column -= 2;                                                  \
        }                                                                     \
        if (side == GIBBON_POSITION_SIDE_WHITE) {                             \
                pad_white_action (32);                                        \
        }


        for (action_num = 0; ; ++action_num) {
                last_action = action;
                action = gibbon_game_get_nth_action (game, action_num, &side);
                if (!action)
                        break;
                if (action_num)
                        position = gibbon_game_get_nth_position (game,
                                                                 action_num
                                                                 - 1);
                if (GIBBON_IS_ROLL (action)) {
                        if (!side)
                                continue;
                        new_half_move ();
                        buffer = gibbon_jelly_fish_writer_roll (self,
                                                          GIBBON_ROLL (action));
                        write_buffer ();
                } else if (GIBBON_IS_MOVE (action)) {
                        buffer = gibbon_jelly_fish_writer_move (self, position,
                                                                side,
                                                          GIBBON_MOVE (action));
                        write_buffer ();
                } else if (GIBBON_IS_DOUBLE (action)) {
                        new_half_move ();
                        buffer = gibbon_jelly_fish_writer_double (self,
                                                                  position);
                        write_buffer ();
                } else if (GIBBON_IS_DROP (action)) {
                        new_half_move ();
                        buffer = gibbon_jelly_fish_writer_drop (self);
                        write_buffer ();
                } else if (GIBBON_IS_TAKE (action)) {
                        new_half_move ();
                        buffer = gibbon_jelly_fish_writer_take (self);
                        write_buffer ();
                }
        }

        score = gibbon_game_over (game);
        if (score) {
                if (score > 0 && GIBBON_IS_MOVE (last_action)) {
                        if (!g_output_stream_write_all (out, "\015\012", 2,
                                                        NULL, NULL, error)) {
                                return FALSE;
                        }
                        column = 0;
                }
                if (score > 0)
                        pad_white_action(33);
                position = gibbon_game_get_nth_position (game, -1);
                buffer = g_strdup_printf ("%s Wins %u point%s%s\015\012",
                                          score < 0 ? "\015\012" : "",
                                          abs (score),
                                          (score == 1 || score == -1)
                                                  ? "" : "s",
                                          gibbon_position_match_over (position)
                                                  ? " and the match" : "");
                write_buffer ();
        }

        if (last_char != '\012') {
                if (!g_output_stream_write_all (out, "\015\012", 2,
                                                NULL, NULL, error)) {
                        return FALSE;
                }
        }

        return TRUE;
}

static gchar *
gibbon_jelly_fish_writer_roll (const GibbonJellyFishWriter *self,
                               const GibbonRoll *roll)
{
        gchar *buffer;

        buffer = g_strdup_printf (" %d%d:", roll->die1, roll->die2);

        return buffer;
}

static gchar *
gibbon_jelly_fish_writer_move (const GibbonJellyFishWriter *self,
                               const GibbonPosition *position,
                               GibbonPositionSide side,
                               const GibbonMove *move)
{
        GString *s = g_string_new ("");
        gchar *buffer;
        gsize i;
        gint from, to;

        for (i = 0; i < move->number; ++i) {
                from = move->movements[i].from;
                to = move->movements[i].to;

                /* JellyFish moves are always high to low.  */
                if (from < to) {
                        from = -from + 25;
                        to = -to + 25;
                }
                g_string_append_printf (s, " %d/%d", from, to);
        }

        buffer = s->str;
        g_string_free (s, FALSE);

        return buffer;
}

static gchar *
gibbon_jelly_fish_writer_double (const GibbonJellyFishWriter *self,
                                  const GibbonPosition *position)
{
        return g_strdup_printf ("  Doubles => %llu",
                                (unsigned long long) position->cube << 1);
}

static gchar *
gibbon_jelly_fish_writer_take (const GibbonJellyFishWriter *self)
{
        return g_strdup_printf ("  Takes");
}

static gchar *
gibbon_jelly_fish_writer_drop (const GibbonJellyFishWriter *self)
{
        return g_strdup_printf ("  Drops");
}
