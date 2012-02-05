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

#ifndef _LIBGSGF_COOKED_VALUE_H
# define _LIBGSGF_COOKED_VALUE_H

#include <glib.h>
#include <gio/gio.h>

G_BEGIN_DECLS

#define GSGF_TYPE_COOKED_VALUE             (gsgf_cooked_value_get_type ())
#define GSGF_COOKED_VALUE(obj)             (G_TYPE_CHECK_INSTANCE_CAST ((obj), \
                GSGF_TYPE_COOKED_VALUE, GSGFCookedValue))
#define GSGF_COOKED_VALUE_CLASS(klass)     (G_TYPE_CHECK_CLASS_CAST ((klass), \
                GSGF_TYPE_COOKED_VALUE, GSGFCookedValueClass))
#define GSGF_IS_COOKED_VALUE(obj)          (G_TYPE_CHECK_INSTANCE_TYPE ((obj), \
                GSGF_TYPE_COOKED_VALUE))
#define GSGF_IS_COOKED_VALUE_CLASS(klass)  (G_TYPE_CHECK_CLASS_TYPE ((klass), \
                GSGF_TYPE_COOKED_VALUE))
#define GSGF_COOKED_VALUE_GET_CLASS(obj)   (G_TYPE_INSTANCE_GET_CLASS ((obj), \
                GSGF_TYPE_COOKED_VALUE, GSGFCookedValueClass))

/**
 * GSGFCookedValue:
 *
 * Instance of a #GSGFCookedValueClass.  All properties are private.
 **/
typedef struct _GSGFCookedValue        GSGFCookedValue;
struct _GSGFCookedValue
{
        GSGFValue parent_instance;
};

/**
 * GSGFCookedValueClass:
 *
 * A class representing a cooked value of a #GSGFProperty.  All other
 * properties and methods are private.
 *
 * It is the common base class of all other qualified SGF data types.
 */
typedef struct _GSGFCookedValueClass   GSGFCookedValueClass;
struct _GSGFCookedValueClass
{
        /*< private >*/
        GSGFValueClass parent_class;
};

GType gsgf_cooked_value_get_type(void) G_GNUC_CONST;

G_END_DECLS

#endif
