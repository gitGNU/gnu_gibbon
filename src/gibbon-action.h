/*
 * This file is part of gibbon.
 * Gibbon is a Gtk+ frontend for the First Internet Backgammon Server FIBS.
 * Copyright (C) 2009-2011 Guido Flohr, http://guido-flohr.net/.
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

#ifndef _GIBBON_ACTION_H
# define _GIBBON_ACTION_H

#include <glib.h>
#include <glib-object.h>

#define GIBBON_TYPE_ACTION \
        (gibbon_action_get_type ())
#define GIBBON_ACTION(obj) \
        (G_TYPE_CHECK_INSTANCE_CAST ((obj), GIBBON_TYPE_ACTION, \
                GibbonAction))
#define GIBBON_ACTION_CLASS(klass) (G_TYPE_CHECK_CLASS_CAST ((klass), \
        GIBBON_TYPE_ACTION, GibbonActionClass))
#define GIBBON_IS_ACTION(obj) \
        (G_TYPE_CHECK_INSTANCE_TYPE ((obj), \
                GIBBON_TYPE_ACTION))
#define GIBBON_IS_ACTION_CLASS(klass) \
        (G_TYPE_CHECK_CLASS_TYPE ((klass), \
                GIBBON_TYPE_ACTION))
#define GIBBON_ACTION_GET_CLASS(obj) \
        (G_TYPE_INSTANCE_GET_CLASS ((obj), \
                GIBBON_TYPE_ACTION, GibbonActionClass))

/**
 * GibbonAction:
 *
 * One instance of a #GibbonAction.  All properties are private.
 **/
typedef struct _GibbonAction GibbonAction;
struct _GibbonAction
{
        GObject parent_instance;

        /*< private >*/
        struct _GibbonActionPrivate *priv;
};

/**
 * GibbonActionClass:
 *
 * FIXME! The author was negligent enough to not document this class!
 **/
typedef struct _GibbonActionClass GibbonActionClass;
struct _GibbonActionClass
{
        /* <private >*/
        GObjectClass parent_class;
};

GType gibbon_action_get_type (void) G_GNUC_CONST;

GibbonAction *gibbon_action_new (/* FIXME! Argument list! */ const gchar *dummy);

#endif
