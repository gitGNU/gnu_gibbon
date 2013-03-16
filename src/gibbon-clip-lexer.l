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
POINT		(off|bar|[1-9]|1[0-9]|2[0-4])
DOUBLE		[-+]?(0|[1-9][0-9]*)(\.[0-9]+)?
FLAG		[01]
WS		[ \t]+
SEP		[- \t]+
OWS		[ \t]*
/* Think full stop.  */
FS		[ \t]*\.[ \t]*

%x MOTD
%x MESSAGE
%x SETTINGS
%x TOGGLES
%x SAVED

%%
<INITIAL>^1{WS}{USER}{WS}{NUMBER}{WS}{CHUNK}	{
		if (!gibbon_clip_reader_set_result (reader, yytext, 4,
		                                    " \t",
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
		                                    " \t",
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
		                                    GIBBON_TT_N0, 10,
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
		                                    NULL,
		                                    GIBBON_CLIP_MOTD_START,
		                                    GIBBON_TT_END))
			return -1;
		BEGIN (MOTD);
		return 1;
	}
<MOTD>^4	{
		if (!gibbon_clip_reader_set_result (reader, yytext, 0,
		                                    NULL,
		                                    GIBBON_CLIP_MOTD_END,
		                                    GIBBON_TT_END))
			return -1;
		BEGIN (INITIAL);
		return 1;
	}
<MOTD>^(.|\n)*	{
		if (!gibbon_clip_reader_set_result (reader, yytext, 1,
		                                    " ",
		                                    GIBBON_CLIP_MOTD,
		                                    GIBBON_TT_MESSAGE, 0,
		                                    GIBBON_TT_END))
			return -1;
		return 1;
	}
<INITIAL>^5{WS}{USER}({WS}{CHUNK}){2}({WS}{FLAG}){2}{WS}{DOUBLE}({WS}{NUMBER}){3}({WS}{CHUNK}){3}	{
		if (!gibbon_clip_reader_set_result (reader, yytext, 13,
		                                    " \t",
		                                    GIBBON_CLIP_WHO_INFO,
		                                    GIBBON_TT_WORD, 12,
		                                    GIBBON_TT_WORD, 11,
		                                    GIBBON_TT_HOSTNAME, 10,
		                                    GIBBON_TT_TIMESTAMP, 9,
		                                    GIBBON_TT_N0, 8,
		                                    GIBBON_TT_N0, 7,
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
		                                    NULL,
		                                    GIBBON_CLIP_WHO_INFO_END,
		                                    GIBBON_TT_END))
			return -1;
		return 1;
	}
<INITIAL>^7{WS}{USER}.+	{
		if (!gibbon_clip_reader_set_result (reader, yytext, 3,
		                                    " \t",
		                                    GIBBON_CLIP_LOGIN,
		                                    GIBBON_TT_MESSAGE, 2,
		                                    GIBBON_TT_USER, 1,
		                                    GIBBON_TT_END))
			return -1;

		return 1;
	}
<INITIAL>^8{WS}{USER}.+	{
		if (!gibbon_clip_reader_set_result (reader, yytext, 3,
		                                    " \t",
		                                    GIBBON_CLIP_LOGOUT,
		                                    GIBBON_TT_MESSAGE, 2,
		                                    GIBBON_TT_USER, 1,
		                                    GIBBON_TT_END))
			return -1;

		return 1;
	}
<INITIAL>^9{WS}{USER}{WS}{NUMBER}	{
		if (!gibbon_clip_reader_set_result (reader, yytext, 3,
		                                    " \t",
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
		                                    " \t",
		                                    GIBBON_CLIP_MESSAGE_DELIVERED,
		                                    GIBBON_TT_USER, 1,
		                                    GIBBON_TT_END))
			return -1;

		return 1;
	}
<INITIAL>^11{WS}{USER}	{
		if (!gibbon_clip_reader_set_result (reader, yytext, 2,
		                                    " \t",
		                                    GIBBON_CLIP_MESSAGE_SAVED,
		                                    GIBBON_TT_USER, 1,
		                                    GIBBON_TT_END))
			return -1;

		return 1;
	}
<INITIAL>^(1[2-6]|20){WS}{USER}	{
		if (!gibbon_clip_reader_set_result (reader, yytext, 2,
		                                    " \t",
		                                    10 * (yytext[0] - '0')
		                                    + yytext[1] - '0',
		                                    GIBBON_TT_USER, 1,
		                                    GIBBON_TT_END))
			return -1;

		BEGIN (MESSAGE);

		return 1;
	}
<INITIAL>^(1[7-9])	{
		if (!gibbon_clip_reader_set_result (reader, yytext, 1,
		                                    " \t",
		                                    10 + yytext[1] - '0',
		                                    GIBBON_TT_END))
			return -1;

		BEGIN (MESSAGE);

		return 1;
	}
<INITIAL>^\*\*{WS}There{WS}is{WS}no{OWS}one{WS}called{WS}{USER} {
		if (!gibbon_clip_reader_set_result (reader, yytext, 7,
		                                    " \t",
		                                    GIBBON_CLIP_NO_SUCH_USER,
		                                    GIBBON_TT_USER, -1,
		                                    GIBBON_TT_END))
			return -1;

		return 1;
	}
<INITIAL>^board:{USER}:{USER}(:{NUMBER}){50}	{
		gchar **tokens = g_strsplit (yytext + 6, ":", -1);
		gboolean success = gibbon_clip_reader_set_board (reader, 
		                                                 tokens);
		
		g_strfreev (tokens);

		if (success)
			return 1;
		else
			return -1;
	}
