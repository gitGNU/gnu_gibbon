/*
 * This file is part of Gibbon.
 * Gibbon is a Gtk+ frontend for the First Internet Backgammon Server FIBS.
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

#ifndef _GIBBON_MATCH_PLAY_H
# define _GIBBON_MATCH_PLAY_H

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <glib.h>
#include <glib-object.h>

#include "gibbon-game-action.h"
#include "gibbon-position.h"

#define GIBBON_TYPE_MATCH_PLAY (gibbon_match_play_get_type ())

typedef struct _GibbonMatchPlay GibbonMatchPlay;
struct _GibbonMatchPlay
{
        GibbonGameAction *action;
        GibbonPositionSide side;
};

GType gibbon_match_play_get_type (void) G_GNUC_CONST;

GibbonMatchPlay *gibbon_match_play_new (GibbonGameAction *action,
                                            GibbonPositionSide side);
void gibbon_match_play_free (GibbonMatchPlay *self);

#endif
