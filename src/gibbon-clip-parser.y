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
 * Parser for FIBS output.  The parser parses one line at a time, and relies
 * on lexer start conditions for preserving state.
 */

%{
#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include <glib.h>
#include <glib/gi18n.h>

#include "gibbon-clip.h"
#include "gibbon-clip-parser.h"
#include "gibbon-clip-reader-priv.h"

/*
 * Remap normal yacc parser interface names (yyparse, yylex, yyerror, etc),
 * as well as gratuitiously global symbol names, so we can have multiple
 * yacc generated parsers in the same program.  Note that these are only
 * the variables produced by yacc.  If other parser generators (bison,
 * byacc, etc) produce additional global names that conflict at link time,
 * then those parser generators need to be fixed instead of adding those
 * names to this list. 
 */

#define yymaxdepth gibbon_clip_parser_maxdepth
#define yyparse(s)    gibbon_clip_parser_parse(s)
#define yylex      gibbon_clip_lexer_lex
extern int gibbon_clip_lexer_lex (YYSTYPE * lvalp, void *scanner);
#define yyerror    gibbon_clip_reader_yyerror
#define yylval     gibbon_clip_parser_lval
#define yychar     gibbon_clip_parser_char
#define yydebug    gibbon_clip_parser_debug
#define yypact     gibbon_clip_parser_pact
#define yyr1       gibbon_clip_parser_r1
#define yyr2       gibbon_clip_parser_r2
#define yydef      gibbon_clip_parser_def
#define yychk      gibbon_clip_parser_chk
#define yypgo      gibbon_clip_parser_pgo
#define yyact      gibbon_clip_parser_act
#define yyexca     gibbon_clip_parser_exca
#define yyerrflag  gibbon_clip_parser_errflag
#define yynerrs    gibbon_clip_parser_nerrs
#define yyps       gibbon_clip_parser_ps
#define yypv       gibbon_clip_parser_pv
#define yys        gibbon_clip_parser_s
#define yy_yys     gibbon_clip_parser_yys
#define yystate    gibbon_clip_parser_state
#define yytmp      gibbon_clip_parser_tmp
#define yyv        gibbon_clip_parser_v
#define yy_yyv     gibbon_clip_parser_yyv
#define yyval      gibbon_clip_parser_val
#define yylloc     gibbon_clip_parser_lloc
#define yyreds     gibbon_clip_parser_reds    /* With YYDEBUG defined */
#define yytoks     gibbon_clip_parser_toks    /* With YYDEBUG defined */
#define yylhs      gibbon_clip_parser_yylhs
#define yylen      gibbon_clip_parser_yylen
#define yydefred   gibbon_clip_parser_yydefred
#define yysdgoto   gibbon_clip_parser_yydgoto
#define yysindex   gibbon_clip_parser_yysindex
#define yyrindex   gibbon_clip_parser_yyrindex
#define yygindex   gibbon_clip_parser_yygindex
#define yytable    gibbon_clip_parser_yytable
#define yycheck    gibbon_clip_parser_yycheck

#define YYDEBUG 42

#define reader gibbon_clip_lexer_get_extra(scanner)

static gboolean gibbon_clip_parser_fixup_uint (void *raw, 
                                               gint64 lower, gint64 upper);
static gboolean gibbon_clip_parser_fixup_uint64 (void *raw, 
                                                 gint64 lower, gint64 upper);
static gboolean gibbon_clip_parser_fixup_int64 (void *raw, 
                                                gint64 lower, gint64 upper);
static gboolean gibbon_clip_parser_fixup_boolean (void *raw);
static gboolean gibbon_clip_parser_fixup_user (void *raw);
static gboolean gibbon_clip_parser_fixup_optional_user (void *raw);

%}

%union {
	void *value;
}

%token <value> CLIP_WELCOME
%token <value> CLIP_OWN_INFO
%token <value> CLIP_MOTD_START
%token <value> CLIP_MOTD
%token <value> CLIP_MOTD_END
%token <value> CLIP_WHO_INFO
%token <value> CLIP_WHO_INFO_END
%token <value> CLIP_LOGIN
%token <value> CLIP_LOGOUT
%token <value> CLIP_MESSAGE
%token <value> CLIP_MESSAGE_DELIVERED
%token <value> GSTRING
%token <value> GINT64
%token <value> GDOUBLE

%type <value> redoubles

%pure-parser
%lex-param {void *scanner}
%parse-param {void *scanner}

%%
line: { yydebug = 0; } message
    ;

message: clip_welcome
       | clip_own_info
       | clip_motd_start
       | clip_motd
       | clip_motd_end
       | clip_who_info
       | clip_who_info_end
       | clip_login
       | clip_logout
       | clip_message
       | clip_message_delivered
       ;

clip_welcome: CLIP_WELCOME
		{
			if (!gibbon_clip_parser_fixup_uint (
					$1, GIBBON_CLIP_WELCOME, 
					GIBBON_CLIP_WELCOME))
				YYABORT;
		}
              GSTRING 
		{
			if (!gibbon_clip_parser_fixup_user ($3))
				YYABORT;
		}
	      GINT64 GSTRING
            ;
            
