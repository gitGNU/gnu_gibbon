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

#ifndef _GIBBON_CLIP_READER_PRIV_H
# define _GIBBON_CLIP_READER_PRIV_H

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

enum GibbonCLIPLexerTokenType {
        GIBBON_TT_END = 0,
        GIBBON_TT_USER,
        GIBBON_TT_MAYBE_YOU,
        GIBBON_TT_MAYBE_USER,
        GIBBON_TT_TIMESTAMP,
        GIBBON_TT_WORD,
        GIBBON_TT_BOOLEAN,
        GIBBON_TT_N0,
        GIBBON_TT_POSITIVE,
        GIBBON_TT_DOUBLE,
        GIBBON_TT_REDOUBLES,
        GIBBON_TT_MESSAGE,
        GIBBON_TT_HOSTNAME,
        GIBBON_TT_DIE,
        GIBBON_TT_POINT,
        GIBBON_TT_CUBE,
        GIBBON_TT_MATCH_LENGTH,
        GIBBON_TT_YESNO
};

#include <stdio.h>

#include <glib.h>

#include "gibbon-clip-reader.h"

G_BEGIN_DECLS

int gibbon_clip_lexer_lex (void *yyscanner);
int gibbon_clip_lexer_lex_init_extra (void *self, void **yyscanner);
int gibbon_clip_lexer_lex_destroy (void *yyscanner);
void *gibbon_clip_lexer_get_extra (void *yyscanner);
int gibbon_clip_parser_parse (void *yyscanner);
void gibbon_clip_lexer_current_buffer (void *yyscanner, const gchar *line);
void gibbon_clip_lexer_reset_condition_stack (void *yyscanner);
gboolean gibbon_clip_reader_set_result (GibbonCLIPReader *self,
                                        const gchar *line, gint max_tokens,
                                        const gchar *delimiter,
                                        gint clip_code, ...);
void gibbon_clip_reader_set_error (GibbonCLIPReader *self,
                                   enum GibbonCLIPErrorCode code,
                                   const gchar *format, ...)
                                   G_GNUC_PRINTF (3, 4);
gboolean gibbon_clip_reader_set_board (GibbonCLIPReader *self,
                                       gchar **tokens);
gboolean gibbon_clip_reader_append_message (GibbonCLIPReader *self,
                                            const gchar *line);
gboolean gibbon_clip_reader_fixup_moves (GibbonCLIPReader *self);

G_END_DECLS

#endif