<INITIAL>^{USER}{WS}rolls?{WS}{NUMBER}{WS}and{WS}{NUMBER}	{
		if (!gibbon_clip_reader_set_result (reader, yytext, 5,
		                                    " \t",
		                                    GIBBON_CLIP_ROLLS,
		                                    GIBBON_TT_DIE, 4,
		                                    GIBBON_TT_DIE, 2,
		                                    GIBBON_TT_MAYBE_YOU, 0,
		                                    GIBBON_TT_END))
			return -1;

		return 1;
	}
<INITIAL>^{USER}{WS}moves?({WS}{POINT}{OWS}-{OWS}{POINT}){4}	{
		if (!gibbon_clip_reader_set_result (reader, yytext, 10,
		                                    "- \t",
		                                    GIBBON_CLIP_MOVES,
		                                    GIBBON_TT_POINT, 9,
		                                    GIBBON_TT_POINT, 8,
		                                    GIBBON_TT_POINT, 7,
		                                    GIBBON_TT_POINT, 6,
		                                    GIBBON_TT_POINT, 5,
		                                    GIBBON_TT_POINT, 4,
		                                    GIBBON_TT_POINT, 3,
		                                    GIBBON_TT_POINT, 2,
		                                    GIBBON_TT_MAYBE_YOU, 0,
		                                    GIBBON_TT_END))
			return -1;
		if (!gibbon_clip_reader_fixup_moves (reader))
			return -1;
		return 1;
	}
<INITIAL>^{USER}{WS}moves?({WS}{POINT}{OWS}-{OWS}{POINT}){3}	{
		if (!gibbon_clip_reader_set_result (reader, yytext, 8,
		                                    "- \t",
		                                    GIBBON_CLIP_MOVES,
		                                    GIBBON_TT_POINT, 7,
		                                    GIBBON_TT_POINT, 6,
		                                    GIBBON_TT_POINT, 5,
		                                    GIBBON_TT_POINT, 4,
		                                    GIBBON_TT_POINT, 3,
		                                    GIBBON_TT_POINT, 2,
		                                    GIBBON_TT_MAYBE_YOU, 0,
		                                    GIBBON_TT_END))
			return -1;
		if (!gibbon_clip_reader_fixup_moves (reader))
			return -1;
		
		return 1;
	}
<INITIAL>^{USER}{WS}moves?({WS}{POINT}{OWS}-{OWS}{POINT}){2}	{
		if (!gibbon_clip_reader_set_result (reader, yytext, 6,
		                                    "- \t",
		                                    GIBBON_CLIP_MOVES,
		                                    GIBBON_TT_POINT, 5,
		                                    GIBBON_TT_POINT, 4,
		                                    GIBBON_TT_POINT, 3,
		                                    GIBBON_TT_POINT, 2,
		                                    GIBBON_TT_MAYBE_YOU, 0,
		                                    GIBBON_TT_END))
			return -1;
		if (!gibbon_clip_reader_fixup_moves (reader))
			return -1;
		
		return 1;
	}
<INITIAL>^{USER}{WS}moves?{WS}{POINT}{OWS}-{OWS}{POINT}	{
		if (!gibbon_clip_reader_set_result (reader, yytext, 4,
		                                    "- \t",
		                                    GIBBON_CLIP_MOVES,
		                                    GIBBON_TT_POINT, 3,
		                                    GIBBON_TT_POINT, 2,
		                                    GIBBON_TT_MAYBE_YOU, 0,
		                                    GIBBON_TT_END))
			return -1;
		if (!gibbon_clip_reader_fixup_moves (reader))
			return -1;
		
		return 1;
	}
<INITIAL>^Starting{WS}a{WS}new{WS}game{WS}with{WS}{USER}	{
		if (!gibbon_clip_reader_set_result (reader, yytext, 6,
		                                    " \t",
		                                    GIBBON_CLIP_START_GAME,
		                                    GIBBON_TT_USER, -1,
		                                    GIBBON_TT_END))
			return -1;

		return 1;
	}
<INITIAL>^\*\*{WS}{USER}{WS}terminated{WS}the{WS}game\.{WS}The{WS}game{WS}was{WS}saved	{
		if (!gibbon_clip_reader_set_result (reader, yytext, 9,
		                                    " \t",
		                                    GIBBON_CLIP_LEFT_GAME,
		                                    GIBBON_TT_MAYBE_YOU, 1,
		                                    GIBBON_TT_END))
			return -1;

		return 1;
	}
<INITIAL>^\*\*{WS}Player{WS}{USER}{WS}terminated{WS}the{WS}game\.{WS}The{WS}game{WS}was{WS}saved	{
		if (!gibbon_clip_reader_set_result (reader, yytext, 10,
		                                    " \t",
		                                    GIBBON_CLIP_LEFT_GAME,
		                                    GIBBON_TT_MAYBE_YOU, 2,
		                                    GIBBON_TT_END))
			return -1;

		return 1;
	}
<INITIAL>^{USER}{WS}logs{WS}out\.{WS}The{WS}game{WS}was{WS}saved	{
		if (!gibbon_clip_reader_set_result (reader, yytext, 7,
		                                    " \t",
		                                    GIBBON_CLIP_DROPS_GAME,
		                                    GIBBON_TT_USER, 0,
		                                    GIBBON_TT_END))
			return -1;

		return 1;
	}
<INITIAL>^{USER}{WS}drops{WS}connection\.{WS}The{WS}game{WS}was{WS}saved	{
		if (!gibbon_clip_reader_set_result (reader, yytext, 7,
		                                    " \t",
		                                    GIBBON_CLIP_DROPS_GAME,
		                                    GIBBON_TT_USER, 0,
		                                    GIBBON_TT_END))
			return -1;

		return 1;
	}
