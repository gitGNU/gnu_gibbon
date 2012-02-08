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

/**
 * SECTION:gibbon-double
 * @short_description: Abstraction for a double!
 *
 * Since: 0.1.1
 *
 * A GibbonDouble is a GibbonGameAction to represent a turned cube.  It is
 * more or less a strongly typed constant without any functionality.
 */

#include <glib.h>
#include <glib/gi18n.h>

#include "gibbon-double.h"
#include "gibbon-game-action.h"

G_DEFINE_TYPE (GibbonDouble, gibbon_double, GIBBON_TYPE_GAME_ACTION)

static void 
gibbon_double_init (GibbonDouble *self)
{
}

static void
gibbon_double_finalize (GObject *object)
{
        G_OBJECT_CLASS (gibbon_double_parent_class)->finalize(object);
}

static void
gibbon_double_class_init (GibbonDoubleClass *klass)
{
        GObjectClass *object_class = G_OBJECT_CLASS (klass);

        object_class->finalize = gibbon_double_finalize;
}

/**
 * gibbon_double_new:
 *
 * Creates a new #GibbonDouble.
 *
 * Returns: The newly created #GibbonDouble or %NULL in case of failure.
 */
GibbonDouble *
gibbon_double_new ()
{
        GibbonDouble *self = g_object_new (GIBBON_TYPE_DOUBLE, NULL);

        return self;
}
