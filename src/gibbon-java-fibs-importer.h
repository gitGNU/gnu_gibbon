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

#ifndef _GIBBON_JAVA_FIBS_IMPORTER_H
# define _GIBBON_JAVA_FIBS_IMPORTER_H

#include <glib-object.h>

#include "gibbon-app.h"

#define GIBBON_TYPE_JAVA_FIBS_IMPORTER \
        (gibbon_java_fibs_importer_get_type ())
#define GIBBON_JAVA_FIBS_IMPORTER(obj) \
        (G_TYPE_CHECK_INSTANCE_CAST ((obj), GIBBON_TYPE_JAVA_FIBS_IMPORTER, \
                GibbonJavaFIBSImporter))
#define GIBBON_JAVA_FIBS_IMPORTER_CLASS(klass) (G_TYPE_CHECK_CLASS_CAST ((klass), \
        GIBBON_TYPE_JAVA_FIBS_IMPORTER, GibbonJavaFIBSImporterClass))
#define GIBBON_IS_JAVA_FIBS_IMPORTER(obj) \
        (G_TYPE_CHECK_INSTANCE_TYPE ((obj), \
                GIBBON_TYPE_JAVA_FIBS_IMPORTER))
#define GIBBON_IS_JAVA_FIBS_IMPORTER_CLASS(klass) \
        (G_TYPE_CHECK_CLASS_TYPE ((klass), \
                GIBBON_TYPE_JAVA_FIBS_IMPORTER))
#define GIBBON_JAVA_FIBS_IMPORTER_GET_CLASS(obj) \
        (G_TYPE_INSTANCE_GET_CLASS ((obj), \
                GIBBON_TYPE_JAVA_FIBS_IMPORTER, GibbonJavaFIBSImporterClass))

/**
 * GibbonJavaFIBSImporter:
 *
 * One instance of a #GibbonJavaFIBSImporter.  All properties are private.
 **/
typedef struct _GibbonJavaFIBSImporter GibbonJavaFIBSImporter;
struct _GibbonJavaFIBSImporter
{
        GObject parent_instance;

        /*< private >*/
        struct _GibbonJavaFIBSImporterPrivate *priv;
};

/**
 * GibbonJavaFIBSImporterClass:
 *
 * Class running an importer for JavaFIBS!
 **/
typedef struct _GibbonJavaFIBSImporterClass GibbonJavaFIBSImporterClass;
struct _GibbonJavaFIBSImporterClass
{
        /* <private >*/
        GObjectClass parent_class;
};

GType gibbon_java_fibs_importer_get_type (void) G_GNUC_CONST;

GibbonJavaFIBSImporter *gibbon_java_fibs_importer_new ();
void gibbon_java_fibs_importer_run (GibbonJavaFIBSImporter *self);

#endif
