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
 * SECTION:gibbon-movement
 * @short_description: One single checker movement!
 *
 * Since: 0.1.1
 *
 * A GibbonMovement represents the movement of one single checker.
 */

#include <glib.h>
#include <glib/gi18n.h>

#include "gibbon-movement.h"

static GibbonMovement *gibbon_movement_copy (GibbonMovement *self);

G_DEFINE_BOXED_TYPE (GibbonMovement, gibbon_movement, 
                     gibbon_movement_copy, g_free)

static GibbonMovement *
gibbon_movement_copy (GibbonMovement *self)
{
        GibbonMovement *copy = g_malloc (sizeof *self);

        copy->from = self->from;
        copy->to = self->to;
        copy->die = self->die;

        return copy;
}

/**
 * gibbon_movement_new:
 * @from: The source point.
 * @to: The destionation point.
 * @die: The die used.
 *
 * Creates a new #GibbonMovement.
 *
 * Points are numbered from 1 to 24.  The bar is represented by 0, borne-off
 * checkers are numbered to point 25.
 *
 * Returns: The newly created #GibbonMovement or %NULL in case of failure.
 */
GibbonMovement *
gibbon_movement_new (gint from, gint to, gint die)
{
        GibbonMovement *self;

        self = g_malloc (sizeof *self);
        self->from = from;
        self->to = to;
        self->die = die;

        return self;
}
