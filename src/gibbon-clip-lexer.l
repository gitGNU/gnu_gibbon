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

#include "gibbon-clip-parser.h"
#include "gibbon-clip-reader-priv.h"
#include "gibbon-util.h"

#define alloc_value(s, type) gibbon_clip_reader_alloc_value(		\
	gibbon_clip_lexer_get_extra(yyscanner), s, type)

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
%option bison-bridge
%option stack
%option noyy_top_state

/* %option debug */

%s MOTD
%s USER
%s INTEGER
%s MESSAGE
%s FS
%x PARSED

WS		[ \t]+
OWS		[ \t]*
/* Think full stop.  */
FS		[ \t]*\.[ \t]*

%%
^10{WS}		{
			((struct yyguts_t *) yyscanner)->yylval_r->value
				= alloc_value ("10", G_TYPE_INT64); 
			return CLIP_MESSAGE_DELIVERED;
		}
^11{WS}		{
			((struct yyguts_t *) yyscanner)->yylval_r->value
				= alloc_value ("11", G_TYPE_INT64); 
			return CLIP_MESSAGE_SAVED;
		}
^12[ \t]		{
			((struct yyguts_t *) yyscanner)->yylval_r->value
				= alloc_value ("12", G_TYPE_INT64);
			yy_push_state (MESSAGE, yyscanner);
			yy_push_state (USER, yyscanner);
			return CLIP_SAYS;
		}
^13[ \t]		{
			((struct yyguts_t *) yyscanner)->yylval_r->value
				= alloc_value ("13", G_TYPE_INT64);
			yy_push_state (MESSAGE, yyscanner);
			yy_push_state (USER, yyscanner);
			return CLIP_SHOUTS;
		}
^14[ \t]		{
			((struct yyguts_t *) yyscanner)->yylval_r->value
				= alloc_value ("14", G_TYPE_INT64);
			yy_push_state (MESSAGE, yyscanner);
			yy_push_state (USER, yyscanner);
			return CLIP_WHISPERS;
		}
^15[ \t]		{
			((struct yyguts_t *) yyscanner)->yylval_r->value
				= alloc_value ("15", G_TYPE_INT64);
			yy_push_state (MESSAGE, yyscanner);
			yy_push_state (USER, yyscanner);
			return CLIP_KIBITZES;
		}
^16[ \t]		{
			((struct yyguts_t *) yyscanner)->yylval_r->value
				= alloc_value ("16", G_TYPE_INT64);
			yy_push_state (MESSAGE, yyscanner);
			yy_push_state (USER, yyscanner);
			return CLIP_YOU_SAY;
		}
^17[ \t]		{
			((struct yyguts_t *) yyscanner)->yylval_r->value
				= alloc_value ("17", G_TYPE_INT64);
			yy_push_state (MESSAGE, yyscanner);
			return CLIP_YOU_SHOUT;
		}
^18[ \t]		{
			((struct yyguts_t *) yyscanner)->yylval_r->value
				= alloc_value ("18", G_TYPE_INT64);
			yy_push_state (MESSAGE, yyscanner);
			return CLIP_YOU_WHISPER;
		}
^19[ \t]		{
			((struct yyguts_t *) yyscanner)->yylval_r->value
				= alloc_value ("19", G_TYPE_INT64);
			yy_push_state (MESSAGE, yyscanner);
			return CLIP_YOU_KIBITZ;
		}
^20[ \t]		{
			((struct yyguts_t *) yyscanner)->yylval_r->value
				= alloc_value ("20", G_TYPE_INT64);
			yy_push_state (MESSAGE, yyscanner);
			yy_push_state (USER, yyscanner);
			return CLIP_ALERTS;
		}
^1{WS}		{
			((struct yyguts_t *) yyscanner)->yylval_r->value
				= alloc_value ("1", G_TYPE_INT64); 
			return CLIP_WELCOME;
		}
^2{WS}		{
			((struct yyguts_t *) yyscanner)->yylval_r->value
				= alloc_value ("2", G_TYPE_INT64); 
			return CLIP_OWN_INFO;
		}
^3{OWS}		{
			((struct yyguts_t *) yyscanner)->yylval_r->value
				= alloc_value ("3", G_TYPE_INT64); 
			yy_push_state (MOTD, yyscanner);
			return CLIP_MOTD_START;
		}
