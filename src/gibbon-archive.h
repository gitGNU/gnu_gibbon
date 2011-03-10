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

#ifndef _GIBBON_ARCHIVE_H
# define _GIBBON_ARCHIVE_H

#include <gtk/gtk.h>

#include "gibbon-connection.h"

#define GIBBON_TYPE_ARCHIVE \
        (gibbon_archive_get_type ())
#define GIBBON_ARCHIVE(obj) \
        (G_TYPE_CHECK_INSTANCE_CAST ((obj), GIBBON_TYPE_ARCHIVE, \
                GibbonArchive))
#define GIBBON_ARCHIVE_CLASS(klass) (G_TYPE_CHECK_CLASS_CAST ((klass), \
        GIBBON_TYPE_ARCHIVE, GibbonArchiveClass))
#define GIBBON_IS_ARCHIVE(obj) \
        (G_TYPE_CHECK_INSTANCE_TYPE ((obj), \
                GIBBON_TYPE_ARCHIVE))
#define GIBBON_IS_ARCHIVE_CLASS(klass) \
        (G_TYPE_CHECK_CLASS_TYPE ((klass), \
                GIBBON_TYPE_ARCHIVE))
#define GIBBON_ARCHIVE_GET_CLASS(obj) \
        (G_TYPE_INSTANCE_GET_CLASS ((obj), \
                GIBBON_TYPE_ARCHIVE, GibbonArchiveClass))

/**
 * GibbonArchive:
 *
 * One instance of a #GibbonArchive.  All properties are private.
 **/
typedef struct _GibbonArchive GibbonArchive;
struct _GibbonArchive
{
        GObject parent_instance;

        /*< private >*/
        struct _GibbonArchivePrivate *priv;
};

/**
 * GibbonArchiveClass:
 *
 * FIXME! The author was negligent enough to not document this class!
 **/
typedef struct _GibbonArchiveClass GibbonArchiveClass;
struct _GibbonArchiveClass
{
        /* <private >*/
        GObjectClass parent_class;
};

GType gibbon_archive_get_type (void) G_GNUC_CONST;

GibbonArchive *gibbon_archive_new (GibbonConnection *connection);
GibbonArchive *gibbon_archive_new_from_session_info (const gchar *host,
                                                     guint port,
                                                     const gchar *login);

#endif