<INITIAL>^Connection{WS}with{WS}{USER}{WS}timed{WS}out\.{WS}The{WS}game{WS}was{WS}saved	{
		if (!gibbon_clip_reader_set_result (reader, yytext, 9,
		                                    " \t",
		                                    GIBBON_CLIP_DROPS_GAME,
		                                    GIBBON_TT_USER, 2,
		                                    GIBBON_TT_END))
			return -1;

		return 1;
	}
<INITIAL>^{USER}{WS}can\x27t{WS}move	{
		if (!gibbon_clip_reader_set_result (reader, yytext, 3,
		                                    " \t",
		                                    GIBBON_CLIP_CANNOT_MOVE,
		                                    GIBBON_TT_MAYBE_YOU, 0,
		                                    GIBBON_TT_END))
			return -1;

		return 1;
	}
<INITIAL>^You{WS}double\.{WS}Please{WS}wait{WS}for{WS}{USER}{WS}to{WS}accept{WS}or{WS}reject	{
		if (!gibbon_clip_reader_set_result (reader, yytext, 10,
		                                    " \t",
		                                    GIBBON_CLIP_DOUBLES,
		                                    GIBBON_TT_MAYBE_YOU, 0,
		                                    GIBBON_TT_END))
			return -1;

		return 1;
	}
<INITIAL>^{USER}{WS}doubles\.{WS}Type{WS}\x27accept\x27{WS}or{WS}\x27reject\x27	{
		if (!gibbon_clip_reader_set_result (reader, yytext, 6,
		                                    " \t",
		                                    GIBBON_CLIP_DOUBLES,
		                                    GIBBON_TT_USER, 0,
		                                    GIBBON_TT_END))
			return -1;

		return 1;
	}
<INITIAL>^{USER}{WS}accepts?{WS}the{WS}double\.{WS}The{WS}cube{WS}shows{WS}{NUMBER}	{
		if (!gibbon_clip_reader_set_result (reader, yytext, 9,
		                                    " \t",
		                                    GIBBON_CLIP_ACCEPTS_DOUBLE,
		                                    GIBBON_TT_CUBE, -1,
		                                    GIBBON_TT_MAYBE_YOU, 0,
		                                    GIBBON_TT_END))
			return -1;

		return 1;
	}
<INITIAL>^{USER}{WS}wants{WS}to{WS}play{WS}a{WS}{NUMBER}{WS}point{WS}match{WS}with{WS}you {
		if (!gibbon_clip_reader_set_result (reader, yytext, 10,
		                                    " \t",
		                                    GIBBON_CLIP_INVITATION,
		                                    GIBBON_TT_MATCH_LENGTH, 5,
		                                    GIBBON_TT_USER, 0,
		                                    GIBBON_TT_END))
			return -1;

		return 1;
	}
<INITIAL>^{USER}{WS}wants{WS}to{WS}play{WS}an{WS}unlimited{WS}match{WS}with{WS}you {
		if (!gibbon_clip_reader_set_result (reader, yytext, 9,
		                                    " \t",
		                                    GIBBON_CLIP_INVITATION,
		                                    GIBBON_TT_MATCH_LENGTH, 5,
		                                    GIBBON_TT_USER, 0,
		                                    GIBBON_TT_END))
			return -1;

		return 1;
	}
<INITIAL>^{USER}{WS}wants{WS}to{WS}resume{WS}a{WS}saved{WS}match{WS}with{WS}you {
		if (!gibbon_clip_reader_set_result (reader, yytext, 9,
		                                    " \t",
		                                    GIBBON_CLIP_INVITATION,
		                                    GIBBON_TT_MATCH_LENGTH, 3,
		                                    GIBBON_TT_USER, 0,
		                                    GIBBON_TT_END))
			return -1;

		return 1;
	}
<INITIAL>^Type{WS}\x27join{WS}{USER}\x27{WS}to{WS}accept   {
		if (!gibbon_clip_reader_set_result (reader, yytext, 5,
		                                    "' \t",
		                                    GIBBON_CLIP_TYPE_JOIN,
		                                    GIBBON_TT_USER, 2,
		                                    GIBBON_TT_END))
			return -1;

		return 1;
	}
<INITIAL>^You\x27re{WS}now{WS}watching{WS}{USER}   {
		if (!gibbon_clip_reader_set_result (reader, yytext, 4,
		                                    " \t",
		                                    GIBBON_CLIP_YOURE_WATCHING,
		                                    GIBBON_TT_USER, -1,
		                                    GIBBON_TT_END))
			return -1;

		return 1;
	}
<INITIAL>^\*\*{WS}You{WS}are{WS}now{WS}playing{WS}a{WS}{NUMBER}{WS}point{WS}match{WS}with{WS}{USER}   {
		if (!gibbon_clip_reader_set_result (reader, yytext, 11,
		                                    " \t",
		                                    GIBBON_CLIP_NOW_PLAYING,
		                                    GIBBON_TT_USER, -1,
		                                    GIBBON_TT_MATCH_LENGTH, 6,
		                                    GIBBON_TT_END))
			return -1;

		return 1;
	}
<INITIAL>^\*\*{WS}You{WS}are{WS}now{WS}playing{WS}an{WS}unlimited{WS}match{WS}with{WS}{USER}   {
		if (!gibbon_clip_reader_set_result (reader, yytext, 10,
		                                    " \t",
		                                    GIBBON_CLIP_NOW_PLAYING,
		                                    GIBBON_TT_USER, -1,
		                                    GIBBON_TT_MATCH_LENGTH, 6,
		                                    GIBBON_TT_END))
			return -1;

		return 1;
	}
