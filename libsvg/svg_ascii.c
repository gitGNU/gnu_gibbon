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
 * Originally lifted by Carl D. Worth from GLIB:
 *
 * GLIB - Library of useful routines for C programming
 * Copyright (C) 1995-1997  Peter Mattis, Spencer Kimball and Josh MacDonald
 */

/*
 * Modified by the GLib Team and others 1997-2000.  See the AUTHORS
 * file for a list of people on the GLib Team.  See the ChangeLog
 * files for a list of changes.  These files are distributed with
 * GLib at ftp://ftp.gtk.org/pub/gtk/. 
 */

/*
 * MT safe
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "svg_ascii.h"

#include <stdlib.h>
#include <locale.h>
#include <errno.h>
#include <ctype.h>
#include <string.h>

static const uint16_t svg_ascii_table_data[256] = {
  0x004, 0x004, 0x004, 0x004, 0x004, 0x004, 0x004, 0x004,
  0x004, 0x104, 0x104, 0x004, 0x104, 0x104, 0x004, 0x004,
  0x004, 0x004, 0x004, 0x004, 0x004, 0x004, 0x004, 0x004,
  0x004, 0x004, 0x004, 0x004, 0x004, 0x004, 0x004, 0x004,
  0x140, 0x0d0, 0x0d0, 0x0d0, 0x0d0, 0x0d0, 0x0d0, 0x0d0,
  0x0d0, 0x0d0, 0x0d0, 0x0d0, 0x0d0, 0x0d0, 0x0d0, 0x0d0,
  0x459, 0x459, 0x459, 0x459, 0x459, 0x459, 0x459, 0x459,
  0x459, 0x459, 0x0d0, 0x0d0, 0x0d0, 0x0d0, 0x0d0, 0x0d0,
  0x0d0, 0x653, 0x653, 0x653, 0x653, 0x653, 0x653, 0x253,
  0x253, 0x253, 0x253, 0x253, 0x253, 0x253, 0x253, 0x253,
  0x253, 0x253, 0x253, 0x253, 0x253, 0x253, 0x253, 0x253,
  0x253, 0x253, 0x253, 0x0d0, 0x0d0, 0x0d0, 0x0d0, 0x0d0,
  0x0d0, 0x473, 0x473, 0x473, 0x473, 0x473, 0x473, 0x073,
  0x073, 0x073, 0x073, 0x073, 0x073, 0x073, 0x073, 0x073,
  0x073, 0x073, 0x073, 0x073, 0x073, 0x073, 0x073, 0x073,
  0x073, 0x073, 0x073, 0x0d0, 0x0d0, 0x0d0, 0x0d0, 0x004
  /* the upper 128 are all zeroes */
};

const uint16_t * const svg_ascii_table = svg_ascii_table_data;

/**
 * _svg_ascii_tolower:
 * @c: any character.
 * 
 * Convert a character to ASCII lower case.
 *
 * Unlike the standard C library tolower() function, this only
 * recognizes standard ASCII letters and ignores the locale, returning
 * all non-ASCII characters unchanged, even if they are lower case
 * letters in a particular character set. Also unlike the standard
 * library function, this takes and returns a char, not an int, so
 * don't call it on %EOF but no need to worry about casting to #unsigned char
 * before passing a possibly non-ASCII character in.
 * 
 * Return value: the result of converting @c to lower case.
 *               If @c is not an ASCII upper case letter,
 *               @c is returned unchanged.
 **/
char
_svg_ascii_tolower (char c)
{
  return _svg_ascii_isupper (c) ? c - 'A' + 'a' : c;
}

/**
 * _svg_ascii_toupper:
 * @c: any character.
 * 
 * Convert a character to ASCII upper case.
 *
 * Unlike the standard C library toupper() function, this only
 * recognizes standard ASCII letters and ignores the locale, returning
 * all non-ASCII characters unchanged, even if they are upper case
 * letters in a particular character set. Also unlike the standard
 * library function, this takes and returns a char, not an int, so
 * don't call it on %EOF but no need to worry about casting to #unsigned char
 * before passing a possibly non-ASCII character in.
 * 
 * Return value: the result of converting @c to upper case.
 *               If @c is not an ASCII lower case letter,
 *               @c is returned unchanged.
 **/
