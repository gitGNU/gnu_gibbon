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

#ifndef _LIBGSGF_GENERAL_FLAVOR_H
# define _LIBGSGF_GENERAL_FLAVOR_H

#include <glib.h>
#include <gio/gio.h>

G_BEGIN_DECLS

#define GSGF_TYPE_GENERAL_FLAVOR             (gsgf_general_flavor_get_type ())
#define GSGF_GENERAL_FLAVOR(obj)             (G_TYPE_CHECK_INSTANCE_CAST ((obj), GSGF_TYPE_GENERAL_FLAVOR, GSGFGeneralFlavor))
#define GSGF_GENERAL_FLAVOR_CLASS(klass)     (G_TYPE_CHECK_CLASS_CAST ((klass), GSGF_TYPE_GENERAL_FLAVOR, GSGFGeneralFlavorClass))
#define GSGF_IS_GENERAL_FLAVOR(obj)          (G_TYPE_CHECK_INSTANCE_TYPE ((obj), GSGF_TYPE_GENERAL_FLAVOR))
#define GSGF_IS_GENERAL_FLAVOR_CLASS(klass)  (G_TYPE_CHECK_CLASS_TYPE ((klass), GSGF_TYPE_GENERAL_FLAVOR))
#define GSGF_GENERAL_FLAVOR_GET_CLASS(obj)   (G_TYPE_INSTANCE_GET_CLASS ((obj), GSGF_TYPE_GENERAL_FLAVOR, GSGFGeneralFlavorClass))

/**
 * GSGFGeneralFlavor:
 *
 * Class representing a general_flavor of SGF.
 **/
typedef struct _GSGFGeneralFlavorClass   GSGFGeneralFlavorClass;
typedef struct _GSGFGeneralFlavor        GSGFGeneralFlavor;
typedef struct _GSGFGeneralFlavorPrivate GSGFGeneralFlavorPrivate;

struct _GSGFGeneralFlavorClass
{
        GSGFFlavorClass parent_class;
};

GType gsgf_general_flavor_get_type(void) G_GNUC_CONST;

struct _GSGFGeneralFlavor
{
        GSGFFlavor parent_instance;

        /*< private >*/
        GSGFGeneralFlavorPrivate *priv;
};

GSGFGeneralFlavor* gsgf_general_flavor_new(const gchar *id, const gchar *name, 
                                           const GSGFGeneralFlavor *parent);

G_END_DECLS

#endif
