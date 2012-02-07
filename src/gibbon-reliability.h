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

#ifndef _GIBBON_RELIABILITY_H
# define _GIBBON_RELIABILITY_H

#include <glib.h>
#include <glib-object.h>

#define GIBBON_TYPE_RELIABILITY (gibbon_reliability_get_type ())

/**
 * GibbonReliability:
 *
 * A boxed type for reliability values.
 **/
typedef struct _GibbonReliability GibbonReliability;
struct _GibbonReliability
{
        gdouble value;
        guint confidence;
};

GType gibbon_reliability_get_type (void) G_GNUC_CONST;

GibbonReliability *gibbon_reliability_new (gdouble value, guint confidence);
void gibbon_reliability_free (GibbonReliability *self);

#endif
