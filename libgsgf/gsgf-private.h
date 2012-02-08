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

#ifndef _LIBGSGF_PRIVATE_H
# define _LIBGSGF_PRIVATE_H

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#if (HAVE_USELOCALE && HAVE_NEWLOCALE)
# define HAVE_XLOCALE_API 1
#else
# undef HAVE_XLOCALE_API
#endif

#include <libgsgf/gsgf.h>

extern GHashTable *_libgsgf_flavors;

struct _GSGFGameTree;
struct _GSGFFlavor;

G_BEGIN_DECLS

#define gsgf_return_val_if_fail(expr, val, error) G_STMT_START{         \
     if G_LIKELY(expr) { } else                                         \
       {                                                                \
         g_set_error (error, GSGF_ERROR, GSGF_ERROR_USAGE_ERROR,        \
                      _("In function `%s': assertion `%s' failed."),    \
                      __PRETTY_FUNCTION__, #expr);                      \
         g_return_if_fail_warning (G_LOG_DOMAIN,                        \
                                   __PRETTY_FUNCTION__,                 \
                                   #expr);                              \
         return (val);                                                  \
       };                               }G_STMT_END

GSGFGameTree *_gsgf_game_tree_new (const GSGFFlavor *flavor);
GSGFNode *_gsgf_node_new (GSGFNode *previous, GSGFGameTree *parent);
GSGFProperty *_gsgf_property_new(const gchar *id, GSGFNode *node);

gboolean _gsgf_property_add_value(GSGFProperty *property, const gchar *text);

GSGFCookedValue *_gsgf_property_get_raw(const GSGFProperty* property);
void _gsgf_raw_set_value(GSGFRaw *self, const gchar *value, gsize i, gboolean copy);
gboolean _gsgf_raw_convert (GSGFRaw *self, const gchar *charset,
                            GError **error);

/* Private constructors.  */
GSGFReal *_gsgf_real_new(const gchar *value, GError **error);

/* Schedule a property for deletion.  This function must only be called
 * from within gsgf_node_cook().
 */
void _gsgf_node_mark_loser_property (GSGFNode *node, const gchar *id);

void _libgsgf_init();

struct _GSGFFlavor *_libgsgf_get_flavor(const gchar *id);

G_END_DECLS

#endif