^4{OWS}		{
			((struct yyguts_t *) yyscanner)->yylval_r->value
				= alloc_value ("4", G_TYPE_INT64); 
			yy_pop_state (yyscanner);
			return CLIP_MOTD_END;
		}	
<MOTD>(.|\n)*	{
			((struct yyguts_t *) yyscanner)->yylval_r->value
				= alloc_value (yytext, G_TYPE_STRING);
			return CLIP_MOTD;
		}
^5{WS}		{
			((struct yyguts_t *) yyscanner)->yylval_r->value
				= alloc_value ("5", G_TYPE_INT64); 
			return CLIP_WHO_INFO;
		}
^6{OWS}		{
			((struct yyguts_t *) yyscanner)->yylval_r->value
				= alloc_value ("6", G_TYPE_INT64); 
			return CLIP_WHO_INFO_END;
		}
^7{WS}		{
			((struct yyguts_t *) yyscanner)->yylval_r->value
				= alloc_value ("7", G_TYPE_INT64);
			yy_push_state (MESSAGE, yyscanner);
			yy_push_state (USER, yyscanner);			
			return CLIP_LOGIN;
		}
^8{WS}		{
			((struct yyguts_t *) yyscanner)->yylval_r->value
				= alloc_value ("8", G_TYPE_INT64);
			yy_push_state (MESSAGE, yyscanner);
			yy_push_state (USER, yyscanner);			
			return CLIP_LOGOUT;
		}	
^9{WS}		{
			((struct yyguts_t *) yyscanner)->yylval_r->value
				= alloc_value ("9", G_TYPE_INT64);
			yy_push_state (MESSAGE, yyscanner);
			yy_push_state (INTEGER, yyscanner);
			yy_push_state (USER, yyscanner);			
			return CLIP_MESSAGE;
		}
^\*\*{WS}	{
			yy_push_state (PARSED, yyscanner);
			return TOKEN_2STARS;
		}
^board		{
			((struct yyguts_t *) yyscanner)->yylval_r->value
				= alloc_value ("200", G_TYPE_INT64);
			return CLIP_BOARD;
		}
^[^:[:space:]]+{WS}rolls?	{
		char *ptr = yytext;
		while (*ptr != ' ' && *ptr != '\t')
			++ptr;
		*ptr = 0;
		((struct yyguts_t *) yyscanner)->yylval_r->value
			= alloc_value (yytext, G_TYPE_STRING);
		yy_push_state (PARSED, yyscanner); 
		return CLIP_ROLLS;
	}
^[^[:space:]]+{WS}moves?	{
		char *ptr = yytext;
		while (*ptr != ' ' && *ptr != '\t')
			++ptr;
		*ptr = 0;
		((struct yyguts_t *) yyscanner)->yylval_r->value
			= alloc_value (yytext, G_TYPE_STRING);
		yy_push_state (PARSED, yyscanner); 
		return CLIP_MOVES;
	}
^Starting{WS}a{WS}new{WS}game{WS}with	{
		yy_push_state (PARSED, yyscanner);
		return CLIP_START_GAME;
	}
^[^[:space:]]+{WS}logs{WS}out.{WS}The{WS}game{WS}was{WS}saved.* {
		char *ptr = yytext;
		while (*ptr != ' ' && *ptr != '\t')
			++ptr;
		*ptr = 0;
		((struct yyguts_t *) yyscanner)->yylval_r->value
			= alloc_value (yytext, G_TYPE_STRING);
		return CLIP_DROPS_GAME;
	}
^[^[:space:]]+{WS}drops{WS}connection.{WS}The{WS}game{WS}was{WS}saved.* {
		char *ptr = yytext;
		while (*ptr != ' ' && *ptr != '\t')
			++ptr;
		*ptr = 0;
		((struct yyguts_t *) yyscanner)->yylval_r->value
			= alloc_value (yytext, G_TYPE_STRING);
		return CLIP_DROPS_GAME;
	}
^Connection{WS}with{WS}[^[:space:]]+{WS}timed{WS}out\.{WS}The{WS}game{WS}was{WS}saved.* {
		char *ptr = 4 + strstr (yytext, "with");
		char *user;
		while (*ptr == ' ' || *ptr == '\t')
			++ptr;
		user = ptr;
		while (*ptr != ' ' && *ptr != '\t')
			++ptr;
		*ptr = 0;
		((struct yyguts_t *) yyscanner)->yylval_r->value
			= alloc_value (user, G_TYPE_STRING);
		return CLIP_DROPS_GAME;
	}
