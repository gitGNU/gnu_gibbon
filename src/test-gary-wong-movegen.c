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

#include <stdlib.h>
#include <time.h>
#include <errno.h>
#include <string.h>

#include <glib.h>

#include <gibbon-position.h>
#include <gibbon-move.h>

/* The code below was taken from a newsgroup archive found on
 * http://www.bkgm.com/rgb/rgb.cgi?view+593
 *
 * It is considered to be under the GPL.
 */

/*
 * Code for checking the legality of a backgammon move.
 *
 * by Gary Wong, 1997-8.
 *
 * Takes a starting position, ending position and dice roll as input,
 * and as output tells you whether the "move" to the end position was
 * legal, and if so, gives a chequer-by-chequer description of what the
 * move was.
 *
 * Boards are represented as arrays of 28 ints for the 24 points (0 is
 * the opponent's bar; 1 to 24 are the board points from the point of
 * view of the player moving; 25 is the player's bar; 26 is the player's
 * home and 27 is the opponent's home (unused)).  The player's chequers
 * are represented by positive integers and the opponent's by negatives.
 * This is compatible with FIBS or pubeval or something like that, I
 * forget who I originally stole it from :-)  The dice roll is an array of
 * 2 integers.  The function returns true if the move is legal (and fills
 * in the move array with up to 4 moves, as source/destination pairs),
 * and zero otherwise.  For instance, playing an opening 31 as 8/5 6/5
 * would be represented as:

 anBoardPre[] = { 0 -2 0 0 0 0 5 0 3 0 0 0 -5 5 0 0 0 -3 0 -5 0 0 0 0 2 0 0 0 }
anBoardPost[] = { 0 -2 0 0 0 2 4 0 2 0 0 0 -5 5 0 0 0 -3 0 -5 0 0 0 0 2 0 0 0 }
     anRoll[] = { 3 1 }

 * and LegalMove (anBoardPre, anBoardPost, anRoll, anMove) would return true
 * and set anMove[] to { 8 5 6 5 0 0 0 0 }.
 */

typedef struct {
    int fFound, cMaxMoves, cMaxPips, cMoves, cPips, *anBoard, *anRoll,
        *anMove;
} movedata;

static void
ApplyMove (int anBoard[], int iSrc, int nRoll)
{
    int iDest = iSrc - nRoll;

    if (iDest < 1)
        iDest = 26;

    anBoard[iSrc]--;

    if (anBoard[iDest] < 0) {
        anBoard[iDest] = 1;
        anBoard[0]++;
    } else
        anBoard[iDest]++;
}

static int
EqualBoard (int an0[], int an1[])
{
    int i;

    for (i = 0; i < 28; i++)
        if (an0[i] != an1[i])
            return 0;

    return 1;
}

static int
CanMove (int anBoard[], int iSrc, int nPips)
{
    int i, nBack = 0, iDest = iSrc - nPips;

    if (iDest > 0)
        return  (anBoard[iDest] >= -1);

    for (i = 1; i < 26; i++)
        if (anBoard[i] > 0)
            nBack = i;

    return  (nBack <= 6 &&  (iSrc == nBack || !iDest));
}

static void
SaveMoves (int cMoves, int cPips, int anBoard[28], int anMove[8],
           movedata *pmd) {

    int i;

    if (cMoves < pmd->cMaxMoves || cPips < pmd->cMaxPips)
        return;

    pmd->cMaxMoves = cMoves;
    pmd->cMaxPips = cPips;

    if (EqualBoard (anBoard, pmd->anBoard)) {
        pmd->fFound = 1;
        pmd->cMoves = cMoves;
        pmd->cPips = cPips;

        for (i = 0; i < 8; i++)
            pmd->anMove[i] = i < cMoves * 2 ? anMove[i] : 0;
    } else if (pmd->cMaxMoves > pmd->cMoves || pmd->cMaxPips > pmd->cPips)
        pmd->fFound = 0;
}

