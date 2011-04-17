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

#define DEBUG_MOVE_GEN 1

static GibbonPosition *generate_position (void);
static void test_position (GibbonPosition *position, gint board[28]);
static void test_position_with_dice (GibbonPosition *position, gint board[28]);
static void test_position_with_double (GibbonPosition *position,
                                       gint board[28]);
static void test_position_with_non_double (GibbonPosition *position,
                                           gint board[28]);
static void compare_results (GibbonPosition *position,
                             GibbonPosition *post_position,
                             GibbonMove *move,
                             gint success, gint moves[8]);
static void dump_position (const GibbonPosition *position);
static void dump_move (const GibbonMove *move);
static void translate_position (gint board[28], const GibbonPosition *position);
static gboolean apply_move (GibbonPosition *position, const GibbonMove *move);
static gint test_position_with_move (GibbonPosition *position, gint board[28],
                                     const GibbonMove *move);

int
main (int argc, char *argv[])
{
        guint64 num_positions = 1000;
        guint64 i;
        guint64 random_seed = time (NULL);
        gboolean verbose = FALSE;
        GibbonPosition *position;
        int board[28];

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

        for (i = 0; i < num_positions; ++i) {
                position = generate_position ();
#if (DEBUG_MOVE_GEN)
                g_printerr ("================ New Position ===============\n");
                dump_position (position);
#endif
                translate_position (board, position);
                test_position (position, board);
                gibbon_position_free (position);
        }

        return 0;
}

static GibbonPosition *
generate_position (void)
{
        int i;
        gint point;
        GibbonPosition *position = gibbon_position_new ();
        guint white_home = 0;
        guint black_home = 0;

        memset (position->points, 0, sizeof position->points);

        for (i = 0; i < 15; ++i) {
                point = -1;
                while (1) {
                        point = random () % 26;
                        if (point == 0 || point == 25) {
                                break;
                        }
                        if (position->points[point - 1] >= 0)
                                break;
                }
                switch (point) {
                        case 0:
                                ++position->bar[0];
                                break;
                        case 25:
                                ++white_home;
                                break;
                        default:
                                ++position->points[point];
                                break;
                }
                while (1) {
                        point = random () % 26;
                        if (point == 0 || point == 25) {
                                break;
                        }
                        if (position->points[point - 1] <= 0)
                                break;
                }
                switch (point) {
                        case 0:
                                ++position->bar[0];
                                break;
                        case 25:
                                ++black_home;
                                break;
                        default:
                                --position->points[point];
                                break;
                }
        }

        if (white_home == 15 || black_home == 15) {
                gibbon_position_free (position);
                position = generate_position ();
        }

        return position;
}

static void
test_position (GibbonPosition *position, gint board[28])
{
        int i, j;

        for (i = 1; i <= 6; ++i) {
                position->dice[0] = i;
                for (j = i; j <= 6; ++j) {
                        position->dice[1] = j;
                        test_position_with_dice (position, board);
                }
        }
}

