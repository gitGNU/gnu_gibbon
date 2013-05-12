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

#include <stdlib.h>

#include <glib.h>
#include <glib/gi18n.h>

#include "gibbon-java-fibs-writer.h"
#include "gibbon-match.h"
#include "gibbon-game.h"
#include "gibbon-game-actions.h"

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
static gboolean gibbon_java_fibs_writer_move (const GibbonJavaFIBSWriter *self,
                                              GOutputStream *out,
                                              GibbonPositionSide side,
                                              GibbonMove *move,
                                              const GibbonMatch *match,
                                              gboolean swap,
                                              GError **error);
static gboolean gibbon_java_fibs_writer_double (const GibbonJavaFIBSWriter *self,
                                               GOutputStream *out,
                                               GibbonPositionSide side,
                                               GibbonDouble *dbl,
                                               const GibbonMatch *match,
                                               GError **error);
static gboolean gibbon_java_fibs_writer_take (const GibbonJavaFIBSWriter *self,
                                              GOutputStream *out,
                                              GibbonPositionSide side,
                                              GibbonTake *take,
                                              const GibbonMatch *match,
                                              GError **error);
static gboolean gibbon_java_fibs_writer_drop (const GibbonJavaFIBSWriter *self,
                                              GOutputStream *out,
                                              GibbonPositionSide side,
                                              GibbonDrop *drop,
                                              const GibbonMatch *match,
                                              GError **error);
static gboolean gibbon_java_fibs_writer_resign (const GibbonJavaFIBSWriter *self,
                                               GOutputStream *out,
                                               GibbonPositionSide side,
                                               GibbonResign *resign,
                                               const GibbonMatch *match,
                                               GError **error);
