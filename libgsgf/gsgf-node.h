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

#ifndef _LIBGSGF_NODE_H
# define _LIBGSGF_NODE_H

#include <glib.h>
#include <gio/gio.h>

G_BEGIN_DECLS

#define GSGF_TYPE_NODE                  (gsgf_node_get_type ())
#define GSGF_NODE(obj)                  (G_TYPE_CHECK_INSTANCE_CAST ((obj), \
		GSGF_TYPE_NODE, GSGFNode))
#define GSGF_NODE_CLASS(klass)          (G_TYPE_CHECK_CLASS_CAST ((klass), \
		GSGF_TYPE_NODE, GSGFNodeClass))
#define GSGF_IS_NODE(obj)               (G_TYPE_CHECK_INSTANCE_TYPE ((obj), \
		GSGF_TYPE_NODE))
#define GSGF_IS_NODE_CLASS(klass)       (G_TYPE_CHECK_CLASS_TYPE ((klass), \
		GSGF_TYPE_NODE))
#define GSGF_NODE_GET_CLASS(obj)        (G_TYPE_INSTANCE_GET_CLASS ((obj), \
		GSGF_TYPE_NODE, GSGFNodeClass))

/**
 * GSGFNode:
 *
 * One instance of a #GSGFNodeClass.  All properties are private.
 **/
typedef struct _GSGFNode        GSGFNode;
struct _GSGFNode
{
        GObject parent_instance;

        /*< private >*/
        struct _GSGFNodePrivate *priv;
};

/**
 * GSGFNodeClass:
 *
 * Class representing a node of a game in a Simple Game Format (SGF) file.
 **/
typedef struct _GSGFNodeClass   GSGFNodeClass;
struct _GSGFNodeClass
{
        /*< private >*/
        GObjectClass parent_class;
};

GType gsgf_node_get_type(void) G_GNUC_CONST;

struct _GSGFProperty;

struct _GSGFProperty *gsgf_node_add_property(GSGFNode *self, const gchar *id,
                                             GError **error);
struct _GSGFProperty *gsgf_node_get_property(const GSGFNode *self, const gchar *id);
GList *gsgf_node_get_property_ids(const GSGFNode *self);
void gsgf_node_remove_property(GSGFNode *self, const gchar *id);

GSGFValue *gsgf_node_get_property_value (const GSGFNode *self,
                                         const gchar *id);
GSGFNode *gsgf_node_get_previous_node(const GSGFNode *self);
const GSGFFlavor *gsgf_node_get_flavor(const GSGFNode *self);
GSGFGameTree *gsgf_node_get_game_tree (const GSGFNode *self);
gboolean gsgf_node_set_property (GSGFNode *self,
                                 const gchar *id, GSGFValue *value,
                                 GError **error);

G_END_DECLS

#endif