static void
test_position_with_dice (GibbonPosition *position, int board[28])
{
        GibbonMove *move;

        /* First check if no move at all is legal in this position.  */
        move = gibbon_position_alloc_move (0);
        test_position_with_move (position, board, move);

        g_free (move);

        if (position->dice[0] == position->dice[1]) {
                test_position_with_double (position, board);
        } else {
                test_position_with_non_double (position, board);
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
translate_position (gint board[28], const GibbonPosition *position)
{
        /* Translate position into structure expected by Gary's move
         * legality checker.
         */
        board[0] = -position->bar[1];
        memcpy (board + 1, position->points, sizeof position->points);
        board[25] = position->bar[0];
        board[26] = gibbon_position_get_borne_off (position,
                        GIBBON_POSITION_SIDE_WHITE);
        board[27] = gibbon_position_get_borne_off (position,
                        GIBBON_POSITION_SIDE_BLACK);
}

static void
test_position_with_double (GibbonPosition *position, gint board[28])
{
        /* Try to use just one die.  */
        gint i, j;
        GibbonMove *move = gibbon_position_alloc_move (4);
        gint found = 0;
        guint die = position->dice[0];

        /* Try to move a single checker once.  */
        g_printerr ("Move a single checker once.\n");
        move->number = 1;
        move->movements[0].num = 1;
        for (i = 25; i > 0; --i) {
                if (board[i] <= 0)
                        continue;
                move->movements[0].from = i;
                move->movements[0].to = i - die;
                found += test_position_with_move (position, board, move);
        }

        /* If there was no legal move after one round we can take an early
         * exit here.
         */
        if (!found) {
                g_free (move);
                return;
        }

        /* Now try to use two dice values.  */
        found = 0;

        /* Try to move a single checker twice.  */
        g_printerr ("Move a single checker twice.\n");
        move->number = 2;
        move->movements[0].num = 1;
        move->movements[1].num = 1;
        for (i = 25; i > 0; --i) {
                if (board[i] <= 0)
                        continue;
                move->movements[0].from = i;
                move->movements[0].to = i - die;
                move->movements[1].from = i - die;
                move->movements[1].to = i - 2 * die;
                found += test_position_with_move (position, board, move);
        }

        /* Try to move a pair of checkers once.  */
        g_printerr ("Move a pair of checkers once.\n");
        move->number = 1;
        move->movements[0].num = 2;
        for (i = 25; i > 0; --i) {
                if (board[i] <= 1)
                        continue;
                move->movements[0].from = i;
                move->movements[0].to = i - die;
                found += test_position_with_move (position, board, move);
        }

        /* Try to move two checkers once.  */
        g_printerr ("Move two checkers once.\n");
        move->number = 2;
        move->movements[0].num = 1;
        move->movements[0].num = 1;
        for (i = 25; i > 0; --i) {
                if (board[i] <= 0)
                        continue;
                move->movements[0].from = i;
                move->movements[0].to = i - die;
                for (j = i - 1; j > 0; --j) {
                        if (board[j] <= 0)
                                continue;
                        move->movements[1].from = j;
                        move->movements[1].to = j - die;
                        if (move->movements[0].to == move->movements[1].from)
                                continue;
                        found += test_position_with_move (position, board, move);
                }
        }

        if (!found) {
                g_free (move);
                return;
        }

        g_free (move);

        exit (1);
}

static void
test_position_with_non_double (GibbonPosition *position,
                               gint board[28])
{
        GibbonMove *move = gibbon_position_alloc_move (2);
        GibbonMovement *movement;
        gint i, j;

        /* Test single checker movements.  */
        g_printerr ("One checker once.\n");
        move->number = 1;
        movement = move->movements;
        movement->num = 1;
        for (i = 25; i > 0; --i) {
                if (board[i] > 0) {
                        movement->from = i;
                        movement->to = i - position->dice[0];
                        test_position_with_move (position, board, move);
                        movement->to = i - position->dice[1];
                        test_position_with_move (position, board, move);
                }
        }

        /* Now try to move the same checker twice.  */
        g_printerr ("One checker twice.\n");
        move->number = 2;
        move->movements[0].num = 1;
        move->movements[1].num = 1;
        for (i = 25; i > 0; --i) {
                if (board[i] > 0) {
                        move->movements[0].from = i;
                        move->movements[0].to = i - position->dice[0];
                        if (move->movements[0].to > 0) {
                                move->movements[1].from = move->movements[0].to;
                                move->movements[1].to = move->movements[1].from
                                                - position->dice[1];
                                test_position_with_move (position, board, move);
                        }
                        move->movements[0].to = i - position->dice[1];
                        if (move->movements[0].to > 0) {
                                move->movements[1].from = move->movements[0].to;
                                move->movements[1].to = move->movements[1].from
                                                - position->dice[0];
                                test_position_with_move (position, board, move);
                        }
                }
        }

        g_printerr ("Two checkers from one point.\n");
        /* Now try to move two checkers from one point.  */
        for (i = 25; i > 0; --i) {
                if (board[i] < 2)
                        continue;
                move->movements[0].from = i;
                move->movements[0].to = i - position->dice[0];
                move->movements[1].from = i;
                move->movements[1].to = i - position->dice[1];
                test_position_with_move (position, board, move);
        }

        g_printerr ("Two checkers from different points.\n");
        /* And finally two, in both orders.  */
        for (i = 25; i > 0; --i) {
                if (board[i] < 1)
                        continue;
                move->movements[0].from = i;
                for (j = i - 1; j > 0; --j) {
                        if (board[j] < 1)
                                continue;
                        move->movements[1].from = j;
                        move->movements[0].to = i - position->dice[0];
                        move->movements[1].to = j - position->dice[1];
                        /* If  the starting point of the second submove is the
                         * landing point of the first, we have seen that move
                         * already.
                         */
                        if (move->movements[0].to != move->movements[1].from)
                                test_position_with_move (position, board, move);
                        move->movements[0].to = i - position->dice[1];
                        move->movements[1].to = j - position->dice[0];
                        if (move->movements[0].to != move->movements[1].from)
                                test_position_with_move (position, board, move);
                }
        }

        g_free (move);
        g_printerr ("Next roll/position.\n");
}

/* Only works for white.  And only checks that the starting checker is
 * inside the board, and that the landing point is not occupied.
 */
static gboolean
apply_move (GibbonPosition *pos, const GibbonMove *move)
{
        gsize i;
        gint from, to;
        const GibbonMovement *movement;

        for (i = 0; i < move->number; ++i) {
                movement = move->movements + i;

                from = movement->from - 1;
                to = movement->to - 1;
                if (to < 0)
                        to = 0;
                if (to > 0 && pos->points[to] < -1)
                        return FALSE;

                if (from == 25)
                        pos->bar[0] -= movement->num;
                else
                        pos->points[from] -= movement->num;

                if (pos->points[to] == -1)
                        ++pos->bar[1];

                pos->points[to] += movement->num;
        }

        return TRUE;
}

static gint
test_position_with_move (GibbonPosition *position, gint board[28],
                         const GibbonMove *move)
{
        int post_board[28];
        GibbonPosition *post_position;
        GibbonMove *gibbon_move;
        gint moves[8];
        int gary_legal;
        gint dice[2];

#if (DEBUG_MOVE_GEN)
        g_printerr ("Testing move: %d%d", position->dice[0], position->dice[1]);
        dump_move (move);
#endif

        post_position = gibbon_position_copy (position);
        if (!apply_move (post_position, move)) {
                gibbon_position_free (post_position);
                return 0;
        }

        memcpy (post_board, board, sizeof post_board);
        dice[0] = abs (position->dice[0]);
        dice[1] = abs (position->dice[1]);
        gary_legal = LegalMove (board, post_board, dice, moves);
        gibbon_move = gibbon_position_check_move (position, post_position,
                                                  GIBBON_POSITION_SIDE_WHITE);
        compare_results (position, post_position, gibbon_move,
                         gary_legal, moves);

        g_free (gibbon_move);
        gibbon_position_free (post_position);

        return 1;
}
