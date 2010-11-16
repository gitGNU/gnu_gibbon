/*
 * This file is part of Gibbon, a graphical frontend to the First Internet 
 * Backgammon Server FIBS.
 * Copyright (C) 2009 Guido Flohr, http://guido-flohr.net/.
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
 * GSGF_ERROR:
 *
 * Error domain for GSGF.  Errors in this domain will be from the
 * #GSGFError enumeration.
 * See #GError for more information on error domains.
 **/
#define GSGF_ERROR gsgf_error_quark ()

/**
 * GSGFError:
 * @GSGF_ERROR_NONE: No error.
 * @GSGF_ERROR_SYNTAX: Syntax error.
 * @GSGF_ERROR_EMPTY_COLLECTION: Collection is empty.
 * @GSGF_ERROR_PROPERTY_EXISTS: Attempt to insert a property with an already existing name.
 */
typedef enum {
        GSGF_ERROR_NONE = 0,
        GSGF_ERROR_SYNTAX = 1,
        GSGF_ERROR_EMPTY_COLLECTION = 2,
        GSGF_ERROR_PROPERTY_EXISTS = 3,
        GSGF_ERROR_EMPTY_PROPERTY = 4
} GSGFError;

extern GQuark gsgf_error_quark (void);

G_END_DECLS

#endif
