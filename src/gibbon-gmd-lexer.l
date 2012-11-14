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

/*
 * Lexical scanner for the internal format of Gibbon.
 */

%{
#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include "gibbon-util.h"

#include <glib.h>
#include <glib/gi18n.h>

#include "gibbon-gmd-parser.h"
#include "gibbon-gmd-reader-priv.h"

#define alloc_name(s) gibbon_gmd_reader_alloc_name(		\
	gibbon_gmd_lexer_get_extra(yyscanner), s)
		
%}

%top{
#define yy_create_buffer gibbon_gmd_lexer__create_buffer
#define yy_delete_buffer gibbon_gmd_lexer__delete_buffer
#define yy_flush_buffer gibbon_gmd_lexer__flush_buffer
#define yy_scan_buffer gibbon_gmd_lexer__scan_buffer
#define yy_scan_bytes gibbon_gmd_lexer__scan_bytes
#define yy_scan_string gibbon_gmd_lexer__scan_string
#define yy_switch_to_buffer gibbon_gmd_lexer__switch_to_buffer
#define yyalloc gibbon_gmd_lexer_alloc
#define yyfree gibbon_gmd_lexer_free
#define yyget_column gibbon_gmd_lexer_get_column
#define yyget_debug gibbon_gmd_lexer_get_debug
#define yyget_extra gibbon_gmd_lexer_get_extra
#define yyget_in gibbon_gmd_lexer_get_in
#define yyget_leng gibbon_gmd_lexer_get_leng
#define yyget_lineno gibbon_gmd_lexer_get_lineno
#define yyget_lval gibbon_gmd_lexer_get_lval
#define yyget_out gibbon_gmd_lexer_get_out
#define yyget_text gibbon_gmd_lexer_get_text
#define yylex gibbon_gmd_lexer_lex
#define yylex_destroy gibbon_gmd_lexer_lex_destroy
#define yylex_init gibbon_gmd_lexer_lex_init
#define yylex_init_extra gibbon_gmd_lexer_lex_init_extra
#define yypop_buffer_state gibbon_gmd_lexer_pop_buffer_state
#define yypush_buffer_state gibbon_gmd_lexer_push_buffer_state
#define yyrealloc gibbon_gmd_lexer_realloc
#define yyrestart gibbon_gmd_lexer_restart
#define yyset_column gibbon_gmd_lexer_set_column
#define yyset_debug gibbon_gmd_lexer_set_debug
#define yyset_extra gibbon_gmd_lexer_set_extra
#define yyset_in gibbon_gmd_lexer_set_in
#define yyset_lineno gibbon_gmd_lexer_set_lineno
#define yyset_lval gibbon_gmd_lexer_set_lval
#define yyset_out gibbon_gmd_lexer_set_out
}

%{
extern void gibbon_gmd_reader_yyerror (void *scanner, const gchar *msg);
#define yyerror(scanner, msg) gibbon_gmd_reader_yyerror (scanner, msg)
%}

%option yylineno
%option nounput
%option noinput
%option noyywrap
%option case-insensitive
%option stack
%option noyy_top_state
%option reentrant
%option bison-bridge

/* %option debug */

NL (\r\n|\n|\r\n|\r)

%s OPCODE
%s SCOLON
%s SCOLOR
%s STIMESTAMP
%s SLENGTH
%s SRULE
%s SNAME
%s SMOVE
%s EAT_UP
%s SGAME
%s SLBRACE
%s STUPLE

%%

  /***************************************************************************
   * General rules, valid for all scanner states.                            *
   ***************************************************************************/
   
  /* Ignore comments.  */
#.*{NL}		   		{
		BEGIN (OPCODE);
	}
  /* Ignore all whitespace.  */
[ \t\f\v][ \t\f\v]*		;

<INITIAL>GMD			{
		/* The file magic is case-sensitive! */
		if ('G' == yytext[0] && 'M' == yytext[1] && 'D' == yytext[2])
	        	return MAGIC;
	        return yytext[0];
	}
<INITIAL>-                      return HYPHEN;

