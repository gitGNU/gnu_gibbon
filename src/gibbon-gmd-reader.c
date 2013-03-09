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
 * SECTION:gibbon-gmd-reader
 * @short_description: Read GMD (Gibbon Match Dump) format.
 *
 * Since: 0.2.0
 *
 * A #GibbonMatchReader for reading match files in the GMD format.!
 */

#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <stdlib.h>

#include <glib.h>
#include <glib/gi18n.h>
#include <gdk/gdk.h>

#include "gibbon-gmd-reader-priv.h"

#include "gibbon-app.h"
#include "gibbon-game.h"
#include "gibbon-game-action.h"
#include "gibbon-game-actions.h"

typedef struct _GibbonGMDReaderPrivate GibbonGMDReaderPrivate;
struct _GibbonGMDReaderPrivate {
        GibbonMatchReaderErrorFunc yyerror;
        gpointer user_data;
        const gchar *filename;
        void *yyscanner;
        GibbonMatch *match;

        GSList *names;
};

#define GIBBON_GMD_READER_PRIVATE(obj) (G_TYPE_INSTANCE_GET_PRIVATE ((obj), \
        GIBBON_TYPE_GMD_READER, GibbonGMDReaderPrivate))

G_DEFINE_TYPE (GibbonGMDReader, gibbon_gmd_reader, GIBBON_TYPE_MATCH_READER)

static GibbonMatch *gibbon_gmd_reader_parse (GibbonMatchReader *match_reader,
                                             const gchar *filename);
static gboolean gibbon_gmd_reader_add_action (GibbonGMDReader *self,
                                              GibbonPositionSide side,
                                              gint64 timestamp,
                                              GibbonGameAction *action);
static void gibbon_gmd_reader_error (const GibbonGMDReader *self,
                                     const gchar *msg);

static void 
gibbon_gmd_reader_init (GibbonGMDReader *self)
{
        self->priv = G_TYPE_INSTANCE_GET_PRIVATE (self,
                GIBBON_TYPE_GMD_READER, GibbonGMDReaderPrivate);

        self->priv->yyerror = NULL;
        self->priv->user_data = NULL;

        /* Per parser-instance data.  */
        self->priv->filename = NULL;
        self->priv->match = NULL;
        self->priv->names = NULL;
}

static void
gibbon_gmd_reader_finalize (GObject *object)
{
        GibbonGMDReader *self = GIBBON_GMD_READER (object);

        _gibbon_gmd_reader_free_names (self);

        G_OBJECT_CLASS (gibbon_gmd_reader_parent_class)->finalize(object);
}

static void
gibbon_gmd_reader_class_init (GibbonGMDReaderClass *klass)
{
        GObjectClass *object_class = G_OBJECT_CLASS (klass);
        GibbonMatchReaderClass *gibbon_match_reader_class =
                        GIBBON_MATCH_READER_CLASS (klass);

        gibbon_match_reader_class->parse = gibbon_gmd_reader_parse;
        
        g_type_class_add_private (klass, sizeof (GibbonGMDReaderPrivate));

        object_class->finalize = gibbon_gmd_reader_finalize;
}

/**
 * gibbon_gmd_reader_new:
 * @error_func: Error reporting function or %NULL
 * @user_data: Pointer to pass to @error_func or %NULL
 *
 * Creates a new #GibbonGMDReader.
 *
 * Returns: The newly created #GibbonGMDReader.
 */
GibbonGMDReader *
gibbon_gmd_reader_new (GibbonMatchReaderErrorFunc yyerror,
                       gpointer user_data)
{
        GibbonGMDReader *self = g_object_new (GIBBON_TYPE_GMD_READER,
                                                   NULL);

        self->priv->user_data = user_data;
        self->priv->yyerror = yyerror;

        return self;
}

