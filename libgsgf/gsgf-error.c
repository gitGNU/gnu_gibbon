/*
 * This file is part of Gibbon, a graphical frontend to the First Internet 
 * Backgammon Server FIBS.
 * Copyright (C) 2009-2010 Guido Flohr, http://guido-flohr.net/.
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

/**
 * SECTION:gsgf-error
 * @short_description: Error helper functions
 * @include: libgsgf/gsgf.h
 *
 * Currently contains only the #GQuark for GSGF errors.
 **/

#include <libgsgf/gsgf-error.h>

/**
 * gsgf_error_quark:
 *
 * Gets The GSGF Error Quark.
 *
 * Returns: a #GQuark.
 **/
GQuark gsgf_error_quark (void)
{
        return g_quark_from_static_string ("gsgf-error-quark");
}