<INITIAL>^\*\*{WS}{USER}{WS}is{WS}refusing{WS}games   {
		gchar *ptr = yytext + 3;
		gchar *user;
		gchar *msg;
		
		if (!gibbon_clip_reader_set_result (reader, yytext, 5,
		                                    " \t",
		                                    GIBBON_CLIP_INVITE_ERROR,
		                                    GIBBON_TT_USER, 1,
		                                    GIBBON_TT_END))
			return -1;

		while (*ptr == ' ' || *ptr == '\t')
			++ptr;
		user = ptr;
		while (*ptr && *ptr != ' ' && *ptr != '\t')
			++ptr;
		*ptr = 0;
		
		msg = g_strdup_printf (_("Player `%s' is now refusing to play!"),
		                       user);
		if (!gibbon_clip_reader_append_message (reader, msg))
			return -1;
		g_free (msg);
		return 1;
	}
<INITIAL>^\*\*{WS}Error:{WS}{USER}{WS}is{WS}already{WS}playing{WS}with{WS}someone{WS}else   {
		gchar *ptr = 6 + strstr (yytext + 3, "Error:");
		gchar *user;
		gchar *msg;
		
		if (!gibbon_clip_reader_set_result (reader, yytext, 4,
		                                    " \t",
		                                    GIBBON_CLIP_INVITE_ERROR,
		                                    GIBBON_TT_USER, 2,
		                                    GIBBON_TT_END))
			return -1;

		while (*ptr == ' ' || *ptr == '\t')
			++ptr;
		user = ptr;
		while (*ptr && *ptr != ' ' && *ptr != '\t')
			++ptr;
		*ptr = 0;
		
		msg = g_strdup_printf (_("Player `%s' is already playing with"
		                         " someone else!"),
		                       user);
		if (!gibbon_clip_reader_append_message (reader, msg))
			return -1;
		g_free (msg);
		return 1;
	}
<INITIAL>^\*\*{WS}{USER}{WS}didn\x27t{WS}invite{WS}you   {
		gchar *ptr = yytext + 3;
		gchar *user;
		gchar *msg;
		
		if (!gibbon_clip_reader_set_result (reader, yytext, 3,
		                                    " \t",
		                                    GIBBON_CLIP_INVITE_ERROR,
		                                    GIBBON_TT_USER, 1,
		                                    GIBBON_TT_END))
			return -1;

		while (*ptr == ' ' || *ptr == '\t')
			++ptr;
		user = ptr;
		while (*ptr && *ptr != ' ' && *ptr != '\t')
			++ptr;
		*ptr = 0;
		
		msg = g_strdup_printf (_("Player `%s' is already playing with"
		                         " someone else!"),
		                       user);
		if (!gibbon_clip_reader_append_message (reader, msg))
			return -1;
		g_free (msg);
		return 1;
	}
<INITIAL>^\*\*{WS}Error:{WS}can\x27t{WS}find{WS}player{WS}{USER}   {
		gchar *ptr = 6 + strstr (yytext + 21, "player");
		gchar *user;
		gchar *msg;
		
		if (!gibbon_clip_reader_set_result (reader, yytext, 6,
		                                    " \t",
		                                    GIBBON_CLIP_INVITE_ERROR,
		                                    GIBBON_TT_USER, -1,
		                                    GIBBON_TT_END))
			return -1;

		while (*ptr == ' ' || *ptr == '\t')
			++ptr;
		user = ptr;
		while (*ptr && *ptr != ' ' && *ptr != '\t')
			++ptr;
		*ptr = 0;
		
		msg = g_strdup_printf (_("Player `%s' has logged out!"),
		                       user);
		if (!gibbon_clip_reader_append_message (reader, msg))
			return -1;
		g_free (msg);
		return 1;
	}
<INITIAL>^\*\*{WS}You{WS}can\x27t{WS}play{WS}two{WS}games{WS}at{WS}once {
		gibbon_clip_reader_set_error (reader,
   	                                      GIBBON_CLIP_ERROR_NO_TWO_MATCHES,
		                              "%s",
		                              _("You cannot play two games at"
		                                " once!"));

		return 1;
	}
<INITIAL>^\*\*{WS}Player{WS}{USER}{WS}has{WS}joined{WS}you{WS}for{WS}a{WS}{NUMBER}{WS}point{WS}match {
		if (!gibbon_clip_reader_set_result (reader, yytext, 9,
		                                    " \t",
		                                    GIBBON_CLIP_NOW_PLAYING,
		                                    GIBBON_TT_MATCH_LENGTH, 8,
		                                    GIBBON_TT_USER, 2,
		                                    GIBBON_TT_END))
			return -1;

		return 1;
	}
<INITIAL>^Player{WS}{USER}{WS}has{WS}joined{WS}you{WS}for{WS}an{WS}unlimited{WS}match {
		if (!gibbon_clip_reader_set_result (reader, yytext, 9,
		                                    " \t",
		                                    GIBBON_CLIP_NOW_PLAYING,
		                                    GIBBON_TT_MATCH_LENGTH, 7,
		                                    GIBBON_TT_USER, 1,
		                                    GIBBON_TT_END))
			return -1;

		return 1;
	}
<INITIAL>^Type{WS}\x27join\x27{WS}if{WS}you{WS}want{WS}to{WS}play{WS}the{WS}next{WS}game,{WS}type{WS}\x27leave\x27{WS}if{WS}you{WS}don\x27t {
		if (!gibbon_clip_reader_set_result (reader, yytext, 0,
		                                    "', \t",
		                                    GIBBON_CLIP_JOIN_OR_LEAVE,
		                                    GIBBON_TT_END))
			return -1;

		return 1;
	}