static int
GenerateMoves (int anBoard[28], int nMoveDepth, int iPip, int cPip,
               int anMove[8], movedata *pmd)
{
    int i, iCopy, fUsed = 0;
    int anBoardNew[28];


    if (nMoveDepth > 3 || !pmd->anRoll[nMoveDepth])
        return -1;

    if (anBoard[25]) {
        if (anBoard[25 - pmd->anRoll[nMoveDepth]] <= -2)
            return -1;

        anMove[nMoveDepth * 2] = 25;
        anMove[nMoveDepth * 2 + 1] = 25 - pmd->anRoll[nMoveDepth];

        for (i = 0; i < 28; i++)
            anBoardNew[i] = anBoard[i];

        ApplyMove (anBoardNew, 25, pmd->anRoll[nMoveDepth]);

        if (GenerateMoves (anBoardNew, nMoveDepth + 1, 24, cPip +
                           pmd->anRoll[nMoveDepth], anMove, pmd))
            SaveMoves (nMoveDepth + 1, cPip + pmd->anRoll[nMoveDepth],
                       anBoardNew, anMove, pmd);

        return 0;
    } else {
        for (i = iPip; i; i--)
            if (anBoard[i] > 0 && CanMove (anBoard, i,
                                             pmd->anRoll[nMoveDepth])) {
                anMove[nMoveDepth * 2] = i;
                anMove[nMoveDepth * 2 + 1] = i - pmd->anRoll[nMoveDepth];

                if (anMove[nMoveDepth * 2 + 1] < 1)
                    anMove[nMoveDepth * 2 + 1] = 26;

                for (iCopy = 0; iCopy < 28; iCopy++)
                    anBoardNew[iCopy] = anBoard[iCopy];

                ApplyMove (anBoardNew, i, pmd->anRoll[nMoveDepth]);

                if (GenerateMoves (anBoardNew, nMoveDepth + 1, pmd->anRoll[0]
                                   == pmd->anRoll[1] ? i : 24, cPip +
                                   pmd->anRoll[nMoveDepth], anMove, pmd))
                    SaveMoves (nMoveDepth + 1, cPip +
                               pmd->anRoll[nMoveDepth], anBoardNew, anMove,
                               pmd);

                fUsed = 1;
            }
    }

    return fUsed ? 0 : -1;
}

static int
LegalMove (int anBoardPre[28], int anBoardPost[28], int anRoll[2],
           int anMove[8])
{
    movedata md;
    int i, anMoveTemp[8], anRollRaw[4];
    int fLegalMoves;

    md.fFound = md.cMaxMoves = md.cMaxPips = md.cMoves = md.cPips = 0;
    md.anBoard = anBoardPost;
    md.anRoll = anRollRaw;
    md.anMove = anMove;

    anRollRaw[0] = anRoll[0];
    anRollRaw[1] = anRoll[1];

    anRollRaw[2] = anRollRaw[3] =  (anRoll[0] == anRoll[1]) ?
        anRoll[0] : 0;

    fLegalMoves = !GenerateMoves (anBoardPre, 0, 24, 0, anMoveTemp, &md);

    if (anRoll[0] != anRoll[1]) {
        anRollRaw[0] = anRoll[1];
        anRollRaw[1] = anRoll[0];

        fLegalMoves |= !GenerateMoves (anBoardPre, 0, 24, 0, anMoveTemp, &md);
    }

    if (!fLegalMoves) {
        for (i = 0; i < 8; i++)
            anMove[i] = 0;

        return EqualBoard (anBoardPre, anBoardPost);
    }

    return md.fFound;
}

/* End of Gary Wong's code.  */

#define DEBUG_TEST_ENGINE 0

static void compare_results (GibbonPosition *position,
                             GibbonPosition *post_position,
                             GibbonMove *move,
                             gint success, gint moves[8],
                             GibbonPositionSide side);
