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

/**
 * SECTION:gsgf-color
 * @short_description: Primitive type for an SGF color.
 *
 * Since: 0.1.1
 *
 * A #GSGFColor encapsulates an SGF color, that is either black or white.
 **/

#include <glib.h>
#include <glib/gi18n.h>

#include <libgsgf/gsgf.h>

typedef struct _GSGFColorPrivate GSGFColorPrivate;
struct _GSGFColorPrivate {
        enum GSGFColorEnum color;
};

#define GSGF_COLOR_PRIVATE(obj) (G_TYPE_INSTANCE_GET_PRIVATE ((obj), \
        GSGF_TYPE_COLOR, GSGFColorPrivate))

G_DEFINE_TYPE (GSGFColor, gsgf_color, GSGF_TYPE_COOKED_VALUE)

static void 
gsgf_color_init (GSGFColor *self)
{        self->priv = G_TYPE_INSTANCE_GET_PRIVATE (self,
                GSGF_TYPE_COLOR, GSGFColorPrivate);

        self->priv->color = GSGF_COLOR_BLACK;
}

static void
gsgf_color_finalize (GSGFColor *object)
{
        GSGFColor *self = GSGF_COLOR (object);

        G_OBJECT_CLASS (gsgf_color_parent_class)->finalize(object);
}

static void
gsgf_color_class_init (GSGFColorClass *klass)
{
        GObjectClass *object_class = G_OBJECT_CLASS (klass);
        GSGFCookedValueClass *gsgf_cooked_value_class = GSGF_COOKED_VALUE_CLASS (klass);

        /* FIXME! Initialize pointers to methods from parent class! */
        /* gsgf_cooked_value_class->do_this = gsgf_color_do_this; */
        
        g_type_class_add_private(klass, sizeof (GSGFColorPrivate));

        object_class->finalize = gsgf_color_finalize;
}

GSGFColor *
gsgf_color_new (enum GSGFColorEnum color)
{
        GSGFColor *self = g_object_new (GSGF_TYPE_COLOR, NULL);

        self->priv->color = color;

        return self;
}
