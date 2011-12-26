/*
 * This file is part of gibbon.
 * Gibbon is a Gtk+ frontend for the First Internet Backgammon Server FIBS.
 * Copyright (C) 2009-2011 Guido Flohr, http://guido-flohr.net/.
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

#ifndef _GIBBON_MOVE_H
# define _GIBBON_MOVE_H

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <glib.h>
#include <glib-object.h>

#include "gibbon-game-action.h"

#define GIBBON_TYPE_MOVE \
        (gibbon_move_get_type ())
#define GIBBON_MOVE(obj) \
        (G_TYPE_CHECK_INSTANCE_CAST ((obj), GIBBON_TYPE_MOVE, \
                GibbonMove))
#define GIBBON_MOVE_CLASS(klass) (G_TYPE_CHECK_CLASS_CAST ((klass), \
        GIBBON_TYPE_MOVE, GibbonMoveClass))
#define GIBBON_IS_MOVE(obj) \
        (G_TYPE_CHECK_INSTANCE_TYPE ((obj), \
                GIBBON_TYPE_MOVE))
#define GIBBON_IS_MOVE_CLASS(klass) \
        (G_TYPE_CHECK_CLASS_TYPE ((klass), \
                GIBBON_TYPE_MOVE))
#define GIBBON_MOVE_GET_CLASS(obj) \
        (G_TYPE_INSTANCE_GET_CLASS ((obj), \
                GIBBON_TYPE_MOVE, GibbonMoveClass))

/**
 * GibbonMove:
 *
 * One instance of a #GibbonMove.  All properties are private.
 */
typedef struct _GibbonMove GibbonMove;
struct _GibbonMove
{
        struct _GibbonGameAction parent_instance;

        /*< private >*/
        struct _GibbonMovePrivate *priv;
};

/**
 * GibbonMoveClass:
 *
 * A complete move in Gibbon.
 */
typedef struct _GibbonMoveClass GibbonMoveClass;
struct _GibbonMoveClass
{
        /* <private >*/
        struct _GibbonGameActionClass parent_class;
};

GType gibbon_move_get_type (void) G_GNUC_CONST;

GibbonMove *gibbon_move_new (guint die1, guint die2, ...);

#endif
