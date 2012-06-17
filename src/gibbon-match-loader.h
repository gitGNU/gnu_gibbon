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

#ifndef _GIBBON_MATCH_LOADER_H
# define _GIBBON_MATCH_LOADER_H

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <glib.h>
#include <glib-object.h>

#include "gibbon-match.h"

#define GIBBON_TYPE_MATCH_LOADER \
        (gibbon_match_loader_get_type ())
#define GIBBON_MATCH_LOADER(obj) \
        (G_TYPE_CHECK_INSTANCE_CAST ((obj), GIBBON_TYPE_MATCH_LOADER, \
                GibbonMatchLoader))
#define GIBBON_MATCH_LOADER_CLASS(klass) (G_TYPE_CHECK_CLASS_CAST ((klass), \
        GIBBON_TYPE_MATCH_LOADER, GibbonMatchLoaderClass))
#define GIBBON_IS_MATCH_LOADER(obj) \
        (G_TYPE_CHECK_INSTANCE_TYPE ((obj), \
                GIBBON_TYPE_MATCH_LOADER))
#define GIBBON_IS_MATCH_LOADER_CLASS(klass) \
        (G_TYPE_CHECK_CLASS_TYPE ((klass), \
                GIBBON_TYPE_MATCH_LOADER))
#define GIBBON_MATCH_LOADER_GET_CLASS(obj) \
        (G_TYPE_INSTANCE_GET_CLASS ((obj), \
                GIBBON_TYPE_MATCH_LOADER, GibbonMatchLoaderClass))

/**
 * GibbonMatchLoader:
 *
 * One instance of a #GibbonMatchLoader.  All properties are private.
 */
typedef struct _GibbonMatchLoader GibbonMatchLoader;
struct _GibbonMatchLoader
{
        GObject parent_instance;

        /*< private >*/
        struct _GibbonMatchLoaderPrivate *priv;
};

/**
 * GibbonMatchLoaderClass:
 *
 * Load a backgammon match for display in Gibbon.
 */
typedef struct _GibbonMatchLoaderClass GibbonMatchLoaderClass;
struct _GibbonMatchLoaderClass
{
        /* <private >*/
        GObjectClass parent_class;
};

GType gibbon_match_loader_get_type (void) G_GNUC_CONST;

GibbonMatchLoader *gibbon_match_loader_new (void);
GibbonMatch *gibbon_match_loader_read_match (GibbonMatchLoader *loader,
                                             const gchar *filename,
                                             GError **error);

#endif
