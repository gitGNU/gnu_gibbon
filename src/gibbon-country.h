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

#ifndef _GIBBON_COUNTRY_H
# define _GIBBON_COUNTRY_H

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <gtk/gtk.h>

#define GIBBON_TYPE_COUNTRY \
        (gibbon_country_get_type ())
#define GIBBON_COUNTRY(obj) \
        (G_TYPE_CHECK_INSTANCE_CAST ((obj), GIBBON_TYPE_COUNTRY, \
                GibbonCountry))
#define GIBBON_COUNTRY_CLASS(klass) (G_TYPE_CHECK_CLASS_CAST ((klass), \
        GIBBON_TYPE_COUNTRY, GibbonCountryClass))
#define GIBBON_IS_COUNTRY(obj) \
        (G_TYPE_CHECK_INSTANCE_TYPE ((obj), \
                GIBBON_TYPE_COUNTRY))
#define GIBBON_IS_COUNTRY_CLASS(klass) \
        (G_TYPE_CHECK_CLASS_TYPE ((klass), \
                GIBBON_TYPE_COUNTRY))
#define GIBBON_COUNTRY_GET_CLASS(obj) \
        (G_TYPE_INSTANCE_GET_CLASS ((obj), \
                GIBBON_TYPE_COUNTRY, GibbonCountryClass))

/**
 * GibbonCountry:
 *
 * One instance of a #GibbonCountry.  All properties are private.
 */
typedef struct _GibbonCountry GibbonCountry;
struct _GibbonCountry
{
        GObject parent_instance;

        /*< private >*/
        struct _GibbonCountryPrivate *priv;
};

/**
 * GibbonCountryClass:
 *
 * Classs representing a country.
 */
typedef struct _GibbonCountryClass GibbonCountryClass;
struct _GibbonCountryClass
{
        /* <private >*/
        GObjectClass parent_class;
};

GType gibbon_country_get_type (void) G_GNUC_CONST;

GibbonCountry *gibbon_country_new (const gchar *alpha2);
const gchar *gibbon_country_get_alpha2 (const GibbonCountry *self);
const gchar *gibbon_country_get_name (const GibbonCountry *self);
const GdkPixbuf *gibbon_country_get_pixbuf (const GibbonCountry *self);

#endif