clip_own_info: CLIP_OWN_INFO
		{
			if (!gibbon_clip_parser_fixup_uint (
					$1, GIBBON_CLIP_OWN_INFO, 
					GIBBON_CLIP_OWN_INFO))
				YYABORT;
		}
 	       GSTRING 
		{
			if (!gibbon_clip_parser_fixup_user ($3))
				YYABORT;
		}
 	       GINT64
		{
			if (!gibbon_clip_parser_fixup_boolean ($5))
				YYABORT;
		}
 	       GINT64
		{
			if (!gibbon_clip_parser_fixup_boolean ($7))
				YYABORT;
		}
 	       GINT64
		{
			if (!gibbon_clip_parser_fixup_boolean ($9))
				YYABORT;
		}
 	       GINT64
		{
			if (!gibbon_clip_parser_fixup_boolean ($11))
				YYABORT;
		}
 	       GINT64
		{
			if (!gibbon_clip_parser_fixup_boolean ($13))
				YYABORT;
		}
 	       GINT64
		{
			if (!gibbon_clip_parser_fixup_boolean ($15))
				YYABORT;
		}
 	       GINT64
		{
			if (!gibbon_clip_parser_fixup_boolean ($17))
				YYABORT;
		}
 	       GINT64
		{
			if (!gibbon_clip_parser_fixup_boolean ($19))
				YYABORT;
		}
 	       GINT64
		{
			if (!gibbon_clip_parser_fixup_uint64 ($21,
			                                      0, G_MAXINT64))
				YYABORT;
		}
 	       GINT64
		{
			if (!gibbon_clip_parser_fixup_boolean ($23))
				YYABORT;
		}
 	       GINT64
		{
			if (!gibbon_clip_parser_fixup_boolean ($25))
				YYABORT;
		}
 	       GINT64
		{
			if (!gibbon_clip_parser_fixup_boolean ($27))
				YYABORT;
		}
 	       GINT64
		{
			if (!gibbon_clip_parser_fixup_boolean ($29))
				YYABORT;
		}
	       GDOUBLE
 	       GINT64
		{
			if (!gibbon_clip_parser_fixup_boolean ($32))
				YYABORT;
		}
 	       GINT64
		{
			if (!gibbon_clip_parser_fixup_boolean ($34))
				YYABORT;
		}
	       redoubles
 	       GINT64
		{
			if (!gibbon_clip_parser_fixup_boolean ($37))
				YYABORT;
		}
 	       GINT64
		{
			if (!gibbon_clip_parser_fixup_boolean ($39))
				YYABORT;
		}
	       GSTRING
             ;
            
redoubles: GINT64
		{
			if (!gibbon_clip_parser_fixup_int64 ($1, -1,
			                                     G_MAXINT64))
				YYABORT;
		}
         | GSTRING
         	{
         		GValue *v = (GValue *) $1;
         		
         		if (g_strcmp0 (g_value_get_string (v), "unlimited"))
         		        YYABORT;
         		g_value_unset (v);
         		g_value_init (v, G_TYPE_INT64);
         		g_value_set_int64 (v, -1);
         	}
         ;
         
clip_motd_start: CLIP_MOTD_START
		{
			if (!gibbon_clip_parser_fixup_uint (
					$1, GIBBON_CLIP_MOTD_START,
					GIBBON_CLIP_MOTD_START))
				YYABORT;
		}
            ;

clip_motd: CLIP_MOTD
		{	GValue *s = gibbon_clip_reader_alloc_value (
					reader,
					"", G_TYPE_STRING);
			GValue *v = (GValue *) $1;
			
			g_value_copy (v, s);
			g_value_unset (v);
			g_value_init (v, G_TYPE_UINT);
			g_value_set_uint (v, GIBBON_CLIP_MOTD);
		}
	    ;
            
clip_motd_end: CLIP_MOTD_END
		{
			if (!gibbon_clip_parser_fixup_uint (
					$1, GIBBON_CLIP_MOTD_END,
					GIBBON_CLIP_MOTD_END))
				YYABORT;
		}
            ;
                        
clip_who_info: CLIP_WHO_INFO
		{
			if (!gibbon_clip_parser_fixup_uint (
					$1, GIBBON_CLIP_WHO_INFO, 
					GIBBON_CLIP_WHO_INFO))
				YYABORT;
		}
 	       GSTRING
		{
			if (!gibbon_clip_parser_fixup_user ($3))
				YYABORT;
		}
 	       GSTRING
		{
			if (!gibbon_clip_parser_fixup_optional_user ($5))
				YYABORT;
		}
 	       GSTRING
		{
			if (!gibbon_clip_parser_fixup_optional_user ($7))
				YYABORT;
		}
 	       GINT64
		{
			if (!gibbon_clip_parser_fixup_boolean ($9))
				YYABORT;
		}
 	       GINT64
		{
			if (!gibbon_clip_parser_fixup_boolean ($11))
				YYABORT;
		}
	       GDOUBLE
 	       GINT64
		{
			if (!gibbon_clip_parser_fixup_uint64 ($14,
			                                      0, G_MAXINT64))
				YYABORT;
		}
 	       GINT64
		{
			if (!gibbon_clip_parser_fixup_uint64 ($16,
			                                      0, G_MAXINT64))
				YYABORT;
		}
	       GINT64
	       GSTRING
	        {
	        	GValue *v = (GValue *) $19;
	        	const gchar *ip = g_value_get_string (v);        	
		        size_t l = strlen (ip);
	        	gchar *real_ip;
        		if ('*' == ip[l - 1]) {
        			real_ip = g_strdup (ip);
        			real_ip[l - 1] = 0;
            			g_value_set_string (v, real_ip);
            			g_free (real_ip);
			}
	        }
	       GSTRING
	       GSTRING
             ;