static void dump_position (const GibbonPosition *position);
static void dump_move (const GibbonMove *move);
static void translate_position (gint board[28], const GibbonPosition *position,
                                GibbonPositionSide turn);
static void test_game (void);
static void test_roll (GibbonPosition *position);
static void move_checker (GibbonPosition *position, gint board[28],
                          guint die, GibbonPositionSide side);
static void find_any_move (const GibbonPosition *position, gint board[28],
                           GibbonPosition *post_position,
                           GibbonPositionSide side, guint dice[5]);
static void find_good_move (const GibbonPosition *position, gint board[28],
                            GibbonPosition *post_position,
                            GibbonPositionSide side, guint dice[5]);
static void find_good_movement (gint board[28], guint dice[2],
                                GibbonPositionSide turn);
static gboolean come_in_from_bar (gint board[28], gint die,
                                  GibbonPositionSide turn);

static void print_moves (gint moves[8], GibbonPositionSide side, guint dice[2]);

#if (DEBUG_TEST_ENGINE)
static void print_movement (gint board[28], gint from, gint die,
                            GibbonPositionSide turn);
#endif

static unsigned long long total_positions = 100000;
static unsigned long long done_positions = 0;

int
main (int argc, char *argv[])
{
        long long random_seed = time (NULL);
        gboolean verbose = FALSE;

        g_type_init ();

        if (argc > 1) {
                errno = 0;
                total_positions = g_ascii_strtoull (argv[1], NULL, 10);
                if (errno) {
                        g_printerr ("Invalid number of positions `%s': %s!\n",
                                    argv[1], strerror (errno));
                        return -1;
                }
                g_print ("Testing %llu positions.\n", total_positions);
                verbose = TRUE;
        }

        if (argc > 2) {
                errno = 0;
                random_seed = g_ascii_strtoull (argv[2], NULL, 10);
                if (errno) {
                        g_printerr ("Invalid random seed `%s': %s!\n",
                                    argv[2], strerror (errno));
                        return -1;
                }
                g_print ("Using %lld as random seed.\n", random_seed);
        }
        srandom (random_seed);

        while (done_positions < total_positions)
                test_game ();

        return 0;
}

static void
test_game (void)
{
        GibbonPosition *position = gibbon_position_new ();
        GibbonPositionSide side = random () % 2
                        ? GIBBON_POSITION_SIDE_WHITE
                                        : GIBBON_POSITION_SIDE_BLACK;
        while (done_positions < total_positions) {
                position->dice[0] = 1 + random () % 6;
                position->dice[1] = 1 + random () % 6;
                position->turn = side;

                if (gibbon_position_game_over (position)) {
#if (DEBUG_TEST_ENGINE)
                        g_printerr ("***** Game over! *****\n");
#endif
                        break;
                }

                test_roll (position);

                side = -side;
        }

        gibbon_position_free (position);
}

