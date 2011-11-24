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
 * @cube_turned: %GIBBON_POSITION_SIDE_WHITE if white has turned the cube,
 *               %GIBBON_POSITION_SIDE_BLACK if black has turned the cube,
 *               %GIBBON_POSITION_SIDE_NONE otherwise.
 * @match_length: Length of the match of 0 for unlimited.
 * @unused_dice: Can be filled with the dice not yet used in this roll.
 *               They should be normalized, i.e. all positive for white or all
 *               negative for black, even for the opening roll.
 * @resigned: 1 if white resigned normally
 *            2 if white resigned with a gammon
 *            3 if white resigned with a backgammon
 *           -1 if black resigned normally
 *           -2 if black resigned with a gammon
 *           -3 if black resigned with a backgammon
 * @game_info: Free-form string describing the game ("Crawford", ...).
 * @status: Free-form string describing the status ("It's your move", ...).
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
        guint match_length;
        guint scores[2];

        gint points[24];

        guint bar[2];

        gint dice[2];

        guint cube;
        gboolean may_double[2];
        GibbonPositionSide cube_turned;


        gint unused_dice[4];

        gchar *players[2];

        gint resigned;

        gchar *game_info;
        gchar *status;
};

/**
 * GibbonMovement:
 * @from: The starting point for a move.  1 is the ace point for white, O,
 *        or the player with positive checker counts.  23 is the ace point
 *        for black, X, or the player with negative checker counts.  0 and
 *        24 represent home and the bar accordingly.
 * @to: The end point for a move, see @from for semantics.
 * @die: The die value used for the move.
 *
 * Structure representing a single backgammon checker movement.
 */
typedef struct _GibbonMovement GibbonMovement;
struct _GibbonMovement
{
        gint from;
        gint to;
        gint die;
};

/**
 * GibbonMoveError:
 * @GIBBON_MOVE_LEGAL: legal move
 * @GIBBON_MOVE_ILLEGAL: illegal move
 * @GIBBON_MOVE_TOO_MANY_MOVES: more checkers moved than dice rolled
 * @GIBBON_MOVE_BLOCKED: one of the intermediate landing points was occupied,
 *                       never used here
 * @GIBBON_MOVE_USE_ALL: at least one more checker can be moved
 * @GIBBON_MOVE_USE_HIGHER: in doubt, you must use the higher value
 * @GIBBON_MOVE_TRY_SWAP: two checkers can be moved by swapping the dice order
 * @GIBOBN_MOVE_PREMATURE_BEAR_OFF: checker borne off with checkers outhside
 *                                  home board
 * @GIBBON_MOVE_ILLEGAL_WASTE: move higher before bearing off with waste
 * @GIBBON_MOVE_DANCING: Must come in from the bar first
 *
 * Use these symbolic constants, when referring to one side of the board.
 */
typedef enum {
        GIBBON_MOVE_LEGAL = 0,
        GIBBON_MOVE_ILLEGAL = 1,
        GIBBON_MOVE_TOO_MANY_MOVES = 2,
        GIBBON_MOVE_BLOCKED = 3,
        GIBBON_MOVE_USE_ALL = 4,
        GIBBON_MOVE_USE_HIGHER = 5,
        GIBBON_MOVE_TRY_SWAP = 6,
        GIBBON_MOVE_PREMATURE_BEAR_OFF = 7,
        GIBBON_MOVE_ILLEGAL_WASTE = 8,
        GIBBON_MOVE_DANCING = 9
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
gboolean gibbon_position_equals_technically (const GibbonPosition *self,
                                             const GibbonPosition *other);
void gibbon_position_dump_position (const GibbonPosition *self);

/* Apply a move to a position.  The function only does a plausability test,
 * not a legality test.
 *
 * If reverse is true, assume the board is turned.
 *
 * White is considered to move from 23 to 0, black the other way round.  24
 * is white's bar and black's home, 0 is black's bar and white's home.
 */
gboolean gibbon_position_apply_move (GibbonPosition *self,
                                     const GibbonMove *move,
                                     GibbonPositionSide side,
                                     gboolean reverse);
gboolean gibbon_position_game_over (const GibbonPosition *position);
GibbonPositionSide gibbon_position_on_move (const GibbonPosition *position);

/* Free return value with g_free()!  */
gchar *gibbon_position_format_move (GibbonPosition *self,
                                    const GibbonMove *move,
                                    GibbonPositionSide side,
                                    gboolean reverse);
gchar *gibbon_position_fibs_move (GibbonPosition *self,
                                  const GibbonMove *move,
                                  GibbonPositionSide side,
                                  gboolean reverse);

#endif
