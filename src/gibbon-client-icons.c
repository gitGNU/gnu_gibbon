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
 * SECTION:gibbon-client-icons
 * @short_description: Container for pre-loaded client icons!
 *
 * Since: 0.1.1
 *
 * This purpose maps (software) clients to icons representing their type.
 */

#include <glib.h>
#include <glib/gi18n.h>

#include "gibbon-client-icons.h"
#include "gibbon-util.h"

typedef struct _GibbonClientIconsPrivate GibbonClientIconsPrivate;
struct _GibbonClientIconsPrivate {
        gchar *pixmaps_dir;
};

#define GIBBON_CLIENT_ICONS_PRIVATE(obj) (G_TYPE_INSTANCE_GET_PRIVATE ((obj), \
        GIBBON_TYPE_CLIENT_ICONS, GibbonClientIconsPrivate))

G_DEFINE_TYPE (GibbonClientIcons, gibbon_client_icons, G_TYPE_OBJECT)

static void 
gibbon_client_icons_init (GibbonClientIcons *self)
{
        self->priv = G_TYPE_INSTANCE_GET_PRIVATE (self,
                GIBBON_TYPE_CLIENT_ICONS, GibbonClientIconsPrivate);

        self->priv->pixmaps_dir = NULL;
}

static void
gibbon_client_icons_finalize (GObject *object)
{
        GibbonClientIcons *self = GIBBON_CLIENT_ICONS (object);

        if (self->priv->pixmaps_dir)
                g_free (self->priv->pixmaps_dir);

        G_OBJECT_CLASS (gibbon_client_icons_parent_class)->finalize(object);
}

static void
gibbon_client_icons_class_init (GibbonClientIconsClass *klass)
{
        GObjectClass *object_class = G_OBJECT_CLASS (klass);
        
        g_type_class_add_private (klass, sizeof (GibbonClientIconsPrivate));

        object_class->finalize = gibbon_client_icons_finalize;
}

/**
 * gibbon_client_icons_new:
 * @directory: Paths to the package pixmaps dir..
 *
 * Creates a new #GibbonClientIcons.
 *
 * Returns: The newly created #GibbonClientIcons or %NULL in case of failure.
 */
GibbonClientIcons *
gibbon_client_icons_new (const gchar *dir)
{
        GibbonClientIcons *self = g_object_new (GIBBON_TYPE_CLIENT_ICONS, NULL);

        self->priv->pixmaps_dir = g_strdup (dir);

        return self;
}

GdkPixbuf *
gibbon_client_icons_get_icon (GibbonClientIcons *self,
                              enum GibbonClientType type)
{
        g_return_val_if_fail (GIBBON_IS_CLIENT_ICONS (self), NULL);

        return NULL;
}
