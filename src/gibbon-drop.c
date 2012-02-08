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
 * SECTION:gibbon-drop
 * @short_description: Abstraction for a dropped cube!
 *
 * Since: 0.1.1
 *
 * A #GibbonGameAction that represents a dropped cube.
 */

#include <glib.h>
#include <glib/gi18n.h>

#include "gibbon-drop.h"

G_DEFINE_TYPE (GibbonDrop, gibbon_drop, GIBBON_TYPE_GAME_ACTION)

static void 
gibbon_drop_init (GibbonDrop *self)
{
}

static void
gibbon_drop_finalize (GObject *object)
{
        G_OBJECT_CLASS (gibbon_drop_parent_class)->finalize(object);
}

static void
gibbon_drop_class_init (GibbonDropClass *klass)
{
        GObjectClass *object_class = G_OBJECT_CLASS (klass);

        object_class->finalize = gibbon_drop_finalize;
}

/**
 * gibbon_drop_new:
 *
 * Creates a new #GibbonDrop.
 *
 * Returns: The newly created #GibbonDrop or %NULL in case of failure.
 */
GibbonDrop *
gibbon_drop_new ()
{
        GibbonDrop *self = g_object_new (GIBBON_TYPE_DROP, NULL);

        return self;
}
