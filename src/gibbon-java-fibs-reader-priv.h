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

#include <stdio.h>

#include "gibbon-java-fibs-reader.h"

G_BEGIN_DECLS

extern GibbonJavaFIBSReader *_gibbon_java_fibs_reader_instance;

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
gboolean _gibbon_java_fibs_reader_move (GibbonJavaFIBSReader *self,
                                        const gchar *name,
                                        guint64 encoded);
gboolean _gibbon_java_fibs_reader_double (GibbonJavaFIBSReader *self,
                                          const gchar *name);
gboolean _gibbon_java_fibs_reader_drop (GibbonJavaFIBSReader *self,
                                        const gchar *name);
gboolean _gibbon_java_fibs_reader_take (GibbonJavaFIBSReader *self,
                                        const gchar *name);
gboolean _gibbon_java_fibs_reader_resign (GibbonJavaFIBSReader *self,
                                          const gchar *name,
                                          guint points);
gboolean _gibbon_java_fibs_reader_reject_resign (GibbonJavaFIBSReader *self,
                                                 const gchar *name);
gboolean _gibbon_java_fibs_reader_win_game (GibbonJavaFIBSReader *self,
                                            const gchar *name,
                                            guint points);
gboolean _gibbon_java_fibs_reader_score (GibbonJavaFIBSReader *self,
                                         const gchar *winner,
                                         guint points_winner,
                                         const gchar *loser,
                                         guint points_loser);
gchar *gibbon_java_fibs_reader_alloc_name (GibbonJavaFIBSReader *self,
                                           const gchar *name);
void gibbon_java_fibs_reader_free_names (GibbonJavaFIBSReader *self);

int gibbon_java_fibs_lexer_get_lineno (void *);
void gibbon_java_fibs_reader_yyerror (void *scanner, const gchar *msg);
void gibbon_java_fibs_lexer_set_in (FILE *input, void *yyscanner);
int gibbon_java_fibs_lexer_lex_init_extra (void *self, void **yyscanner);
int gibbon_java_fibs_lexer_lex_destroy (void *yyscanner);
void *gibbon_java_fibs_lexer_get_extra (void *yyscanner);
int gibbon_java_fibs_parser_parse ();

G_END_DECLS

#endif
