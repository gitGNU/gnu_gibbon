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

#include <glib/gi18n.h>
#include <gtk/gtk.h>

#include "gibbon-design.h"

struct _GibbonDesignPrivate {
        gint dummy;
};

#define GIBBON_DESIGN_PRIVATE(o) (G_TYPE_INSTANCE_GET_PRIVATE ((o), \
                                  GIBBON_TYPE_DESIGN,           \
                                  GibbonDesignPrivate))
G_DEFINE_TYPE (GibbonDesign, gibbon_design, G_TYPE_OBJECT);

static void
gibbon_design_init (GibbonDesign *self)
{
        self->priv = G_TYPE_INSTANCE_GET_PRIVATE (self, 
                                                  GIBBON_TYPE_DESIGN, 
                                                  GibbonDesignPrivate);
}

static void
gibbon_design_finalize (GObject *object)
{
/*        GibbonDesign *self = GIBBON_DESIGN (object); */

        G_OBJECT_CLASS (gibbon_design_parent_class)->finalize (object);
}

static void
gibbon_design_class_init (GibbonDesignClass *klass)
{
        GtkDrawingAreaClass* parent_class = G_OBJECT_CLASS (klass);

        g_type_class_add_private (klass, sizeof (GibbonDesignPrivate));

        G_OBJECT_CLASS (parent_class)->finalize = gibbon_design_finalize;
}

GibbonDesign *
gibbon_design_new ()
{
        GibbonDesign *self = g_object_new (GIBBON_TYPE_DESIGN, NULL);

        return self;
}
