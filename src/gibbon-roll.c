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
 * SECTION:gibbon-roll
 * @short_description: FIXME! Short description missing!
 *
 * Since: 0.1.0
 *
 * FIXME! Long description missing!
 */

#include <glib.h>
#include <glib/gi18n.h>

#include "gibbon-roll.h"

G_DEFINE_TYPE (GibbonRoll, gibbon_roll, GIBBON_TYPE_GAME_ACTION)

static void 
gibbon_roll_init (GibbonRoll *self)
{
}

static void
gibbon_roll_finalize (GObject *object)
{
        G_OBJECT_CLASS (gibbon_roll_parent_class)->finalize(object);
}

static void
gibbon_roll_class_init (GibbonRollClass *klass)
{
        GObjectClass *object_class = G_OBJECT_CLASS (klass);

        object_class->finalize = gibbon_roll_finalize;
}

/**
 * gibbon_roll_new:
 * @die1: Value of the left die (1 to 6)
 * @die2: Value of the right die (1 to 6)
 *
 * Creates a new #GibbonRoll.
 *
 * Returns: The newly created #GibbonRoll or %NULL in case of failure.
 */
GibbonRoll *
gibbon_roll_new (guint die1, guint die2)
{
        GibbonRoll *self;

        g_return_val_if_fail (die1 != 0, NULL);
        g_return_val_if_fail (die2 != 0, NULL);
        g_return_val_if_fail (die1 <= 6, NULL);
        g_return_val_if_fail (die2 <= 6, NULL);

        self = g_object_new (GIBBON_TYPE_ROLL, NULL);

        self->die1 = die1;
        self->die2 = die2;

        return self;
}