static GibbonMatch *
gibbon_gmd_reader_parse (GibbonMatchReader *_self, const gchar *filename)
{
        GibbonGMDReader *self;
        FILE *in;
        int parse_status;
        void *yyscanner;

        g_return_val_if_fail (GIBBON_IS_GMD_READER (_self), NULL);
        self = GIBBON_GMD_READER (_self);

        if (gibbon_gmd_lexer_lex_init_extra (self, &yyscanner)) {
                g_error (_("Error creating tokenizer: %s!"),
                         strerror (errno));
                /* NOTREACHED */
                return NULL;
        }

        self->priv->filename = filename;
        self->priv->yyscanner = yyscanner;
        if (self->priv->match)
                g_object_unref (self->priv->match);
        self->priv->match = gibbon_match_new (NULL, NULL, 0, FALSE);
        _gibbon_gmd_reader_free_names (self);

        if (filename)
                in = fopen (filename, "rb");
        else
                in = stdin;

        if (in) {
                gibbon_gmd_lexer_set_in (in, yyscanner);

                parse_status = gibbon_gmd_parser_parse (yyscanner);
                if (filename)
                        fclose (in);
                if (parse_status) {
                        g_object_unref (self->priv->match);
                        self->priv->match = NULL;
                }
        } else {
                g_object_unref (self->priv->match);
                self->priv->match = NULL;
                gibbon_gmd_reader_error (self, strerror (errno));
        }

        self->priv->filename = NULL;
        self->priv->yyscanner = NULL;

        gibbon_gmd_lexer_lex_destroy (yyscanner);

        return self->priv->match;
}

static void
gibbon_gmd_reader_error (const GibbonGMDReader *self, const gchar *msg)
{
        gchar *full_msg;
        const gchar *filename;
        extern int gibbon_gmd_lexer_get_lineno ();
        int lineno;

        if (self->priv->filename)
                filename = self->priv->filename;
        else
                filename = _("[standard input]");

        lineno = gibbon_gmd_lexer_get_lineno (self->priv->yyscanner);

        if (lineno)
                full_msg = g_strdup_printf ("%s:%d: %s", filename, lineno, msg);
        else
                full_msg = g_strdup_printf ("%s: %s", filename, msg);

        if (self->priv->yyerror)
                self->priv->yyerror (self->priv->user_data, full_msg);
        else
                g_printerr ("%s\n", full_msg);

        g_free (full_msg);
}

void
gibbon_gmd_reader_yyerror (void *scanner, const gchar *msg)
{
        const GibbonGMDReader *reader;

        reader = (GibbonGMDReader *) gibbon_gmd_lexer_get_extra (scanner);

        if (reader)
                gibbon_gmd_reader_error (reader, msg);
}

void
_gibbon_gmd_reader_set_player (GibbonGMDReader *self,
                               GibbonPositionSide side,
                               const gchar *name)
{
        g_return_if_fail (GIBBON_IS_GMD_READER (self));
        g_return_if_fail (self->priv->match);

        if (side > 0)
                gibbon_match_set_white (self->priv->match, name);
        else if (side < 0)
                gibbon_match_set_black (self->priv->match, name);
}

void
_gibbon_gmd_reader_set_rank (GibbonGMDReader *self,
                               GibbonPositionSide side,
                               const gchar *rank)
{
        g_return_if_fail (GIBBON_IS_GMD_READER (self));
        g_return_if_fail (self->priv->match);

        if (!side)
                return;

        gibbon_match_set_rank (self->priv->match, side, rank);
}

void
_gibbon_gmd_reader_set_location (GibbonGMDReader *self,
                                 const gchar *location)
{
        g_return_if_fail (GIBBON_IS_GMD_READER (self));
        g_return_if_fail (self->priv->match);

        gibbon_match_set_location (self->priv->match, location);
}

void
_gibbon_gmd_reader_set_match_length (GibbonGMDReader *self, gint length)
{
        g_return_if_fail (GIBBON_IS_GMD_READER (self));
        g_return_if_fail (self->priv->match);

        gibbon_match_set_length (self->priv->match, length);
}

void
_gibbon_gmd_reader_set_crawford (GibbonGMDReader *self)
{
        g_return_if_fail (GIBBON_IS_GMD_READER (self));
        g_return_if_fail (self->priv->match);

        gibbon_match_set_crawford (self->priv->match, TRUE);
}

gboolean
_gibbon_gmd_reader_add_game (GibbonGMDReader *self)
{
        GError *error = NULL;

        g_return_val_if_fail (GIBBON_IS_GMD_READER (self), FALSE);
        g_return_val_if_fail (self->priv->match, FALSE);

        if (!gibbon_match_add_game (self->priv->match, &error)) {
                gibbon_gmd_reader_error (self, error->message);
                g_error_free (error);
                return FALSE;
        }

        return TRUE;
}

