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

/**
 * SECTION:gibbon-board
 * @short_description: Visual representation of a backgammon position.
 *
 * Since: 0.1.1
 *
 * The #GibbonBoard interface defines the methods and properties that the
 * visual representation of a certain state in a backgammon match must
 * implement.
 */

#include <glib.h>
#include <glib/gi18n.h>

#include "gibbon-board.h"
#include "gibbon-position.h"

typedef GibbonBoardIface GibbonBoardInterface;
G_DEFINE_INTERFACE (GibbonBoard, gibbon_board, G_TYPE_OBJECT)

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
gibbon_board_set_position (GibbonBoard *self, GibbonPosition *position)
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
GibbonPosition *
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
                                  gint button)
{
        GibbonPosition *pos;
        GibbonPositionSide turn;
        guint pips;
        gint i;

        g_return_if_fail (GIBBON_IS_BOARD (self));
        g_return_if_fail (point >= 1);
        g_return_if_fail (point <= 24);

        /*
         * This is an inpartial legality check.  We only handle the trivial
         * cases here.
         */
        pos = gibbon_board_get_position (self);

        /* Any dice left? */
        if (!pos->unused_dice[0])
                return;

        /* Only one die left? */
        if (!pos->unused_dice[1])
                button = 1;

        turn = gibbon_position_on_move (pos);

        if (turn != GIBBON_POSITION_SIDE_WHITE)
                return;

        /* Any checkers on the bar?  */
        if (pos->bar[0])
                return;

        /* Are there checkers to move?  */
        if (pos->points[point - 1] <= 0)
                return;

        pips = button == 1 ? pos->unused_dice[0] : pos->unused_dice[1];

        if (point - pips < 1) {
                /* This is a bear-off.  */
                for (i = point; i < 24; ++i)
                        if (pos->points[i] > 0)
                                goto bail_out_point_click;
                --pos->points[point - 1];
        } else {
                /* Occupied? */
                if (pos->points[point - 1 - pips] < -1)
                        goto bail_out_point_click;
                if (pos->points[point - 1 - pips] == -1) {
                        pos->points[point - 1 - pips] = 0;
                        ++pos->bar[1];
                }
                --pos->points[point - 1];
                ++pos->points[point - 1 - pips];
        }

        /* Delete used die and move the rest.  */
        if (button == 1)
                pos->unused_dice[0] = pos->unused_dice[1];
        pos->unused_dice[1] = pos->unused_dice[2];
        pos->unused_dice[2] = pos->unused_dice[3];
        pos->unused_dice[3] = 0;

        gibbon_board_redraw (self);

        return;

bail_out_point_click:
        /*
         * If it was not a legal move with the left die it could still be
         * one for the right die.
         */
        if (button == 1 && pos->unused_dice[1]
            && pos->unused_dice[0] != pos->unused_dice[1])
                gibbon_board_process_point_click (self, point, 3);
}
