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

#ifndef _GIBBON_GAME_ACTION_H
# define _GIBBON_GAME_ACTION_H

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <glib.h>
#include <glib-object.h>

#define GIBBON_TYPE_GAME_ACTION \
        (gibbon_game_action_get_type ())
#define GIBBON_GAME_ACTION(obj) \
        (G_TYPE_CHECK_INSTANCE_CAST ((obj), GIBBON_TYPE_GAME_ACTION, \
                GibbonGameAction))
#define GIBBON_GAME_ACTION_CLASS(klass) (G_TYPE_CHECK_CLASS_CAST ((klass), \
        GIBBON_TYPE_GAME_ACTION, GibbonGameActionClass))
#define GIBBON_IS_GAME_ACTION(obj) \
        (G_TYPE_CHECK_INSTANCE_TYPE ((obj), \
                GIBBON_TYPE_GAME_ACTION))
#define GIBBON_IS_GAME_ACTION_CLASS(klass) \
        (G_TYPE_CHECK_CLASS_TYPE ((klass), \
                GIBBON_TYPE_GAME_ACTION))
#define GIBBON_GAME_ACTION_GET_CLASS(obj) \
        (G_TYPE_INSTANCE_GET_CLASS ((obj), \
                GIBBON_TYPE_GAME_ACTION, GibbonGameActionClass))

/**
 * GibbonGameAction:
 *
 * One instance of a #GibbonGameAction.  All properties are private.
 */
typedef struct _GibbonGameAction GibbonGameAction;
struct _GibbonGameAction
{
        GObject parent_instance;
};

/**
 * GibbonGameActionClass:
 *
 * Abstract base class for all different kinds of things you can do during a
 * backgammon game.
 */
typedef struct _GibbonGameActionClass GibbonGameActionClass;
struct _GibbonGameActionClass
{
        /* <private >*/
        GObjectClass parent_class;
};

GType gibbon_game_action_get_type (void) G_GNUC_CONST;

#endif