gboolean
_gibbon_gmd_reader_roll (GibbonGMDReader *self, GibbonPositionSide side,
                         gint64 timestamp, guint64 die1, guint64 die2)
{
        GibbonGameAction *action;

        g_return_val_if_fail (GIBBON_IS_GMD_READER (self), FALSE);
        g_return_val_if_fail (self->priv->match, FALSE);

        action = GIBBON_GAME_ACTION (gibbon_roll_new (die1, die2));

        return gibbon_gmd_reader_add_action (self, side, timestamp, action);
}

gboolean
_gibbon_gmd_reader_move (GibbonGMDReader *self, GibbonPositionSide side,
                         gint64 timestamp, guint64 encoded)
{
        GibbonMove *move;
        guint p[8], i;

        g_return_val_if_fail (GIBBON_IS_GMD_READER (self), FALSE);
        g_return_val_if_fail (self->priv->match, FALSE);

#define HOME 26
#define BAR 27
        if (encoded & 0xffff000000000000ULL) {
                p[0] = (encoded & 0xff00000000000000ULL) >> 56;
                p[1] = (encoded & 0x00ff000000000000ULL) >> 48;
                p[2] = (encoded & 0x0000ff0000000000ULL) >> 40;
                p[3] = (encoded & 0x000000ff00000000ULL) >> 32;
                p[4] = (encoded & 0x00000000ff000000ULL) >> 24;
                p[5] = (encoded & 0x0000000000ff0000ULL) >> 16;
                p[6] = (encoded & 0x000000000000ff00ULL) >> 8;
                p[7] = (encoded & 0x00000000000000ffULL);
                for (i = 0; i < 8; i += 2) {
                        if (p[i] == BAR) {
                                if (p[i + 1] > 18)
                                        p[i] = 25;
                                else if (p[i + 1] <= 6)
                                        p[i] = 0;
                        }
                }
                for (i = 1; i < 8; i += 2) {
                        if (p[i] == HOME) {
                                if (p[i - 1] > 18)
                                        p[i] = 25;
                                else if (p[i - 1] <= 6)
                                        p[i] = 0;
                        }
                }
                move = gibbon_move_newv (0, 0,
                                         p[0], p[1], p[2], p[3],
                                         p[4], p[5], p[6], p[7],
                                         -1);
        } else if (encoded & 0xffff00000000ULL) {
                p[0] = (encoded & 0x0000ff0000000000ULL) >> 40;
                p[1] = (encoded & 0x000000ff00000000ULL) >> 32;
                p[2] = (encoded & 0x00000000ff000000ULL) >> 24;
                p[3] = (encoded & 0x0000000000ff0000ULL) >> 16;
                p[4] = (encoded & 0x000000000000ff00ULL) >> 8;
                p[5] = (encoded & 0x00000000000000ffULL);
                for (i = 0; i < 6; i += 2) {
                        if (p[i] == BAR) {
                                if (p[i + 1] > 18)
                                        p[i] = 25;
                                else if (p[i + 1] <= 6)
                                        p[i] = 0;
                        }
                }
                for (i = 1; i < 6; i += 2) {
                        if (p[i] == HOME) {
                                if (p[i - 1] > 18)
                                        p[i] = 25;
                                else if (p[i - 1] <= 6)
                                        p[i] = 0;
                        }
                }
                move = gibbon_move_newv (0, 0,
                                         p[0], p[1], p[2], p[3],
                                         p[4], p[5],
                                         -1);
        } else if (encoded & 0xffff0000ULL) {
                p[0] = (encoded & 0x00000000ff000000ULL) >> 24;
                p[1] = (encoded & 0x0000000000ff0000ULL) >> 16;
                p[2] = (encoded & 0x000000000000ff00ULL) >> 8;
                p[3] = (encoded & 0x00000000000000ffULL);
                for (i = 0; i < 4; i += 2) {
                        if (p[i] == BAR) {
                                if (p[i + 1] > 18)
                                        p[i] = 25;
                                else if (p[i + 1] <= 6)
                                        p[i] = 0;
                        }
                }
                for (i = 1; i < 4; i += 2) {
                        if (p[i] == HOME) {
                                if (p[i - 1] > 18)
                                        p[i] = 25;
                                else if (p[i - 1] <= 6)
                                        p[i] = 0;
                        }
                }
                move = gibbon_move_newv (0, 0,
                                         p[0], p[1], p[2], p[3],
                                         -1);
        } else if (encoded & 0xffffULL) {
                p[0] = (encoded & 0x000000000000ff00ULL) >> 8;
                p[1] = (encoded & 0x00000000000000ffULL);
                if (p[0] == BAR) {
                        if (p[1] > 18)
                                p[0] = 25;
                        else if (p[1] <= 6)
                                p[0] = 0;
                }
                if (p[1] == HOME) {
                        if (p[0] > 18)
                                p[1] = 25;
                        else if (p[0] <= 6)
                                p[1] = 0;
                }
                move = gibbon_move_newv (0, 0,
                                         p[0], p[1],
                                         -1);
        } else {
                move = gibbon_move_newv (0, 0, -1);
        }

         return gibbon_gmd_reader_add_action (self, side, timestamp,
                                              GIBBON_GAME_ACTION (move));
}

