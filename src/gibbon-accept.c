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
 * SECTION:gibbon-accept
 * @short_description: Abstraction for an accepted resignation!
 *
 * Since: 0.1.1
 *
 * A #GibbonGameAction that represents an accepted resignation.
 */

#include <glib.h>
#include <glib/gi18n.h>

#include "gibbon-accept.h"

G_DEFINE_TYPE (GibbonAccept, gibbon_accept, GIBBON_TYPE_GAME_ACTION)

static void 
gibbon_accept_init (GibbonAccept *self)
{
}

static void
gibbon_accept_finalize (GObject *object)
{
        G_OBJECT_CLASS (gibbon_accept_parent_class)->finalize(object);
}

static void
gibbon_accept_class_init (GibbonAcceptClass *klass)
{
        GObjectClass *object_class = G_OBJECT_CLASS (klass);

        object_class->finalize = gibbon_accept_finalize;
}

/**
 * gibbon_accept_new:
 *
 * Creates a new #GibbonAccept.
 *
 * Returns: The newly created #GibbonAccept or %NULL in case of failure.
 */
GibbonAccept *
gibbon_accept_new ()
{
        GibbonAccept *self = g_object_new (GIBBON_TYPE_ACCEPT, NULL);

        return self;
}
