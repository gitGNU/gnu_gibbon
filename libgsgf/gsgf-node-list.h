/*
 * This file is part of Gibbon, a graphical frontend to the First Internet 
 * Backgammon Server FIBS.
 * Copyright (C) 2009 Guido Flohr, http://guido-flohr.net/.
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

#ifndef _LIBGSGF_NODE_LIST_H
# define _LIBGSGF_NODE_LIST_H

#include <glib.h>
#include <gio/gio.h>

G_BEGIN_DECLS

#define GSGF_TYPE_NODE_LIST             (gsgf_node_list_get_type ())
#define GSGF_NODE_LIST(obj)             (G_TYPE_CHECK_INSTANCE_CAST ((obj), GSGF_TYPE_NODE_LIST, GSGFNodeList))
#define GSGF_NODE_LIST_CLASS(klass)     (G_TYPE_CHECK_CLASS_CAST ((klass), GSGF_TYPE_NODE_LIST, GSGFNodeListClass))
#define GSGF_IS_NODE_LIST(obj)          (G_TYPE_CHECK_INSTANCE_TYPE ((obj), GSGF_TYPE_NODE_LIST))
#define GSGF_IS_NODE_LIST_CLASS(klass)  (G_TYPE_CHECK_CLASS_TYPE ((klass), GSGF_TYPE_NODE_LIST))
#define GSGF_NODE_LIST_GET_CLASS(obj)   (G_TYPE_INSTANCE_GET_CLASS ((obj), GSGF_TYPE_NODE_LIST, GSGFNodeListClass))

/**
 * GSGFNodeList:
 *
 * Class representing a node_list of games resp. game trees in a
 * Simple Game Format (SGF) file.
 **/
typedef struct _GSGFNodeListClass   GSGFNodeListClass;
typedef struct _GSGFNodeList        GSGFNodeList;
typedef struct _GSGFNodeListPrivate GSGFNodeListPrivate;

struct _GSGFNodeListClass
{
        GObjectClass parent_class;
};

GType gsgf_node_list_get_type(void) G_GNUC_CONST;

struct _GSGFNodeList
{
        GObject parent_instance;

        /*< private >*/
        GSGFNodeListPrivate *priv;
};

GSGFNodeList *gsgf_node_list_new();

GSGFNodeList *gsgf_node_list_add_child(GSGFNodeList *self);

G_END_DECLS

#endif
