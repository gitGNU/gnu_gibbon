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

%start MOTD
%start USER
%start INTEGER
%start MESSAGE

WS		[ \t]+
OWS		[ \t]*

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
			BEGIN (MOTD);
			return CLIP_MOTD_START;
		}
^4{OWS}		{
			((struct yyguts_t *) yyscanner)->yylval_r->value
				= alloc_value ("4", G_TYPE_INT64); 
			BEGIN (INITIAL);
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
<USER>[^[:space:]]+{WS}		{
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
[-+]?(0|[1-9][0-9]*)\.[0-9]+	{
		((struct yyguts_t *) yyscanner)->yylval_r->value
			= alloc_value (yytext, G_TYPE_DOUBLE); 
		return GDOUBLE;
	}
[-+]?0|[1-9][0-9]*	{
		((struct yyguts_t *) yyscanner)->yylval_r->value
			= alloc_value (yytext, G_TYPE_INT64); 
		return GINT64;
	}
[^[:space:]]+		{
		((struct yyguts_t *) yyscanner)->yylval_r->value
			= alloc_value (yytext, G_TYPE_STRING); 
		return GSTRING;
	}
{WS}			;
.			return yytext[0];
%%

void 
gibbon_clip_lexer_current_buffer (yyscan_t yyscanner, const gchar *line)
{
	/*
	 * Serious hack alarm!  YY_CURRENT_BUFFER needs a variable yyg
	 * and there is no clean way to get it.
	 */
	struct yyguts_t * yyg = (struct yyguts_t*) yyscanner;
	
	yy_flex_debug = 0;

	yy_delete_buffer (YY_CURRENT_BUFFER, yyscanner);
	yy_switch_to_buffer (yy_scan_string (line, yyscanner), yyscanner);
}
