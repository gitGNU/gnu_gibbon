/*
 * This file is part of gibbon.
 * Gibbon is a Gtk+ frontend for the First Internet Backgammon Server FIBS.
 * Copyright (C) 2009-2011 Guido Flohr, http://guido-flohr.net/.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 3 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with gibbon; if not, see <http://www.gnu.org/licenses/>.
 */

#ifndef _LIBGSGF_RESULT_H
# define _LIBGSGF_RESULT_H

#include <glib.h>

#define GSGF_TYPE_RESULT \
        (gsgf_result_get_type ())
#define GSGF_RESULT(obj) \
        (G_TYPE_CHECK_INSTANCE_CAST ((obj), GSGF_TYPE_RESULT, \
                GSGFResult))
#define GSGF_RESULT_CLASS(klass) (G_TYPE_CHECK_CLASS_CAST ((klass), \
        GSGF_TYPE_RESULT, GSGFResultClass))
#define GSGF_IS_RESULT(obj) \
        (G_TYPE_CHECK_INSTANCE_TYPE ((obj), \
                GSGF_TYPE_RESULT))
#define GSGF_IS_RESULT_CLASS(klass) \
        (G_TYPE_CHECK_CLASS_TYPE ((klass), \
                GSGF_TYPE_RESULT))
#define GSGF_RESULT_GET_CLASS(obj) \
        (G_TYPE_INSTANCE_GET_CLASS (obj), \
                GSGF_TYPE_RESULT, GSGFResultClass))

/**
 * GSGFResult:
 *
 * One instance of a #GSGFResult.  All properties are private.
 **/
typedef struct _GSGFResult GSGFResult;
struct _GSGFResult
{
        GSGFSimpleText parent_instance;

        /*< private >*/
        struct _GSGFResultPrivate *priv;
};

/**
 * GSGFResultClass:
 *
 * FIXME! The author was negligent enough to not document this class!
 **/
typedef struct _GSGFResultClass GSGFResultClass;
struct _GSGFResultClass
{
        /* <private >*/
        GSGFSimpleTextClass parent_class;
};

GType gsgf_result_get_type (void) G_GNUC_CONST;

GSGFResult *gsgf_result_new (/* FIXME! Argument list! */ const gchar *dummy);

#endif
