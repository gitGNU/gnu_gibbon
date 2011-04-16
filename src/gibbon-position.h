/*
 * This file is part of gibbon.
 * Gibbon is a Gtk+ frontend for the First Internet Backgammon Server FIBS.
 * Copyright (C) 2009-2011 Guido Flohr, http://guido-flohr.net/.
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

#ifndef _GIBBON_POSITION_H
# define _GIBBON_POSITION_H

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <glib.h>
#include <glib-object.h>

#define GIBBON_TYPE_POSITION (gibbon_position_get_type ())

/**
 * GibbonPositionSide:
 * @GIBBON_POSITION_SIDE_BLACK: black or X
 * @GIBBON_POSITION_SIDE_NONE: neither side
 * @GIBBON_POSITION_SIDE_WHITE: white or O
 *
 * Use these symbolic constants, when referring to one side of the board.
 */
typedef enum {
        GIBBON_POSITION_SIDE_BLACK = -1,
        GIBBON_POSITION_SIDE_NONE = 0,
        GIBBON_POSITION_SIDE_WHITE = 1
} GibbonPositionSide;

/**
 * GibbonPosition:
 * @players: @players[0] is the white player name, @players[1] the black player;
 *           %NULL representing unknown.  If you use gibbon_position_free()
 *           for disposing the object the pointers used will be passed
 *           to g_free()!
 * @points: points[0] is the ace point for white, and the 24 point for
 *          black; points[23] is the ace point for black, and the 24 point
 *          for number.  The absolute value gives the number of checkers on
 *          that point.  A negative value means that this point is occupied
 *          by black, a positive value means that it is occupied by white.
 * @bar: bar[0] holds the number of white checkers on the bar, bar[1] the
 *       number of black checkers on the bar.  For consistency, black's
 *       checkers are negative, white's checkers are positive.  Yes, and
 *       for consistency this should be called bars not bar but that sounds
 *       too ugly.
 * @dice: value of the two dice if currently visible.  Negative values are
 *        used for black's dice, positive ones for white's dice.  In the
 *        opening roll(s) the signs of the two integers may differ from
 *        each other.  A zero value means that the dice are not visible
 *        (for whatever reason).  If both dice are 0 the game was not yet
 *        started or is over.
 * @cube: current value of the cube or -1 if doubling is not possible during
 *        this game.
 * @match_length: length of the match.
 * @scores: white and black score (white first).
 * @may_double: %TRUE if respective player may double, %FALSE otherwise.
 * @match_length: Length of the match of 0 for unlimited.
 * @game_info: Free-form string describing the game ("Crawford", ...).
 *
 * A boxed type representing a backgammon position.
 *
 * The player colors black and white must not be taken literally.  They are
 * just placeholders.  In Gibbon the user or the player that the user is
 * watching always plays with the "white" checkers, the opponent with the
 * black checkers.  "White" has its home board in the bottom right corner,
 * "black" in the top-right corner.  "White" is sometimes referred to
 * as O, and "Black" as X.  Checker counts for "white" are positive,
 * checker counts for "black" are negative.
 *
 * The visual board representation may differ.  The colors black and white
 * may be swapped and there are four distinct options for the location of
 * the "white" home board, with each option having an impact on the direction
 * of movement of the checkers.
 */
typedef struct _GibbonPosition GibbonPosition;
struct _GibbonPosition
{
        gchar *players[2];

        gint match_length;
        gint scores[2];

        /*< private >*/
        gint left_pad[6];

        /*< public >*/
        gint points[24];

        /*< private >*/
        gint right_pad[6];

        /*< public >*/
        gint bar[2];

        gint dice[2];

        gint cube;
        gboolean may_double[2];

        gchar *game_info;
};

/**
 * GibbonMovement:
 * @from: The starting point for a move.  1 is the ace point for white, O,
 *        or the player with positive checker counts.  23 is the ace point
 *        for black, X, or the player with negative checker counts.  0 and
 *        24 represent home and the bar accordingly.
 * @to: The end point for a move, see @from for semantics.
 * @num: How many checkers were moved?
 *
 * Structure representing a single backgammon checker movement.
 */
typedef struct _GibbonMovement GibbonMovement;
struct _GibbonMovement
{
        gint from;
        gint to;
        gsize num;
};

/**
 * GibbonMoveError:
 * @GIBBON_MOVE_LEGAL: legal move
 * @GIBBON_MOVE_ILLEGAL: illegal move
 * @GIBBON_MOVE_TOO_MANY_MOVES: more checkers moved than dice rolled
 * @GIBBON_MOVE_OCCUPIED: one of the intermediate landing points was occupied,
 *                        probably never used
 *
 * Use these symbolic constants, when referring to one side of the board.
 */
typedef enum {
        GIBBON_MOVE_LEGAL = 0,
        GIBBON_MOVE_ILLEGAL = 1,
        GIBBON_MOVE_TOO_MANY_MOVES = 2,
        GIBBON_MOVE_BLOCKED = 3
} GibbonMoveError;

/**
 * GibbonMove:
 * @number: number of movements following (0 to 4).
 * @movements: the individual movements.
 * @status: status of this move.
 *
 * Structure representing a backgammon move.  This is always a checker move.
 * Other actions like doubling, resigning, etc. are not covered.
 */
typedef struct _GibbonMove GibbonMove;
struct _GibbonMove
{
        gint number;
        GibbonMoveError status;
        GibbonMovement movements[];
};

GType gibbon_position_get_type (void) G_GNUC_CONST;

GibbonPosition *gibbon_position_new (void);
void gibbon_position_free (GibbonPosition *self);
GibbonPosition *gibbon_position_copy (const GibbonPosition *self);

void gibbon_position_set_player (GibbonPosition *self,
                                 const gchar *name, GibbonPositionSide side);

guint gibbon_position_get_pip_count (const GibbonPosition *self,
                                     GibbonPositionSide side);
guint gibbon_position_get_borne_off (const GibbonPosition *self,
                                     GibbonPositionSide side);

GibbonMove *gibbon_position_check_move (const GibbonPosition *before,
                                        const GibbonPosition *after,
                                        GibbonPositionSide side);
GibbonMove *gibbon_position_alloc_move (gsize num_movements);

#endif
