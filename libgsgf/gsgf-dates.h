/*
 * This file is part of gibbon.
 * Gibbon is a Gtk+ frontend for the First Internet Backgammon Server FIBS.
 * Copyright (C) 2009-2011 Guido Flohr, http://guido-flohr.net/.
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

#ifndef _LIBGSGF_DATES_H
# define _LIBGSGF_DATES_H

#include <glib.h>

#define GSGF_TYPE_DATES \
        (gsgf_dates_get_type ())
#define GSGF_DATES(obj) \
        (G_TYPE_CHECK_INSTANCE_CAST ((obj), GSGF_TYPE_DATES, \
                GSGFDates))
#define GSGF_DATES_CLASS(klass) (G_TYPE_CHECK_CLASS_CAST ((klass), \
        GSGF_TYPE_DATES, GSGFDatesClass))
#define GSGF_IS_DATES(obj) \
        (G_TYPE_CHECK_INSTANCE_TYPE ((obj), \
                GSGF_TYPE_DATES))
#define GSGF_IS_DATES_CLASS(klass) \
        (G_TYPE_CHECK_CLASS_TYPE ((klass), \
                GSGF_TYPE_DATES))
#define GSGF_DATES_GET_CLASS(obj) \
        (G_TYPE_INSTANCE_GET_CLASS (obj), \
                GSGF_TYPE_DATES, GSGFDatesClass))

/**
 * GSGFDates:
 *
 * One instance of a #GSGFDates.  All properties are private.
 **/
typedef struct _GSGFDates GSGFDates;
struct _GSGFDates
{
        GSGFSimpleText parent_instance;

        /*< private >*/
        struct _GSGFDatesPrivate *priv;
};

/**
 * GSGFDatesClass:
 *
 * FIXME! The author was negligent enough to not document this class!
 **/
typedef struct _GSGFDatesClass GSGFDatesClass;
struct _GSGFDatesClass
{
        /* <private >*/
        GSGFSimpleTextClass parent_class;
};

GType gsgf_dates_get_type (void) G_GNUC_CONST;

GSGFDates *gsgf_dates_new (/* FIXME! Argument list! */ const gchar *dummy);

#endif
