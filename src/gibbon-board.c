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
 * SECTION:gibbon-board
 * @short_description: Visual representation of a backgammon position.
 *
 * Since: 0.1.0
 *
 * The #GibbonBoard interface defines the methods and properties that the
 * visual representation of a certain state in a backgammon match must
 * implement.
 */

#include <stdlib.h>

#include <glib.h>
#include <glib/gi18n.h>

#include "gibbon-board.h"
#include "gibbon-position.h"

typedef GibbonBoardIface GibbonBoardInterface;
G_DEFINE_INTERFACE (GibbonBoard, gibbon_board, G_TYPE_OBJECT)

static void gibbon_board_quick_bear_off_with_waste (GibbonBoard *self);
static void gibbon_board_quick_bear_off_with_waste2 (GibbonBoard *self);

static void
gibbon_board_default_init (GibbonBoardInterface *iface)
{
}

/**
 * gibbon_board_set_position:
 * @self: the #GibbonBoard
 * @position: the #GibbonPosition to display
 *
 * Apply a #GibbonPosition to a #GibbonBoard.  The #GibbonPosition will now
 * be owned by the #GibbonBoard.  Do not gibbon_position_free() it.
 */
void
gibbon_board_set_position (GibbonBoard *self, const GibbonPosition *position)
{
        GibbonBoardIface *iface;

        g_return_if_fail (self != NULL);
        g_return_if_fail (GIBBON_IS_BOARD (self));
        g_return_if_fail (position != NULL);

        iface = GIBBON_BOARD_GET_IFACE (self);

        g_return_if_fail (iface->set_position);

        (*iface->set_position) (self, position);
}

/**
 * gibbon_board_get_position:
 * @self: the #GibbonBoard
 *
 * Get the #GibbonPosition the board is currently displaying.
 *
 * Returns: the #GibbonPosition or %NULL for failure.
 */
const GibbonPosition *
gibbon_board_get_position (const GibbonBoard *self)
{
        GibbonBoardIface *iface;

        g_return_val_if_fail (self != NULL, NULL);
        g_return_val_if_fail (GIBBON_IS_BOARD (self), NULL);

        iface = GIBBON_BOARD_GET_IFACE (self);

        g_return_val_if_fail (iface->get_position, NULL);

        return (*iface->get_position) (self);
}

/**
 * gibbon_board_animate_move:
 * @self: The #GibbonBoard
 * @move: The #GibbonMove to animate
 * @side: The #GibbonPositionSide that is on move
 * @target_position: The position to display at the end.
 *
 * Whereas @move gets copied, @target_position is hijacked!
 */
void
gibbon_board_animate_move (GibbonBoard *self, const GibbonMove *move,
                           GibbonPositionSide side,
                           GibbonPosition *target_position)
{
        GibbonBoardIface *iface;

        g_return_if_fail (GIBBON_IS_BOARD (self));
        g_return_if_fail (move != NULL);

        iface = GIBBON_BOARD_GET_IFACE (self);

        g_return_if_fail (iface->animate_move);

        return (*iface->animate_move) (self, move, side, target_position);
}

/**
 * gibbon_board_fade_out_dice:
 * @self: The #GibbonBoard
 *
 * Hide the dice after a short time.
 */
void
gibbon_board_fade_out_dice (GibbonBoard *self)
{
        GibbonBoardIface *iface;

        g_return_if_fail (GIBBON_IS_BOARD (self));

        iface = GIBBON_BOARD_GET_IFACE (self);

        g_return_if_fail (iface->fade_out_dice);

        return (*iface->fade_out_dice) (self);
}

/**
 * gibbon_board_redraw:
 * @self: The #GibbonBoard
 *
 * Redraw the board after it has been modified!
 */
void
gibbon_board_redraw (const GibbonBoard *self)
{
        GibbonBoardIface *iface;

        g_return_if_fail (GIBBON_IS_BOARD (self));

        iface = GIBBON_BOARD_GET_IFACE (self);

        g_return_if_fail (iface->redraw);

        return (*iface->redraw) (self);
}

