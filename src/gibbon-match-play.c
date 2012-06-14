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

#include <glib.h>
#include <glib/gi18n.h>

#include "gibbon-match-play.h"

G_DEFINE_BOXED_TYPE (GibbonMatchPlay, gibbon_match_play,            \
                     NULL, gibbon_match_play_free)

GibbonMatchPlay *
gibbon_match_play_new (GibbonGameAction *action, GibbonPositionSide side)
{
        GibbonMatchPlay *self = g_malloc (sizeof *self);

        self->action = action;
        self->side = side;

        return self;
}

void
gibbon_match_play_free (GibbonMatchPlay *self)
{
        if (self) {
                if (self->action)
                        g_object_unref (self->action);

                g_free (self);
        }
}
