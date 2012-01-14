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

#ifndef _GIBBON_MATCH_H
# define _GIBBON_MATCH_H

#include <glib.h>
#include <glib-object.h>

#include "gibbon-position.h"

#define GIBBON_TYPE_MATCH \
        (gibbon_match_get_type ())
#define GIBBON_MATCH(obj) \
        (G_TYPE_CHECK_INSTANCE_CAST ((obj), GIBBON_TYPE_MATCH, \
                GibbonMatch))
#define GIBBON_MATCH_CLASS(klass) (G_TYPE_CHECK_CLASS_CAST ((klass), \
        GIBBON_TYPE_MATCH, GibbonMatchClass))
#define GIBBON_IS_MATCH(obj) \
        (G_TYPE_CHECK_INSTANCE_TYPE ((obj), \
                GIBBON_TYPE_MATCH))
#define GIBBON_IS_MATCH_CLASS(klass) \
        (G_TYPE_CHECK_CLASS_TYPE ((klass), \
                GIBBON_TYPE_MATCH))
#define GIBBON_MATCH_GET_CLASS(obj) \
        (G_TYPE_INSTANCE_GET_CLASS ((obj), \
                GIBBON_TYPE_MATCH, GibbonMatchClass))

/**
 * GibbonMatch:
 *
 * One instance of a #GibbonMatch.  All properties are private.
 **/
typedef struct _GibbonMatch GibbonMatch;
struct _GibbonMatch
{
        GObject parent_instance;

        /*< private >*/
        struct _GibbonMatchPrivate *priv;
};

/**
 * GibbonMatchClass:
 *
 * Class representing a complete match of backgammon!
 **/
typedef struct _GibbonMatchClass GibbonMatchClass;
struct _GibbonMatchClass
{
        /* <private >*/
        GObjectClass parent_class;
};

GType gibbon_match_get_type (void) G_GNUC_CONST;

GibbonMatch *gibbon_match_new (const gchar *white, const gchar *black,
                               guint length, gboolean crawford);

gboolean gibbon_match_get_crawford (const GibbonMatch *self);

struct _GibbonGame *gibbon_match_get_current_game (const GibbonMatch *self);
const GibbonPosition *gibbon_match_get_current_position (const GibbonMatch *
                                                         self);
struct _GibbonGame *gibbon_match_add_game (GibbonMatch *self);

#endif
