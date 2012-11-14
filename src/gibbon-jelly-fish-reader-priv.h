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

#ifndef _GIBBON_JELLY_FISH_READER_PRIV_H
# define _GIBBON_JELLY_FISH_READER_PRIV_H

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <stdio.h>

#include "gibbon-jelly-fish-reader.h"

G_BEGIN_DECLS

void _gibbon_jelly_fish_reader_set_white (GibbonJellyFishReader *self,
                                          const gchar *white);
void _gibbon_jelly_fish_reader_set_black (GibbonJellyFishReader *self,
                                          const gchar *black);
void _gibbon_jelly_fish_reader_set_match_length (GibbonJellyFishReader *self,
                                                 gsize length);
gboolean _gibbon_jelly_fish_reader_add_game (GibbonJellyFishReader *self);
void _gibbon_jelly_fish_reader_set_side (GibbonJellyFishReader *self,
                                         GibbonPositionSide side);
gboolean _gibbon_jelly_fish_reader_move (GibbonJellyFishReader *self,
                                         guint64 dice, guint64 encoded);
gboolean _gibbon_jelly_fish_reader_double (GibbonJellyFishReader *self);
gboolean _gibbon_jelly_fish_reader_drop (GibbonJellyFishReader *self);
gboolean _gibbon_jelly_fish_reader_take (GibbonJellyFishReader *self);
gboolean _gibbon_jelly_fish_reader_win_game (GibbonJellyFishReader *self,
                                             guint points);
gchar *gibbon_jelly_fish_reader_alloc_name (GibbonJellyFishReader *self,
                                             const gchar *name);
void gibbon_jelly_fish_reader_free_names (GibbonJellyFishReader *self);

int gibbon_jelly_fish_lexer_get_lineno (void *);
void gibbon_jelly_fish_reader_yyerror (void *scanner, const gchar *msg);
void gibbon_jelly_fish_lexer_set_in (FILE *input, void *yyscanner);
int gibbon_jelly_fish_lexer_lex_init_extra (void *self, void **yyscanner);
int gibbon_jelly_fish_lexer_lex_destroy (void *yyscanner);
void *gibbon_jelly_fish_lexer_get_extra (void *yyscanner);
int gibbon_jelly_fish_parser_parse ();

G_END_DECLS

#endif
