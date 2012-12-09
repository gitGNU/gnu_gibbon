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

#ifndef _GIBBON_SAVED_INFO_H
# define _GIBBON_SAVED_INFO_H

#include <glib.h>
#include <glib-object.h>

#define GIBBON_TYPE_SAVED_INFO (gibbon_saved_info_get_type ())

/**
 * GibbonSavedInfo:
 *
 * A boxed type for messages in FIBS.
 **/
typedef struct _GibbonSavedInfo GibbonSavedInfo;
struct _GibbonSavedInfo
{
        gchar *opponent;
        guint match_length;
        guint scores[2];
};

GType gibbon_saved_info_get_type (void) G_GNUC_CONST;

GibbonSavedInfo *gibbon_saved_info_new (const gchar *opponent,
                                        guint match_length,
                                        guint score1, guint score2);
void gibbon_saved_info_free (GibbonSavedInfo *self);
gboolean gibbon_saved_info_equals (const GibbonSavedInfo *self,
                                   const GibbonSavedInfo *other);

#endif
