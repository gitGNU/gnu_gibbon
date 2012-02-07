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

#ifndef _GIBBON_SHOUTS_H
# define _GIBBON_SHOUTS_H

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <glib.h>
#include <glib-object.h>

#include "gibbon-app.h"
#include "gibbon-fibs-message.h"

#define GIBBON_TYPE_SHOUTS \
        (gibbon_shouts_get_type ())
#define GIBBON_SHOUTS(obj) \
        (G_TYPE_CHECK_INSTANCE_CAST ((obj), GIBBON_TYPE_SHOUTS, \
                GibbonShouts))
#define GIBBON_SHOUTS_CLASS(klass) (G_TYPE_CHECK_CLASS_CAST ((klass), \
        GIBBON_TYPE_SHOUTS, GibbonShoutsClass))
#define GIBBON_IS_SHOUTS(obj) \
        (G_TYPE_CHECK_INSTANCE_TYPE ((obj), \
                GIBBON_TYPE_SHOUTS))
#define GIBBON_IS_SHOUTS_CLASS(klass) \
        (G_TYPE_CHECK_CLASS_TYPE ((klass), \
                GIBBON_TYPE_SHOUTS))
#define GIBBON_SHOUTS_GET_CLASS(obj) \
        (G_TYPE_INSTANCE_GET_CLASS ((obj), \
                GIBBON_TYPE_SHOUTS, GibbonShoutsClass))

/**
 * GibbonShouts:
 *
 * One instance of a #GibbonShouts.  All properties are private.
 **/
typedef struct _GibbonShouts GibbonShouts;
struct _GibbonShouts
{
        GObject parent_instance;

        /*< private >*/
        struct _GibbonShoutsPrivate *priv;
};

/**
 * GibbonShoutsClass:
 *
 * Abstraction for the shout area!
 */
typedef struct _GibbonShoutsClass GibbonShoutsClass;
struct _GibbonShoutsClass
{
        /* <private >*/
        GObjectClass parent_class;
};

GType gibbon_shouts_get_type (void) G_GNUC_CONST;

GibbonShouts *gibbon_shouts_new (GibbonApp *app);
void gibbon_shouts_set_my_name (GibbonShouts *self, const gchar *me);
void gibbon_shouts_append_message (const GibbonShouts *self,
                                   const GibbonFIBSMessage *message);

#endif
