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

#include <glib.h>
#include <glib/gi18n.h>

#include "gibbon-jelly-fish-parser.h"
#include "gibbon-jelly-fish-reader-priv.h"

#define alloc_name(s) gibbon_jelly_fish_reader_alloc_name(		\
	gibbon_jelly_fish_lexer_get_extra(yyscanner), s)

%}

%top{
#define yy_create_buffer gibbon_jelly_fish_lexer__create_buffer
#define yy_delete_buffer gibbon_jelly_fish_lexer__delete_buffer
#define yy_flush_buffer gibbon_jelly_fish_lexer__flush_buffer
#define yy_scan_buffer gibbon_jelly_fish_lexer__scan_buffer
#define yy_scan_bytes gibbon_jelly_fish_lexer__scan_bytes
#define yy_scan_string gibbon_jelly_fish_lexer__scan_string
#define yy_switch_to_buffer gibbon_jelly_fish_lexer__switch_to_buffer
#define yyalloc gibbon_jelly_fish_lexer_alloc
#define yyfree gibbon_jelly_fish_lexer_free
#define yyget_column gibbon_jelly_fish_lexer_get_column
#define yyget_debug gibbon_jelly_fish_lexer_get_debug
#define yyget_extra gibbon_jelly_fish_lexer_get_extra
#define yyget_in gibbon_jelly_fish_lexer_get_in
#define yyget_leng gibbon_jelly_fish_lexer_get_leng
#define yyget_lineno gibbon_jelly_fish_lexer_get_lineno
#define yyget_lval gibbon_jelly_fish_lexer_get_lval
#define yyget_out gibbon_jelly_fish_lexer_get_out
#define yyget_text gibbon_jelly_fish_lexer_get_text
#define yylex gibbon_jelly_fish_lexer_lex
#define yylex_destroy gibbon_jelly_fish_lexer_lex_destroy
#define yylex_init gibbon_jelly_fish_lexer_lex_init
#define yylex_init_extra gibbon_jelly_fish_lexer_lex_init_extra
#define yypop_buffer_state gibbon_jelly_fish_lexer_pop_buffer_state
#define yypush_buffer_state gibbon_jelly_fish_lexer_push_buffer_state
#define yyrealloc gibbon_jelly_fish_lexer_realloc
#define yyrestart gibbon_jelly_fish_lexer_restart
#define yyset_column gibbon_jelly_fish_lexer_set_column
#define yyset_debug gibbon_jelly_fish_lexer_set_debug
#define yyset_extra gibbon_jelly_fish_lexer_set_extra
#define yyset_in gibbon_jelly_fish_lexer_set_in
#define yyset_lineno gibbon_jelly_fish_lexer_set_lineno
#define yyset_lval gibbon_jelly_fish_lexer_set_lval
#define yyset_out gibbon_jelly_fish_lexer_set_out
}

%option yylineno
%option nounput
%option noinput
%option noyywrap
%option case-insensitive
%option reentrant
%option bison-bridge

/* %option debug */

%x GAME_START
%x OPPONENTS1
%x OPPONENTS2
%x OPPONENTS3
%x OPPONENTS4
%x OPPONENTS5
%x OPPONENTS6
%x MOVES
%x HALF_MOVE
%x ACTION

%%

<INITIAL>[1-9][0-9]*	{
		((struct yyguts_t *) yyscanner)->yylval_r->num
			= (gint) g_ascii_strtoull (yytext, NULL, 10);
		return INTEGER;
	}
	
<INITIAL>point[ \t\r][ \t\r]*match	BEGIN (GAME_START); return MATCH_LENGTH;
<INITIAL>[ \t\r\n]+		;

<GAME_START>Game		return GAME;
<GAME_START>[1-9][0-9]*		{
		((struct yyguts_t *) yyscanner)->yylval_r->num
			= (gint) g_ascii_strtoull (yytext, NULL, 10);
		BEGIN (OPPONENTS1); 
		return INTEGER;
	}

  /***************************************************************************
   * The line with the opponents.                                            
   * This is really a little awkward to use six start conditions for six
   * simple tokens.  But I cannot see a simpler solution right now.
   ***************************************************************************/
<OPPONENTS1>[\x21-\xff]+	{
		((struct yyguts_t *) yyscanner)->yylval_r->name = alloc_name (yytext);
		BEGIN (OPPONENTS2);
		return PLAYER;
	}
<OPPONENTS2>:			{
					BEGIN (OPPONENTS3);
					return COLON;
				}
<OPPONENTS3>0|[1-9][0-9]*	{
		((struct yyguts_t *) yyscanner)->yylval_r->num
			= (gint) g_ascii_strtoull (yytext, NULL, 10);
		BEGIN (OPPONENTS4); 
		return INTEGER;
	}
<OPPONENTS4>[\x21-\xff]+	{
		((struct yyguts_t *) yyscanner)->yylval_r->name = alloc_name (yytext);
		BEGIN (OPPONENTS5);
		return PLAYER;
	}
<OPPONENTS5>:			{
					BEGIN (OPPONENTS6);
					return COLON;
				}
<OPPONENTS6>0|[1-9][0-9]*	{
		((struct yyguts_t *) yyscanner)->yylval_r->num
			= (gint) g_ascii_strtoull (yytext, NULL, 10);
		BEGIN (MOVES); 
		return INTEGER;
	}

<MOVES>[1-9][0-9]*		{
		((struct yyguts_t *) yyscanner)->yylval_r->num
			= (gint) g_ascii_strtoull (yytext, NULL, 10);
		return INTEGER;
	}
<MOVES>[)]			{
					BEGIN (HALF_MOVE);
					return PAREN;
				}
<HALF_MOVE>[ \t]{3,}		{
					return WHITESPACE;
				}
<HALF_MOVE,ACTION>[1-6][1-6][ \t]*:	{
		((struct yyguts_t *) yyscanner)->yylval_r->num 
			= (yytext[0] - '0') * 10 + yytext[1] - '0';
		BEGIN (ACTION);
		return ROLL; 
	}
<ACTION>[0-9]|1[0-9]|2[0-5]	{
		((struct yyguts_t *) yyscanner)->yylval_r->num
			= (gint) g_ascii_strtoull (yytext, NULL, 10);
		return POINT;
	}
<ACTION>\/			{
					return SLASH;
				}
<ACTION,HALF_MOVE>Doubles?[ \t]*=>[ \t]*[1-9][0-9]*	{
					BEGIN (ACTION);
					return DOUBLES;
				}
<ACTION,HALF_MOVE>Drops?	{
					BEGIN (ACTION);
					return DROPS;
				}
<ACTION,HALF_MOVE>Takes?	{
					BEGIN (ACTION);
					return TAKES;
				}
<ACTION,HALF_MOVE,MOVES>
	Wins?[ \t]*[1-9][0-9]*[ \t]*points?([ \t]*and[ \t]*the[ \t]*match)? {
		while (*yytext < '1' || *yytext > '9')
			++yytext;
			((struct yyguts_t *) yyscanner)->yylval_r->num
				= (gint) g_ascii_strtoull (yytext, NULL, 10);
			BEGIN (GAME_START);					
			return WINS;
		}
<ACTION>\n			BEGIN (MOVES);

  /***************************************************************************
   * General rules, valid for all scanner states.                            *
   ***************************************************************************/
   
  /* Ignore all whitespace.  */
<*>[ \t\r\n]			;
<*>.				return yytext[0];
%%
