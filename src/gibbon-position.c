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
 * SECTION:gibbon-position
 * @short_description: Boxed type for a position in backgammon.
 *
 * Since: 0.1.1
 *
 * This structure holds all information needed to display a
 * normalized backgammon position.
 *
 * If X is black, and O is white, the normalized starting position of a
 * game of backgammon looks like this:
 *
 * <programlisting>
 *  +13-14-15-16-17-18-------19-20-21-22-23-24-+ O: white
 *  | O           X    |   |  X              O |
 *  | O           X    |   |  X              O |
 *  | O           X    |   |  X                |
 *  | O                |   |  X                |
 *  | O                |   |  X                |
 * v|                  |BAR|                   |
 *  | X                |   |  O                |
 *  | X                |   |  O                |
 *  | X           O    |   |  O                |
 *  | X           O    |   |  O              X |
 *  | X           O    |   |  O              X |
 *  +12-11-10--9--8--7--------6--5--4--3--2--1-+ X: black
 * </programlisting>
 *
 * Translated into the checkers array of a #GibbonPosition the image
 * looks like this:
 *
 * <programlisting>
 *  +12-13-14-15-16-17-------18-19-20-21-22-23-+ negative: black or X
 *  |+5          -3    |   | -5             +2 |
 * v|                  |BAR|                   |
 *  |-5          +3    |   | +5             -2 |
 *  +11-10--9--8--7--6--------5--4--3--2--1--0-+ positive: white or O
 * </programlisting>
 */

#include <glib.h>
#include <glib/gi18n.h>

#include "gibbon-position.h"

G_DEFINE_BOXED_TYPE (GibbonPosition, gibbon_position,            \
                     gibbon_position_copy, gibbon_position_free)

GibbonPosition initial = {
                { NULL, NULL },
                0,
                { 0, 0 },
                { 0, 0, 0, 0, 0, 0 },
                { -2,  0,  0,  0,  0,  5,  0,  3,  0,  0,  0, -5,
                   5,  0,  0,  0, -3,  0, -5,  0,  0,  0,  0,  2 },
                { 0, 0, 0, 0, 0, 0 },
                { 0, 0 },
                { 0, 0 },
                1,
                { FALSE, FALSE }
};

/**
 * gibbon_position_new:
 *
 * Creates a new #GibbonPosition, set to the backgammon starting position.
 * Both player names are %NULL, the cube is at 0, and both dice are set
 * to 0.  The @may_double flag is %FALSE.
 *
 * Returns: The newly created #GibbonPosition or %NULL in case of failure.
 */
GibbonPosition *
gibbon_position_new (void)
{
        GibbonPosition *self = g_malloc (sizeof initial);

        *self = initial;

        return self;
}

/**
 * gibbon_position_free:
 *
 * Free all resources associated with the #GibbonPosition.  Note that this
 * function calls g_free() with the player names if not %NULL.
 */
void
gibbon_position_free (GibbonPosition *self)
{
        if (self) {
                if (self->players[0])
                        g_free (self->players[0]);
                if (self->players[1])
                        g_free (self->players[1]);
                g_free (self);
        }
}

/**
 * gibbon_position_copy:
 * @self: the original #GibbonPosition.
 *
 * Creates an exact copy of @self.  If player names were set, they are
 * identical in the copy but will use new buffers that can be freed with
 * g_free().
 *
 * Returns: The copied #GibbonPosition or %NULL if @self was %NULL;
 */
GibbonPosition *
gibbon_position_copy (const GibbonPosition *self)
{
        GibbonPosition *copy;

        g_return_val_if_fail (self != NULL, NULL);

        copy = g_malloc (sizeof *self);
        *copy = *self;
        if (copy->players[0])
                copy->players[0] = g_strdup (copy->players[0]);
        if (copy->players[1])
                copy->players[1] = g_strdup (copy->players[1]);

        return copy;
}
