/*
 * This file is part of Gibbon, a graphical frontend to the First Internet 
 * Backgammon Server FIBS.
 * Copyright (C) 2009-2011 Guido Flohr, http://guido-flohr.net/.
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

static void compare_results (GibbonPosition *position,
                             GibbonPosition *post_position,
                             GibbonMove *move,
                             gint success, gint moves[8]);
static void dump_position (const GibbonPosition *position);
static void dump_move (const GibbonMove *move);
static void translate_position (gint board[28], const GibbonPosition *position,
                                GibbonPositionSide turn);
static guint test_game (guint64 max_positions);
static guint test_roll (GibbonPosition *position, guint64 max_positions);
static gboolean game_over (const GibbonPosition *position);
static void move_checker (GibbonPosition *position, guint die,
                          GibbonPositionSide side);

int
main (int argc, char *argv[])
{
        guint64 num_positions = 1000;
        guint64 i;
        guint64 random_seed = time (NULL);
        gboolean verbose = FALSE;

        g_type_init ();

        if (argc > 1) {
                errno = 0;
                num_positions = g_ascii_strtoull (argv[1], NULL, 10);
                if (errno) {
                        g_printerr ("Invalid number of positions `%s': %s!\n",
                                    argv[1], strerror (errno));
                        return -1;
                }
                g_print ("Testing %llu positions.\n", num_positions);
                verbose = TRUE;
        }

        if (argc > 2) {
                random_seed = g_ascii_strtoull (argv[2], NULL, 10);
                if (errno) {
                        g_printerr ("Invalid random seed `%s': %s!\n",
                                    argv[2], strerror (errno));
                        return -1;
                }
                g_print ("Using %llu as random seed.\n", random_seed);
        }
        srandom (random_seed);

        for (i = 0; i < num_positions; /* empty */) {
                i += test_game(num_positions - i);
        }

        return 0;
}

static guint
test_game (guint64 max_positions)
{
        GibbonPosition *position = gibbon_position_new ();
        GibbonPositionSide side = random () % 2
                        ? GIBBON_POSITION_SIDE_WHITE
                                        : GIBBON_POSITION_SIDE_BLACK;
        guint num_positions = 0;

        while (num_positions < max_positions) {
                if (side == GIBBON_POSITION_SIDE_WHITE) {
                        position->dice[0] = 1 + random () % 6;
                        position->dice[1] = 1 + random () % 6;
                } else {
                        position->dice[0] = -6 + random () % 6;
                        position->dice[1] = -6 + random () % 6;
                }

                if (game_over (position))
                        break;

                num_positions += test_roll (position,
                                            max_positions - num_positions);

                side = -side;
        }

        gibbon_position_free (position);

        return num_positions;
}

static guint
test_roll (GibbonPosition *position, guint64 max_positions)
{
        gboolean is_double = position->dice[0] == position->dice[1];
        guint max_movements = is_double ? 4 : 2;
        guint num_movements;
        GibbonPosition *post_position;
        guint num_positions = 0;
        guint i;
        GibbonPositionSide turn;
        gint dice[4];
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

        for (i = 0; i < max_movements; ++i)
                dice[i] = abs (position->dice[i % 2]);

        free_checkers = 15 - gibbon_position_get_borne_off (position, turn);
        if (free_checkers < max_movements)
                max_movements = free_checkers;

        translate_position (board, position, turn);

        dump_position (position);

        while (num_positions++ < max_positions) {
                num_movements = random () % (max_movements + 1);
// g_printerr ("Number of movements: %u\n", num_movements);
                post_position = gibbon_position_copy (position);

                for (i = 0; i < num_movements; ++i) {
                        move_checker (post_position, dice[i], turn);
                }
                translate_position (post_board, post_position, turn);

                move = gibbon_position_check_move (position, post_position,
                                                   turn);
                legal = LegalMove (board, post_board, dice, moves);
                compare_results (position, post_position, move,
                                 legal, moves);
                g_free (move);
                if (legal) {
                        if (turn == GIBBON_POSITION_SIDE_WHITE)
                                g_printerr ("W: ");
                        else
                                g_printerr ("B: ");
                        g_printerr ("%u%u", dice[0], dice[1]);
                        for (i = 0; moves[i] && i < 8; i += 2) {
                                g_printerr (" %u/%u", moves[i], moves[i + 1]);
                        }
                        g_printerr ("\n");

                        memcpy (position->points, post_position->points,
                                sizeof position->points);
                        gibbon_position_free (post_position);
                        break;
                }
                gibbon_position_free (post_position);
        }

        return num_positions;
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
move_checker (GibbonPosition *position, guint die, GibbonPositionSide turn)
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
                                ++position->points[24 - die];
// g_printerr (" w: bar/%d\n", 24 - die);
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
// g_printerr (" w: %d/%d\n", from, to);
                                if (to < 0)
                                        return;
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
                                --position->points[die - 1];
// g_printerr (" b: bar/%d", die - 1);
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
// g_printerr (" b: %d/%d\n", from, to);
                                if (to > 23)
                                        return;
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
        g_printerr (" v| dice: %+d : %+d     ",
                    pos->dice[0], pos->dice[1]);
        g_printerr ("|BAR|                   | ");
        g_printerr (" Cube: %d\n", pos->cube);
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
                if (move->movements[i].num != 1)
                        g_printerr ("(%u)", move->movements[i].num);
        }
        g_printerr ("\n");
}

static void
compare_results (GibbonPosition *position,
                GibbonPosition *post_position,
                GibbonMove *move,
                gint success, gint moves[8])
{
        gboolean match = TRUE;
        gint i;

return;
        if (move->status == GIBBON_MOVE_LEGAL && !success)
                match = FALSE;
        else if (move->status != GIBBON_MOVE_LEGAL && success)
                match = FALSE;

        if (match)
                return;

        g_printerr ("Legality checks differ:\n");
        g_printerr ("Gary Wong: %s, Gibbon: %s\n",
                    success ? "legal" : "illegal",
                    move->status == GIBBON_MOVE_LEGAL ? "legal" : "illegal");

        g_printerr ("Starting position:\n");
        dump_position (position);
        g_printerr ("End position:\n");
        dump_position (post_position);

        if (success) {
                g_printerr ("Move according to Gary Wong:");
                for (i = 0; moves[i] && i < 8; i += 2) {
                        g_printerr (" %u/%u", moves[i], moves[i + 1]);
                }
                g_printerr ("\n");
        }
        if (move->status == GIBBON_MOVE_LEGAL) {
                g_printerr ("Move according to Gibbon:");
                dump_move (move);
        }

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
                board[0] = -position->bar[1];
                memcpy (board + 1, position->points, sizeof position->points);
                board[25] = position->bar[0];
                board[26] = gibbon_position_get_borne_off (position,
                                GIBBON_POSITION_SIDE_WHITE);
                board[27] = gibbon_position_get_borne_off (position,
                                GIBBON_POSITION_SIDE_BLACK);
        } else {
                board[0] = -position->bar[0];
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

static gboolean
game_over (const GibbonPosition *position)
{
        guint num_checkers =
                gibbon_position_get_borne_off (position,
                                               GIBBON_POSITION_SIDE_WHITE);

        if (num_checkers >= 15)
                return TRUE;

        num_checkers =
                gibbon_position_get_borne_off (position,
                                               GIBBON_POSITION_SIDE_BLACK);

        if (num_checkers >= 15)
                return TRUE;

        return FALSE;
}
