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

%{
#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

static int yywrap ();

#include <glib.h>
#include <glib/gi18n.h>

#define yylval gibbon_java_fibs_parser_lval

#include "gibbon-jelly-fish-parser.h"
#include "gibbon-jelly-fish-reader-priv.h"

#define alloc_name(s) \
      _gibbon_jelly_fish_reader_alloc_name(_gibbon_jelly_fish_reader_instance, \
	                                   s)

%}

%top{
#define yy_create_buffer gibbon_jelly_fish_lexer_create_buffer
#define yy_delete_buffer gibbon_jelly_fish_lexer_delete_buffer
#define yy_flush_buffer gibbon_jelly_fish_lexer_flush_buffer
#define yy_scan_buffer gibbon_jelly_fish_lexer_scan_buffer
#define yy_scan_bytes gibbon_jelly_fish_lexer_scan_bytes
#define yy_scan_string gibbon_jelly_fish_lexer_scan_string
#define yy_switch_to_buffer gibbon_jelly_fish_lexer_switch_to_buffer
#define yyalloc gibbon_jelly_fish_lexer_alloc
#define yyfree gibbon_jelly_fish_lexer_free
#define yyget_debug gibbon_jelly_fish_lexer_get_debug
#define yyget_in gibbon_jelly_fish_lexer_get_in
#define yyget_leng gibbon_jelly_fish_lexer_get_leng
#define yyget_lineno gibbon_jelly_fish_lexer_get_lineno
#define yyget_out gibbon_jelly_fish_lexer_get_out
#define yyget_text gibbon_jelly_fish_lexer_get_text
#define yylex gibbon_jelly_fish_lexer_lex
#define yylex_destroy gibbon_jelly_fish_lexer_lex_destroy
#define yypop_buffer_state gibbon_jelly_fish_lexer_pop_buffer_state
#define yypush_buffer_state gibbon_jelly_fish_lexer_push_buffer_state
#define yyrealloc gibbon_jelly_fish_lexer_realloc
#define yyrestart gibbon_jelly_fish_lexer_restart
#define yyset_debug gibbon_jelly_fish_lexer_set_debug
#define yyset_in gibbon_jelly_fish_lexer_set_in
#define yyset_lineno gibbon_jelly_fish_lexer_set_lineno
#define yyset_out gibbon_jelly_fish_lexer_set_out
#define yy_flex_debug gibbon_jelly_fish_lexer_flex_debug
#define yyin gibbon_jelly_fish_lexer_in
#define yyleng gibbon_jelly_fish_lexer_leng
#define yylineno gibbon_jelly_fish_lexer_lineno
#define yyout gibbon_jelly_fish_lexer_out
#define yytext gibbon_jelly_fish_lexer_text
}

%option yylineno
%option nounput
%option noinput

%option debug

%x GAME_START
%x OPPONENTS1
%x OPPONENTS2
%x OPPONENTS3
%x ACTIONS

%%

<INITIAL>[1-9][0-9]*	{
					yylval.num = (gint) g_ascii_strtoull 
							     (yytext, NULL, 10); 
					return INTEGER;

                                }
<INITIAL>point[ \t\r][ \t\r]*match	BEGIN (GAME_START); return MATCH_LENGTH;
<INITIAL>[ \t\r\n]+		;

<GAME_START>Game		return GAME;
<GAME_START>[1-9][0-9]*		{
					yylval.num = (gint) g_ascii_strtoull 
							     (yytext, NULL, 10);
					BEGIN (OPPONENTS1); 
					return INTEGER;

                                }

<OPPONENTS1>[\x21-\xff]+	{
					yylval.name = alloc_name (yytext);
					BEGIN (OPPONENTS2);
					return PLAYER;
				}
<OPPONENTS2>:			{
					BEGIN (OPPONENTS3);
					return COLON;
				}
<OPPONENTS3>0			{
					yylval.num = (gint) 0;
					BEGIN (OPPONENTS1); 
					return INTEGER;
                                }
<OPPONENTS3>[1-9][0-9]*		{
					yylval.num = (gint) g_ascii_strtoull 
							     (yytext, NULL, 10);
					BEGIN (OPPONENTS1); 
					return INTEGER;
                                }
<OPPONENTS3>\n			BEGIN (ACTIONS);

  /***************************************************************************
   * General rules, valid for all scanner states.                            *
   ***************************************************************************/
   
  /* Ignore all whitespace.  */
<*>[ \t\r\n]			;
<*>.				return JUNK;
%%

static int
yywrap ()
{
	return 1;
}
