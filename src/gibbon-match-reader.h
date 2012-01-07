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

#ifndef _GIBBON_MATCH_READER_H
# define _GIBBON_MATCH_READER_H

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <glib.h>
#include <glib-object.h>

#include "gibbon-match.h"

#define GIBBON_TYPE_MATCH_READER \
        (gibbon_match_reader_get_type ())
#define GIBBON_MATCH_READER(obj) \
        (G_TYPE_CHECK_INSTANCE_CAST ((obj), GIBBON_TYPE_MATCH_READER, \
                GibbonMatchReader))
#define GIBBON_MATCH_READER_CLASS(klass) (G_TYPE_CHECK_CLASS_CAST ((klass), \
        GIBBON_TYPE_MATCH_READER, GibbonMatchReaderClass))
#define GIBBON_IS_MATCH_READER(obj) \
        (G_TYPE_CHECK_INSTANCE_TYPE ((obj), \
                GIBBON_TYPE_MATCH_READER))
#define GIBBON_IS_MATCH_READER_CLASS(klass) \
        (G_TYPE_CHECK_CLASS_TYPE ((klass), \
                GIBBON_TYPE_MATCH_READER))
#define GIBBON_MATCH_READER_GET_CLASS(obj) \
        (G_TYPE_INSTANCE_GET_CLASS ((obj), \
                GIBBON_TYPE_MATCH_READER, GibbonMatchReaderClass))

typedef void (*GibbonMatchReaderErrorFunc) (gpointer user_data,
                                            const gchar *msg);

/**
 * GibbonMatchReader:
 *
 * One instance of a #GibbonMatchReader.  All properties are private.
 */
typedef struct _GibbonMatchReader GibbonMatchReader;
struct _GibbonMatchReader
{
        GObject parent_instance;
};

/**
 * GibbonMatchReaderClass:
 * @parse: Parse the given filename or %NULL for standard input.
 *
 * IMPORTANT: The @parse method is usually NOT thread-safe!
 *
 * Abstract base class for readers for backgammon match files.
 */
typedef struct _GibbonMatchReaderClass GibbonMatchReaderClass;
struct _GibbonMatchReaderClass
{
        /* <private >*/
        GObjectClass parent_class;

        /* <public> */
        GibbonMatch * (*parse) (GibbonMatchReader *self, const gchar *filename);
};

GType gibbon_match_reader_get_type (void) G_GNUC_CONST;

GibbonMatch *gibbon_match_reader_parse (GibbonMatchReader *self,
                                        const gchar *filename);

#endif
