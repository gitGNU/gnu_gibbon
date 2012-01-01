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

#ifndef _LIBGSGF_STONE_BACKGAMMON_H
# define _LIBGSGF_STONE_BACKGAMMON_H

#include <glib.h>
#include <gio/gio.h>

#include <libgsgf/gsgf-stone.h>

G_BEGIN_DECLS

#define GSGF_TYPE_STONE_BACKGAMMON  (gsgf_stone_backgammon_get_type ())
#define GSGF_STONE_BACKGAMMON(obj)             \
	(G_TYPE_CHECK_INSTANCE_CAST ((obj), GSGF_TYPE_STONE_BACKGAMMON, \
	                GSGFStoneBackgammon))
#define GSGF_STONE_BACKGAMMON_CLASS(klass)     \
	(G_TYPE_CHECK_CLASS_CAST ((klass), GSGF_TYPE_STONE_BACKGAMMON, \
	                GSGFStoneBackgammonClass))
#define GSGF_IS_STONE_BACKGAMMON(obj)         \
	(G_TYPE_CHECK_INSTANCE_TYPE ((obj), GSGF_TYPE_STONE_BACKGAMMON))
#define GSGF_IS_STONE_BACKGAMMON_CLASS(klass)  \
	(G_TYPE_CHECK_CLASS_TYPE ((klass), GSGF_TYPE_STONE_BACKGAMMON))
#define GSGF_STONE_BACKGAMMON_GET_CLASS(obj)   \
	(G_TYPE_INSTANCE_GET_CLASS ((obj), GSGF_TYPE_STONE_BACKGAMMON, \
			GSGFStoneBackgammonClass))

/**
 * GSGFStoneBackgammon:
 *
 * One instance of a #GSGFStoneBackgammonClass.  All properties are private.
 **/
typedef struct _GSGFStoneBackgammon        GSGFStoneBackgammon;
struct _GSGFStoneBackgammon
{
        GSGFStone parent_instance;

        /*< private >*/
        struct _GSGFStoneBackgammonPrivate *priv;
};

/**
 * GSGFStoneBackgammonClass:
 *
 * Class implementing the backgammon stone of SGF.
 **/
typedef struct _GSGFStoneBackgammonClass   GSGFStoneBackgammonClass;
struct _GSGFStoneBackgammonClass
{
        /*< private >*/
        GSGFStoneClass parent_class;
};

GType gsgf_stone_backgammon_get_type(void) G_GNUC_CONST;

GSGFStoneBackgammon *gsgf_stone_backgammon_new(gint stone);
GSGFStoneBackgammon *gsgf_stone_backgammon_new_from_raw(const GSGFRaw *raw,
                                                        gsize i,
                                                        GError **error);

gint gsgf_stone_backgammon_get_stone(const GSGFStoneBackgammon *self);

G_END_DECLS

#endif
