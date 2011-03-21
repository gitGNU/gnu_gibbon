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

typedef GibbonBoardIface GibbonBoardInterface;
G_DEFINE_INTERFACE (GibbonBoard, gibbon_board, G_TYPE_OBJECT)

static void
gibbon_board_default_init (GibbonBoardInterface *iface)
{
}
