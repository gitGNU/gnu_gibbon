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

/**
 * SECTION:gibbon-reliability
 * @short_description: Boxed type for reliability values!
 *
 * Since: 0.1.0
 *
 * Reliability in Gibbon has a value and a confidence.
 **/

#include <glib.h>
#include <glib/gi18n.h>

#include "gibbon-reliability.h"

static GibbonReliability *gibbon_reliability_copy (GibbonReliability *self);

G_DEFINE_BOXED_TYPE (GibbonReliability, gibbon_reliability,            \
                     gibbon_reliability_copy, gibbon_reliability_free)

static GibbonReliability *
gibbon_reliability_copy (GibbonReliability *self)
{
        GibbonReliability *copy = g_malloc (sizeof *self);

        memcpy (copy, self, sizeof *copy);

        return copy;
}

void
gibbon_reliability_free (GibbonReliability *self)
{
        g_free (self);
}

GibbonReliability *
gibbon_reliability_new (gdouble value, guint confidence)
{
        GibbonReliability *self = g_malloc (sizeof *self);

        self->value = value;
        self->confidence = confidence;

        return self;
}
