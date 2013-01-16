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
                GibbonCLIPReader))
#define GIBBON_CLIP_READER_CLASS(klass) (G_TYPE_CHECK_CLASS_CAST ((klass), \
        GIBBON_TYPE_CLIP_READER, GibbonCLIPReaderClass))
#define GIBBON_IS_CLIP_READER(obj) \
        (G_TYPE_CHECK_INSTANCE_TYPE ((obj), \
                GIBBON_TYPE_CLIP_READER))
#define GIBBON_IS_CLIP_READER_CLASS(klass) \
        (G_TYPE_CHECK_CLASS_TYPE ((klass), \
                GIBBON_TYPE_CLIP_READER))
#define GIBBON_CLIP_READER_GET_CLASS(obj) \
        (G_TYPE_INSTANCE_GET_CLASS ((obj), \
                GIBBON_TYPE_CLIP_READER, GibbonCLIPReaderClass))

/**
 * GibbonCLIPReader:
 *
 * One instance of a #GibbonCLIPReader, a stateful line-based parser for
 * FIBS output.
 */
typedef struct _GibbonCLIPReader GibbonCLIPReader;
struct _GibbonCLIPReader
{
        GObject parent_instance;

        /*< private >*/
        struct _GibbonCLIPReaderPrivate *priv;
};

/**
 * GibbonCLIPReaderClass:
 *
 * Parse FIBS server output.
 */
typedef struct _GibbonCLIPReaderClass GibbonCLIPReaderClass;
struct _GibbonCLIPReaderClass
{
        /* <private >*/
        GObjectClass parent_class;
};

GType gibbon_clip_reader_get_type (void) G_GNUC_CONST;

GibbonCLIPReader *gibbon_clip_reader_new ();
GSList *gibbon_clip_reader_parse (GibbonCLIPReader *self, const gchar *line);

#endif