gboolean
_gibbon_gmd_reader_double (GibbonGMDReader *self, GibbonPositionSide side,
                           gint64 timestamp)
{
        GibbonGameAction *action;

        g_return_val_if_fail (GIBBON_IS_GMD_READER (self), FALSE);
        g_return_val_if_fail (self->priv->match, FALSE);

        action = GIBBON_GAME_ACTION (gibbon_double_new ());

        return gibbon_gmd_reader_add_action (self, side, timestamp, action);
}

gboolean
_gibbon_gmd_reader_drop (GibbonGMDReader *self, GibbonPositionSide side,
                         gint64 timestamp)
{
        GibbonGameAction *action;

        g_return_val_if_fail (GIBBON_IS_GMD_READER (self), FALSE);
        g_return_val_if_fail (self->priv->match, FALSE);

        action = GIBBON_GAME_ACTION (gibbon_drop_new ());

        return gibbon_gmd_reader_add_action (self, side, timestamp, action);
}

gboolean
_gibbon_gmd_reader_take (GibbonGMDReader *self, GibbonPositionSide side,
                         gint64 timestamp)
{
        GibbonGameAction *action;

        g_return_val_if_fail (GIBBON_IS_GMD_READER (self), FALSE);
        g_return_val_if_fail (self->priv->match, FALSE);

        action = GIBBON_GAME_ACTION (gibbon_take_new ());

        return gibbon_gmd_reader_add_action (self, side, timestamp, action);
}

gboolean
_gibbon_gmd_reader_resign (GibbonGMDReader *self, GibbonPositionSide side,
                           gint64 timestamp, guint value)
{
        GibbonGameAction *action;

        g_return_val_if_fail (GIBBON_IS_GMD_READER (self), FALSE);
        g_return_val_if_fail (self->priv->match, FALSE);

        action = GIBBON_GAME_ACTION (gibbon_resign_new (value));

        return gibbon_gmd_reader_add_action (self, side, timestamp, action);
}

gboolean
_gibbon_gmd_reader_reject (GibbonGMDReader *self, GibbonPositionSide side,
                           gint64 timestamp)
{
        GibbonGameAction *action;

        g_return_val_if_fail (GIBBON_IS_GMD_READER (self), FALSE);
        g_return_val_if_fail (self->priv->match, FALSE);

        action = GIBBON_GAME_ACTION (gibbon_reject_new ());

        return gibbon_gmd_reader_add_action (self, side, timestamp, action);
}

gboolean
_gibbon_gmd_reader_accept (GibbonGMDReader *self, GibbonPositionSide side,
                           gint64 timestamp)
{
        GibbonGameAction *action;

        g_return_val_if_fail (GIBBON_IS_GMD_READER (self), FALSE);
        g_return_val_if_fail (self->priv->match, FALSE);

        action = GIBBON_GAME_ACTION (gibbon_accept_new ());

        return gibbon_gmd_reader_add_action (self, side, timestamp, action);

        return TRUE;
}

static gboolean
gibbon_gmd_reader_add_action (GibbonGMDReader *self, GibbonPositionSide side,
                              gint64 timestamp, GibbonGameAction *action)
{
        GibbonGame *game;
        GError *error = NULL;

        game = gibbon_match_get_current_game (self->priv->match);
        if (!game) {
                gibbon_gmd_reader_error (self, _("No game in progress!"));
                g_object_unref (action);
                return FALSE;
        }

        if (!gibbon_game_add_action (game, side, action, timestamp, &error)) {
                gibbon_gmd_reader_error (self, error->message);
                g_object_unref (action);
                return FALSE;
        }

        return TRUE;
}

