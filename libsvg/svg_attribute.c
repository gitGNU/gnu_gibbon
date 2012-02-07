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

#include "svgint.h"

#include <glib.h>

svgint_status_t
_svg_attribute_get_double (const gchar	**attributes,
			   const char	*name,
			   gdouble	*value,
			   double	default_value)
{
    int i;

    *value = default_value;

    if (attributes == NULL)
	return SVGINT_STATUS_ATTRIBUTE_NOT_FOUND;

    for (i=0; attributes[i]; i += 2) {
	if (strcmp (attributes[i], name) == 0) {
	    *value = g_ascii_strtod (attributes[i+1], NULL);
	    return SVG_STATUS_SUCCESS;
	}
    }

    return SVGINT_STATUS_ATTRIBUTE_NOT_FOUND;
}

svgint_status_t
_svg_attribute_get_string (const char	**attributes,
			   const char	*name,
			   const char	**value,
			   const char	*default_value)
{
    int i;

    *value = default_value;

    if (attributes == NULL)
	return SVGINT_STATUS_ATTRIBUTE_NOT_FOUND;

    for (i=0; attributes[i]; i += 2) {
	if (strcmp (attributes[i], name) == 0) {
	    *value = attributes[i+1];
	    return SVG_STATUS_SUCCESS;
	}
    }

    return SVGINT_STATUS_ATTRIBUTE_NOT_FOUND;
}

svgint_status_t
_svg_attribute_get_length (const char	**attributes,
			   const char	*name,
			   svg_length_t	*value,
			   const char	*default_value)
{
    int i;

    _svg_length_init_from_str (value, default_value);

    if (attributes == NULL)
	return SVGINT_STATUS_ATTRIBUTE_NOT_FOUND;

    for (i=0; attributes[i]; i += 2) {
	if (strcmp (attributes[i], name) == 0) {
	    _svg_length_init_from_str (value, attributes[i+1]);
	    return SVG_STATUS_SUCCESS;
	}
    }
    
    return SVGINT_STATUS_ATTRIBUTE_NOT_FOUND;
}