<INITIAL>^{USER}{WS}wins?{WS}the{WS}game{WS}and{WS}gets?{WS}{NUMBER}{WS}point.* 	{
		if (!gibbon_clip_reader_set_result (reader, yytext, 8,
		                                    " \t",
		                                    GIBBON_CLIP_WIN_GAME,
		                                    GIBBON_TT_POSITIVE, 6,
		                                    GIBBON_TT_MAYBE_YOU, 0,
		                                    GIBBON_TT_END))
			return -1;

		return 1;
	}
<INITIAL>^{USER}{WS}gives?{WS}up\.{WS}{USER}{WS}wins?{WS}{NUMBER}{WS}point.* 	{
		if (!gibbon_clip_reader_set_result (reader, yytext, 7,
		                                    ". \t",
		                                    GIBBON_CLIP_WIN_GAME,
		                                    GIBBON_TT_POSITIVE, 5,
		                                    GIBBON_TT_MAYBE_YOU, 3,
		                                    GIBBON_TT_END))
			return -1;

		return 1;
	}
<INITIAL>^{USER}{WS}accepts?{WS}and{WS}wins?{WS}{NUMBER}{WS}point.* 	{
		if (!gibbon_clip_reader_set_result (reader, yytext, 5,
		                                    " \t",
		                                    GIBBON_CLIP_WIN_GAME,
		                                    GIBBON_TT_POSITIVE, 4,
		                                    GIBBON_TT_MAYBE_YOU, 0,
		                                    GIBBON_TT_END))
			return -1;

		return 1;
	}
<INITIAL>^score{WS}in{WS}{NUMBER}{WS}point{WS}match:{WS}{USER}{OWS}-{OWS}{NUMBER}{WS}{USER}{OWS}-{OWS}{NUMBER} 	{
		if (!gibbon_clip_reader_set_result (reader, yytext, 9,
		                                    "- \t",
		                                    GIBBON_CLIP_GAME_SCORE,
		                                    GIBBON_TT_N0, 8,
		                                    GIBBON_TT_USER, 7,
		                                    GIBBON_TT_N0, 6,
		                                    GIBBON_TT_USER, 5,
		                                    GIBBON_TT_MATCH_LENGTH, 2,
		                                    GIBBON_TT_END))
			return -1;

		return 1;
	}
<INITIAL>^score{WS}in{WS}unlimited{WS}match:{WS}{USER}{OWS}-{OWS}{NUMBER}{WS}{USER}{OWS}-{OWS}{NUMBER} 	{
		if (!gibbon_clip_reader_set_result (reader, yytext, 8,
		                                    "- \t",
		                                    GIBBON_CLIP_GAME_SCORE,
		                                    GIBBON_TT_N0, 7,
		                                    GIBBON_TT_USER, 6,
		                                    GIBBON_TT_N0, 5,
		                                    GIBBON_TT_USER, 4,
		                                    GIBBON_TT_MATCH_LENGTH, 2,
		                                    GIBBON_TT_END))
			return -1;

		return 1;
	}
<INITIAL>^\*\*{WS}Please{WS}wait{WS}for{WS}{USER}{WS}to{WS}join{WS}too 	{
		if (!gibbon_clip_reader_set_result (reader, yytext, 8,
		                                    " \t",
		                                    GIBBON_CLIP_WAIT_JOIN_TOO,
		                                    GIBBON_TT_USER, 4,
		                                    GIBBON_TT_END))
			return -1;

		return 1;
	}
<INITIAL>^\*\*{WS}You{WS}invited{WS}{USER}{WS}to{WS}resume{WS}a{WS}saved{WS}match 	{
		if (!gibbon_clip_reader_set_result (reader, yytext, 7,
		                                    " \t",
		                                    GIBBON_CLIP_INVITE_CONFIRMATION,
		                                    GIBBON_TT_MATCH_LENGTH, 5,
		                                    GIBBON_TT_USER, 3,
		                                    GIBBON_TT_END))
			return -1;

		return 1;
	}
<INITIAL>^\*\*{WS}You{WS}invited{WS}{USER}{WS}to{WS}a{WS}{NUMBER}{WS}point{WS}match 	{
		if (!gibbon_clip_reader_set_result (reader, yytext, 8,
		                                    " \t",
		                                    GIBBON_CLIP_INVITE_CONFIRMATION,
		                                    GIBBON_TT_MATCH_LENGTH, 6,
		                                    GIBBON_TT_USER, 3,
		                                    GIBBON_TT_END))
			return -1;

		return 1;
	}
<INITIAL>^\*\*{WS}You{WS}invited{WS}{USER}{WS}to{WS}an?{WS}unlimited{WS}match 	{
		if (!gibbon_clip_reader_set_result (reader, yytext, 8,
		                                    " \t",
		                                    GIBBON_CLIP_INVITE_CONFIRMATION,
		                                    GIBBON_TT_MATCH_LENGTH, 6,
		                                    GIBBON_TT_USER, 3,
		                                    GIBBON_TT_END))
			return -1;

		return 1;
	}
<INITIAL>^{USER}{WS}wins?{WS}the{WS}{NUMBER}{WS}point{WS}match{WS}{NUMBER}{OWS}-{OWS}{NUMBER} 	{
		if (!gibbon_clip_reader_set_result (reader, yytext, 9,
		                                    "- \t",
		                                    GIBBON_CLIP_WIN_MATCH,
		                                    GIBBON_TT_N0, 7,
		                                    GIBBON_TT_N0, 6,
		                                    GIBBON_TT_MATCH_LENGTH, 3,
		                                    GIBBON_TT_MAYBE_YOU, 0,
		                                    GIBBON_TT_END))
			return -1;

		return 1;
	}
<INITIAL>^You{WS}stop{WS}watching{WS}{USER} 	{
		if (!gibbon_clip_reader_set_result (reader, yytext, 5,
		                                    " \t",
		                                    GIBBON_CLIP_STOP_WATCHING,
		                                    GIBBON_TT_MAYBE_YOU, -1,
		                                    GIBBON_TT_END))
			return -1;

		return 1;
	}
