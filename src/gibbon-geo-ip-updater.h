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

#ifndef _GIBBON_GEO_IP_UPDATER_H
# define _GIBBON_GEO_IP_UPDATER_H

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <glib.h>
#include <glib-object.h>

#include "gibbon-app.h"
#include "gibbon-database.h"

#define GIBBON_TYPE_GEO_IP_UPDATER \
        (gibbon_geo_ip_updater_get_type ())
#define GIBBON_GEO_IP_UPDATER(obj) \
        (G_TYPE_CHECK_INSTANCE_CAST ((obj), GIBBON_TYPE_GEO_IP_UPDATER, \
                GibbonGeoIPUpdater))
#define GIBBON_GEO_IP_UPDATER_CLASS(klass) (G_TYPE_CHECK_CLASS_CAST ((klass), \
        GIBBON_TYPE_GEO_IP_UPDATER, GibbonGeoIPUpdaterClass))
#define GIBBON_IS_GEO_IP_UPDATER(obj) \
        (G_TYPE_CHECK_INSTANCE_TYPE ((obj), \
                GIBBON_TYPE_GEO_IP_UPDATER))
#define GIBBON_IS_GEO_IP_UPDATER_CLASS(klass) \
        (G_TYPE_CHECK_CLASS_TYPE ((klass), \
                GIBBON_TYPE_GEO_IP_UPDATER))
#define GIBBON_GEO_IP_UPDATER_GET_CLASS(obj) \
        (G_TYPE_INSTANCE_GET_CLASS ((obj), \
                GIBBON_TYPE_GEO_IP_UPDATER, GibbonGeoIPUpdaterClass))

/**
 * GibbonGeoIPUpdater:
 *
 * One instance of a #GibbonGeoIPUpdater.  All properties are private.
 */
typedef struct _GibbonGeoIPUpdater GibbonGeoIPUpdater;
struct _GibbonGeoIPUpdater
{
        GObject parent_instance;

        /*< private >*/
        struct _GibbonGeoIPUpdaterPrivate *priv;
};

/**
 * GibbonGeoIPUpdaterClass:
 *
 * Class handling updates of the GeoIP database!
 */
typedef struct _GibbonGeoIPUpdaterClass GibbonGeoIPUpdaterClass;
struct _GibbonGeoIPUpdaterClass
{
        /* <private >*/
        GObjectClass parent_class;
};

GType gibbon_geo_ip_updater_get_type (void) G_GNUC_CONST;

GibbonGeoIPUpdater *gibbon_geo_ip_updater_new (GibbonDatabase *database,
                                               gint64 last_update);
void gibbon_geo_ip_updater_start (GibbonGeoIPUpdater *self);

#endif