gchar *
gibbon_gmd_reader_alloc_name (GibbonGMDReader *self, const gchar *name)
{
        gchar *unescaped;

        g_return_val_if_fail (GIBBON_IS_GMD_READER (self), NULL);

        unescaped = g_strcompress (name);
        self->priv->names = g_slist_prepend (self->priv->names, unescaped);

        return self->priv->names->data;
}

void
_gibbon_gmd_reader_free_names (GibbonGMDReader *self)
{
        g_return_if_fail (GIBBON_IS_GMD_READER (self));

        g_slist_foreach (self->priv->names, (GFunc) g_free, NULL);
        g_slist_free (self->priv->names);
        self->priv->names = NULL;
}

gboolean
_gibbon_gmd_reader_check_setup (GibbonGMDReader *self)
{
        g_return_val_if_fail (GIBBON_IS_GMD_READER (self), FALSE);

        if (1 != gibbon_match_get_number_of_games (self->priv->match)) {
                gibbon_gmd_reader_error (self,
                                         _("Position setup is only"
                                           " allowed for first game!"));
                return FALSE;
        }
        return TRUE;
}

gboolean
_gibbon_gmd_reader_setup_position (GibbonGMDReader *self, gint64 b1,
                                   gint64 p1, gint64 p2, gint64 p3,
                                   gint64 p4, gint64 p5, gint64 p6,
                                   gint64 p7, gint64 p8, gint64 p9,
                                   gint64 p10, gint64 p11, gint64 p12,
                                   gint64 p13, gint64 p14, gint64 p15,
                                   gint64 p16, gint64 p17, gint64 p18,
                                   gint64 p19, gint64 p20, gint64 p21,
                                   gint64 p22, gint64 p23, gint64 p24,
                                   gint64 b2)
{
        GibbonPosition *pos;
        GibbonGame *game;
        gint i;
        guint64 wcheckers, bcheckers;

        g_return_val_if_fail (GIBBON_IS_GMD_READER (self), FALSE);

        if (b1 < 0) {
                gibbon_gmd_reader_error (self,
                                         _("Only positive number allowed"
                                           " for white's bar!"));
                return FALSE;
        }
        if (b2 < 0) {
                gibbon_gmd_reader_error (self,
                                         _("Only positive number allowed"
                                           " for black's bar!"));
                return FALSE;
        }

        game = gibbon_match_get_current_game (self->priv->match);
        pos = gibbon_game_get_initial_position_editable (game);

        pos->bar[0] = b1;
        pos->bar[1] = b2;
        pos->points[0] = p1;
        pos->points[1] = p2;
        pos->points[2] = p3;
        pos->points[3] = p4;
        pos->points[4] = p5;
        pos->points[5] = p6;
        pos->points[6] = p7;
        pos->points[7] = p8;
        pos->points[8] = p9;
        pos->points[9] = p10;
        pos->points[10] = p11;
        pos->points[11] = p12;
        pos->points[12] = p13;
        pos->points[13] = p14;
        pos->points[14] = p15;
        pos->points[15] = p16;
        pos->points[16] = p17;
        pos->points[17] = p18;
        pos->points[18] = p19;
        pos->points[19] = p20;
        pos->points[20] = p21;
        pos->points[21] = p22;
        pos->points[22] = p23;
        pos->points[23] = p24;

        /*
         * Check the sum of black's and white's checkers.  There is no need
         * to bother about overflow here.  The parser already checked that
         * all values are in the range [-15 ... +15].
         */
        wcheckers = pos->bar[0];
        bcheckers = pos->bar[1];
        for (i = 0; i < 24; ++i) {
                if (pos->points[i] < 0)
                        bcheckers -= pos->points[i];
                else
                        wcheckers += pos->points[i];
        }
        if (wcheckers > 15 || bcheckers > 15) {
                gibbon_gmd_reader_error (self,
                                         _("Number of checkers out of"
                                           " range (must be less or equal"
                                           " than 15)!"));
                return FALSE;
        }
        if (!wcheckers && !bcheckers) {
                gibbon_gmd_reader_error (self, _("Impossible position!"));
                return FALSE;
        }

        /*
         * There are more impossible positions but we allow them deliberately.
         */

        return TRUE;
}

