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
 * SECTION:gibbon-saved-info
 * @short_description: Boxed type for saved match info
 *
 * Since: 0.1.0
 *
 * Information about a saved match contains a pointer to the opponent's
 * name, the length of the match, and the current score.
 */

#include <glib.h>
#include <glib/gi18n.h>

#include "gibbon-saved-info.h"

static GibbonSavedInfo *gibbon_saved_info_copy (GibbonSavedInfo *self);

G_DEFINE_BOXED_TYPE (GibbonSavedInfo, gibbon_saved_info,            \
                     gibbon_saved_info_copy, gibbon_saved_info_free)

static GibbonSavedInfo *
gibbon_saved_info_copy (GibbonSavedInfo *self)
{
        GibbonSavedInfo *copy = g_malloc (sizeof *self);

        copy->opponent = g_strdup (self->opponent);

        return copy;
}

void
gibbon_saved_info_free (GibbonSavedInfo *self)
{
        if (self) {
                if (self->opponent)
                        g_free (self->opponent);
                g_free (self);
        }
}

GibbonSavedInfo *
gibbon_saved_info_new (const gchar *opponent, guint match_length,
                       guint score1, guint score2)
{
        GibbonSavedInfo *self;

        self = g_malloc (sizeof *self);
        self->opponent = g_strdup (opponent);
        self->match_length = match_length;
        self->scores[0] = score1;
        self->scores[1] = score2;

        return self;
}

gboolean
gibbon_saved_info_equals (const GibbonSavedInfo *self,
                          const GibbonSavedInfo *other)
{
        if (!self && other)
                return FALSE;
        if (self && !other)
                return FALSE;
        if (!self && !other)
                return TRUE;
        if (self->match_length != other->match_length)
                return FALSE;
        if (self->scores[0] != other->scores[0])
                return FALSE;
        if (self->scores[1] != other->scores[1])
                return FALSE;
        if (g_strcmp0 (self->opponent, other->opponent))
                return FALSE;

        return TRUE;
}
