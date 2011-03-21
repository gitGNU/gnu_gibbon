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

#ifndef _GIBBON_BOARD_H
# define _GIBBON_BOARD_H

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <glib.h>
#include <glib-object.h>

#define GIBBON_TYPE_BOARD \
        (gibbon_board_get_type ())
#define GIBBON_BOARD(obj) \
        (G_TYPE_CHECK_INSTANCE_CAST ((obj), GIBBON_TYPE_BOARD, \
                GibbonBoard))
#define GIBBON_IS_BOARD(obj) \
        (G_TYPE_CHECK_INSTANCE_TYPE ((obj), \
                GIBBON_TYPE_BOARD))
#define GIBBON_BOARD_GET_IFACE(obj) \
        (G_TYPE_INSTANCE_GET_INTERFACE ((obj), \
                GIBBON_TYPE_BOARD, GibbonBoardIface))

/**
 * GibbonBoardIface:
 *
 * Visual representation of a backgammon position.  It is the view for the
 * model #GibbonPosition.
 */
typedef struct _GibbonBoardIface GibbonBoardIface;
struct _GibbonBoardIface
{
        GTypeInterface g_iface;
};

GType gibbon_board_get_type (void) G_GNUC_CONST;

#endif
