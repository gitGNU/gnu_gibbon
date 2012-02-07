/*
 * This file is part of Gibbon, a graphical frontend to the First Internet 
 * Backgammon Server FIBS.
 * Copyright (C) 2009-2012 Guido Flohr, http://guido-flohr.net/.
 *
 * Gibbon is free software: you can redistribute it and/or modify 
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Gibbon is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Gibbon.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef _GIBBON_CAIROBOARD_H
#define _GIBBON_CAIROBOARD_H

#include <glib.h>

#include "gibbon-app.h"
#include "gibbon-position.h"

G_BEGIN_DECLS

#define GIBBON_TYPE_CAIROBOARD             (gibbon_cairoboard_get_type ())
#define GIBBON_CAIROBOARD(obj)             (G_TYPE_CHECK_INSTANCE_CAST ((obj), GIBBON_TYPE_CAIROBOARD, GibbonCairoboard))
#define GIBBON_CAIROBOARD_CLASS(klass)     (G_TYPE_CHECK_CLASS_CAST ((klass), GIBBON_TYPE_CAIROBOARD, GibbonCairoboardClass))
#define GIBBON_IS_CAIROBOARD(obj)          (G_TYPE_CHECK_INSTANCE_TYPE ((obj), GIBBON_TYPE_CAIROBOARD))
#define GIBBON_IS_CAIROBOARD_CLASS(klass)  (G_TYPE_CHECK_CLASS_TYPE ((klass), GIBBON_TYPE_CAIROBOARD))
#define GIBBON_CAIROBOARD_GET_CLASS(obj)   (G_TYPE_INSTANCE_GET_CLASS ((obj), GIBBON_TYPE_CAIROBOARD, GibbonCairoboardClass))

typedef struct _GibbonCairoboardClass   GibbonCairoboardClass;
typedef struct _GibbonCairoboard        GibbonCairoboard;
typedef struct _GibbonCairoboardPrivate GibbonCairoboardPrivate;

struct _GibbonCairoboardClass
{
        GtkDrawingAreaClass parent_class;
};

GType gibbon_cairoboard_get_type (void) G_GNUC_CONST;

struct _GibbonCairoboard
{
        GtkDrawingArea parent_instance;
        GibbonCairoboardPrivate *priv;
};

GibbonCairoboard *gibbon_cairoboard_new (GibbonApp *app, const gchar *path);

G_END_DECLS

#endif