static gboolean gibbon_java_fibs_writer_reject (const GibbonJavaFIBSWriter *self,
                                                GOutputStream *out,
                                                GibbonPositionSide side,
                                                GibbonReject *reject,
                                                const GibbonMatch *match,
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
        gchar *prolog, *buffer;
        guint scores[2] = {0, 0};
        unsigned long long cs[2];
        gint score;
        gsize match_length;
        const gchar *winner, *loser;

        self = GIBBON_JAVA_FIBS_WRITER (_self);
        g_return_val_if_fail (self != NULL, FALSE);

        match_length = gibbon_match_get_length (match);

        prolog = g_strdup_printf ("%s\015\0128:%s:%llu\015\012",
                                  GIBBON_JAVA_FIBS_PROLOG,
                                  gibbon_match_get_black (match),
                                  match_length < 0 ? 9999ULL :
                                  (unsigned long long) match_length);
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

                score = gibbon_game_over (game);
                if (score) {
                        if (score < 0) {
                                winner = gibbon_match_get_black (match);
                                scores[1] -= score;
                                cs[0] = scores[0];
                                cs[1] = scores[1];
                        } else {
                                winner = "You";
                                scores[0] += score;
                                cs[0] = scores[1];
                                cs[1] = scores[0];
                        }
                        buffer = g_strdup_printf ("7:%s:%llu\015\012",
                                                  winner,
                                                  (unsigned long long)
                                                  abs (score));
                        if (!g_output_stream_write_all (out, buffer,
                                                        strlen (buffer),
                                                        NULL, NULL, error)) {
                                g_free (buffer);
                                return FALSE;
                        }
                        g_free (buffer);
                        buffer = NULL;

                        if (match_length > 0
                            && (scores[0] >= match_length
                                || scores[1] >= match_length)) {
                                buffer = g_strdup_printf ("9:%s:%llu\015\012",
                                                          scores[0]
                                                          > scores[1] ?
                                                          "You" :
                                                          winner,
                                                          (unsigned long long)
                                                          match_length);
                        } else if (match_length > 0) {
                                if (score < 0) {
                                        winner = gibbon_match_get_black (match);
                                        loser = gibbon_match_get_white (match);
                                } else {
                                        winner = gibbon_match_get_white (match);
                                        loser = gibbon_match_get_black (match);
                                }

                                buffer = g_strdup_printf ("14:%s-%llu:%s-%llu"
                                                          "\015\012",
                                                winner, cs[1],
                                                loser, cs[0]);
                        }

                        if (buffer) {
                                if (!g_output_stream_write_all (out, buffer,
                                                                strlen (buffer),
                                                                NULL, NULL,
                                                                error)) {
                                        g_free (buffer);
                                        return FALSE;
                                }
                                g_free (buffer);
                        }
                }
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

        prolog = g_strdup_printf ("13:%s:\015\0126:You:%s\015\012",
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
                } else if (GIBBON_IS_MOVE (action)) {
                        if (!side)
                                continue;
                        if (!gibbon_java_fibs_writer_move (self, out, side,
                                                           GIBBON_MOVE (action),
                                                           match,
                                                           game_number % 2,
                                                           error))
                                return FALSE;
                } else if (GIBBON_IS_DOUBLE (action)) {
                        if (!side)
                                continue;
                        if (!gibbon_java_fibs_writer_double (self, out, side,
                                                         GIBBON_DOUBLE (action),
                                                             match, error))
                                return FALSE;
                } else if (GIBBON_IS_TAKE (action)) {
                        if (!side)
                                continue;
                        if (!gibbon_java_fibs_writer_take (self, out, side,
                                                           GIBBON_TAKE (action),
                                                           match, error))
                                return FALSE;
                } else if (GIBBON_IS_DROP (action)) {
                        if (!side)
                                continue;
                        if (!gibbon_java_fibs_writer_drop (self, out, side,
                                                           GIBBON_DROP (action),
                                                           match, error))
                                return FALSE;
                } else if (GIBBON_IS_RESIGN (action)) {
                        if (!side)
                                continue;
                        if (!gibbon_java_fibs_writer_resign (self, out, side,
                                                         GIBBON_RESIGN (action),
                                                             match, error))
                                return FALSE;
                } else if (GIBBON_IS_ACCEPT (action)) {
                        /* Ignored.  We write the game or match result
                         * instead.
                         */
                        continue;
                } else if (GIBBON_IS_REJECT (action)) {
                        if (!side)
                                continue;
                        if (!gibbon_java_fibs_writer_reject (self, out, side,
                                                         GIBBON_REJECT (action),
                                                             match, error))
                                return FALSE;
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

        buffer = g_strdup_printf ("%u:%s:%u %u\015\012",
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

static gboolean
gibbon_java_fibs_writer_move (const GibbonJavaFIBSWriter *self,
                              GOutputStream *out,
                              GibbonPositionSide side, GibbonMove *move,
                              const GibbonMatch *match, gboolean noswap,
                              GError **error)
{
        gchar *buffer;
        gsize i;
        GibbonMovement *movement;
        gint from, to;

        buffer = g_strdup_printf ("%u:%s:",
                                  move->number ? 1 : 15,
                                  side == GIBBON_POSITION_SIDE_WHITE ?
                                  "You" : gibbon_match_get_black (match));

        if (!g_output_stream_write_all (out, buffer, strlen (buffer),
                                        NULL, NULL, error)) {
                g_free (buffer);
                return FALSE;
        }
        g_free (buffer);

        for (i = 0; i < move->number; ++i) {
                movement = move->movements + i;
                from = noswap ? movement->from : 25 - movement->from;
                to = noswap ? movement->to : 25 - movement->to;
                if (from == 25 || from == 0)
                        buffer = g_strdup_printf ("%sbar-%u",
                                                  i ? " " : "", to);
                else if (to == 25 || to == 0)
                        buffer = g_strdup_printf ("%s%u-off",
                                                  i ? " " : "", from);
                else
                        buffer = g_strdup_printf ("%s%u-%u",
                                                  i ? " " : "", from, to);
                if (!g_output_stream_write_all (out, buffer, strlen (buffer),
                                                NULL, NULL, error)) {
                        g_free (buffer);
                        return FALSE;
                }
                g_free (buffer);
        }

        if (!g_output_stream_write_all (out, "\015\012", 2,
                                        NULL, NULL, error))
                return FALSE;

        return TRUE;
}

static gboolean
gibbon_java_fibs_writer_double (const GibbonJavaFIBSWriter *self,
                               GOutputStream *out,
                               GibbonPositionSide side, GibbonDouble *dbl,
                               const GibbonMatch *match, GError **error)
{
        gchar *buffer;

        buffer = g_strdup_printf ("2:%s:\015\012",
                                  side == GIBBON_POSITION_SIDE_WHITE ?
                                  "You" : gibbon_match_get_black (match));

        if (!g_output_stream_write_all (out, buffer, strlen (buffer),
                                        NULL, NULL, error)) {
                g_free (buffer);
                return FALSE;
        }
        g_free (buffer);

        return TRUE;
}

static gboolean
gibbon_java_fibs_writer_take (const GibbonJavaFIBSWriter *self,
                              GOutputStream *out,
                              GibbonPositionSide side, GibbonTake *take,
                              const GibbonMatch *match, GError **error)
{
        gchar *buffer;

        buffer = g_strdup_printf ("4:%s:\015\012",
                                  side == GIBBON_POSITION_SIDE_WHITE ?
                                  "You" : gibbon_match_get_black (match));

        if (!g_output_stream_write_all (out, buffer, strlen (buffer),
                                        NULL, NULL, error)) {
                g_free (buffer);
                return FALSE;
        }
        g_free (buffer);

        return TRUE;
}

static gboolean
gibbon_java_fibs_writer_drop (const GibbonJavaFIBSWriter *self,
                              GOutputStream *out,
                              GibbonPositionSide side, GibbonDrop *drop,
                              const GibbonMatch *match, GError **error)
{
        gchar *buffer;

        buffer = g_strdup_printf ("5:%s:\015\012",
                                  side == GIBBON_POSITION_SIDE_WHITE ?
                                  "You" : gibbon_match_get_black (match));

        if (!g_output_stream_write_all (out, buffer, strlen (buffer),
                                        NULL, NULL, error)) {
                g_free (buffer);
                return FALSE;
        }
        g_free (buffer);

        return TRUE;
}

static gboolean
gibbon_java_fibs_writer_resign (const GibbonJavaFIBSWriter *self,
                                GOutputStream *out,
                                GibbonPositionSide side, GibbonResign *resign,
                                const GibbonMatch *match, GError **error)
{
        gchar *buffer;

        buffer = g_strdup_printf ("3:%s:%u\015\012",
                                  side == GIBBON_POSITION_SIDE_WHITE ?
                                  "You" : gibbon_match_get_black (match),
                                  resign->value);

        if (!g_output_stream_write_all (out, buffer, strlen (buffer),
                                        NULL, NULL, error)) {
                g_free (buffer);
                return FALSE;
        }
        g_free (buffer);

        return TRUE;
}

static gboolean
gibbon_java_fibs_writer_reject (const GibbonJavaFIBSWriter *self,
                               GOutputStream *out,
                               GibbonPositionSide side, GibbonReject *reject,
                               const GibbonMatch *match, GError **error)
{
        gchar *buffer;

        buffer = g_strdup_printf ("12:%s:\015\012",
                                  side == GIBBON_POSITION_SIDE_WHITE ?
                                  "You" : gibbon_match_get_black (match));

        if (!g_output_stream_write_all (out, buffer, strlen (buffer),
                                        NULL, NULL, error)) {
                g_free (buffer);
                return FALSE;
        }
        g_free (buffer);

        return TRUE;
}
