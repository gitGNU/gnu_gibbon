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

#ifndef _GIBBON_JAVA_FIBS_READER_PRIV_H
# define _GIBBON_JAVA_FIBS_READER_PRIV_H

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include "gibbon-java-fibs-reader.h"

G_BEGIN_DECLS

extern GibbonJavaFIBSReader *_gibbon_java_fibs_reader_instance;

void _gibbon_java_fibs_reader_yyerror (const gchar *msg);

void _gibbon_java_fibs_reader_set_white (GibbonJavaFIBSReader *self,
                                         const gchar *white);
void _gibbon_java_fibs_reader_set_black (GibbonJavaFIBSReader *self,
                                         const gchar *black);
void _gibbon_java_fibs_reader_set_match_length (GibbonJavaFIBSReader *self,
                                                gsize length);
gboolean _gibbon_java_fibs_reader_add_game (GibbonJavaFIBSReader *self);
gboolean _gibbon_java_fibs_reader_roll (GibbonJavaFIBSReader *self,
                                        const gchar *name,
                                        guint die1, guint die2);
gchar *_gibbon_java_fibs_reader_alloc_name (GibbonJavaFIBSReader *self,
                                          const gchar *name);
void _gibbon_java_fibs_reader_free_names (GibbonJavaFIBSReader *self);

G_END_DECLS

#endif