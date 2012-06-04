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

#ifndef _LIBGSGF_DATE_H
# define _LIBGSGF_DATE_H

#include <glib.h>

#define GSGF_TYPE_DATE \
        (gsgf_date_get_type ())
#define GSGF_DATE(obj) \
        (G_TYPE_CHECK_INSTANCE_CAST ((obj), GSGF_TYPE_DATE, \
                GSGFDate))
#define GSGF_DATE_CLASS(klass) (G_TYPE_CHECK_CLASS_CAST ((klass), \
        GSGF_TYPE_DATE, GSGFDateClass))
#define GSGF_IS_DATE(obj) \
        (G_TYPE_CHECK_INSTANCE_TYPE ((obj), \
                GSGF_TYPE_DATE))
#define GSGF_IS_DATE_CLASS(klass) \
        (G_TYPE_CHECK_CLASS_TYPE ((klass), \
                GSGF_TYPE_DATE))
#define GSGF_DATE_GET_CLASS(obj) \
        (G_TYPE_INSTANCE_GET_CLASS ((obj), \
                GSGF_TYPE_DATE, GSGFDateClass))

/**
 * GSGFDate:
 *
 * One instance of a #GSGFDate.  All properties are private.
 **/
typedef struct _GSGFDate GSGFDate;
struct _GSGFDate
{
        GSGFSimpleText parent_instance;

        /*< private >*/
        struct _GSGFDatePrivate *priv;
};

/**
 * GSGFDateClass:
 *
 * Representation of a date (or dates!) in SGF!
 **/
typedef struct _GSGFDateClass GSGFDateClass;
struct _GSGFDateClass
{
        /* <private >*/
        GSGFSimpleTextClass parent_class;
};

/**
 * GSGFDateDMY:
 * @day: Day of the month (1-31).
 * @month: Month of the year (1-12).
 * @year: The year.
 *
 * Broken down representation of a calendar date.
 */
typedef struct _GSGFDateDMY GSGFDateDMY;
struct _GSGFDateDMY {
        guint day: 6;
        guint month: 4;
        guint year: 16;
};

GType gsgf_date_get_type (void) G_GNUC_CONST;

GSGFDate *gsgf_date_new (GSGFDateDMY *date, GError **error);
GSGFCookedValue *gsgf_date_new_from_raw (const GSGFRaw *raw,
                                         const GSGFFlavor *flavor,
                                         const struct _GSGFProperty *property,
                                         GError **error);
gboolean gsgf_date_append (GSGFDate *self, GSGFDateDMY *date, GError **error);

#endif