^[^[:space:]]+{WS}can't{WS}move.* {
		char *ptr = yytext;
		while (*ptr != ' ' && *ptr != '\t')
			++ptr;
		*ptr = 0;
		((struct yyguts_t *) yyscanner)->yylval_r->value
			= alloc_value (yytext, G_TYPE_STRING);
		return CLIP_CANNOT_MOVE;
	}
^[^[:space:]]+{WS}doubles?{OWS}\..*    {
		char *ptr = yytext;
		while (*ptr != ' ' && *ptr != '\t')
			++ptr;
		*ptr = 0;
		((struct yyguts_t *) yyscanner)->yylval_r->value
			= alloc_value (yytext, G_TYPE_STRING);
		return CLIP_DOUBLES;
	}
^[^[:space:]]+{WS}accepts?{WS}the{WS}double{OWS}\.{OWS}The{WS}cube{WS}shows  {
		char *ptr = yytext;
		while (*ptr != ' ' && *ptr != '\t')
			++ptr;
		*ptr = 0;
		((struct yyguts_t *) yyscanner)->yylval_r->value
			= alloc_value (yytext, G_TYPE_STRING);
		yy_push_state (PARSED, yyscanner);
		return CLIP_ACCEPTS_DOUBLE;
	}
^[^[:space:]]+{WS}wants{WS}to{WS}play{WS}an? {
		char *ptr = yytext;
		while (*ptr != ' ' && *ptr != '\t')
			++ptr;
		*ptr = 0;
		((struct yyguts_t *) yyscanner)->yylval_r->value
			= alloc_value (yytext, G_TYPE_STRING);
		yy_push_state (PARSED, yyscanner);
		return CLIP_INVITATION;
	}
^[^[:space:]]+{WS}wants{WS}to{WS}resume{WS}a {
		char *ptr = yytext;
		while (*ptr != ' ' && *ptr != '\t')
			++ptr;
		*ptr = 0;
		((struct yyguts_t *) yyscanner)->yylval_r->value
			= alloc_value (yytext, G_TYPE_STRING);
		yy_push_state (PARSED, yyscanner);
		return CLIP_INVITATION;
	}
^Type{WS}\x27join{WS}[^\x27[:space:]]+\x27{WS}to{WS}accept.*    {
		char *ptr = 4 + strstr (yytext, "join");
		char *user;
		while (*ptr == ' ' || *ptr == '\t')
			++ptr;
		user = ptr;
		while (*ptr != '\'')
			++ptr;
		*ptr = 0;		
		((struct yyguts_t *) yyscanner)->yylval_r->value
			= alloc_value (user, G_TYPE_STRING);
		return CLIP_TYPE_JOIN;
	}
^You\x27re{WS}now{WS}watching{WS}[^[:space:]]\.?.*    {
		char *ptr = 8 + strstr (yytext, "watching");
		char *user;
		while (*ptr == ' ' || *ptr == '\t')
			++ptr;
		user = ptr;
		while (*ptr != '.' && *ptr != ' ' && *ptr != '\t')
			++ptr;
		*ptr = 0;		
		((struct yyguts_t *) yyscanner)->yylval_r->value
			= alloc_value (user, G_TYPE_STRING);
		return CLIP_YOURE_WATCHING;
	}
<PARSED>and	return TOKEN_AND;
	/*
	 * We do not distinguish between bar and off.  When we convert it to
	 * a number (0 or 25), we have to know the context anyway so that
	 * we can take the move direction into account.  The parser does that
	 * for us here.
	 */
<PARSED>bar|off|unlimited	{
		((struct yyguts_t *) yyscanner)->yylval_r->value
			= alloc_value ("0", G_TYPE_INT64);
		return GINT64;
	}
<PARSED>saved	{
		((struct yyguts_t *) yyscanner)->yylval_r->value
			= alloc_value ("-1", G_TYPE_INT64);
		return GINT64;
	}
<PARSED>You	{
		((struct yyguts_t *) yyscanner)->yylval_r->value
			= alloc_value (yytext, G_TYPE_STRING);
		return GSTRING;
	}
<PARSED>Player{WS}[^[:space:]]+	{
		char *ptr = yytext + 6;
		while (*ptr == ' ' || *ptr == '\t')
			++ptr;
		((struct yyguts_t *) yyscanner)->yylval_r->value
			= alloc_value (ptr, G_TYPE_STRING);
		return GSTRING;
	}	
