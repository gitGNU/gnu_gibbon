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

#ifndef _GIBBON_BOARD_H
# define _GIBBON_BOARD_H

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <glib.h>
#include <glib-object.h>

#include "gibbon-position.h"
#include "gibbon-move.h"

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

typedef struct _GibbonBoard GibbonBoard;

/**
 * GibbonBoardIface:
 * @g_iface: The parent interface.
 * @set_position: Transfer a #GibbonPosition to the #GibbonBoard.
 *
 * Visual representation of a backgammon position.  It is the view for the
 * model #GibbonPosition.
 */
typedef struct _GibbonBoardIface GibbonBoardIface;
struct _GibbonBoardIface
{
        GTypeInterface g_iface;

        void (*set_position) (GibbonBoard *self, const GibbonPosition *pos);
        const GibbonPosition *(*get_position) (const GibbonBoard *self);
        void (*animate_move) (GibbonBoard *self, const GibbonMove *move,
                              GibbonPositionSide side,
                              GibbonPosition *target_position);
        void (*fade_out_dice) (GibbonBoard *self);
        void (*redraw) (const GibbonBoard *self);
};

GType gibbon_board_get_type (void) G_GNUC_CONST;

void gibbon_board_set_position (GibbonBoard *board, const GibbonPosition
                                *position);
const GibbonPosition *gibbon_board_get_position (const GibbonBoard *board);
void gibbon_board_animate_move (GibbonBoard *self, const GibbonMove *move,
                                GibbonPositionSide side,
                                GibbonPosition *target_position);
void gibbon_board_fade_out_dice (GibbonBoard *self);
void gibbon_board_redraw (const GibbonBoard *self);
void gibbon_board_process_point_click (GibbonBoard *self, gint point,
                                       gint button);
void gibbon_board_process_bar_click (GibbonBoard *self, gint button);
void gibbon_board_process_quick_bear_off (GibbonBoard *self);

#endif
