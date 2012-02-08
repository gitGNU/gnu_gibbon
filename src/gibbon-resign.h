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

#ifndef _GIBBON_RESIGN_H
# define _GIBBON_RESIGN_H

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <glib.h>
#include <glib-object.h>

#include "gibbon-game-action.h"

#define GIBBON_TYPE_RESIGN \
        (gibbon_resign_get_type ())
#define GIBBON_RESIGN(obj) \
        (G_TYPE_CHECK_INSTANCE_CAST ((obj), GIBBON_TYPE_RESIGN, \
                GibbonResign))
#define GIBBON_RESIGN_CLASS(klass) (G_TYPE_CHECK_CLASS_CAST ((klass), \
        GIBBON_TYPE_RESIGN, GibbonResignClass))
#define GIBBON_IS_RESIGN(obj) \
        (G_TYPE_CHECK_INSTANCE_TYPE ((obj), \
                GIBBON_TYPE_RESIGN))
#define GIBBON_IS_RESIGN_CLASS(klass) \
        (G_TYPE_CHECK_CLASS_TYPE ((klass), \
                GIBBON_TYPE_RESIGN))
#define GIBBON_RESIGN_GET_CLASS(obj) \
        (G_TYPE_INSTANCE_GET_CLASS ((obj), \
                GIBBON_TYPE_RESIGN, GibbonResignClass))

/**
 * GibbonResign:
 *
 * One instance of a #GibbonResign.
 */
typedef struct _GibbonResign GibbonResign;
struct _GibbonResign
{
        GibbonGameAction parent_instance;

        guint value;
};

/**
 * GibbonResignClass:
 *
 * Abstraction for a resignation offer!
 */
typedef struct _GibbonResignClass GibbonResignClass;
struct _GibbonResignClass
{
        /* <private >*/
        GibbonGameActionClass parent_class;
};

GType gibbon_resign_get_type (void) G_GNUC_CONST;

GibbonResign *gibbon_resign_new (guint value);

#endif
