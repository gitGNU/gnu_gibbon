/* libsvg-cairo - Render SVG documents using the cairo library
 *
 * Copyright (C) 2002 University of Southern California
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

#include <stdlib.h>
#include <stdarg.h>

#include "svg-cairo-internal.h"

int
_svg_cairo_sprintf_alloc (char **str, const char *fmt, ...)
{
    int ret;
    va_list ap;

    va_start(ap, fmt);
    ret = _svg_cairo_vsprintf_alloc(str, fmt, ap);
    va_end(ap);

    return ret;
}

/* ripped more or less straight out of PRINTF(3) */
int
_svg_cairo_vsprintf_alloc (char **str, const char *fmt, va_list ap)
{
    char *new_str;
    /* Guess we need no more than 100 bytes. */
    int n, size = 100;
 
    if ((*str = malloc (size)) == NULL)
	return -1;
    while (1) {
	/* Try to print in the allocated space. */
	n = vsnprintf (*str, size, fmt, ap);
	/* If that worked, return the size. */
	if (n > -1 && n < size)
	    return n;
	/* Else try again with more space. */
	if (n > -1)    /* glibc 2.1 */
	    size = n+1; /* precisely what is needed */
	else           /* glibc 2.0 */
	    size *= 2;  /* twice the old size */
	new_str = realloc(*str, size);
	if (new_str == NULL) {
	    free(*str);
	    *str = NULL;
	    return -1;
	}
	*str = new_str;
    }
}