static void
test_roll (GibbonPosition *position)
{
        gboolean is_double = position->dice[0] == position->dice[1];
        guint max_movements = is_double ? 4 : 2;
        GibbonPosition *post_position;
        guint i;
        GibbonPositionSide turn;
        guint dice[5], die;
        gint board[28];
        gint post_board[28];
        gint moves[8];
        GibbonMove *move;
        int legal;
        guint free_checkers;

        if (position->dice[0] < 0)
                turn = GIBBON_POSITION_SIDE_BLACK;
        else
                turn = GIBBON_POSITION_SIDE_WHITE;

        memset (dice, 0, sizeof dice);
        for (i = 0; i < max_movements; ++i)
                dice[i] = position->dice[i % 2];

        translate_position (board, position, turn);

        free_checkers = 15 - gibbon_position_get_borne_off (position, turn);
        if (free_checkers < max_movements)
                max_movements = free_checkers;

#if (DEBUG_TEST_ENGINE)
        dump_position (position);
#endif

        while (done_positions++ < total_positions) {
                /* Swap the dice after every try.  The "reasonable" move
                 * generator always uses the dice in order.  In the
                 * "tricky" situations it would then fail to find a move.
                 * The "tricky" ones are those where the first die has to
                 * be used first, in order to be able to use the second.
                 * Or where the first is the higher, and it has to be
                 * used in favor of the other.
                 */
                die = dice[0];
                dice[0] = dice[1];
                dice[1] = die;

                post_position = gibbon_position_copy (position);

                /* Half of the moves are more or less random, the other half
                 * more or less reasonable.
                 */
                if (!(random () & 0x1)) {
#if (DEBUG_TEST_ENGINE)
                        if (turn == GIBBON_POSITION_SIDE_WHITE)
                                g_printerr ("  ?? w: %u%u:", dice[0], dice[1]);
                        else
                                g_printerr ("  ?? b: %u%u:", dice[0], dice[1]);
#endif
                        find_any_move (position, board, post_position,
                                       turn, dice);
                } else {
#if (DEBUG_TEST_ENGINE)
                        if (turn == GIBBON_POSITION_SIDE_WHITE)
                                g_printerr ("  !! w: %u%u:", dice[0], dice[1]);
                        else
                                g_printerr ("  !! b: %u%u:", dice[0], dice[1]);
#endif
                        find_good_move (position, board, post_position,
                                        turn, dice);
                }

                translate_position (post_board, post_position, turn);

                move = gibbon_position_check_move (position, post_position,
                                                   turn);
                legal = LegalMove (board, post_board, (int *) dice, moves);
                compare_results (position, post_position, move,
                                 legal, moves, turn);
                g_object_unref (move);
                if (legal) {
#if (DEBUG_TEST_ENGINE)
                        print_moves (moves, turn, dice);
#endif

                        memcpy (position->points, post_position->points,
                                sizeof position->points);
                        memcpy (position->bar, post_position->bar,
                                sizeof position->bar);
                        gibbon_position_free (post_position);
                        break;
                }
                gibbon_position_free (post_position);
        }

        return;
}

/* This function moves a checker more or less randomly.
 *
 * However, the checkers are not moved completely randomly.  If there is
 * a checker on the bar, mostly moves from the bar are possible.
 *
 * A move to an occupied checker position is avoided most of the time.
 *
 * Bear-offs, when there are still checkers outside home, are also mostly
 * refuted.
 */
