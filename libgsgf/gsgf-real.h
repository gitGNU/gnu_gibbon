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

#ifndef _LIBGSGF_REAL_H
# define _LIBGSGF_REAL_H

#include <glib.h>
#include <gio/gio.h>

#include <libgsgf/gsgf-cooked-value.h>

G_BEGIN_DECLS

#define GSGF_TYPE_REAL             (gsgf_real_get_type ())
#define GSGF_REAL(obj)             (G_TYPE_CHECK_INSTANCE_CAST ((obj), GSGF_TYPE_REAL, GSGFReal))
#define GSGF_REAL_CLASS(klass)     (G_TYPE_CHECK_CLASS_CAST ((klass), GSGF_TYPE_REAL, GSGFRealClass))
#define GSGF_IS_REAL(obj)          (G_TYPE_CHECK_INSTANCE_TYPE ((obj), GSGF_TYPE_REAL))
#define GSGF_IS_REAL_CLASS(klass)  (G_TYPE_CHECK_CLASS_TYPE ((klass), GSGF_TYPE_REAL))
#define GSGF_REAL_GET_CLASS(obj)   (G_TYPE_INSTANCE_GET_CLASS ((obj), GSGF_TYPE_REAL, GSGFRealClass))

/**
 * GSGFReal:
 *
 * Class representing a real of SGF.
 **/
typedef struct _GSGFRealClass   GSGFRealClass;
typedef struct _GSGFReal        GSGFReal;
typedef struct _GSGFRealPrivate GSGFRealPrivate;

struct _GSGFRealClass
{
        GSGFCookedValueClass parent_class;
};

GType gsgf_real_get_type(void) G_GNUC_CONST;

struct _GSGFReal
{
        GSGFCookedValue parent_instance;

        /*< private >*/
        GSGFRealPrivate *priv;
};

GSGFReal* gsgf_real_new(gdouble value);
void gsgf_real_set_value(GSGFReal *self, gdouble value);
gdouble gsgf_real_get_value(const GSGFReal *self);

G_END_DECLS

#endif