<INITIAL>^{USER}{WS}wants?{WS}to{WS}resign\.{WS}{USER}{WS}will{WS}win{WS}{NUMBER}{WS}point.* 	{
		if (!gibbon_clip_reader_set_result (reader, yytext, 9,
		                                    " \t",
		                                    GIBBON_CLIP_RESIGNS,
		                                    GIBBON_TT_POSITIVE, 7,
		                                    GIBBON_TT_MAYBE_YOU, 0,
		                                    GIBBON_TT_END))
			return -1;

		return 1;
	}
<INITIAL>^{USER}{WS}rejects?\.{WS}The{WS}game{WS}continues 	{
		if (!gibbon_clip_reader_set_result (reader, yytext, 2,
		                                    " \t",
		                                    GIBBON_CLIP_REJECTS_RESIGNATION,
		                                    GIBBON_TT_END))
			return -1;

		return 1;
	}
<INITIAL>^You{WS}are{WS}now{WS}playing{WS}with{WS}{USER}\.{WS}Your{WS}running{WS}match{WS}was{WS}loaded 	{
		if (!gibbon_clip_reader_set_result (reader, yytext, 7,
		                                    ". \t",
		                                    GIBBON_CLIP_RESUME,
		                                    GIBBON_TT_USER, 5,
		                                    GIBBON_TT_END))
			return -1;

		return 1;
	}
<INITIAL>^{USER}{WS}has{WS}joined{WS}you\.{WS}Your{WS}running{WS}match{WS}was{WS}loaded 	{
		if (!gibbon_clip_reader_set_result (reader, yytext, 2,
		                                    " \t",
		                                    GIBBON_CLIP_RESUME,
		                                    GIBBON_TT_USER, 0,
		                                    GIBBON_TT_END))
			return -1;

		return 1;
	}
<INITIAL>^turn:{WS}{USER} 	{
		if (!gibbon_clip_reader_set_result (reader, yytext, 2,
		                                    ": \t",
		                                    GIBBON_CLIP_RESUME_INFO_TURN,
		                                    GIBBON_TT_USER, 1,
		                                    GIBBON_TT_END))
			return -1;

		return 1;
	}
<INITIAL>^match{WS}length:{WS}{NUMBER} 	{
		if (!gibbon_clip_reader_set_result (reader, yytext, 4,
		                                    ": \t",
		                                    GIBBON_CLIP_RESUME_INFO_MATCH_LENGTH,
		                                    GIBBON_TT_MATCH_LENGTH, 2,
		                                    GIBBON_TT_END))
			return -1;

		return 1;
	}
<INITIAL>^unlimited{WS}match 	{
		if (!gibbon_clip_reader_set_result (reader, yytext, 3,
		                                    ": \t",
		                                    GIBBON_CLIP_RESUME_INFO_MATCH_LENGTH,
		                                    GIBBON_TT_MATCH_LENGTH, 0,
		                                    GIBBON_TT_END))
			return -1;

		return 1;
	}
<INITIAL>^points{WS}for{WS}user{WS}{USER}{OWS}:{OWS}{NUMBER} 	{
		if (!gibbon_clip_reader_set_result (reader, yytext, 5,
		                                    ": \t",
		                                    GIBBON_CLIP_RESUME_INFO_POINTS,
		                                    GIBBON_TT_N0, 4,
		                                    GIBBON_TT_USER, 3,
		                                    GIBBON_TT_END))
			return -1;

		return 1;
	}
<INITIAL>^{USER}{WS}and{WS}{USER}{WS}start{WS}a{WS}{NUMBER}{WS}point{WS}match 	{
		if (!gibbon_clip_reader_set_result (reader, yytext, 6,
		                                    " \t",
		                                    GIBBON_CLIP_START_MATCH,
		                                    GIBBON_TT_MATCH_LENGTH, 5,
		                                    GIBBON_TT_USER, 2,
		                                    GIBBON_TT_USER, 0,
		                                    GIBBON_TT_END))
			return -1;

		return 1;
	}
<INITIAL>^{USER}{WS}and{WS}{USER}{WS}start{WS}an?{WS}unlimited{WS}match 	{
		if (!gibbon_clip_reader_set_result (reader, yytext, 7,
		                                    " \t",
		                                    GIBBON_CLIP_START_MATCH,
		                                    GIBBON_TT_MATCH_LENGTH, 5,
		                                    GIBBON_TT_USER, 2,
		                                    GIBBON_TT_USER, 0,
		                                    GIBBON_TT_END))
			return -1;

		return 1;
	}
<INITIAL>^{USER}{WS}wins{WS}a{WS}{NUMBER}{WS}point{WS}match{WS}against{WS}{USER}{WS}{NUMBER}{OWS}-{OWS}{NUMBER} 	{
		if (!gibbon_clip_reader_set_result (reader, yytext, 10,
		                                    "- \t",
		                                    GIBBON_CLIP_ASYNC_WIN_MATCH,
		                                    GIBBON_TT_N0, 9,
		                                    GIBBON_TT_N0, 8,
		                                    GIBBON_TT_MATCH_LENGTH, 3,
		                                    GIBBON_TT_USER, 7,
		                                    GIBBON_TT_USER, 0,
		                                    GIBBON_TT_END))
			return -1;

		return 1;
	}
<INITIAL>^{USER}{WS}and{WS}{USER}{WS}are{WS}resuming{WS}their{WS}unlimited{WS}match 	{
		if (!gibbon_clip_reader_set_result (reader, yytext, 8,
		                                    " \t",
		                                    GIBBON_CLIP_RESUME_MATCH,
		                                    GIBBON_TT_MATCH_LENGTH, 6,
		                                    GIBBON_TT_USER, 2,
		                                    GIBBON_TT_USER, 0,
		                                    GIBBON_TT_END))
			return -1;

		return 1;
	}
