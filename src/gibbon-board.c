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
