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

#ifndef _LIBGSGF_COMPONENT_H
# define _LIBGSGF_COMPONENT_H

#include <glib.h>

#define GSGF_TYPE_COMPONENT \
        (gsgf_component_get_type ())
#define GSGF_COMPONENT(obj) \
        (G_TYPE_CHECK_INSTANCE_CAST ((obj), GSGF_TYPE_COMPONENT, \
                GSGFComponent))
#define GSGF_IS_COMPONENT(obj) \
        (G_TYPE_CHECK_INSTANCE_TYPE ((obj), \
                GSGF_TYPE_COMPONENT))
#define GSGF_COMPONENT_GET_IFACE(obj) \
        (G_TYPE_INSTANCE_GET_INTERFACE ((obj), \
                GSGF_TYPE_COMPONENT, GSGFComponentIface))

/**
 * GSGFComponent:
 *
 * Any component of an SGF structure, that is a #GSGFCollection,
 * a #GSGFGameTree, a #GSGFNode, or a #GSGFProperty.
 **/
typedef struct _GSGFComponent GSGFComponent;
struct _GSGFComponent {
        /* Make gtk-doc happy! */
};

/**
 * GSGFComponentIface:
 * @g_iface: The parent interface.
 * @cook: Cook unqualified raw data into structured date.
 * @write_stream: Serialize component into an output stream.
 *
 * Common methods to all hierarchy levels of an SGF stream.
 **/
typedef struct _GSGFComponentIface GSGFComponentIface;
struct _GSGFComponentIface
{
        GTypeInterface g_iface;

        /* Virtual table.  */
        gboolean (*cook) (GSGFComponent *self,
                          GSGFComponent **culprit,
                          GError **error);

        gboolean (*write_stream) (const GSGFComponent *self,
                                  GOutputStream *out,
                                  gsize *bytes_written,
                                  GCancellable *cancellable,
                                  GError **error);

        /*< private >*/
        gboolean (*_convert) (GSGFComponent *self, const gchar *charset,
                              GError **error);
};

gboolean gsgf_component_write_stream (const GSGFComponent *self,
                                      GOutputStream *out,
                                      gsize *bytes_written,
                                      GCancellable *cancellable,
                                      GError **error);

GType gsgf_component_get_type (void) G_GNUC_CONST;
gboolean gsgf_component_cook (GSGFComponent *self,
                              GSGFComponent **culprit, GError **error);

#endif
