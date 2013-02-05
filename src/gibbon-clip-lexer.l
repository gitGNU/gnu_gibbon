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
 * Lexical scanner for FIBS output.
 */

%{
#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include <glib.h>
#include <glib/gi18n.h>

#include "gibbon-clip-reader-priv.h"
#include "gibbon-util.h"
#include "gibbon-clip.h"

#define reader gibbon_clip_lexer_get_extra(yyscanner)

%}

%top{
#define yy_create_buffer gibbon_clip_lexer__create_buffer
#define yy_delete_buffer gibbon_clip_lexer__delete_buffer
#define yy_flush_buffer gibbon_clip_lexer__flush_buffer
#define yy_scan_buffer gibbon_clip_lexer__scan_buffer
#define yy_scan_bytes gibbon_clip_lexer__scan_bytes
#define yy_scan_string gibbon_clip_lexer__scan_string
#define yy_switch_to_buffer gibbon_clip_lexer__switch_to_buffer
#define yyalloc gibbon_clip_lexer_alloc
#define yyfree gibbon_clip_lexer_free
#define yyget_column gibbon_clip_lexer_get_column
#define yyget_debug gibbon_clip_lexer_get_debug
#define yyget_extra gibbon_clip_lexer_get_extra
#define yyget_in gibbon_clip_lexer_get_in
#define yyget_leng gibbon_clip_lexer_get_leng
#define yyget_lineno gibbon_clip_lexer_get_lineno
#define yyget_lval gibbon_clip_lexer_get_lval
#define yyget_out gibbon_clip_lexer_get_out
#define yyget_text gibbon_clip_lexer_get_text
#define yylex gibbon_clip_lexer_lex
#define yylex_destroy gibbon_clip_lexer_lex_destroy
#define yylex_init gibbon_clip_lexer_lex_init
#define yylex_init_extra gibbon_clip_lexer_lex_init_extra
#define yypop_buffer_state gibbon_clip_lexer_pop_buffer_state
#define yypush_buffer_state gibbon_clip_lexer_push_buffer_state
#define yyrealloc gibbon_clip_lexer_realloc
#define yyrestart gibbon_clip_lexer_restart
#define yyset_column gibbon_clip_lexer_set_column
#define yyset_debug gibbon_clip_lexer_set_debug
#define yyset_extra gibbon_clip_lexer_set_extra
#define yyset_in gibbon_clip_lexer_set_in
#define yyset_lineno gibbon_clip_lexer_set_lineno
#define yyset_lval gibbon_clip_lexer_set_lval
#define yyset_out gibbon_clip_lexer_set_out
}

%option yylineno
%option nounput
%option noinput
%option noyywrap
%option reentrant

%option debug

USER		[^-.:, \t]+
CHUNK		[^ \t]+
NUMBER		[-+]?(0|[1-9][0-9]*)
DOUBLE		[-+]?(0|[1-9][0-9]*)(\.[0-9]+)?
FLAG		[01]
WS		[ \t]+
OWS		[ \t]*
/* Think full stop.  */
FS		[ \t]*\.[ \t]*

%x MOTD
%x MESSAGE

%%
<INITIAL>^1{WS}{USER}{WS}{NUMBER}{WS}{CHUNK}	{
		if (!gibbon_clip_reader_set_result (reader, yytext, 4,
		                                    TRUE,
		                                    GIBBON_CLIP_WELCOME,
		                                    GIBBON_TT_HOSTNAME, 3,
		                                    GIBBON_TT_TIMESTAMP, 2,
		                                    GIBBON_TT_USER, 1,
		                                    GIBBON_TT_END))
			return -1;

		return 1;
	}
<INITIAL>^2{WS}{USER}({WS}{FLAG}){8}{WS}{NUMBER}({WS}{FLAG}){4}{WS}{DOUBLE}({WS}{FLAG}){2}{WS}({NUMBER}|unlimited)({WS}{FLAG}){2}{WS}{CHUNK}	{
		if (!gibbon_clip_reader_set_result (reader, yytext, 22,
		                                    TRUE,
		                                    GIBBON_CLIP_OWN_INFO,
		                                    GIBBON_TT_WORD, 21,
		                                    GIBBON_TT_BOOLEAN, 20,
		                                    GIBBON_TT_BOOLEAN, 19,
		                                    GIBBON_TT_REDOUBLES, 18,
		                                    GIBBON_TT_BOOLEAN, 17,
		                                    GIBBON_TT_BOOLEAN, 16,
		                                    GIBBON_TT_DOUBLE, 15,
		                                    GIBBON_TT_BOOLEAN, 14,
		                                    GIBBON_TT_BOOLEAN, 13,
		                                    GIBBON_TT_BOOLEAN, 12,
		                                    GIBBON_TT_BOOLEAN, 11,
		                                    GIBBON_TT_NATURAL, 10,
		                                    GIBBON_TT_BOOLEAN, 9,
		                                    GIBBON_TT_BOOLEAN, 8,
		                                    GIBBON_TT_BOOLEAN, 7,
		                                    GIBBON_TT_BOOLEAN, 6,
		                                    GIBBON_TT_BOOLEAN, 5,
		                                    GIBBON_TT_BOOLEAN, 4,
		                                    GIBBON_TT_BOOLEAN, 3,
		                                    GIBBON_TT_BOOLEAN, 2,
		                                    GIBBON_TT_USER, 1,
		                                    GIBBON_TT_END))
			return -1;

		return 1;
	}
