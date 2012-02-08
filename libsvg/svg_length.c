/* libsvg-cairo - Render SVG documents using the cairo library
 *
 * Copyright (C) 2002 USC/Information Sciences Institute
 * Copyright (C) 2009-2012 Guido Flohr
 * 
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 3 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with GCinema; if not, see <http://www.gnu.org/licenses/>.
 *
 * Original author: Carl D. Worth <cworth@isi.edu>
 */

#include <string.h>

#include <glib.h>

#include "svgint.h"

svg_status_t
_svg_length_init (svg_length_t *length, double value)
{
    return _svg_length_init_unit (length, value, SVG_LENGTH_UNIT_PX, SVG_LENGTH_ORIENTATION_OTHER);
}

svg_status_t
_svg_length_init_unit (svg_length_t		*length,
		       double			value,
		       svg_length_unit_t	unit,
		       svg_length_orientation_t orientation)
{
    length->value = value;
    length->unit = unit;
    length->orientation = orientation;

    return SVG_STATUS_SUCCESS;
}

svg_status_t
_svg_length_init_from_str (svg_length_t *length, const char *str)
{
    gdouble value;
    svg_length_unit_t unit;
    gchar *unit_str;

    value = g_ascii_strtod (str, &unit_str);

    if (unit_str == str)
	return SVG_STATUS_PARSE_ERROR;

    if (unit_str) {
	if (strcmp (unit_str, "px") == 0)
	    unit = SVG_LENGTH_UNIT_PX;
	else if (strcmp (unit_str, "pt") == 0)
	    unit = SVG_LENGTH_UNIT_PT;
	else if (strcmp (unit_str, "in") == 0)
	    unit = SVG_LENGTH_UNIT_IN;
	else if (strcmp (unit_str, "cm") == 0)
	    unit = SVG_LENGTH_UNIT_CM;
	else if (strcmp (unit_str, "mm") == 0)
	    unit = SVG_LENGTH_UNIT_MM;
	else if (strcmp (unit_str, "pc") == 0)
	    unit = SVG_LENGTH_UNIT_PC;
	else if (strcmp (unit_str, "em") == 0)
	    unit = SVG_LENGTH_UNIT_EM;
	else if (strcmp (unit_str, "ex") == 0)
	    unit = SVG_LENGTH_UNIT_EX;
	else if (strcmp (unit_str, "%") == 0)
	    unit = SVG_LENGTH_UNIT_PCT;
	else {
	    unit = SVG_LENGTH_UNIT_PX;
	}
    } else {
	unit = SVG_LENGTH_UNIT_PX;
    }

    return _svg_length_init_unit (length, value, unit, length->orientation);
}

svg_status_t
_svg_length_deinit (svg_length_t *length)
{
    length->value = 0.0;
    length->unit = SVG_LENGTH_UNIT_PX;

    return SVG_STATUS_SUCCESS;
}

