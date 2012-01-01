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
 * SECTION:gibbon-take
 * @short_description: Abstraction for a taken cube!
 *
 * Since: 0.1.1
 *
 * A #GibbonGameAction that represents a dropped cube.
 */

#include <glib.h>
#include <glib/gi18n.h>

#include "gibbon-take.h"

G_DEFINE_TYPE (GibbonTake, gibbon_take, GIBBON_TYPE_GAME_ACTION)

static void 
gibbon_take_init (GibbonTake *self)
{
}

static void
gibbon_take_finalize (GObject *object)
{
        G_OBJECT_CLASS (gibbon_take_parent_class)->finalize(object);
}

static void
gibbon_take_class_init (GibbonTakeClass *klass)
{
        GObjectClass *object_class = G_OBJECT_CLASS (klass);

        object_class->finalize = gibbon_take_finalize;
}

/**
 * gibbon_take_new:
 *
 * Creates a new #GibbonTake.
 *
 * Returns: The newly created #GibbonTake or %NULL in case of failure.
 */
GibbonTake *
gibbon_take_new ()
{
        GibbonTake *self = g_object_new (GIBBON_TYPE_TAKE, NULL);

        return self;
}
