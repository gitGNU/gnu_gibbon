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

#ifndef _GIBBON_ROLL_H
# define _GIBBON_ROLL_H

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <glib.h>
#include <glib-object.h>

#include "gibbon-game-action.h"

#define GIBBON_TYPE_ROLL \
        (gibbon_roll_get_type ())
#define GIBBON_ROLL(obj) \
        (G_TYPE_CHECK_INSTANCE_CAST ((obj), GIBBON_TYPE_ROLL, \
                GibbonRoll))
#define GIBBON_ROLL_CLASS(klass) (G_TYPE_CHECK_CLASS_CAST ((klass), \
        GIBBON_TYPE_ROLL, GibbonRollClass))
#define GIBBON_IS_ROLL(obj) \
        (G_TYPE_CHECK_INSTANCE_TYPE ((obj), \
                GIBBON_TYPE_ROLL))
#define GIBBON_IS_ROLL_CLASS(klass) \
        (G_TYPE_CHECK_CLASS_TYPE ((klass), \
                GIBBON_TYPE_ROLL))
#define GIBBON_ROLL_GET_CLASS(obj) \
        (G_TYPE_INSTANCE_GET_CLASS ((obj), \
                GIBBON_TYPE_ROLL, GibbonRollClass))

/**
 * GibbonRoll:
 *
 * One instance of a #GibbonRoll.
 */
typedef struct _GibbonRoll GibbonRoll;
struct _GibbonRoll
{
        GibbonGameAction parent_instance;

        guint die1;
        guint die2;
};

/**
 * GibbonRollClass:
 *
 * Abstraction for a dice roll!
 */
typedef struct _GibbonRollClass GibbonRollClass;
struct _GibbonRollClass
{
        /* <private >*/
        GibbonGameActionClass parent_class;
};

GType gibbon_roll_get_type (void) G_GNUC_CONST;

GibbonRoll *gibbon_roll_new (guint die1, guint die2);

#endif
