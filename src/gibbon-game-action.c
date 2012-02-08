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
 * SECTION:gibbon-game-action
 * @short_description: FIXME! Short description missing!
 *
 * Since: 0.1.1
 *
 * A GibbonGameAction is an abstraction for all different kinds of things that
 * you may do during a game, that is doing a complete move, rolling the dice,
 * doubling, resiging, or accepting or resigning such proposals.
 */

#include <glib.h>
#include <glib/gi18n.h>

#include "gibbon-game-action.h"

G_DEFINE_TYPE (GibbonGameAction, gibbon_game_action, G_TYPE_OBJECT)

static void 
gibbon_game_action_init (GibbonGameAction *self)
{
}

static void
gibbon_game_action_finalize (GObject *object)
{
        G_OBJECT_CLASS (gibbon_game_action_parent_class)->finalize(object);
}

static void
gibbon_game_action_class_init (GibbonGameActionClass *klass)
{
        GObjectClass *object_class = G_OBJECT_CLASS (klass);

        object_class->finalize = gibbon_game_action_finalize;
}