<INITIAL>^{USER}{WS}and{WS}{USER}{WS}are{WS}resuming{WS}their{WS}{NUMBER}{SEP}point{WS}match 	{
		if (!gibbon_clip_reader_set_result (reader, yytext, 8,
		                                    "- \t",
		                                    GIBBON_CLIP_RESUME_MATCH,
		                                    GIBBON_TT_MATCH_LENGTH, 6,
		                                    GIBBON_TT_USER, 2,
		                                    GIBBON_TT_USER, 0,
		                                    GIBBON_TT_END))
			return -1;

		return 1;
	}
<INITIAL>^Settings{WS}of{WS}variables:	{
		if (!gibbon_clip_reader_set_result (reader, yytext, 0,
		                                    " \t",
		                                    GIBBON_CLIP_START_SETTINGS,
		                                    GIBBON_TT_END))
			return -1;

		BEGIN (SETTINGS);
		
		return 1;
	}
<SETTINGS>^{CHUNK}:{OWS}{CHUNK}	{
		if (!gibbon_clip_reader_set_result (reader, yytext, 2,
		                                    ": \t",
		                                    GIBBON_CLIP_SHOW_SETTING,
		                                    GIBBON_TT_WORD, 1,
		                                    GIBBON_TT_WORD, 0,
		                                    GIBBON_TT_END))
			return -1;

		return 1;
	}
<SETTINGS,TOGGLES,SAVED>[ \t\n]+	/* Ignore.  */ ;
<SETTINGS,TOGGLES,SAVED>.		BEGIN (INITIAL); return 2;
<INITIAL>Value{WS}of{WS}\x27{CHUNK}\x27{WS}set{WS}to{WS}{CHUNK}	{
		if (!gibbon_clip_reader_set_result (reader, yytext, 7,
		                                    "'. \t",
		                                    GIBBON_CLIP_SHOW_SETTING,
		                                    GIBBON_TT_WORD, 5,
		                                    GIBBON_TT_WORD, 2,
		                                    GIBBON_TT_END))
			return -1;

		return 1;
	}
<INITIAL>^The{WS}current{WS}settings{WS}are:	{
		if (!gibbon_clip_reader_set_result (reader, yytext, 0,
		                                    " \t",
		                                    GIBBON_CLIP_START_TOGGLES,
		                                    GIBBON_TT_END))
			return -1;

		BEGIN (TOGGLES);
		
		return 1;
	}
<TOGGLES>^{CHUNK}{WS}(YES|NO)	{
		if (!gibbon_clip_reader_set_result (reader, yytext, 2,
		                                    " \t",
		                                    GIBBON_CLIP_SHOW_TOGGLE,
		                                    GIBBON_TT_YESNO, 1,
		                                    GIBBON_TT_WORD, 0,
		                                    GIBBON_TT_END))
			return -1;

		return 1;
	}
<INITIAL>^\*\*{WS}You\x27re{WS}now{WS}ready{WS}to{WS}invite{WS}or{WS}join{WS}someone {
		strcpy (yytext, "ready YES");
		if (!gibbon_clip_reader_set_result (reader, yytext, 2,
		                                    " \t",
		                                    GIBBON_CLIP_SHOW_TOGGLE,
		                                    GIBBON_TT_YESNO, 1,
		                                    GIBBON_TT_WORD, 0,
		                                    GIBBON_TT_END))
			return -1;

		return 1;		
	}
<INITIAL>^\*\*{WS}You\x27re{WS}now{WS}refusing{WS}to{WS}play{WS}with{WS}someone {
		strcpy (yytext, "ready NO");
		if (!gibbon_clip_reader_set_result (reader, yytext, 2,
		                                    " \t",
		                                    GIBBON_CLIP_SHOW_TOGGLE,
		                                    GIBBON_TT_YESNO, 1,
		                                    GIBBON_TT_WORD, 0,
		                                    GIBBON_TT_END))
			return -1;

		return 1;		
	}
<INITIAL>^\*\*{WS}You\x27ll{WS}be{WS}notified{WS}when{WS}new{WS}users{WS}log{WS}in {
		strcpy (yytext, "notify YES");
		if (!gibbon_clip_reader_set_result (reader, yytext, 2,
		                                    " \t",
		                                    GIBBON_CLIP_SHOW_TOGGLE,
		                                    GIBBON_TT_YESNO, 1,
		                                    GIBBON_TT_WORD, 0,
		                                    GIBBON_TT_END))
			return -1;

		return 1;		
	}
<INITIAL>^\*\*{WS}You{WS}won\x27t{WS}be{WS}notified{WS}when{WS}new{WS}users{WS}log{WS}in {
		strcpy (yytext, "notify NO");
		if (!gibbon_clip_reader_set_result (reader, yytext, 2,
		                                    " \t",
		                                    GIBBON_CLIP_SHOW_TOGGLE,
		                                    GIBBON_TT_YESNO, 1,
		                                    GIBBON_TT_WORD, 0,
		                                    GIBBON_TT_END))
			return -1;

		return 1;		
	}
<INITIAL>^\*\*{WS}The{WS}board{WS}will{WS}be{WS}refreshed{WS}after{WS}every{WS}move {
		strcpy (yytext, "autoboard YES");
		if (!gibbon_clip_reader_set_result (reader, yytext, 2,
		                                    " \t",
		                                    GIBBON_CLIP_SHOW_TOGGLE,
		                                    GIBBON_TT_YESNO, 1,
		                                    GIBBON_TT_WORD, 0,
		                                    GIBBON_TT_END))
			return -1;

		return 1;		
	}