char
_svg_ascii_toupper (char c)
{
  return _svg_ascii_islower (c) ? c - 'a' + 'A' : c;
}

/**
 * _svg_ascii_digit_value:
 * @c: an ASCII character.
 *
 * Determines the numeric value of a character as a decimal
 * digit. Differs from svg_unichar_digit_value() because it takes
 * a char, so there's no worry about sign extension if characters
 * are signed.
 *
 * Return value: If @c is a decimal digit (according to
 * _svg_ascii_isdigit()), its numeric value. Otherwise, -1.
 **/
int
_svg_ascii_digit_value (char c)
{
  if (_svg_ascii_isdigit (c))
    return c - '0';
  return -1;
}

/**
 * _svg_ascii_xdigit_value:
 * @c: an ASCII character.
 *
 * Determines the numeric value of a character as a hexidecimal
 * digit. Differs from svg_unichar_xdigit_value() because it takes
 * a char, so there's no worry about sign extension if characters
 * are signed.
 *
 * Return value: If @c is a hex digit (according to
 * _svg_ascii_isxdigit()), its numeric value. Otherwise, -1.
 **/
int
_svg_ascii_xdigit_value (char c)
{
  if (c >= 'A' && c <= 'F')
    return c - 'A' + 10;
  if (c >= 'a' && c <= 'f')
    return c - 'a' + 10;
  return _svg_ascii_digit_value (c);
}

/**
 * _svg_ascii_strcasecmp:
 * @s1: string to compare with @s2.
 * @s2: string to compare with @s1.
 * 
 * Compare two strings, ignoring the case of ASCII characters.
 *
 * Unlike the BSD strcasecmp() function, this only recognizes standard
 * ASCII letters and ignores the locale, treating all non-ASCII
 * characters as if they are not letters.
 * 
 * Return value: an integer less than, equal to, or greater than
 *               zero if @s1 is found, respectively, to be less than,
 *               to match, or to be greater than @s2.
 **/
int
_svg_ascii_strcasecmp (const char *s1,
		       const char *s2)
{
  int c1, c2;

  if (s1 == NULL || s2 == NULL)
      return 0;

  while (*s1 && *s2)
    {
      c1 = (int)(unsigned char) _svg_ascii_tolower (*s1);
      c2 = (int)(unsigned char) _svg_ascii_tolower (*s2);
      if (c1 != c2)
	return (c1 - c2);
      s1++; s2++;
    }

  return (((int)(unsigned char) *s1) - ((int)(unsigned char) *s2));
}

/**
 * _svg_ascii_strncasecmp:
 * @s1: string to compare with @s2.
 * @s2: string to compare with @s1.
 * @n:  number of characters to compare.
 * 
 * Compare @s1 and @s2, ignoring the case of ASCII characters and any
 * characters after the first @n in each string.
 *
 * Unlike the BSD strcasecmp() function, this only recognizes standard
 * ASCII letters and ignores the locale, treating all non-ASCII
 * characters as if they are not letters.
 * 
 * Return value: an integer less than, equal to, or greater than zero
 *               if the first @n bytes of @s1 is found, respectively,
 *               to be less than, to match, or to be greater than the
 *               first @n bytes of @s2.
 **/
int
_svg_ascii_strncasecmp (const char *s1,
			const char *s2,
			size_t	   n)
{
  int c1, c2;

  if (s1 == NULL || s2 == NULL)
      return 0;

  while (n && *s1 && *s2)
    {
      n -= 1;
      c1 = (int)(unsigned char) _svg_ascii_tolower (*s1);
      c2 = (int)(unsigned char) _svg_ascii_tolower (*s2);
      if (c1 != c2)
	return (c1 - c2);
      s1++; s2++;
    }

  if (n)
    return (((int) (unsigned char) *s1) - ((int) (unsigned char) *s2));
  else
    return 0;
}