static void
move_checker (GibbonPosition *position, gint board[28],
              guint die, GibbonPositionSide turn)
{
        gint from;
        gint to;
        gboolean may_bear_off = TRUE;
        gint i;
        gint first_bear_off = -1;

        if (turn == GIBBON_POSITION_SIDE_WHITE) {
                if (position->bar[0])
                        may_bear_off = FALSE;
                else
                        for (i = 23; i > 5; --i) {
                                if (position->points[i] < -1) {
                                        may_bear_off = FALSE;
                                        break;
                                }
                        }
                if (may_bear_off)
                        for (i = 0; i < 6; ++i)
                                if (position->points[i] > 0)
                                        first_bear_off = i;
        } else {
                if (position->bar[1])
                        may_bear_off = FALSE;
                else
                        for (i = 0; i < 18; ++i) {
                                if (position->points[i] > 1) {
                                        may_bear_off = FALSE;
                                        break;
                                }
                        }
                for (i = 23; i >= 18; --i)
                        if (position->points[i] < 0)
                                first_bear_off = i;
        }

        while (1) {
                from = random () % 25;
                if (turn == GIBBON_POSITION_SIDE_WHITE) {
                        if (position->bar[0] && random () % 10)
                                from = 24;
                        if (from == 24) {
                                if (!position->bar[0])
                                        continue;
                                --position->bar[0];
                                if (position->points[24 - die] == -1) {
                                        ++position->bar[1];
                                        position->points[24 - die] = 0;
                                }
                                ++position->points[24 - die];
#if (DEBUG_TEST_ENGINE)
                                print_movement (board, 25, die, turn);
#endif
                                return;
                        } else if (position->points[from] > 0) {
                                to = from - die;
                                if (to < 0 && !may_bear_off && random () % 10)
                                        continue;
                                if (to < 0 && may_bear_off
                                    && to > first_bear_off && random () % 10)
                                        continue;
                                if (to > 0 && position->points[to] < -1
                                    && random () % 10)
                                        continue;
                                --position->points[from];
#if (DEBUG_TEST_ENGINE)
                                print_movement (board, from + 1, die, turn);
#endif
                                if (to < 0)
                                        return;
                                if (position->points[to] == -1) {
                                        ++position->bar[1];
                                        position->points[to] = 0;
                                }
                                ++position->points[to];
                                return;
                        }
                } else {
                        if (position->bar[1] && random () % 10)
                                from = 24;
                        if (from == 24) {
                                if (!position->bar[1])
                                        continue;
                                --position->bar[1];
                                if (position->points[die - 1] == 1) {
                                        ++position->bar[0];
                                        position->points[die - 1] = 0;
                                }
                                --position->points[die - 1];
#if (DEBUG_TEST_ENGINE)
                                print_movement (board, 25, die, turn);
#endif
                                return;
                        } else if (position->points[from] < 0) {
                                to = from + die;
                                if (to > 23 && !may_bear_off && random () % 10)
                                        continue;
                                if (to <= 23 && position->points[to] > 1
                                    && random () % 10)
                                        continue;
                                if (to >= 23 && may_bear_off
                                    && to < first_bear_off && random () % 10)
                                        continue;
                                ++position->points[from];
#if (DEBUG_TEST_ENGINE)
                                print_movement (board, 24 - from, die, turn);
#endif
                                if (to > 23)
                                        return;
                                if (position->points[to] == 1) {
                                        ++position->bar[0];
                                        position->points[to] = 0;
                                }
                                --position->points[to];
                                return;
                        }
                }
        }
}

static void
dump_position (const GibbonPosition *pos)
{
        gint i;

        /* FIXME! This gibbon_position_transform()! */
        g_printerr ("=== Position ===\n");
        g_printerr ("\
  +-13-14-15-16-17-18-------19-20-21-22-23-24-+ negative: black or X\n");
        g_printerr ("  |");
        for (i = 12; i < 18; ++i)
                if (pos->points[i])
                        g_printerr ("%+3d", pos->points[i]);
                else
                        g_printerr ("%s", "   ");
        g_printerr (" |%+3d|", pos->bar[1]);
        for (i = 18; i < 24; ++i)
                if (pos->points[i])
                        g_printerr ("%+3d", pos->points[i]);
                else
                        g_printerr ("%s", "   ");
        g_printerr (" |\n");
        g_printerr (" v| dice:  %u :  %u     ",
                    pos->dice[0], pos->dice[1]);
        g_printerr ("|BAR|                   | ");
        g_printerr (" Cube: %llu\n", (unsigned long long) pos->cube);
        g_printerr ("  |");
        for (i = 11; i >= 6; --i)
                if (pos->points[i])
                        g_printerr ("%+3d", pos->points[i]);
                else
                        g_printerr ("%s", "   ");
        g_printerr (" |%+3d|", pos->bar[0]);
        for (i = 5; i >= 0; --i)
                if (pos->points[i])
                        g_printerr ("%+3d", pos->points[i]);
                else
                        g_printerr ("%s", "   ");
        g_printerr (" |\n");
        g_printerr ("\
  +-12-11-10--9--8--7--------6--5--4--3--2--1-+ positive: white or O\n");
}

static void
dump_move (const GibbonMove *move)
{
        gint i;

        for (i = 0; i < move->number; ++i) {
                g_printerr (" %d/%d",
                            move->movements[i].from,
                            move->movements[i].to);
        }
        g_printerr ("\n");
}

