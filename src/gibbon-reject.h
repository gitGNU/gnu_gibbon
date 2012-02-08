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

#ifndef _GIBBON_REJECT_H
# define _GIBBON_REJECT_H

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <glib.h>
#include <glib-object.h>

#include "gibbon-game-action.h"

#define GIBBON_TYPE_REJECT \
        (gibbon_reject_get_type ())
#define GIBBON_REJECT(obj) \
        (G_TYPE_CHECK_INSTANCE_CAST ((obj), GIBBON_TYPE_REJECT, \
                GibbonReject))
#define GIBBON_REJECT_CLASS(klass) (G_TYPE_CHECK_CLASS_CAST ((klass), \
        GIBBON_TYPE_REJECT, GibbonRejectClass))
#define GIBBON_IS_REJECT(obj) \
        (G_TYPE_CHECK_INSTANCE_TYPE ((obj), \
                GIBBON_TYPE_REJECT))
#define GIBBON_IS_REJECT_CLASS(klass) \
        (G_TYPE_CHECK_CLASS_TYPE ((klass), \
                GIBBON_TYPE_REJECT))
#define GIBBON_REJECT_GET_CLASS(obj) \
        (G_TYPE_INSTANCE_GET_CLASS ((obj), \
                GIBBON_TYPE_REJECT, GibbonRejectClass))

/**
 * GibbonReject:
 *
 * One instance of a #GibbonReject.  All properties are private.
 */
typedef struct _GibbonReject GibbonReject;
struct _GibbonReject
{
        GibbonGameAction parent_instance;
};

/**
 * GibbonRejectClass:
 *
 * Abstraction for a rejected resignation!
 */
typedef struct _GibbonRejectClass GibbonRejectClass;
struct _GibbonRejectClass
{
        /* <private >*/
        GibbonGameActionClass parent_class;
};

GType gibbon_reject_get_type (void) G_GNUC_CONST;

GibbonReject *gibbon_reject_new ();

#endif
