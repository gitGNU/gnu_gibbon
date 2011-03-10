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

#ifndef _GIBBON_APP_H
# define _GIBBON_APP_H

#include <glib.h>
#include <glib-object.h>

#define GIBBON_TYPE_APP \
        (gibbon_app_get_type ())
#define GIBBON_APP(obj) \
        (G_TYPE_CHECK_INSTANCE_CAST ((obj), GIBBON_TYPE_APP, \
                GibbonApp))
#define GIBBON_APP_CLASS(klass) (G_TYPE_CHECK_CLASS_CAST ((klass), \
        GIBBON_TYPE_APP, GibbonAppClass))
#define GIBBON_IS_APP(obj) \
        (G_TYPE_CHECK_INSTANCE_TYPE ((obj), \
                GIBBON_TYPE_APP))
#define GIBBON_IS_APP_CLASS(klass) \
        (G_TYPE_CHECK_CLASS_TYPE ((klass), \
                GIBBON_TYPE_APP))
#define GIBBON_APP_GET_CLASS(obj) \
        (G_TYPE_INSTANCE_GET_CLASS ((obj), \
                GIBBON_TYPE_APP, GibbonAppClass))

/**
 * GibbonApp:
 *
 * One instance of a #GibbonApp.  All properties are private.
 **/
typedef struct _GibbonApp GibbonApp;
struct _GibbonApp
{
        GObject parent_instance;

        /*< private >*/
        struct _GibbonAppPrivate *priv;
};

/**
 * GibbonAppClass:
 *
 * Class representing the application gibbon!
 **/
typedef struct _GibbonAppClass GibbonAppClass;
struct _GibbonAppClass
{
        /* <private >*/
        GObjectClass parent_class;
};

GType gibbon_app_get_type (void) G_GNUC_CONST;

GibbonApp *gibbon_app_new (const gchar *builder_name, const gchar *pixmaps_dir);
void gibbon_app_display_error (const GibbonApp *self,
                               const gchar *message_format, ...);
void gibbon_app_display_info (const GibbonApp *self,
                              const gchar *message_format, ...);
GObject *gibbon_app_find_object (const GibbonApp *self, const gchar *id,
                                 GType type);
GtkWidget *gibbon_app_get_window (const GibbonApp *self);
const gchar *gibbon_app_get_pixmaps_directory (const GibbonApp *app);

#endif