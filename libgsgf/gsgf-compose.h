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

#ifndef _LIBGSGF_COMPOSE_H
# define _LIBGSGF_COMPOSE_H

#include <glib.h>
#include <gio/gio.h>

#include <libgsgf/gsgf-cooked-value.h>

G_BEGIN_DECLS

#define GSGF_TYPE_COMPOSE             (gsgf_compose_get_type ())
#define GSGF_COMPOSE(obj)             (G_TYPE_CHECK_INSTANCE_CAST ((obj), \
                GSGF_TYPE_COMPOSE, GSGFCompose))
#define GSGF_COMPOSE_CLASS(klass)     (G_TYPE_CHECK_CLASS_CAST ((klass), \
                GSGF_TYPE_COMPOSE, GSGFComposeClass))
#define GSGF_IS_COMPOSE(obj)          (G_TYPE_CHECK_INSTANCE_TYPE ((obj), \
                GSGF_TYPE_COMPOSE))
#define GSGF_IS_COMPOSE_CLASS(klass)  (G_TYPE_CHECK_CLASS_TYPE ((klass), \
                GSGF_TYPE_COMPOSE))
#define GSGF_COMPOSE_GET_CLASS(obj)   (G_TYPE_INSTANCE_GET_CLASS ((obj), \
                GSGF_TYPE_COMPOSE, GSGFComposeClass))

/**
 * GSGFCompose:
 *
 * Composed values are two values separated by a colon.
 **/

typedef struct _GSGFCompose        GSGFCompose;
typedef struct _GSGFComposeClass   GSGFComposeClass;

struct _GSGFCompose
{
        GSGFCookedValue parent_instance;

        /*< private >*/
        struct _GSGFComposePrivate *priv;
};

/**
 * GSGFComposeClass:
 *
 * Class definition for a composed value.
 **/
struct _GSGFComposeClass
{
        /*< private >*/
        GSGFCookedValueClass parent_class;
};

GType gsgf_compose_get_type(void) G_GNUC_CONST;

GSGFCompose *gsgf_compose_new(GSGFCookedValue *value, ...);

gsize gsgf_compose_get_number_of_values(const GSGFCompose *self);
GSGFCookedValue *gsgf_compose_get_value(const GSGFCompose *self, gsize i);

G_END_DECLS

#endif