static void
compare_results (GibbonPosition *position,
                GibbonPosition *post_position,
                GibbonMove *move,
                gint success, gint moves[8],
                GibbonPositionSide turn)
{
        gboolean match = TRUE;

        if (move->status == GIBBON_MOVE_LEGAL && !success)
                match = FALSE;
        else if (move->status != GIBBON_MOVE_LEGAL && success)
                match = FALSE;

        if (match)
                return;

        g_printerr ("Legality checks differ after %llu/%llu positions:\n",
                    done_positions, total_positions);
        g_printerr ("Gary Wong: %s, Gibbon: %s (%d)\n",
                    success ? "legal" : "illegal",
                    move->status == GIBBON_MOVE_LEGAL ? "legal" : "illegal",
                    move->status);

        g_printerr ("Starting position:\n");
        dump_position (position);
        g_printerr ("End position:\n");
        dump_position (post_position);

        if (success) {
                g_printerr ("Move according to Gary Wong:");
                print_moves (moves, turn, position->dice);
        }
        g_printerr ("Move according to Gibbon:");
        dump_move (move);

        exit (1);
}

static void
translate_position (gint board[28], const GibbonPosition *position,
                    GibbonPositionSide turn)
{
        int i;

        /* Translate position into structure expected by Gary's move
         * legality checker.
         */
        if (turn == GIBBON_POSITION_SIDE_WHITE) {
                board[0] = position->bar[1];
                memcpy (board + 1, position->points, sizeof position->points);
                board[25] = position->bar[0];
                board[26] = gibbon_position_get_borne_off (position,
                                GIBBON_POSITION_SIDE_WHITE);
                board[27] = gibbon_position_get_borne_off (position,
                                GIBBON_POSITION_SIDE_BLACK);
        } else {
                board[0] = position->bar[0];
                for (i = 0; i < 24; ++i) {
                        board[i + 1] = -position->points[23 - i];
                }
                board[25] = position->bar[1];
                board[26] = gibbon_position_get_borne_off (position,
                                GIBBON_POSITION_SIDE_BLACK);
                board[27] = gibbon_position_get_borne_off (position,
                                GIBBON_POSITION_SIDE_WHITE);
        }
}

static void
translate_board (GibbonPosition *position, gint board[28],
                 GibbonPositionSide turn)
{
        int i;

        /* Translate position into structure expected by Gary's move
         * legality checker.
         */
        if (turn == GIBBON_POSITION_SIDE_WHITE) {
                position->bar[1] = board[0];
                memcpy (position->points, board + 1, sizeof position->points);
                position->bar[0] = board[25];
        } else {
                position->bar[0] = board[0];
                for (i = 0; i < 24; ++i) {
                        position->points[23 - i] = -board[i + 1];
                }
                position->bar[1] = board[25];
        }
}

static void
print_moves (gint moves[8], GibbonPositionSide turn, guint dice[2])
{
        gint i;

        if (turn == GIBBON_POSITION_SIDE_WHITE)
                g_printerr ("W: %u%u", dice[0], dice[1]);
        else
                g_printerr ("B: %u%u", dice[0], dice[1]);

        if (!*moves) {
                g_printerr (" -\n");
                return;
        }

        for (i = 0; moves[i] && i < 8; i += 2)
                g_printerr (" %d/%d", moves[i], moves[i + 1]);

        g_printerr ("\n");
}