void
gibbon_board_process_point_click (GibbonBoard *self, gint point,
                                  gint real_button)
{
        const GibbonPosition *pos;
        GibbonPosition *new_pos = NULL;
        gint pips;
        gint i;
        gint button;

        g_return_if_fail (GIBBON_IS_BOARD (self));
        g_return_if_fail (point >= 1);
        g_return_if_fail (point <= 24);
        g_return_if_fail (real_button == 1 || real_button == 3);

        /*
         * This is an impartial legality check.  We only handle the trivial
         * cases here.
         */
        pos = gibbon_board_get_position (self);

        /* Any dice left? */
        if (!pos->unused_dice[0])
                return;

        /* Only one die left? */
        if (!pos->unused_dice[1]) {
                button = 1;
        } else if (pos->dice_swapped) {
                if (real_button == 1)
                        button = 3;
                else
                        button = 1;
        } else {
                button = real_button;
        }

        if (pos->turn != GIBBON_POSITION_SIDE_WHITE)
                return;

        /* Any checkers on the bar?  */
        if (pos->bar[0])
                return;

        /* Are there checkers to move?  */
        if (pos->points[point - 1] <= 0)
                return;

        new_pos = gibbon_position_copy (pos);

        pips = button == 1 ? new_pos->unused_dice[0] : new_pos->unused_dice[1];

        if (point - pips < 1) {
                /* This is a bear-off.  */
                for (i = 6; i < 24; ++i)
                        if (new_pos->points[i] > 0)
                                goto bail_out_point_click;
                if (point != pips)
                        for (i = point; i < 6; ++i)
                                if (new_pos->points[i] > 0)
                                        goto bail_out_point_click;
                --new_pos->points[point - 1];
        } else {
                /* Occupied? */
                if (new_pos->points[point - 1 - pips] < -1)
                        goto bail_out_point_click;
                if (new_pos->points[point - 1 - pips] == -1) {
                        new_pos->points[point - 1 - pips] = 0;
                        ++new_pos->bar[1];
                }
                --new_pos->points[point - 1];
                ++new_pos->points[point - 1 - pips];
        }

        /* Delete used die and move the rest.  */
        if (button == 1)
                new_pos->unused_dice[0] = new_pos->unused_dice[1];
        new_pos->unused_dice[1] = new_pos->unused_dice[2];
        new_pos->unused_dice[2] = new_pos->unused_dice[3];
        new_pos->unused_dice[3] = 0;

        gibbon_board_set_position (self, new_pos);
        gibbon_position_free (new_pos);
        gibbon_board_redraw (self);

        return;

bail_out_point_click:
        /*
         * If it was not a legal move with the left die it could still be
         * one for the right die.
         */
        gibbon_position_free (new_pos);
        if (real_button == 1 && pos->unused_dice[1]
            && pos->unused_dice[0] != pos->unused_dice[1])
                gibbon_board_process_point_click (self, point, 3);
}

void
gibbon_board_process_bar_click (GibbonBoard *self, gint button)
{
        const GibbonPosition *pos;
        GibbonPosition *new_pos = NULL;
        gint pips;

        g_return_if_fail (GIBBON_IS_BOARD (self));

        /*
         * This is an impartial legality check.  We only handle the trivial
         * cases here.
         */
        pos = gibbon_board_get_position (self);

        /* Any dice left? */
        if (!pos->unused_dice[0])
                return;

        /* Only one die left? */
        if (!pos->unused_dice[1])
                button = 1;

        if (pos->turn != GIBBON_POSITION_SIDE_WHITE)
                return;

        /* Any checkers on the bar?  */
        if (!pos->bar[0])
                return;

        new_pos = gibbon_position_copy (pos);

        pips = button == 1 ? new_pos->unused_dice[0] : new_pos->unused_dice[1];

        /* Occupied? */
        if (new_pos->points[24 - pips] < -1)
                goto bail_out_bar_click;
        if (new_pos->points[24 - pips] == -1) {
                new_pos->points[24 - pips] = 0;
                ++new_pos->bar[1];
        }
        --new_pos->bar[0];
        ++new_pos->points[24 - pips];

        /* Delete used die and move the rest.  */
        if (button == 1)
                new_pos->unused_dice[0] = new_pos->unused_dice[1];
        new_pos->unused_dice[1] = new_pos->unused_dice[2];
        new_pos->unused_dice[2] = new_pos->unused_dice[3];
        new_pos->unused_dice[3] = 0;

        gibbon_board_set_position (self, new_pos);
        gibbon_position_free (new_pos);
        gibbon_board_redraw (self);

        return;

bail_out_bar_click:
        /*
         * If it was not a legal move with the left die it could still be
         * one for the right die.
         */
        gibbon_position_free (new_pos);
        if (button == 1 && pos->unused_dice[1]
            && pos->unused_dice[0] != pos->unused_dice[1])
                gibbon_board_process_bar_click (self, 3);
}

