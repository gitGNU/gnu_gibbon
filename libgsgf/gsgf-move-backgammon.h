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

#ifndef _LIBGSGF_MOVE_BACKGAMMON_H
# define _LIBGSGF_MOVE_BACKGAMMON_H

#include <glib.h>
#include <gio/gio.h>

#include <libgsgf/gsgf-move.h>

G_BEGIN_DECLS

#define GSGF_TYPE_MOVE_BACKGAMMON  			  \
	(gsgf_move_backgammon_get_type ())
#define GSGF_MOVE_BACKGAMMON(obj)             \
	(G_TYPE_CHECK_INSTANCE_CAST ((obj), GSGF_TYPE_MOVE_BACKGAMMON, \
			GSGFMoveBackgammon))
#define GSGF_MOVE_BACKGAMMON_CLASS(klass)     \
	(G_TYPE_CHECK_CLASS_CAST ((klass), GSGF_TYPE_MOVE_BACKGAMMON, \
			GSGFMoveBackgammonClass))
#define GSGF_IS_MOVE_BACKGAMMON(obj)          \
	(G_TYPE_CHECK_INSTANCE_TYPE ((obj), GSGF_TYPE_MOVE_BACKGAMMON))
#define GSGF_IS_MOVE_BACKGAMMON_CLASS(klass)  \
	(G_TYPE_CHECK_CLASS_TYPE ((klass), GSGF_TYPE_MOVE_BACKGAMMON))
#define GSGF_MOVE_BACKGAMMON_GET_CLASS(obj)   \
	(G_TYPE_INSTANCE_GET_CLASS ((obj), GSGF_TYPE_MOVE_BACKGAMMON, \
			GSGFMoveBackgammonClass))

/**
 * GSGFMoveBackgammon:
 *
 * Instance of #GSGFMoveBackgammonClass.  All properties are private.
 **/
typedef struct _GSGFMoveBackgammon        GSGFMoveBackgammon;
struct _GSGFMoveBackgammon
{
        GSGFMove parent_instance;

        /*< private >*/
        struct _GSGFMoveBackgammonPrivate *priv;
};


/**
 * GSGFMoveBackgammonClass:
 *
 * Class implementing the backgammon move of SGF.
 **/
typedef struct _GSGFMoveBackgammonClass   GSGFMoveBackgammonClass;
struct _GSGFMoveBackgammonClass
{
        /*< private >*/
        GSGFMoveClass parent_class;
};

GType gsgf_move_backgammon_get_type (void) G_GNUC_CONST;

GSGFMoveBackgammon *gsgf_move_backgammon_new (void);
GSGFMoveBackgammon *gsgf_move_backgammon_new_from_raw (const GSGFRaw *raw,
                                                       GError **error);
GSGFMoveBackgammon *gsgf_move_backgammon_new_from_string (const gchar *str,
                                                          GError **error);
GSGFMoveBackgammon *gsgf_move_backgammon_new_regular (guint die1, guint die2,
                                                      GError **error,
                                                      ...);
gboolean gsgf_move_backgammon_is_regular (const GSGFMoveBackgammon *self);
gboolean gsgf_move_backgammon_is_double (const GSGFMoveBackgammon *self);
gboolean gsgf_move_backgammon_is_take (const GSGFMoveBackgammon *self);
gboolean gsgf_move_backgammon_is_drop (const GSGFMoveBackgammon *self);
guint gsgf_move_backgammon_is_resign (const GSGFMoveBackgammon *self);
gboolean gsgf_move_backgammon_is_accept (const GSGFMoveBackgammon *self);
gboolean gsgf_move_backgammon_is_reject (const GSGFMoveBackgammon *self);
gsize gsgf_move_backgammon_get_num_moves (const GSGFMoveBackgammon *self);
guint gsgf_move_backgammon_get_die (const GSGFMoveBackgammon *self, gsize i);
guint gsgf_move_backgammon_get_from (const GSGFMoveBackgammon *self, gsize i);
guint gsgf_move_backgammon_get_to (const GSGFMoveBackgammon *self, gsize i);

G_END_DECLS

#endif