<OPCODE>length			{
		BEGIN (SLENGTH);
		yy_push_state (SCOLON, yyscanner);
		return LENGTH;
	}
<SLENGTH>unlimited		{
		BEGIN (OPCODE);
		((struct yyguts_t *) yyscanner)->yylval_r->num = -1;
		return POSITIVE;
	}
<SLENGTH>money    		{
		BEGIN (OPCODE);
		((struct yyguts_t *) yyscanner)->yylval_r->num = -1;
		return POSITIVE;
	}
	
<STIMESTAMP>:|{NL}             	{
		yy_pop_state (yyscanner);
		yyless (0);
		((struct yyguts_t *) yyscanner)->yylval_r->num = G_MININT64;
		return POSITIVE;
	}
<STIMESTAMP>-?[1-9][0-9]*       {
        	((struct yyguts_t *) yyscanner)->yylval_r->num 
        		= (gint64) g_ascii_strtoll (yytext, NULL, 10);
        	yy_pop_state (yyscanner);
        	return POSITIVE;
        }

<OPCODE>game                    {
                yy_push_state (SGAME, yyscanner);
		yy_push_state (SCOLON, yyscanner);
		return GAME;
	}

<SGAME>{NL}			{
		BEGIN (OPCODE);
	}
	
<SGAME>Points			{
		yy_push_state (SLBRACE, yyscanner);
		return POINTS;
}

<SGAME>Dice			{
		yy_push_state (SLBRACE, yyscanner);
		return DICE;
}

<SGAME>Scores			{
		yy_push_state (SLBRACE, yyscanner);
		return SCORES;
}

<SGAME>Turn			{
		yy_push_state (SLBRACE, yyscanner);
		return TURN;
}

<SGAME>Cube			{
		yy_push_state (SLBRACE, yyscanner);
		return CUBE;
}

<SGAME>May-Double		{
		yy_push_state (SLBRACE, yyscanner);
		return MAY_DOUBLE;
}

<SLBRACE>\{			{
		yy_push_state (STUPLE, yyscanner);
		return LBRACE;
}

<STUPLE>[+-]?[0-9]+             {
		((struct yyguts_t *) yyscanner)->yylval_r->num 
			= (gint) g_ascii_strtoll (yytext, NULL, 10);
		return INTEGER;
	}

<STUPLE>\}             		{
		yy_pop_state (yyscanner);
		BEGIN (SGAME);
		return RBRACE;
	}

<OPCODE>player                  {
		BEGIN (SNAME);
		yy_push_state (SCOLON, yyscanner);
		yy_push_state (SCOLOR, yyscanner);
		yy_push_state (SCOLON, yyscanner);
		return PLAYER;
	}
<OPCODE>rank                    {
		BEGIN (SNAME);
		yy_push_state (SCOLON, yyscanner);
		yy_push_state (SCOLOR, yyscanner);
		yy_push_state (SCOLON, yyscanner);
		return RANK;
	}
<OPCODE>location                {
		BEGIN (SNAME);
		yy_push_state (SCOLON, yyscanner);
		return LOCATION;
	}
<SNAME>.*{NL}			{
		BEGIN (OPCODE);
		((struct yyguts_t *) yyscanner)->yylval_r->name 
			= alloc_name (gibbon_trim (yytext));
		return NAME;
	}

<OPCODE>rule			{
		BEGIN (SRULE);
		yy_push_state (SCOLON, yyscanner);
		return RULE;
	}
<SRULE>[ \t\f\v]*crawford[ \t\f\v]*	{
		BEGIN (OPCODE);
		return CRAWFORD;
	}
<SRULE>.*			{
		gchar *msg = g_strdup_printf (_("warning: unknown rule `%s'"), 
		                              gibbon_trim (yytext));
		yyerror (yyscanner, msg);
		g_free (msg);

		BEGIN (OPCODE);
		return UNKNOWN;
	}
	
<OPCODE>roll			{
		BEGIN (OPCODE);
		yy_push_state (SCOLON, yyscanner);
		yy_push_state (STIMESTAMP, yyscanner);
		yy_push_state (SCOLON, yyscanner);
		yy_push_state (SCOLOR, yyscanner);
		yy_push_state (SCOLON, yyscanner);
		return ROLL;
	}
		
