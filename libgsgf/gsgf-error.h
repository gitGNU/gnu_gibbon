/*
 * This file is part of Gibbon, a graphical frontend to the First Internet 
 * Backgammon Server FIBS.
 * Copyright (C) 2009-2012 Guido Flohr, http://guido-flohr.net/.
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

#ifndef _LIBGSGF_ERROR_H
# define _LIBGSGF_ERROR_H

#include <glib.h>

G_BEGIN_DECLS

/**
 * GSGFError:
 * @GSGF_ERROR_NONE: No error.
 * @GSGF_ERROR_FLAVOR_UNSUPPORTED: Unknown SGF flavor.
 * @GSGF_ERROR_FLAVOR_MISMATCH: Attempt to insert a child of different flavor
 *                              than the parent.
 * @GSGF_ERROR_SYNTAX: Syntax error.
 * @GSGF_ERROR_EMPTY_COLLECTION: Collection is empty.
 * @GSGF_ERROR_PROPERTY_EXISTS: Attempt to insert a property with an already
 *                              existing name.
 * @GSGF_ERROR_EMPTY_PROPERTY: Attempt to write a property without a value.
 * @GSGF_ERROR_INTERNAL_ERROR: Internal error.
 * @GSGF_ERROR_INVALID_NUMBER: Invalid number format.
 * @GSGF_ERROR_NAN: Not a number.
 * @GSGF_ERROR_LIST_TOO_LONG: A list of values was to long for the semantics
 *                            of a particular property.
 * @GSGF_ERROR_LIST_EMPTY: A "list of" was empty.
 * @GSGF_ERROR_SEMANTIC_ERROR: Semantic error i.e. the data did not match the
 *                             expectations.
 * @GSGF_ERROR_INVALID_MOVE: Invalid syntax for a move.
 * @GSGF_ERROR_INVALID_POINT: Invalid syntax for a point.
 * @GSGF_ERROR_INVALID_STONE: Invalid syntax for a stone.
 * @GSGF_ERROR_USAGE_ERROR: Invalid library usage.
 * @GSGF_ERROR_NON_UNIQUE_POINT: Non-unique points in list.
 * @GSGF_ERROR_NON_UNIQUE_STONE: Non-unique stones in list.
 * @GSGF_ERROR_DOUBLE_OUT_OF_RANGE: A #GSGFDouble was set to something out of
 *                                  the range of a #GSGFDoubleEnum.
 * @GSGF_ERROR_INVALID_DATE_FORMAT: A #GSGFDates could not be parsed.
 *
 * Error codes for the domain #GSGF_ERROR.
 */
typedef enum {
        GSGF_ERROR_NONE = 0,
        GSGF_ERROR_FLAVOR_UNSUPPORTED = 1,
        GSGF_ERROR_FLAVOR_MISMATCH = 2,
        GSGF_ERROR_SYNTAX = 3,
        GSGF_ERROR_EMPTY_COLLECTION = 4,
        GSGF_ERROR_PROPERTY_EXISTS = 5,
        GSGF_ERROR_EMPTY_PROPERTY = 6,
        GSGF_ERROR_INTERNAL_ERROR = 7,
        GSGF_ERROR_INVALID_NUMBER = 8,
        GSGF_ERROR_NAN = 9,
        GSGF_ERROR_LIST_TOO_LONG = 10,
        GSGF_ERROR_LIST_EMPTY = 11,
        GSGF_ERROR_SEMANTIC_ERROR = 12,
        GSGF_ERROR_INVALID_MOVE = 14,
        GSGF_ERROR_INVALID_POINT = 15,
        GSGF_ERROR_INVALID_STONE = 16,
        GSGF_ERROR_USAGE_ERROR = 17,
        GSGF_ERROR_NON_UNIQUE_POINT = 18,
        GSGF_ERROR_NON_UNIQUE_STONE = 19,
        GSGF_ERROR_DOUBLE_OUT_OF_RANGE = 20,
        GSGF_ERROR_INVALID_DATE_FORMAT = 21
} GSGFError;

/**
 * GSGF_ERROR:
 *
 * Error domain for GSGF.  Errors in this domain will be from the
 * #GSGFError enumeration.
 * See #GError for more information on error domains.
 **/
#define GSGF_ERROR gsgf_error_quark ()

extern GQuark gsgf_error_quark (void);

G_END_DECLS

#endif
