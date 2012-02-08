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

#ifndef _GIBBON_MOVEMENT_H
# define _GIBBON_MOVEMENT_H

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <glib.h>
#include <glib-object.h>

#define GIBBON_TYPE_MOVEMENT (gibbon_movement_get_type ())

/**
 * GibbonMovement:
 * @from: The starting point for a move.  1 is the ace point for white, O,
 *        or the player with positive checker counts.  23 is the ace point
 *        for black, X, or the player with negative checker counts.  0 and
 *        24 represent home and the bar accordingly.
 * @to: The end point for a move, see @from for semantics.
 * @die: The die value used for the move.
 *
 * A boxed type representing a single backgammon checker movement.
 *
 * The numbers are always positive (greater than zero) but we have to use gint
 * instead of guint for internal reasons.
 */
typedef struct _GibbonMovement GibbonMovement;
struct _GibbonMovement
{
        gint from;
        gint to;
        gint die;
};

GType gibbon_movement_get_type (void) G_GNUC_CONST;

GibbonMovement *gibbon_movement_new (gint from, gint to, gint die);

#endif