void
gibbon_board_process_quick_bear_off (GibbonBoard *self)
{
        const GibbonPosition *pos;
        guint die;
        gint i;
        GibbonPosition *new_pos = NULL;

        g_return_if_fail (GIBBON_IS_BOARD (self));

        pos = gibbon_board_get_position (self);

        if (pos->turn != GIBBON_POSITION_SIDE_WHITE)
                return;

        for (i = 6; i < 24; ++i)
                if (pos->points[i] > 0)
                        return;
        if (pos->bar[0])
                return;

        if (pos->unused_dice[3]) {
                die = pos->unused_dice[3];
                if (pos->points[die - 1] < 4) {
                        gibbon_board_quick_bear_off_with_waste (self);
                        return;
                }
                new_pos = gibbon_position_copy (pos);
                new_pos->points[die - 1] -= 4;
        } else if (pos->unused_dice[2]) {
                die = pos->unused_dice[2];
                if (pos->points[die - 1] < 3) {
                        gibbon_board_quick_bear_off_with_waste (self);
                        return;
                }
                new_pos = gibbon_position_copy (pos);
                new_pos->points[die - 1] -= 3;
        } else if (pos->unused_dice[1]) {
                die = pos->unused_dice[1];
                if (pos->points[die - 1] < 1) {
                        gibbon_board_quick_bear_off_with_waste (self);
                        return;
                }
                die = pos->unused_dice[0];
                if (pos->points[die - 1] < 1) {
                        gibbon_board_quick_bear_off_with_waste (self);
                        return;
                }
                new_pos = gibbon_position_copy (pos);
                --new_pos->points[die - 1];
                die = pos->unused_dice[1];
                --new_pos->points[die - 1];
        } else if (pos->unused_dice[0]) {
                die = pos->unused_dice[0];
                if (pos->points[die - 1] < 3) {
                        gibbon_board_quick_bear_off_with_waste (self);
                        return;
                }
                new_pos = gibbon_position_copy (pos);
                --new_pos->points[die - 1];
        } else {
                return;
        }

        new_pos->unused_dice[0] = new_pos->unused_dice[1]
            = new_pos->unused_dice[2] = new_pos->unused_dice[3] = 0;

        gibbon_board_set_position (self, new_pos);
        gibbon_position_free (new_pos);
        gibbon_board_redraw (self);
}

static void
gibbon_board_quick_bear_off_with_waste (GibbonBoard *self)
{
        gint i;
        const GibbonPosition *pos;
        guint die;
        guint num_checkers;
        GibbonPosition *new_pos;

        /*
         * We have already checked that the player is allowed to bear-off,
         * but there is no bear-off without waste.
         */

        pos = gibbon_board_get_position (self);

        if (pos->unused_dice[3]) {
                die = pos->unused_dice[3];
                num_checkers = 4;
        } else if (pos->unused_dice[2]) {
                die = pos->unused_dice[2];
                num_checkers = 3;
        } else if (pos->unused_dice[1]
                   && pos->unused_dice[0] == pos->unused_dice[1]) {
                die = pos->unused_dice[1];
                num_checkers = 2;
        } else if (pos->unused_dice[0] && !pos->unused_dice[1]) {
                die = pos->unused_dice[0];
                num_checkers = 1;
        } else {
                /*
                 * The difficult case.
                 */
                gibbon_board_quick_bear_off_with_waste2 (self);
                return;
        }

        for (i = die; i < 6; ++i) {
                if (pos->points[i] > 0)
                        return;
        }

        new_pos = gibbon_position_copy (pos);

        new_pos->unused_dice[0] = new_pos->unused_dice[1]
            = new_pos->unused_dice[2] = new_pos->unused_dice[3] = 0;

        for (i = die - 2; i > 0; --i) {
                while (num_checkers && new_pos->points[i] > 0) {
                        --num_checkers;
                        --new_pos->points[i];
                }
        }

        gibbon_board_set_position (self, new_pos);
        gibbon_position_free (new_pos);
        gibbon_board_redraw (self);
}

static void
gibbon_board_quick_bear_off_with_waste2 (GibbonBoard *self)
{
        const GibbonPosition *pos = gibbon_board_get_position (self);
        guint die;
        GibbonPosition *new_pos;
        gint i;

        new_pos = gibbon_position_copy (pos);

        die = new_pos->unused_dice[1];
        if (new_pos->points[die - 1] > 0) {
                new_pos->unused_dice[1] = 0;
                --new_pos->points[die - 1];
        }

        die = new_pos->unused_dice[0];
        if (new_pos->points[die - 1] > 0) {
                new_pos->unused_dice[0] = new_pos->unused_dice[1];
                --new_pos->points[die - 1];
        }

        if (new_pos->unused_dice[1]
            && new_pos->unused_dice[1] > new_pos->unused_dice[0]) {
                new_pos->unused_dice[2] = new_pos->unused_dice[0];
                new_pos->unused_dice[0] = new_pos->unused_dice[1];
                new_pos->unused_dice[1] = new_pos->unused_dice[2];
                new_pos->unused_dice[2] = 0;
        }

        if (new_pos->unused_dice[1]) {
                die = new_pos->unused_dice[1];
                for (i = die - 2; i >= 0; --i) {
                        while (new_pos->points[i] > 0) {
                                --new_pos->points[i];
                                new_pos->unused_dice[1] = 0;
                                i = 0;
                                break;
                        }
                }
        }

        die = new_pos->unused_dice[0];
        for (i = die - 2; i >= 0; --i) {
                while (new_pos->points[i] > 0) {
                        --new_pos->points[i];
                        new_pos->unused_dice[0] = 0;
                        i = 0;
                        break;
                }
        }

        if ((!new_pos->unused_dice[0] && !new_pos->unused_dice[1])
             || 15 == gibbon_position_get_borne_off (
                            new_pos, GIBBON_POSITION_SIDE_WHITE)) {
                gibbon_board_set_position (self, new_pos);
                gibbon_board_redraw (self);
        }

        gibbon_position_free (new_pos);
}
