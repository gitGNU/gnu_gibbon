/*
 * This file is part of Gibbon, a graphical frontend to the First Internet 
 * Backgammon Server FIBS.
 * Copyright (C) 2009-2010 Guido Flohr, http://guido-flohr.net/.
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

#ifndef _LIBGSGF_PROPERTY_HANDLER_H
# define _LIBGSGF_PROPERTY_HANDLER_H

#include <glib.h>
#include <gio/gio.h>

typedef gboolean (*GSGFPropertyTextUpdater) (GSGFProperty *property,
                                             const gchar *text, GError **error);

struct _GSGFPropertyHandler {
        GSGFPropertyTextUpdater text_updater;
};

typedef struct _GSGFPropertyHandler GSGFPropertyHandler;

G_BEGIN_DECLS

GType gsgf_property_handler_get_type (void) G_GNUC_CONST;
#define GSGF_TYPE_PROPERTY_HANDLER (gsgf_property_handler_get_type())

GSGFPropertyHandler* gsgf_property_handler_new(const gchar *id, 
                                               const gchar *name);
GSGFPropertyHandler* gsgf_property_handler_copy (const GSGFPropertyHandler *);
void gsgf_property_handler_destroy(GSGFPropertyHandler *);

G_END_DECLS

#endif
