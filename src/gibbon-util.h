/*
 * This file is part of gibbon.
 * Gibbon is a Gtk+ frontend for the First Internet Backgammon Server FIBS.
 * Copyright (C) 2009-2012 Guido Flohr, http://guido-flohr.net/.
 *
 * gibbon is free software: you can redistribute it and/or modify 
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * gibbon is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with gibbon.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef _GIBBON_UTIL_H
# define _GIBBON_UTIL_H

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <gtk/gtk.h>

G_BEGIN_DECLS

#define GIBBON_STRINGIFY(s) GIBBON_STRINGIFY_IMPL(s)
#define GIBBON_STRINGIFY_IMPL(s) #s

#define GIBBON_ERROR gibbon_error_quark ()

GQuark gibbon_error_quark (void);

#define gibbon_return_val_if_fail(expr, val, error) G_STMT_START{      \
     if G_LIKELY(expr) { } else {                                            \
             g_set_error (error, GIBBON_ERROR, -1,                     \
                          _("In function `%s': assertion `%s' failed."),     \
                          __PRETTY_FUNCTION__, #expr);                       \
             g_return_if_fail_warning (G_LOG_DOMAIN,                         \
                                       __PRETTY_FUNCTION__,                  \
                                       #expr);                               \
             return (val);                                                   \
     };                               }G_STMT_END

enum GibbonClientType {
        GibbonClientUnknown = 0,
        GibbonClientGibbon = 1,
        GibbonClientRegular = 2,
        GibbonClientBot = 3,
        GibbonClientDaemon = 4,
        GibbonClientMobile = 5
};

gchar **gibbon_strsplit_ws (const gchar *string);
const gchar *gibbon_skip_ws_tokens (const gchar *string,
                                    const gchar * const * const tokens,
                                    gsize num);
gchar **gibbon_strsplit_set (const gchar *string, const gchar *set,
                             gint max_tokens);
gchar *gibbon_trim (gchar *string);
enum GibbonClientType gibbon_get_client_type (const gchar *client_name,
                                              const gchar *user_name,
                                              const gchar *host_name,
                                              guint port);
void gibbon_safe_object_unref (gpointer data);
gboolean gibbon_chareq (const char *str1, const char *str2);

gdouble gibbon_money_equity (const gdouble p[5]);
gboolean gibbon_slurp_file (const gchar *path, char **buffer,
                            gsize *bytes_read,
                            GCancellable *cancellable, GError **error);
gint gibbon_compare_string_column (GtkTreeModel *model,
                                   GtkTreeIter *a,
                                   GtkTreeIter *b,
                                   gpointer user_data);
gint gibbon_compare_double_column (GtkTreeModel *model,
                                   GtkTreeIter *a,
                                   GtkTreeIter *b,
                                   gpointer user_data);
gint gibbon_compare_uint_column (GtkTreeModel *model,
                                 GtkTreeIter *a,
                                 GtkTreeIter *b,
                                 gpointer user_data);
gboolean gibbon_debug (const gchar *realm);

G_END_DECLS

#endif
