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

/**
 * SECTION:gsgf-cooked-value
 * @short_description: Abstract base class for qualified data stored in SGF
 * files.
 *
 * A #GSGFCookedValue encapsulates qualified data read from an SGF file.  It
 * is the result of qualifying that data, normally by cooking the top-level
 * #GSGFCollection.
 */

#include <glib.h>
#include <glib/gi18n.h>

#include <libgsgf/gsgf.h>

#define GSGF_COOKED_VALUE_PRIVATE(o) (G_TYPE_INSTANCE_GET_PRIVATE ((o), \
                                      GSGF_TYPE_COOKED_VALUE,           \
                                      GSGFCookedValuePrivate))

G_DEFINE_TYPE (GSGFCookedValue, gsgf_cooked_value, GSGF_TYPE_VALUE)

static void
gsgf_cooked_value_init(GSGFCookedValue *self)
{
}

static void
gsgf_cooked_value_finalize(GObject *object)
{
        G_OBJECT_CLASS (gsgf_cooked_value_parent_class)->finalize(object);
}

static void
gsgf_cooked_value_class_init(GSGFCookedValueClass *klass)
{
        GObjectClass* object_class = G_OBJECT_CLASS (klass);

        object_class->finalize = gsgf_cooked_value_finalize;
}
