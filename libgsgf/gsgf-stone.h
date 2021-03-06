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

#ifndef _LIBGSGF_STONE_H
# define _LIBGSGF_STONE_H

#include <glib.h>
#include <gio/gio.h>

G_BEGIN_DECLS

#define GSGF_TYPE_STONE             (gsgf_stone_get_type ())
#define GSGF_STONE(obj)             (G_TYPE_CHECK_INSTANCE_CAST ((obj), \
                GSGF_TYPE_STONE, GSGFStone))
#define GSGF_STONE_CLASS(klass)     (G_TYPE_CHECK_CLASS_CAST ((klass), \
                GSGF_TYPE_STONE, GSGFStoneClass))
#define GSGF_IS_STONE(obj)          (G_TYPE_CHECK_INSTANCE_TYPE ((obj), \
                GSGF_TYPE_STONE))
#define GSGF_IS_STONE_CLASS(klass)  (G_TYPE_CHECK_CLASS_TYPE ((klass), \
                GSGF_TYPE_STONE))
#define GSGF_STONE_GET_CLASS(obj)   (G_TYPE_INSTANCE_GET_CLASS ((obj), \
                GSGF_TYPE_STONE, GSGFStoneClass))

/**
 * GSGFStone:
 *
 * One instance of a #GSGFStoneClass.  All properties are private.
 **/
typedef struct _GSGFStone        GSGFStone;
struct _GSGFStone
{
        GSGFCookedValue parent_instance;
};

/**
 * GSGFStoneClass:
 *
 * Class representing a stone of SGF.
 **/
typedef struct _GSGFStoneClass   GSGFStoneClass;
struct _GSGFStoneClass
{
        /*< private >*/
        GSGFCookedValueClass parent_class;
};

GType gsgf_stone_get_type(void) G_GNUC_CONST;

G_END_DECLS

#endif