<INITIAL>^\*\*{WS}The{WS}board{WS}won\x27t{WS}be{WS}refreshed{WS}after{WS}every{WS}move {
		strcpy (yytext, "autoboard NO");
		if (!gibbon_clip_reader_set_result (reader, yytext, 2,
		                                    " \t",
		                                    GIBBON_CLIP_SHOW_TOGGLE,
		                                    GIBBON_TT_YESNO, 1,
		                                    GIBBON_TT_WORD, 0,
		                                    GIBBON_TT_END))
			return -1;

		return 1;		
	}
<INITIAL>^{OWS}opponent{WS}matchlength{WS}score{WS}\(your{WS}points{WS}first\) {
		if (!gibbon_clip_reader_set_result (reader, yytext, 0,
		                                    " \t",
		                                    GIBBON_CLIP_SHOW_START_SAVED,
		                                    GIBBON_TT_END))
			return -1;
		
		BEGIN (SAVED);

		return 1;		
	}
<SAVED>^{OWS}\*{0,2}{USER}{WS}({NUMBER}|unlimited){WS}{NUMBER}{OWS}-{OWS}{NUMBER} {
		if (!gibbon_clip_reader_set_result (reader, yytext, 4,
		                                    "*- \t",
		                                    GIBBON_CLIP_SHOW_SAVED,
		                                    GIBBON_TT_N0, 3,
		                                    GIBBON_TT_N0, 2,
		                                    GIBBON_TT_MATCH_LENGTH, 1,
		                                    GIBBON_TT_USER, 0,
		                                    GIBBON_TT_END))
			return -1;

		return 1;		
	}
<SAVED>^no{WS}saved{WS}games {
		if (!gibbon_clip_reader_set_result (reader, yytext, 0,
		                                    " \t",
		                                    GIBBON_CLIP_SHOW_SAVED_NONE,
		                                    GIBBON_TT_END))
			return -1;

		BEGIN (INITIAL);

		return 1;		
	}
<INITIAL>^{USER}{WS}has{WS}no{WS}saved{WS}games {
		char *ptr = yytext;
		while (*ptr && *ptr != ' ' && *ptr != '\t')
			++ptr;
		*ptr++ = ' ';
		*ptr++ = '0';
		*ptr++ = 0;
		if (!gibbon_clip_reader_set_result (reader, yytext, 2,
		                                    " ",
		                                    GIBBON_CLIP_SHOW_SAVED_COUNT,
		                                    GIBBON_TT_N0, 1,
		                                    GIBBON_TT_USER, 0,
		                                    GIBBON_TT_END))
			return -1;
		
		BEGIN (INITIAL);

		return 1;		
	}
<INITIAL>^{USER}{WS}has{WS}{NUMBER}{WS}saved{WS}games? {
		if (!gibbon_clip_reader_set_result (reader, yytext, 5,
		                                    " \t",
		                                    GIBBON_CLIP_SHOW_SAVED_COUNT,
		                                    GIBBON_TT_N0, 2,
		                                    GIBBON_TT_USER, 0,
		                                    GIBBON_TT_END))
			return -1;
		
		BEGIN (INITIAL);

		return 1;		
	}
<INITIAL>^Your{WS}email{WS}address{WS}is{WS}\x27{CHUNK}\x27 {
		if (!gibbon_clip_reader_set_result (reader, yytext, 5,
		                                    "' \t",
		                                    GIBBON_CLIP_SHOW_ADDRESS,
		                                    GIBBON_TT_WORD, -1,
		                                    GIBBON_TT_END))
			return -1;
		
		return 1;		
	}
<INITIAL>^\*\*{WS}\x27{CHUNK}\x27{WS}is{WS}not{WS}an{WS}email{WS}address {
		if (!gibbon_clip_reader_set_result (reader, yytext, 5,
		                                    "' \t",
		                                    GIBBON_CLIP_INVALID_ADDRESS,
		                                    GIBBON_TT_WORD, 1,
		                                    GIBBON_TT_END))
			return -1;
		
		return 1;		
	}
<INITIAL>^\*\*{WS}ERROR:{WS}Saved{WS}match{WS}is{WS}corrupt\.{WS}Please{WS}start{WS}another{WS}one {
		gibbon_clip_reader_set_error (reader,
   	                                      GIBBON_CLIP_ERROR_SAVED_CORRUPT,
		                              "%s",
		                              _("Your saved match was"
		                                " corrupted on the server."
		                                "  Please start a new one!"));

		return 1;
	}
<INITIAL>^\*\*{WS}{NUMBER}{WS}users?{WS}heard{WS}you {
		if (!gibbon_clip_reader_set_result (reader, yytext, 3,
		                                    " \t",
		                                    GIBBON_CLIP_HEARD_YOU,
		                                    GIBBON_TT_N0, 1,
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
<INITIAL>^{WS}	{
		if (!gibbon_clip_reader_set_result (reader, yytext, 0,
		                                    " \t",
		                                    GIBBON_CLIP_EMPTY,
		                                    GIBBON_TT_END))
			return -1;

		BEGIN (INITIAL);

		return 1;
	}
[. \t]+		/* Ignore! Including a full-stop!  */
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
	yy_flex_debug = 0;

	yy_delete_buffer (YY_CURRENT_BUFFER, yyscanner);
	yy_switch_to_buffer (yy_scan_string (line, yyscanner), yyscanner);
}

void 
gibbon_clip_lexer_reset_condition_stack (void *yyscanner)
{
	struct yyguts_t * yyg = (struct yyguts_t*) yyscanner;
	
	BEGIN (0);
}
