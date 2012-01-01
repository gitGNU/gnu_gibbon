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
 * SECTION:gibbon-resign
 * @short_description: Abstraction for a resignation offer.
 *
 * Since: 0.1.1
 *
 * A #GibbonGameAction that represents a resignation offer.
 */

#include <glib.h>
#include <glib/gi18n.h>

#include "gibbon-resign.h"

G_DEFINE_TYPE (GibbonResign, gibbon_resign, GIBBON_TYPE_GAME_ACTION)

static void 
gibbon_resign_init (GibbonResign *self)
{
        self->value = 0;
}

static void
gibbon_resign_finalize (GObject *object)
{
        G_OBJECT_CLASS (gibbon_resign_parent_class)->finalize(object);
}

static void
gibbon_resign_class_init (GibbonResignClass *klass)
{
        GObjectClass *object_class = G_OBJECT_CLASS (klass);
        GibbonGameActionClass *gibbon_game_action_class = GIBBON_GAME_ACTION_CLASS (klass);

        object_class->finalize = gibbon_resign_finalize;
}

/**
 * gibbon_resign_new:
 * @value: Value of the resignation (1 for normal, 2 for gammon,
 *         3 for backgammon).
 *
 * Creates a new #GibbonResign.
 *
 * Returns: The newly created #GibbonResign or %NULL in case of failure.
 */
GibbonResign *
gibbon_resign_new (guint value)
{
        GibbonResign *self;

        g_return_val_if_fail (value != 0, NULL);
        g_return_val_if_fail (value <= 3, NULL);

        self = g_object_new (GIBBON_TYPE_RESIGN, NULL);

        self->value = value;

        return self;
}
