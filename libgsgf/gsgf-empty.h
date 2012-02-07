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

#ifndef _LIBGSGF_EMPTY_H
# define _LIBGSGF_EMPTY_H

#include <glib.h>
#include <gio/gio.h>

#include <libgsgf/gsgf-cooked-value.h>

G_BEGIN_DECLS

#define GSGF_TYPE_EMPTY             (gsgf_empty_get_type ())
#define GSGF_EMPTY(obj)             (G_TYPE_CHECK_INSTANCE_CAST ((obj), \
                GSGF_TYPE_EMPTY, GSGFEmpty))
#define GSGF_EMPTY_CLASS(klass)     (G_TYPE_CHECK_CLASS_CAST ((klass), \
                GSGF_TYPE_EMPTY, GSGFEmptyClass))
#define GSGF_IS_EMPTY(obj)          (G_TYPE_CHECK_INSTANCE_TYPE ((obj), \
                GSGF_TYPE_EMPTY))
#define GSGF_IS_EMPTY_CLASS(klass)  (G_TYPE_CHECK_CLASS_TYPE ((klass), \
                GSGF_TYPE_EMPTY))
#define GSGF_EMPTY_GET_CLASS(obj)   (G_TYPE_INSTANCE_GET_CLASS ((obj), \
                GSGF_TYPE_EMPTY, GSGFEmptyClass))

/**
 * GSGFEmpty:
 *
 * Instance of a #GSGFEmptyClass.  All properties are private.
 *
 * Cooked values contain structured data, whereas a #GSGFRaw holds
 * raw unqualified data.
 **/
typedef struct _GSGFEmpty        GSGFEmpty;
struct _GSGFEmpty
{
        GSGFCookedValue parent_instance;
};

/**
 * GSGFEmptyClass:
 *
 * Class representing an empty, single-valued SGF property.
 **/
typedef struct _GSGFEmptyClass   GSGFEmptyClass;
struct _GSGFEmptyClass
{
        /*< private >*/
        GSGFCookedValueClass parent_class;
};

GType gsgf_empty_get_type(void) G_GNUC_CONST;

struct _GSGFProperty;

GSGFEmpty *gsgf_empty_new(void);
GSGFCookedValue *gsgf_empty_new_from_raw(const GSGFRaw* raw,
                                          const GSGFFlavor *flavor,
                                          const struct _GSGFProperty *property,
                                          GError **error);

G_END_DECLS

#endif
