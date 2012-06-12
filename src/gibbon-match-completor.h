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

#ifndef _GIBBON_MATCH_COMPLETOR_H
# define _GIBBON_MATCH_COMPLETOR_H

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <glib.h>
#include <glib-object.h>

#include "gibbon-position.h"

#define GIBBON_TYPE_MATCH_COMPLETOR \
        (gibbon_match_completor_get_type ())
#define GIBBON_MATCH_COMPLETOR(obj) \
        (G_TYPE_CHECK_INSTANCE_CAST ((obj), GIBBON_TYPE_MATCH_COMPLETOR, \
                GibbonMatchCompletor))
#define GIBBON_MATCH_COMPLETOR_CLASS(klass) (G_TYPE_CHECK_CLASS_CAST ((klass), \
        GIBBON_TYPE_MATCH_COMPLETOR, GibbonMatchCompletorClass))
#define GIBBON_IS_MATCH_COMPLETOR(obj) \
        (G_TYPE_CHECK_INSTANCE_TYPE ((obj), \
                GIBBON_TYPE_MATCH_COMPLETOR))
#define GIBBON_IS_MATCH_COMPLETOR_CLASS(klass) \
        (G_TYPE_CHECK_CLASS_TYPE ((klass), \
                GIBBON_TYPE_MATCH_COMPLETOR))
#define GIBBON_MATCH_COMPLETOR_GET_CLASS(obj) \
        (G_TYPE_INSTANCE_GET_CLASS ((obj), \
                GIBBON_TYPE_MATCH_COMPLETOR, GibbonMatchCompletorClass))

/**
 * GibbonMatchCompletor:
 *
 * One instance of a #GibbonMatchCompletor.  All properties are private.
 */
typedef struct _GibbonMatchCompletor GibbonMatchCompletor;
struct _GibbonMatchCompletor
{
        GObject parent_instance;

        /*< private >*/
        struct _GibbonMatchCompletorPrivate *priv;
};

/**
 * GibbonMatchCompletorClass:
 *
 * Class definition for #GibbonMatchCompletor!
 */
typedef struct _GibbonMatchCompletorClass GibbonMatchCompletorClass;
struct _GibbonMatchCompletorClass
{
        /* <private >*/
        GObjectClass parent_class;
};

GType gibbon_match_completor_get_type (void) G_GNUC_CONST;

GibbonMatchCompletor *gibbon_match_completor_new (const GibbonPosition *target);

#endif
