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
 * SECTION:gibbon-setup
 * @short_description: Abstraction for a game starting at an unknown position.
 *
 * Since: 0.1.0
 *
 * This #GibbonGameSetup is used, when a game starts at an unknown position.
 * This happens, when we cannot find a saved game on disk, or when one or more
 * game actions are missing in our saved match.
 */

#include <glib.h>
#include <glib/gi18n.h>

#include "gibbon-setup.h"
#include "gibbon-position.h"

G_DEFINE_TYPE (GibbonSetup, gibbon_setup, GIBBON_TYPE_GAME_ACTION)

static void 
gibbon_setup_init (GibbonSetup *self)
{
        self->position = NULL;
}

static void
gibbon_setup_finalize (GObject *object)
{
        GibbonSetup *self = GIBBON_SETUP (object);

        if (self->position)
                gibbon_position_free (self->position);

        G_OBJECT_CLASS (gibbon_setup_parent_class)->finalize(object);
}

static void
gibbon_setup_class_init (GibbonSetupClass *klass)
{
        GObjectClass *object_class = G_OBJECT_CLASS (klass);

        object_class->finalize = gibbon_setup_finalize;
}

/**
 * gibbon_setup_new:
 * @position: The #GibbonPosition at that point.
 *
 * Creates a new #GibbonSetup.
 *
 * Returns: The newly created #GibbonSetup or %NULL in case of failure.
 */
GibbonSetup *
gibbon_setup_new (GibbonPosition *position)
{
        GibbonSetup *self = g_object_new (GIBBON_TYPE_SETUP, NULL);

        self->position = position;

        return self;
}
