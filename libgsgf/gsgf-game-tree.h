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

#ifndef _LIBGSGF_GAME_TREE_H
# define _LIBGSGF_GAME_TREE_H

#include <glib.h>
#include <gio/gio.h>

G_BEGIN_DECLS

#define GSGF_TYPE_GAME_TREE             (gsgf_game_tree_get_type ())
#define GSGF_GAME_TREE(obj)             (G_TYPE_CHECK_INSTANCE_CAST ((obj), \
		GSGF_TYPE_GAME_TREE, GSGFGameTree))
#define GSGF_GAME_TREE_CLASS(klass)     (G_TYPE_CHECK_CLASS_CAST ((klass), \
		GSGF_TYPE_GAME_TREE, GSGFGameTreeClass))
#define GSGF_IS_GAME_TREE(obj)          (G_TYPE_CHECK_INSTANCE_TYPE ((obj), \
		GSGF_TYPE_GAME_TREE))
#define GSGF_IS_GAME_TREE_CLASS(klass)  (G_TYPE_CHECK_CLASS_TYPE ((klass), \
		GSGF_TYPE_GAME_TREE))
#define GSGF_GAME_TREE_GET_CLASS(obj)   (G_TYPE_INSTANCE_GET_CLASS ((obj), \
		GSGF_TYPE_GAME_TREE, GSGFGameTreeClass))

/**
 * GSGFGameTree:
 *
 * One instance of a #GSGFGameTreeClass.
 **/
typedef struct _GSGFGameTree        GSGFGameTree;
struct _GSGFGameTree
{
        GObject parent_instance;

        /*< private >*/
        struct _GSGFGameTreePrivate *priv;
};

/**
 * GSGFGameTreeClass:
 *
 * Class representing a game_tree of games resp. game trees in a
 * Simple Game Format (SGF) file.
 **/
typedef struct _GSGFGameTreeClass   GSGFGameTreeClass;
struct _GSGFGameTreeClass
{
        /*< private >*/
        GObjectClass parent_class;
};

GType gsgf_game_tree_get_type(void) G_GNUC_CONST;

struct _GSGFNode;

GSGFGameTree *gsgf_game_tree_add_child(GSGFGameTree *self);
GSGFGameTree *gsgf_game_tree_get_parent(const GSGFGameTree *self);
struct _GSGFNode *gsgf_game_tree_add_node(GSGFGameTree *self);
GList *gsgf_game_tree_get_nodes(const GSGFGameTree *self);
GList *gsgf_game_tree_get_last_node(const GSGFGameTree *self);
GList *gsgf_game_tree_get_children(const GSGFGameTree *self);
const GSGFFlavor *gsgf_game_tree_get_flavor (const GSGFGameTree *self);

gboolean gsgf_game_tree_set_application (GSGFGameTree *self,
                                         const gchar *app,
                                         const gchar *version,
                                         GError **error);

G_END_DECLS

#endif