<INITIAL>^3	{
		if (!gibbon_clip_reader_set_result (reader, yytext, 0,
		                                    FALSE,
		                                    GIBBON_CLIP_MOTD_START,
		                                    GIBBON_TT_END))
			return -1;
		BEGIN (MOTD);
		return 1;
	}
<MOTD>^4	{
		if (!gibbon_clip_reader_set_result (reader, yytext, 0,
		                                    FALSE,
		                                    GIBBON_CLIP_MOTD_END,
		                                    GIBBON_TT_END))
			return -1;
		BEGIN (INITIAL);
		return 1;
	}
<MOTD>^(.|\n)*	{
		if (!gibbon_clip_reader_set_result (reader, yytext, 1,
		                                    FALSE,
		                                    GIBBON_CLIP_MOTD,
		                                    GIBBON_TT_MESSAGE, 0,
		                                    GIBBON_TT_END))
			return -1;
		return 1;
	}
<INITIAL>^5{WS}{USER}({WS}{CHUNK}){2}({WS}{FLAG}){2}{WS}{DOUBLE}({WS}{NUMBER}){3}({WS}{CHUNK}){3}	{
		if (!gibbon_clip_reader_set_result (reader, yytext, 13,
		                                    TRUE,
		                                    GIBBON_CLIP_WHO_INFO,
		                                    GIBBON_TT_WORD, 12,
		                                    GIBBON_TT_WORD, 11,
		                                    GIBBON_TT_HOSTNAME, 10,
		                                    GIBBON_TT_TIMESTAMP, 9,
		                                    GIBBON_TT_NATURAL, 8,
		                                    GIBBON_TT_NATURAL, 7,
		                                    GIBBON_TT_DOUBLE, 6,
		                                    GIBBON_TT_BOOLEAN, 5,
		                                    GIBBON_TT_BOOLEAN, 4,
		                                    GIBBON_TT_MAYBE_USER, 3,
		                                    GIBBON_TT_MAYBE_USER, 2,
		                                    GIBBON_TT_USER, 1,
		                                    GIBBON_TT_END))
			return -1;

		return 1;
	}
<INITIAL>^6	{
		if (!gibbon_clip_reader_set_result (reader, yytext, 0,
		                                    FALSE,
		                                    GIBBON_CLIP_WHO_INFO_END,
		                                    GIBBON_TT_END))
			return -1;
		return 1;
	}
<INITIAL>^7{WS}{USER}.+	{
		if (!gibbon_clip_reader_set_result (reader, yytext, 3,
		                                    TRUE,
		                                    GIBBON_CLIP_LOGIN,
		                                    GIBBON_TT_MESSAGE, 2,
		                                    GIBBON_TT_USER, 1,
		                                    GIBBON_TT_END))
			return -1;

		return 1;
	}
<INITIAL>^8{WS}{USER}.+	{
		if (!gibbon_clip_reader_set_result (reader, yytext, 3,
		                                    TRUE,
		                                    GIBBON_CLIP_LOGOUT,
		                                    GIBBON_TT_MESSAGE, 2,
		                                    GIBBON_TT_USER, 1,
		                                    GIBBON_TT_END))
			return -1;

		return 1;
	}
<INITIAL>^9{WS}{USER}{WS}{NUMBER}	{
		if (!gibbon_clip_reader_set_result (reader, yytext, 3,
		                                    TRUE,
		                                    GIBBON_CLIP_MESSAGE,
		                                    GIBBON_TT_TIMESTAMP, 2,
		                                    GIBBON_TT_USER, 1,
		                                    GIBBON_TT_END))
			return -1;

		BEGIN (MESSAGE);

		return 1;
	}
<INITIAL>^10{WS}{USER}	{
		if (!gibbon_clip_reader_set_result (reader, yytext, 2,
		                                    TRUE,
		                                    GIBBON_CLIP_MESSAGE_DELIVERED,
		                                    GIBBON_TT_USER, 1,
		                                    GIBBON_TT_END))
			return -1;

		return 1;
	}
<INITIAL>^11{WS}{USER}	{
		if (!gibbon_clip_reader_set_result (reader, yytext, 2,
		                                    TRUE,
		                                    GIBBON_CLIP_MESSAGE_SAVED,
		                                    GIBBON_TT_USER, 1,
		                                    GIBBON_TT_END))
			return -1;

		return 1;
	}
<MESSAGE>[ \t](.|\n)*	{
		if (!gibbon_clip_reader_append_message (reader, yytext + 1))
			return -1;
			
		BEGIN (INITIAL);
		return 1;
	}
[. \t]+		/* Ignore (including a full-stop.  */
.|\n			{
		return -1;
		BEGIN (INITIAL);
	}
%%

void 
gibbon_clip_lexer_current_buffer (yyscan_t yyscanner, const gchar *line)
{
	struct yyguts_t * yyg = (struct yyguts_t*) yyscanner;
	
	/*
	 * In a re-entrant scanner, the debug state is initialized to 0,
	 * not to 1.
	 */
	yy_flex_debug = 1;

	yy_delete_buffer (YY_CURRENT_BUFFER, yyscanner);
	yy_switch_to_buffer (yy_scan_string (line, yyscanner), yyscanner);
}

void 
gibbon_clip_lexer_reset_condition_stack (void *yyscanner)
{
	struct yyguts_t * yyg = (struct yyguts_t*) yyscanner;
	
	BEGIN (0);
}