clip_who_info_end: CLIP_WHO_INFO_END
		{
			if (!gibbon_clip_parser_fixup_uint (
					$1, GIBBON_CLIP_WHO_INFO_END,
					GIBBON_CLIP_WHO_INFO_END))
				YYABORT;
		}
            ;
            
clip_login: CLIP_LOGIN
	      {
		if (!gibbon_clip_parser_fixup_uint (
					$1, GIBBON_CLIP_LOGIN,
					GIBBON_CLIP_LOGIN))
				YYABORT;
	      }
	    GSTRING
            ;

clip_logout: CLIP_LOGOUT
	      {
		if (!gibbon_clip_parser_fixup_uint (
					$1, GIBBON_CLIP_LOGOUT,
					GIBBON_CLIP_LOGOUT))
				YYABORT;
	      }
	    GSTRING
            ;

clip_message: CLIP_MESSAGE
	      {
		if (!gibbon_clip_parser_fixup_uint (
					$1, GIBBON_CLIP_MESSAGE,
					GIBBON_CLIP_MESSAGE))
				YYABORT;
	      }
	    GSTRING
    	      {
		if (!gibbon_clip_parser_fixup_user ($3))
			YYABORT;
	      }
	    GINT64
	    GSTRING
            ;

clip_message_delivered: CLIP_MESSAGE_DELIVERED
	      {
		if (!gibbon_clip_parser_fixup_uint (
					$1, GIBBON_CLIP_MESSAGE_DELIVERED,
					GIBBON_CLIP_MESSAGE_DELIVERED))
				YYABORT;
	      }
	    GSTRING
    	      {
		if (!gibbon_clip_parser_fixup_user ($3))
			YYABORT;
	      }
            ;

%%

static gboolean
gibbon_clip_parser_fixup_uint (void *raw, gint64 lower, gint64 upper)
{
	GValue *value = (GValue *) raw;
	gint64 i64 = g_value_get_int64 (value);

	if (i64 < lower) return FALSE;
	if (i64 > upper) return FALSE;
	
	g_value_unset (value);
	g_value_init (value, G_TYPE_UINT);
	g_value_set_uint (value, (guint) i64);
	
	return TRUE;
}

static gboolean
gibbon_clip_parser_fixup_uint64 (void *raw, gint64 lower, gint64 upper)
{
	GValue *value = (GValue *) raw;
	gint64 i64 = g_value_get_int64 (value);

	if (i64 < lower) return FALSE;
	if (i64 > upper) return FALSE;
	
	g_value_unset (value);
	g_value_init (value, G_TYPE_UINT64);
	g_value_set_uint64 (value, (guint64) i64);
	
	return TRUE;
}

static gboolean
gibbon_clip_parser_fixup_int64 (void *raw, gint64 lower, gint64 upper)
{
	GValue *value = (GValue *) raw;
	gint64 i64 = g_value_get_int64 (value);

	if (i64 < lower) return FALSE;
	if (i64 > upper) return FALSE;
	
	g_value_unset (value);
	g_value_init (value, G_TYPE_INT64);
	g_value_set_int64 (value, (gint64) i64);
	
	return TRUE;
}

static gboolean
gibbon_clip_parser_fixup_boolean (void *raw)
{
	GValue *value = (GValue *) raw;
	gint64 i64 = g_value_get_int64 (value);
	
	if (i64 < 0) return FALSE;
	if (i64 > 1) return FALSE;
	
	g_value_unset (value);
	g_value_init (value, G_TYPE_BOOLEAN);
	g_value_set_boolean (value, (gboolean) i64);
	
	return TRUE;
}

static gboolean
gibbon_clip_parser_fixup_user (void *raw)
{
	GValue *value = (GValue *) raw;
	const gchar *string = g_value_get_string (value);

	if (!g_strcmp0 (string, "You"))
		return FALSE;
	
	return TRUE;
}

static gboolean
gibbon_clip_parser_fixup_optional_user (void *raw)
{
	GValue *value = (GValue *) raw;
	const gchar *string = g_value_get_string (value);

	if (!g_strcmp0 (string, "You"))
		return FALSE;
	if (!g_strcmp0 (string, "-"))
		g_value_set_string (value, "");
	
	return TRUE;
}
