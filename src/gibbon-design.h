/*
 * This file is part of Gibbon, a graphical frontend to the First Internet 
 * Backgammon Server FIBS.
 * Copyright (C) 2009 Guido Flohr, http://guido-flohr.net/.
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

#ifndef _GIBBON_DESIGN_H
#define _GIBBON_DESIGN_H

#include <glib.h>

G_BEGIN_DECLS

#define GIBBON_TYPE_DESIGN             (gibbon_design_get_type ())
#define GIBBON_DESIGN(obj)             (G_TYPE_CHECK_INSTANCE_CAST ((obj), GIBBON_TYPE_DESIGN, GibbonDesign))
#define GIBBON_DESIGN_CLASS(klass)     (G_TYPE_CHECK_CLASS_CAST ((klass), GIBBON_TYPE_DESIGN, GibbonDesignClass))
#define GIBBON_IS_DESIGN(obj)          (G_TYPE_CHECK_INSTANCE_TYPE ((obj), GIBBON_TYPE_DESIGN))
#define GIBBON_IS_DESIGN_CLASS(klass)  (G_TYPE_CHECK_CLASS_TYPE ((klass), GIBBON_TYPE_DESIGN))
#define GIBBON_DESIGN_GET_CLASS(obj)   (G_TYPE_INSTANCE_GET_CLASS ((obj), GIBBON_TYPE_DESIGN, GibbonDesignClass))

typedef struct _GibbonDesignClass   GibbonDesignClass;
typedef struct _GibbonDesign        GibbonDesign;
typedef struct _GibbonDesignPrivate GibbonDesignPrivate;

struct _GibbonDesignClass
{
        GObjectClass parent_class;
};

GType gibbon_design_get_type (void) G_GNUC_CONST;

struct _GibbonDesign
{
        GObject parent_instance;
        GibbonDesignPrivate *priv;
};

struct GibbonColor {
        gdouble red;
        gdouble green;
        gdouble blue;
        gdouble alpha;
};

GibbonDesign *gibbon_design_new (void);
GibbonDesign *gibbon_design_copy (const GibbonDesign *original);

guint gibbon_design_get_box_color (GibbonDesign *self);

/* These are resulting properties that cannot be set.  */
gdouble gibbon_design_get_width (const GibbonDesign *self);
gdouble gibbon_design_get_height (const GibbonDesign *self);
gdouble gibbon_design_get_aspect_ratio (const GibbonDesign *self);


G_END_DECLS

#endif
