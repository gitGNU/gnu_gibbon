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
void _gibbon_gmd_reader_set_rank (GibbonGMDReader *self,
                                    GibbonPositionSide side,
                                    const gchar *white);
void _gibbon_gmd_reader_set_location (GibbonGMDReader *self,
                                      const gchar *location);
void _gibbon_gmd_reader_set_match_length (GibbonGMDReader *self, gint length);
gboolean _gibbon_gmd_reader_add_game (GibbonGMDReader *self);
void _gibbon_gmd_reader_set_crawford (GibbonGMDReader *self);
gboolean _gibbon_gmd_reader_roll (GibbonGMDReader *self,
                                  GibbonPositionSide side,
                                  gint64 timestamp,
                                  guint64 die1, guint64 die2);
gboolean _gibbon_gmd_reader_move (GibbonGMDReader *self,
                                  GibbonPositionSide side,
                                  gint64 timestamp,
                                  guint64 encoded);
gboolean _gibbon_gmd_reader_double (GibbonGMDReader *self,
                                    GibbonPositionSide side,
                                    gint64 timestamp);
gboolean _gibbon_gmd_reader_drop (GibbonGMDReader *self,
                                  GibbonPositionSide side,
                                  gint64 timestamp);
gboolean _gibbon_gmd_reader_take (GibbonGMDReader *self,
                                  GibbonPositionSide side,
                                  gint64 timestamp);
gboolean _gibbon_gmd_reader_resign (GibbonGMDReader *self,
                                    GibbonPositionSide side,
                                    gint64 timestamp, guint value);
gboolean _gibbon_gmd_reader_reject (GibbonGMDReader *self,
                                    GibbonPositionSide side,
                                    gint64 timestamp);
gboolean _gibbon_gmd_reader_accept (GibbonGMDReader *self,
                                    GibbonPositionSide side,
                                    gint64 timestamp);
gchar *_gibbon_gmd_reader_alloc_name (GibbonGMDReader *self,
                                             const gchar *name);
void _gibbon_gmd_reader_free_names (GibbonGMDReader *self);

gboolean _gibbon_gmd_reader_check_setup (GibbonGMDReader *self);
gboolean _gibbon_gmd_reader_setup_position (GibbonGMDReader *self, gint64 b1,
                                            gint64 p1, gint64 p2, gint64 p3,
                                            gint64 p4, gint64 p5, gint64 p6,
                                            gint64 p7, gint64 p8, gint64 p9,
                                            gint64 p10, gint64 p11, gint64 p12,
                                            gint64 p13, gint64 p14, gint64 p15,
                                            gint64 p16, gint64 p17, gint64 p18,
                                            gint64 p19, gint64 p20, gint64 p21,
                                            gint64 p22, gint64 p23, gint64 p24,
                                            gint64 b2);

G_END_DECLS

#endif
