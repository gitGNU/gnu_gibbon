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

#ifndef _LIBGSGF_INTERNAL_H
# define _LIBGSGF_INTERNAL_H

#include <gsgf.h>

G_BEGIN_DECLS

struct _GSGFGameTree;

GSGFGameTree *_gsgf_game_tree_new(const gchar *flavor, GError **error);
GSGFNode *_gsgf_node_new(const gchar *flavor, GError **error);
GSGFProperty *_gsgf_property_new(const gchar *flavor, GError **error);

gboolean _gsgf_game_tree_write_stream(const struct _GSGFGameTree *game_tree,
                                      GOutputStream *out, gsize *bytes_written,
                                      GCancellable *cancellable, GError **error);
gboolean _gsgf_node_write_stream(const struct _GSGFNode *node,
                                 GOutputStream *out, gsize *bytes_written,
                                 GCancellable *cancellable, GError **error);
gboolean _gsgf_property_write_stream(const struct _GSGFProperty *node,
                                     GOutputStream *out, gsize *bytes_written,
                                     GCancellable *cancellable, GError **error);

G_END_DECLS

#endif
