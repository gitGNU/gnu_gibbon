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

#ifndef _GIBBON_TAKE_H
# define _GIBBON_TAKE_H

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <glib.h>
#include <glib-object.h>

#include "gibbon-game-action.h"

#define GIBBON_TYPE_TAKE \
        (gibbon_take_get_type ())
#define GIBBON_TAKE(obj) \
        (G_TYPE_CHECK_INSTANCE_CAST ((obj), GIBBON_TYPE_TAKE, \
                GibbonTake))
#define GIBBON_TAKE_CLASS(klass) (G_TYPE_CHECK_CLASS_CAST ((klass), \
        GIBBON_TYPE_TAKE, GibbonTakeClass))
#define GIBBON_IS_TAKE(obj) \
        (G_TYPE_CHECK_INSTANCE_TYPE ((obj), \
                GIBBON_TYPE_TAKE))
#define GIBBON_IS_TAKE_CLASS(klass) \
        (G_TYPE_CHECK_CLASS_TYPE ((klass), \
                GIBBON_TYPE_TAKE))
#define GIBBON_TAKE_GET_CLASS(obj) \
        (G_TYPE_INSTANCE_GET_CLASS ((obj), \
                GIBBON_TYPE_TAKE, GibbonTakeClass))

/**
 * GibbonTake:
 *
 * One instance of a #GibbonTake.  All properties are private.
 */
typedef struct _GibbonTake GibbonTake;
struct _GibbonTake
{
        GibbonGameAction parent_instance;
};

/**
 * GibbonTakeClass:
 *
 * Abstraction for a taken cube!
 */
typedef struct _GibbonTakeClass GibbonTakeClass;
struct _GibbonTakeClass
{
        /* <private >*/
        GibbonGameActionClass parent_class;
};

GType gibbon_take_get_type (void) G_GNUC_CONST;

GibbonTake *gibbon_take_new ();

#endif