<OPCODE>move			{
		BEGIN (SMOVE);
		yy_push_state (SCOLON, yyscanner);
		yy_push_state (STIMESTAMP, yyscanner);
		yy_push_state (SCOLON, yyscanner);
		yy_push_state (SCOLOR, yyscanner);
		yy_push_state (SCOLON, yyscanner);
		return MOVE;
	}
<SMOVE>[-/]			{
		return SLASH;
	}
<SMOVE>b(ar)?			{
		return BAR;
	}
<SMOVE>h(ome)?			{
		return HOME;
	}
<SMOVE>o(ff)?			{
		return HOME;
	}
	
<OPCODE>double			{
		BEGIN (OPCODE);
		yy_push_state (STIMESTAMP, yyscanner);
		yy_push_state (SCOLON, yyscanner);
		yy_push_state (SCOLOR, yyscanner);
		yy_push_state (SCOLON, yyscanner);
		return DOUBLE;
	}

<OPCODE>drop			{
		BEGIN (OPCODE);
		yy_push_state (STIMESTAMP, yyscanner);
		yy_push_state (SCOLON, yyscanner);
		yy_push_state (SCOLOR, yyscanner);
		yy_push_state (SCOLON, yyscanner);
		return DROP;
	}

<OPCODE>take			{
		BEGIN (OPCODE);
		yy_push_state (STIMESTAMP, yyscanner);
		yy_push_state (SCOLON, yyscanner);
		yy_push_state (SCOLOR, yyscanner);
		yy_push_state (SCOLON, yyscanner);
		return TAKE;
	}

<OPCODE>resign			{
		BEGIN (OPCODE);
		yy_push_state (SCOLON, yyscanner);
		yy_push_state (STIMESTAMP, yyscanner);
		yy_push_state (SCOLON, yyscanner);
		yy_push_state (SCOLOR, yyscanner);
		yy_push_state (SCOLON, yyscanner);
		return RESIGN;
	}
	
<OPCODE>reject			{
		BEGIN (OPCODE);
		yy_push_state (STIMESTAMP, yyscanner);
		yy_push_state (SCOLON, yyscanner);
		yy_push_state (SCOLOR, yyscanner);
		yy_push_state (SCOLON, yyscanner);
		return REJ;
	}

<OPCODE>accept			{
		BEGIN (OPCODE);
		yy_push_state (STIMESTAMP, yyscanner);
		yy_push_state (SCOLON, yyscanner);
		yy_push_state (SCOLOR, yyscanner);
		yy_push_state (SCOLON, yyscanner);
		return ACCEPT;
	}

<OPCODE>[_a-z][_a-z0-9]*        {
		gchar *msg = g_strdup_printf (_("warning: unknown opcode `%s'"), 
		                              yytext);
		yyerror (yyscanner, msg);
		g_free (msg);
		BEGIN (EAT_UP);
		yy_push_state (SCOLON, yyscanner);
		return UNKNOWN;
	}

<EAT_UP>.*                      {
		BEGIN (OPCODE);
		return GARBAGE;
	}

<SCOLON>:			{
		yy_pop_state (yyscanner);
		return COLON;
	}
	
<SCOLOR>-			{
		yy_pop_state (yyscanner);
		return NO_COLOR;
	}
<SCOLOR>[0OW]			{
		yy_pop_state (yyscanner);
		return WHITE;
	}
<SCOLOR>white			{
		yy_pop_state (yyscanner);
		return WHITE;
	}
<SCOLOR>[1XB]			{
		yy_pop_state (yyscanner);
		return BLACK;
	}
<SCOLOR>black			{
		yy_pop_state (yyscanner);
		return BLACK;
	}
					
{NL}				BEGIN (OPCODE);

[0-9]+                          {
		((struct yyguts_t *) yyscanner)->yylval_r->num 
			= (gint) g_ascii_strtoull (yytext, NULL, 10);
		return POSITIVE;
	}

.				return yytext[0];
%%