static void
find_any_move (const GibbonPosition *position, gint board[28],
               GibbonPosition *post_position,
               GibbonPositionSide turn, guint dice[5])
{
        gboolean is_double = dice[0] == dice[1];
        guint max_movements = is_double ? 4 : 2;
        gint free_checkers;
        gint i;
        gint num_movements;

        if (position->dice[0] < 0)
                turn = GIBBON_POSITION_SIDE_BLACK;
        else
                turn = GIBBON_POSITION_SIDE_WHITE;

        free_checkers = 15 - gibbon_position_get_borne_off (position, turn);
        if (free_checkers < max_movements)
                max_movements = free_checkers;

        /* Decide how many dice values to use.  We try to avoid those
         * that forfeit two or more checker movements because the
         * legality check for them is trivial.
         */
        num_movements = random () % (max_movements + 1);
        if (max_movements - num_movements >= 2
            && random () & 0x1)
                return find_any_move (position, board, post_position,
                                      turn, dice);

        if (!num_movements) {
#if (DEBUG_TEST_ENGINE)
                g_printerr (" -\n");
#endif
                return;
        }

        for (i = 0; i < num_movements; ++i) {
                move_checker (post_position, board, dice[i], turn);
        }
#if (DEBUG_TEST_ENGINE)
        g_printerr ("\n");
#endif
}

/* This is a mini backgammon engine.  It plays about the same level as "expert"
 * in gnubg.  The gnubg engine will still win most of the time because it
 * cheats.  We all know that very well.
 *
 * The reasoning behind it is not so much to produce realistic game scenarios
 * but to avoid the test engine to hang.  That happens when only a handful
 * of moves from a large quantity are legal.
 */
static void
find_good_move (const GibbonPosition *position, gint _board[28],
                GibbonPosition *post_position,
                GibbonPositionSide turn, guint dice[5])
{
        gint i = 0;
        guint *dice_pair;
        guint die;
        gint brought_in = 0;
        gint bring_in_die = 0;
        gint board[28];

        dice_pair = dice;

        /* Make a copy of the board.  */
        memcpy (board, _board, sizeof board);

        /* Try to come in from the bar in if possible.  */
        for (i = 0; board[25] && dice[i]; ++i) {
                if (come_in_from_bar (board, dice[i], turn)) {
                        ++brought_in;
                        bring_in_die = dice[i];
                }
        }

        if (board[25]) {
                /* Dancing.  */
#if (DEBUG_TEST_ENGINE)
                g_printerr ("\n");
#endif
                translate_board (post_position, board, turn);
                return;
        }

        /* Swap the dice, if we used the second one for coming in.  */
        if (1 == brought_in
            && bring_in_die == dice[1] && bring_in_die != dice[0]) {
                die = dice[0];
                dice[0] = dice[1];
                dice[1] = die;
        }

        /* And advance it to the first unused.  */
        dice_pair += brought_in;

        /* Now with the bar business being done, try to use the remaining
         * dice value.
         */
        while (*dice_pair)
                find_good_movement (board, dice_pair++, turn);

#if (DEBUG_TEST_ENGINE)
        g_printerr ("\n");
#endif

        translate_board (post_position, board, turn);
}

static gboolean
come_in_from_bar (gint board[28], gint die, GibbonPositionSide turn)
{
        if (board[25 - die] >= -1) {
#if (DEBUG_TEST_ENGINE)
                print_movement (board, 25, die, turn);
#endif
                --board[25];
                if (board[25 - die] == -1) {
                        ++board[0];
                        board[25 - die] = 0;
                }
                ++board[25 - die];
                return  TRUE;
        }

        return FALSE;
}

#if (DEBUG_TEST_ENGINE)
static void
print_movement (gint board[28], gint from, gint die,
                GibbonPositionSide turn)
{
        gint to;

        if (from == 25) {
                g_printerr (" bar/");
        } else if (turn == GIBBON_POSITION_SIDE_WHITE) {
                g_printerr (" %d/", from);
        } else {
                g_printerr (" %d/", 25 - from);
        }

        to = from - die;

        if (to <= 0) {
                g_printerr ("off");
        } else {
                if (turn == GIBBON_POSITION_SIDE_WHITE) {
                        g_printerr ("%d", to);
                } else {
                        g_printerr ("%d", 25 - to);
                }
                if (board[to] == -1)
                        g_printerr ("*");
        }
}
#endif

