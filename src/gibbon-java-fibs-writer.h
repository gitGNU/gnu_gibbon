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

#ifndef _GIBBON_JAVA_FIBS_WRITER_H
# define _GIBBON_JAVA_FIBS_WRITER_H

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <glib.h>
#include <glib-object.h>

#include "gibbon-match-writer.h"

#define GIBBON_TYPE_JAVA_FIBS_WRITER \
        (gibbon_java_fibs_writer_get_type ())
#define GIBBON_JAVA_FIBS_WRITER(obj) \
        (G_TYPE_CHECK_INSTANCE_CAST ((obj), GIBBON_TYPE_JAVA_FIBS_WRITER, \
                GibbonJavaFIBSWriter))
#define GIBBON_JAVA_FIBS_WRITER_CLASS(klass) (G_TYPE_CHECK_CLASS_CAST ((klass), \
        GIBBON_TYPE_JAVA_FIBS_WRITER, GibbonJavaFIBSWriterClass))
#define GIBBON_IS_JAVA_FIBS_WRITER(obj) \
        (G_TYPE_CHECK_INSTANCE_TYPE ((obj), \
                GIBBON_TYPE_JAVA_FIBS_WRITER))
#define GIBBON_IS_JAVA_FIBS_WRITER_CLASS(klass) \
        (G_TYPE_CHECK_CLASS_TYPE ((klass), \
                GIBBON_TYPE_JAVA_FIBS_WRITER))
#define GIBBON_JAVA_FIBS_WRITER_GET_CLASS(obj) \
        (G_TYPE_INSTANCE_GET_CLASS ((obj), \
                GIBBON_TYPE_JAVA_FIBS_WRITER, GibbonJavaFIBSWriterClass))

/**
 * GibbonJavaFIBSWriter:
 *
 * One instance of a #GibbonJavaFIBSWriter.  All properties are private.
 */
typedef struct _GibbonJavaFIBSWriter GibbonJavaFIBSWriter;
struct _GibbonJavaFIBSWriter
{
        GibbonMatchWriter parent_instance;
};

/**
 * GibbonJavaFIBSWriterClass:
 *
 * Writer for JavaFIBS internal format.
 */
typedef struct _GibbonJavaFIBSWriterClass GibbonJavaFIBSWriterClass;
struct _GibbonJavaFIBSWriterClass
{
        /* <private >*/
        GibbonMatchWriterClass parent_class;
};

GType gibbon_java_fibs_writer_get_type (void) G_GNUC_CONST;

GibbonJavaFIBSWriter *gibbon_java_fibs_writer_new (void);

#endif