<PARSED>You{WS}are{WS}now{WS}playing{WS}an?    {
		return CLIP_NOW_PLAYING;
	}
<PARSED>point{WS}match{WS}with    {
		return TOKEN_MATCH_WITH;
	}
<PARSED>match{WS}with    {
		return TOKEN_MATCH_WITH;
	}
<PARSED>-	/* 
                 * Ignore.  The hyphen may be part of a move (e.g. "8-5") but
                 * in that case it would be consumed by the regex for a 
                 * (negative) integer.  We can then just as well ignore it
                 * and "correct" the sign in the parser instead.
                 */
<PARSED>\.	;
<PARSED>[-+]?(0|[1-9][0-9]*)	{
		((struct yyguts_t *) yyscanner)->yylval_r->value
			= alloc_value (yytext, G_TYPE_INT64);
		return GINT64;
	}
<PARSED>[^-.0-9[:space:]]+		{
		((struct yyguts_t *) yyscanner)->yylval_r->value
			= alloc_value (yytext, G_TYPE_STRING); 
		return GSTRING;
	}
<PARSED>point{WS}match{WS}with{WS}you	; /* Ignore.  */
<PARSED>match{WS}with{WS}you		; /* Ignore.  */
<PARSED>{WS}	;
<PARSED><<EOF>>	{
			yy_pop_state (yyscanner);
			if (YY_START) {
				g_printerr ("Unbalanced start condition.\n");
				return GARBAGE;
			}
			yyterminate ();
		}
<PARSED>.|\n	return GARBAGE;

	/*
	 * The trailing context is necessary so that matching a user name
	 * plus trailing forbidden characters produces a longer match than
	 * the catch-all GSTRING match at the end.
	 */
<USER>[^.[:space:]]+{OWS}/[.]?		{
		((struct yyguts_t *) yyscanner)->yylval_r->value
			= alloc_value (gibbon_trim (yytext), G_TYPE_STRING); 
		yy_pop_state (yyscanner);
		return GSTRING;
	}
<INTEGER>[-+]?(0|[1-9][0-9]*)[ \t]	{
		((struct yyguts_t *) yyscanner)->yylval_r->value
			= alloc_value (yytext, G_TYPE_INT64);
		yy_pop_state (yyscanner);
		return GINT64;
	}
<MESSAGE>(.|\n)*		{
		((struct yyguts_t *) yyscanner)->yylval_r->value
			= alloc_value (yytext, G_TYPE_STRING); 
		yy_pop_state (yyscanner);
		return GSTRING;
	}
<PARSED>There{WS}is{WS}no{WS}one{WS}called{WS}[^.[:space:]]+.? {
		char *ptr = 6 + strstr (yytext, "called");
		char *user;
		while (*ptr == ' ' || *ptr == '\t')
			++ptr;
		user = ptr;
		while (*ptr != ' ' && *ptr != '\t' && *ptr != '.')
			++ptr;
		*ptr = 0;
		((struct yyguts_t *) yyscanner)->yylval_r->value
			= alloc_value (user, G_TYPE_STRING);
		return CLIP_ERROR_NO_USER;
	}
<PARSED>terminated{WS}the{WS}game.{WS}The{WS}game{WS}was{WS}saved.* {
		return CLIP_LEFT_GAME;
	}
<FS>{OWS}\.{OWS}	;
[-+]?(0|[1-9][0-9]*)\.[0-9]+	{
		((struct yyguts_t *) yyscanner)->yylval_r->value
			= alloc_value (yytext, G_TYPE_DOUBLE); 
		return GDOUBLE;
	}
[-+]?(0|[1-9][0-9]*)	{
		((struct yyguts_t *) yyscanner)->yylval_r->value
			= alloc_value (yytext, G_TYPE_INT64); 
		return GINT64;
	}
	/*
	 * We exclude the colon here so that this rule does not consume an
	 * entire board state.
	 */
[^:[:space:]]+		{
		((struct yyguts_t *) yyscanner)->yylval_r->value
			= alloc_value (yytext, G_TYPE_STRING); 
		return GSTRING;
	}
{WS}			;
.|\n			return yytext[0];
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
	
	while (YY_START) {
		yy_pop_state (yyscanner);
	}
}