gboolean
_gibbon_gmd_reader_setup_dice (GibbonGMDReader *self,
                               guint64 die1, guint64 die2)
{
        GibbonPosition *pos;
        GibbonGame *game;

        g_return_val_if_fail (GIBBON_IS_GMD_READER (self), FALSE);
        g_return_val_if_fail (die1 != 0, FALSE);
        g_return_val_if_fail (die2 != 0, FALSE);
        g_return_val_if_fail (die1 <= 6, FALSE);
        g_return_val_if_fail (die2 <= 6, FALSE);

        game = gibbon_match_get_current_game (self->priv->match);
        pos = gibbon_game_get_initial_position_editable (game);

        pos->dice[0] = die1;
        pos->dice[1] = die2;

        return TRUE;
}

gboolean
_gibbon_gmd_reader_setup_scores (GibbonGMDReader *self,
                                 gint64 score1, gint64 score2)
{
        GibbonPosition *pos;
        GibbonGame *game;

        g_return_val_if_fail (GIBBON_IS_GMD_READER (self), FALSE);
        g_return_val_if_fail (score1 >= 0, FALSE);
        g_return_val_if_fail (score2 >= 0, FALSE);

        game = gibbon_match_get_current_game (self->priv->match);
        pos = gibbon_game_get_initial_position_editable (game);

        pos->scores[0] = score1;
        pos->scores[1] = score2;

        return TRUE;
}

gboolean
_gibbon_gmd_reader_setup_cube (GibbonGMDReader *self, gint64 cube,
                               GibbonPositionSide turned)
{
        GibbonPosition *pos;
        GibbonGame *game;

        g_return_val_if_fail (GIBBON_IS_GMD_READER (self), FALSE);
        g_return_val_if_fail (cube >= 1, FALSE);

        if (cube < -1 || cube > 1) {
                game = gibbon_match_get_current_game (self->priv->match);
                pos = gibbon_game_get_initial_position_editable (game);

                pos->cube = abs (cube);
                pos->turn = turned;

                if (cube < -1) {
                        pos->may_double[0] = TRUE;
                        pos->may_double[1] = FALSE;
                } else {
                        pos->may_double[0] = FALSE;
                        pos->may_double[1] = TRUE;
                }

                if (turned < 0)
                        pos->cube_turned = GIBBON_POSITION_SIDE_BLACK;
                else if (turned > 0)
                        pos->cube_turned = GIBBON_POSITION_SIDE_WHITE;
                else
                        pos->cube_turned = GIBBON_POSITION_SIDE_NONE;
        }

        return TRUE;
}

gboolean
_gibbon_gmd_reader_setup_turn (GibbonGMDReader *self, gint64 turn)
{
        GibbonPosition *pos;
        GibbonGame *game;

        g_return_val_if_fail (GIBBON_IS_GMD_READER (self), FALSE);

        game = gibbon_match_get_current_game (self->priv->match);
        pos = gibbon_game_get_initial_position_editable (game);

        if (turn < 0)
                pos->turn = GIBBON_POSITION_SIDE_BLACK;
        else if (turn > 0)
                pos->turn = GIBBON_POSITION_SIDE_WHITE;
        else
                pos->turn = GIBBON_POSITION_SIDE_NONE;

        return TRUE;
}

gboolean
_gibbon_gmd_reader_setup_may_double (GibbonGMDReader *self,
                                     gint64 flag1, gint64 flag2)
{
        GibbonPosition *pos;
        GibbonGame *game;
        gsize length;

        g_return_val_if_fail (GIBBON_IS_GMD_READER (self), FALSE);

        /* Ignore all other errors silently.  */
        length = gibbon_match_get_length (self->priv->match);
        if (!length)
                return TRUE;

        game = gibbon_match_get_current_game (self->priv->match);
        pos = gibbon_game_get_initial_position_editable (game);

        pos->may_double[0] = flag1 ? TRUE : FALSE;
        pos->may_double[1] = flag2 ? TRUE : FALSE;

        if (length != pos->scores[0] + 1 && length != pos->scores[1] + 1)
                return TRUE;

        if (pos->cube == 1 && !flag1 && !flag2)
                gibbon_game_set_is_crawford (game, TRUE);
        else
                gibbon_game_set_is_crawford (game, FALSE);

        return TRUE;
}
