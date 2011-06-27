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

/**
 * SECTION:gibbon-geo-ip-updater
 * @short_description: Class for updating the Gibbon GeoIP database.
 *
 * Since: 0.1.1
 *
 * Class for updating the Gibbon GeoIP database.
 */

#include <glib.h>
#include <glib/gi18n.h>

#include "gibbon-geo-ip-updater.h"

typedef struct _GibbonGeoIPUpdaterPrivate GibbonGeoIPUpdaterPrivate;
struct _GibbonGeoIPUpdaterPrivate {
        GibbonApp *app;
        GibbonDatabase *database;
};

#define GIBBON_GEO_IP_UPDATER_PRIVATE(obj) (G_TYPE_INSTANCE_GET_PRIVATE ((obj), \
        GIBBON_TYPE_GEO_IP_UPDATER, GibbonGeoIPUpdaterPrivate))

G_DEFINE_TYPE (GibbonGeoIPUpdater, gibbon_geo_ip_updater, G_TYPE_OBJECT)

static void 
gibbon_geo_ip_updater_init (GibbonGeoIPUpdater *self)
{
        self->priv = G_TYPE_INSTANCE_GET_PRIVATE (self,
                GIBBON_TYPE_GEO_IP_UPDATER, GibbonGeoIPUpdaterPrivate);

        self->priv->app = NULL;
        self->priv->database = NULL;
}

static void
gibbon_geo_ip_updater_finalize (GObject *object)
{
        G_OBJECT_CLASS (gibbon_geo_ip_updater_parent_class)->finalize(object);
}

static void
gibbon_geo_ip_updater_class_init (GibbonGeoIPUpdaterClass *klass)
{
        GObjectClass *object_class = G_OBJECT_CLASS (klass);
        
        g_type_class_add_private (klass, sizeof (GibbonGeoIPUpdaterPrivate));

        object_class->finalize = gibbon_geo_ip_updater_finalize;
}

/**
 * gibbon_geo_ip_updater_new:
 * @app: The #GibbonApp.
 * @database: The #GibbonDatabase.
 *
 * Creates a new #GibbonGeoIPUpdater.
 *
 * Returns: The newly created #GibbonGeoIPUpdater or %NULL in case of failure.
 */
GibbonGeoIPUpdater *
gibbon_geo_ip_updater_new (const GibbonApp *app,
                           GibbonDatabase *database,
                           gint64 last_update)
{
        GibbonGeoIPUpdater *self = g_object_new (GIBBON_TYPE_GEO_IP_UPDATER, NULL);

        return self;
}
