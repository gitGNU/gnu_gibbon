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

#ifndef _LIBGSGF_DOUBLE_H
# define _LIBGSGF_DOUBLE_H

#include <glib.h>

#define GSGF_TYPE_DOUBLE \
        (gsgf_double_get_type ())
#define GSGF_DOUBLE(obj) \
        (G_TYPE_CHECK_INSTANCE_CAST ((obj), GSGF_TYPE_DOUBLE, \
                GSGFDouble))
#define GSGF_DOUBLE_CLASS(klass) (G_TYPE_CHECK_CLASS_CAST ((klass), \
        GSGF_TYPE_DOUBLE, GSGFDoubleClass))
#define GSGF_IS_DOUBLE(obj) \
        (G_TYPE_CHECK_INSTANCE_TYPE ((obj), \
                GSGF_TYPE_DOUBLE))
#define GSGF_IS_DOUBLE_CLASS(klass) \
        (G_TYPE_CHECK_CLASS_TYPE ((klass), \
                GSGF_TYPE_DOUBLE))
#define GSGF_DOUBLE_GET_CLASS(obj) \
        (G_TYPE_INSTANCE_GET_CLASS (obj), \
                GSGF_TYPE_DOUBLE, GSGFDoubleClass))

/**
 * GSGFDouble:
 *
 * One instance of a #GSGFDouble.  All properties are private.
 **/
typedef struct _GSGFDouble GSGFDouble;
struct _GSGFDouble
{
        GSGFNumber parent_instance;
};

/**
 * GSGFDoubleClass:
 *
 * A #GSGFDouble can take one of two values, either one or two.  The name
 * is a little misleading as most people will rather associate a
 * double-precision floating point value with it.  It is, however, a
 * binary object that is either one or two.  The value one means "normal"
 * and a value of two means "more than normal".  For example, the property
 * GB (good for black) is a #GSGFDouble.  A value of one means, "good for
 * black", and a value of two means "very good for black".
 **/
typedef struct _GSGFDoubleClass GSGFDoubleClass;
struct _GSGFDoubleClass
{
        /* <private >*/
        GSGFNumberClass parent_class;
};

GType gsgf_double_get_type (void) G_GNUC_CONST;

/**
 * GSGFDoubleEnum:
 * @GSGF_DOUBLE_NORMAL: Yes!
 * @GSGF_DOUBLE_VERY: Yes! YES!
 *
 * A binary value that is not yes or no but yes or very much.
 */
typedef enum {
        GSGF_DOUBLE_NORMAL = 1,
        GSGF_DOUBLE_VERY = 2
} GSGFDoubleEnum;

GSGFDouble *gsgf_double_new (GSGFDoubleEnum grade);
GSGFCookedValue *gsgf_double_new_from_raw (const GSGFRaw* raw,
                                           const GSGFFlavor *flavor,
                                           const struct _GSGFProperty *property,
                                           GError **error);
void gsgf_double_set_value (GSGFDouble *self, GSGFDoubleEnum value);
GSGFDoubleEnum gsgf_double_get_value(const GSGFDouble *self);

#endif
