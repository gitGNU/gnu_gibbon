/*
 * This file is part of Gibbon, a graphical frontend to the First Internet 
 * Backgammon Server FIBS.
 * Copyright (C) 2009-2011 Guido Flohr, http://guido-flohr.net/.
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

#ifndef _LIBGSGF_PRIVATE_H
# define _LIBGSGF_PRIVATE_H

#include <libgsgf/gsgf.h>

extern GHashTable *_libgsgf_flavors;

struct _GSGFGameTree;
struct _GSGFFlavor;

G_BEGIN_DECLS

GSGFGameTree *_gsgf_game_tree_new(void);
GSGFNode *_gsgf_node_new(GSGFNode *previous);
GSGFProperty *_gsgf_property_new(const gchar *id, GSGFNode *node);

gboolean _gsgf_property_add_value(GSGFProperty *property, const gchar *text);

gboolean _gsgf_game_tree_convert(GSGFGameTree *game_tree, GError **error);
gboolean _gsgf_property_convert(GSGFProperty *property, const gchar *charset,
                                GError **error);
gboolean _gsgf_raw_convert(GSGFRaw *raw, const gchar *charset,
                           GError **error);
gboolean _gsgf_game_tree_apply_flavor(GSGFGameTree *game_tree, GError **error);
gboolean _gsgf_node_apply_flavor(GSGFNode *node, const GSGFFlavor *flavor, GError **error);
gboolean _gsgf_property_apply_flavor(GSGFProperty *property, const GSGFFlavor *flavor,
                                     GError **error);
GSGFCookedValue *_gsgf_property_get_raw(const GSGFProperty* property);
void _gsgf_raw_set_value(GSGFRaw *self, const gchar *value, gsize i, gboolean copy);
void _gsgf_raw_add_value(GSGFRaw *self, const gchar *value);

/* Private constructors.  */
GSGFReal *_gsgf_real_new(const gchar *value, GError **error);

/* Schedule a property for deletion.  This function must only be called
 * from within _gsgf_node-apply_flavor().
 */
void _gsgf_node_mark_loser_property (GSGFNode *node, const gchar *id);

void _libgsgf_init();

struct _GSGFFlavor *_libgsgf_get_flavor(const gchar *id);

G_END_DECLS

#endif
