/*
 * This file is part of Gibbon, a graphical frontend to the First Internet 
 * Backgammon Server FIBS.
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

#ifndef _LIBGSGF_MOVE_H
# define _LIBGSGF_MOVE_H

#include <glib.h>
#include <gio/gio.h>

G_BEGIN_DECLS

#define GSGF_TYPE_MOVE             (gsgf_move_get_type ())
#define GSGF_MOVE(obj)             (G_TYPE_CHECK_INSTANCE_CAST ((obj), \
		GSGF_TYPE_MOVE, GSGFMove))
#define GSGF_MOVE_CLASS(klass)     (G_TYPE_CHECK_CLASS_CAST ((klass), \
		GSGF_TYPE_MOVE, GSGFMoveClass))
#define GSGF_IS_MOVE(obj)          (G_TYPE_CHECK_INSTANCE_TYPE ((obj), \
		GSGF_TYPE_MOVE))
#define GSGF_IS_MOVE_CLASS(klass)  (G_TYPE_CHECK_CLASS_TYPE ((klass), \
		GSGF_TYPE_MOVE))
#define GSGF_MOVE_GET_CLASS(obj)   (G_TYPE_INSTANCE_GET_CLASS ((obj), \
		GSGF_TYPE_MOVE, GSGFMoveClass))

/**
 * GSGFMove:
 *
 * One instance of a #GSGFMoveClass.
 **/
typedef struct _GSGFMove        GSGFMove;
struct _GSGFMove
{
        /*< private >*/
        GSGFCookedValue parent_instance;
};

/**
 * GSGFMoveClass:
 *
 * Abstract base class representing a move of SGF.
 **/
typedef struct _GSGFMoveClass   GSGFMoveClass;
struct _GSGFMoveClass
{
        /*< private >*/
        GSGFCookedValueClass parent_class;
};

GType gsgf_move_get_type(void) G_GNUC_CONST;

G_END_DECLS

#endif