static void
find_good_movement (gint board[28], guint dice[2],
                    GibbonPositionSide turn)
{
        gint i;
        gint die = dice[0];
        gint next_die = dice[1];
        gboolean may_bear_off = FALSE;
        gint to;
        gint lost_points;

        /* Can we bear off?  */
        for (i = 24; i > 0; --i) {
                if (board[i] > 0) {
                        if (i > 6)
                                break;
                        may_bear_off = TRUE;
                        break;
                }
        }

        if (may_bear_off && board[die] > 0) {
#if (DEBUG_TEST_ENGINE)
                print_movement (board, die, die, turn);
#endif
                ++board[26];
                --board[die];
                return;
        }

        if (next_die) {
                /* Try to point on the opponent's head.  */
                for (i = 24; i > die && i > next_die; --i) {
                        if (board[i] <= 0)
                                continue;
                        if (board[i - die] != -1)
                                continue;

                        if  (die == next_die) {
                                if (board[i] == 1 || board[i] == 3)
                                        continue;
                        } else {
                                /* Only make the point with a blot or from
                                 * highly stacked points.
                                 */
                                if (board[i - die + next_die] <= 0
                                    || board[i - die + next_die] == 2)
                                        continue;
                        }
#if (DEBUG_TEST_ENGINE)
                        g_printerr (" >>> Point on opp's head <<< ");
                        print_movement (board, i, die, turn);
#endif
                        --board[i];
                        board[i - die] = 1;
                        ++board[0];
                        return;
                }
        }

        /* Try to make a new point.  */
        for (i = 24; i > die; --i) {
                if (board[i] <= 0 || board[i] == 2)
                        continue;
                if (board[i - die] != 1)
                        continue;
#if (DEBUG_TEST_ENGINE)
                        g_printerr (" >>> Make a new point <<< ");
                        print_movement (board, i, die, turn);
#endif
                        --board[i];
                        ++board[i - die];
                        return;
        }

        /* Again, try to make a point on the opponent's head, but this
         * time we accept leaving a blot behind.  We don't do that with
         * doubles though.
         */
        if (next_die && die != next_die) {
                for (i = 24; i > die && i > next_die; --i) {
                        if (board[i] <= 0)
                                continue;
                        if (board[i - die] != -1)
                                continue;
                        if (board[i - die + next_die] < 1)
                                continue;

                        lost_points = 0;

                        if (board[i - die + next_die] == 2)
                                ++lost_points;
                        if (board[i] == 2)
                                ++lost_points;

                        if (lost_points > 1)
                                continue;

#if (DEBUG_TEST_ENGINE)
                        g_printerr (" >>> Point risky on opp's head <<< ");
                        print_movement (board, i, die, turn);
#endif
                        --board[i];
                        board[i - die] = 1;
                        ++board[0];
                        return;
                }
        }

        /* Try to hit loosely.  */
        for (i = 24; i > die; --i) {
                if (board[i] <= 0)
                        continue;
                if (board[i - die] != -1)
                        continue;

#if (DEBUG_TEST_ENGINE)
                g_printerr (" >>> Hit loosely <<< ");
                print_movement (board, i, die, turn);
#endif
               --board[i];
               board[i - die] = 1;
               ++board[0];
               return;
        }

        /* As a last resort, try to move the most backward checker.  */
        for (i = 24; i > 0; --i) {
                /* Too far?  */
                if (!may_bear_off && i <= die)
                        break;

                /* Do we have a checker?  */
                if (board[i] <= 0)
                        continue;
                to = i - die;
                if (to < 0)
                        to = 0;

                /* Blocked? */
                if (to > 0 && board[to] < -1)
                        continue;

#if (DEBUG_TEST_ENGINE)
                g_printerr (" >>> Last resort <<< ");
                print_movement (board, i, die, turn);
#endif
                --board[i];

                if (to > 0 && board[to] == -1) {
                        /* Hit! */
                        ++board[0];
                        board[to] = 0;
                }


                if (to <= 0)
                        ++board[26];

                ++board[to];
                break;
        }
}
