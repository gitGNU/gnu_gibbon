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

#ifndef _GIBBON_CLIP_READER_H
# define _GIBBON_CLIP_READER_H

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <glib.h>
#include <glib-object.h>

#define GIBBON_TYPE_CLIP_READER \
        (gibbon_clip_reader_get_type ())
#define GIBBON_CLIP_READER(obj) \
        (G_TYPE_CHECK_INSTANCE_CAST ((obj), GIBBON_TYPE_CLIP_READER, \
                GibbonClipReader))
#define GIBBON_CLIP_READER_CLASS(klass) (G_TYPE_CHECK_CLASS_CAST ((klass), \
        GIBBON_TYPE_CLIP_READER, GibbonClipReaderClass))
#define GIBBON_IS_CLIP_READER(obj) \
        (G_TYPE_CHECK_INSTANCE_TYPE ((obj), \
                GIBBON_TYPE_CLIP_READER))
#define GIBBON_IS_CLIP_READER_CLASS(klass) \
        (G_TYPE_CHECK_CLASS_TYPE ((klass), \
                GIBBON_TYPE_CLIP_READER))
#define GIBBON_CLIP_READER_GET_CLASS(obj) \
        (G_TYPE_INSTANCE_GET_CLASS ((obj), \
                GIBBON_TYPE_CLIP_READER, GibbonClipReaderClass))

/**
 * GibbonClipReader:
 *
 * One instance of a #GibbonClipReader.  All properties are private.
 */
typedef struct _GibbonClipReader GibbonClipReader;
struct _GibbonClipReader
{
        GObject parent_instance;

        /*< private >*/
        struct _GibbonClipReaderPrivate *priv;
};

/**
 * GibbonClipReaderClass:
 *
 * Parse FIBS server output.
 */
typedef struct _GibbonClipReaderClass GibbonClipReaderClass;
struct _GibbonClipReaderClass
{
        /* <private >*/
        GObjectClass parent_class;
};

GType gibbon_clip_reader_get_type (void) G_GNUC_CONST;

GibbonClipReader *gibbon_clip_reader_new (int (*handler) (gpointer,
                                                          GSList *,
                                                          const gchar *),
                                          gpointer data);
void gibbon_clip_reader_on_data (GibbonClipReader *self, gchar *data,
                                 gsize length);

#endif
