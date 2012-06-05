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

#ifndef _GIBBON_GMD_READER_PRIV_H
# define _GIBBON_GMD_READER_PRIV_H

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include "gibbon-gmd-reader.h"

G_BEGIN_DECLS

extern GibbonGMDReader *_gibbon_gmd_reader_instance;

void _gibbon_gmd_reader_yyerror (const gchar *msg);

void _gibbon_gmd_reader_set_player (GibbonGMDReader *self,
                                    GibbonPositionSide side,
                                    const gchar *white);
void _gibbon_gmd_reader_set_match_length (GibbonGMDReader *self, gint length);
gboolean _gibbon_gmd_reader_add_game (GibbonGMDReader *self);
void _gibbon_gmd_reader_set_crawford (GibbonGMDReader *self);
gboolean _gibbon_gmd_reader_roll (GibbonGMDReader *self,
                                  GibbonPositionSide side,
                                  guint64 die1, guint64 die2);
gboolean _gibbon_gmd_reader_move (GibbonGMDReader *self,
                                  GibbonPositionSide side,
                                  guint64 encoded);
gboolean _gibbon_gmd_reader_double (GibbonGMDReader *self,
                                    GibbonPositionSide side);
gboolean _gibbon_gmd_reader_drop (GibbonGMDReader *self,
                                  GibbonPositionSide side);
gboolean _gibbon_gmd_reader_take (GibbonGMDReader *self,
                                  GibbonPositionSide side);
gboolean _gibbon_gmd_reader_resign (GibbonGMDReader *self,
                                    GibbonPositionSide side, guint value);
gboolean _gibbon_gmd_reader_reject (GibbonGMDReader *self,
                                    GibbonPositionSide side);
gboolean _gibbon_gmd_reader_accept (GibbonGMDReader *self,
                                    GibbonPositionSide side);
gchar *_gibbon_gmd_reader_alloc_name (GibbonGMDReader *self,
                                             const gchar *name);
void _gibbon_gmd_reader_free_names (GibbonGMDReader *self);

G_END_DECLS

#endif
