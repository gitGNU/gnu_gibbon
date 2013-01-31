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
static gboolean gibbon_clip_parser_fixup_int (void *raw, 
                                              gint64 lower, gint64 upper);
static gboolean gibbon_clip_parser_fixup_int64 (void *raw, 
                                                gint64 lower, gint64 upper);
static gboolean gibbon_clip_parser_fixup_boolean (void *raw);
static gboolean gibbon_clip_parser_fixup_user (void *raw);
static gboolean gibbon_clip_parser_fixup_optional_user (void *raw);
static gboolean gibbon_clip_parser_fixup_maybe_you (void *raw);
static gboolean gibbon_clip_parser_fixup_match_scores (void *length_raw, 
                                                       void *score1_raw,
                                                       void *score2_raw);
static gboolean gibbon_clip_parser_fixup_color (void *raw);
static gboolean gibbon_clip_parser_fixup_home_or_bar (void *raw);
static gboolean gibbon_clip_parser_fixup_move (void *from, void *to);

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
%token <value> CLIP_MESSAGE_SAVED
%token <value> CLIP_SAYS
%token <value> CLIP_SHOUTS
%token <value> CLIP_WHISPERS
%token <value> CLIP_KIBITZES
%token <value> CLIP_YOU_SAY
%token <value> CLIP_YOU_SHOUT
%token <value> CLIP_YOU_WHISPER
%token <value> CLIP_YOU_KIBITZ
%token <value> CLIP_ALERTS
%token <value> CLIP_ERROR
%token <value> CLIP_ERROR_NO_EMAIL_ADDRESS
%token <value> CLIP_ERROR_NO_USER
%token <value> CLIP_BOARD
%token <value> CLIP_ROLLS
%token <value> CLIP_MOVES
%token <value> CLIP_START_GAME
%token <value> CLIP_LEFT_GAME
%token <value> CLIP_DROPS_GAME
%token <value> CLIP_CANNOT_MOVE
%token <value> GSTRING
%token <value> GINT64
%token <value> GDOUBLE

%token TOKEN_AND

%token GARBAGE

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
       | clip_message_saved
       | clip_says
       | clip_shouts
       | clip_whispers
       | clip_kibitzes
       | clip_you_say
       | clip_you_shout
       | clip_you_whisper
       | clip_you_kibitz
       | clip_alerts
       | clip_error
       | clip_error_no_user
       | clip_board
       | clip_rolls
       | clip_moves
       | clip_start_game
       | clip_left_game
       | clip_drops_game
       | clip_cannot_move
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
		{
			/*
			 * FIXME! Is this really needed?
			 */
			GValue *s = gibbon_clip_reader_alloc_value (
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

clip_message_saved: CLIP_MESSAGE_SAVED
	      {
		if (!gibbon_clip_parser_fixup_uint (
					$1, GIBBON_CLIP_MESSAGE_SAVED,
					GIBBON_CLIP_MESSAGE_SAVED))
				YYABORT;
	      }
	    GSTRING
    	      {
		if (!gibbon_clip_parser_fixup_user ($3))
			YYABORT;
	      }
            ;

clip_says: CLIP_SAYS
	      {
		if (!gibbon_clip_parser_fixup_uint (
					$1, GIBBON_CLIP_SAYS,
					GIBBON_CLIP_SAYS))
				YYABORT;
	      }
	    GSTRING
	    GSTRING
            ;

clip_shouts: CLIP_SHOUTS
	      {
		if (!gibbon_clip_parser_fixup_uint (
					$1, GIBBON_CLIP_SHOUTS,
					GIBBON_CLIP_SHOUTS))
				YYABORT;
	      }
	    GSTRING
	    GSTRING
            ;

clip_whispers: CLIP_WHISPERS
	      {
		if (!gibbon_clip_parser_fixup_uint (
					$1, GIBBON_CLIP_WHISPERS,
					GIBBON_CLIP_WHISPERS))
				YYABORT;
	      }
	    GSTRING
	    GSTRING
            ;

clip_kibitzes: CLIP_KIBITZES
	      {
		if (!gibbon_clip_parser_fixup_uint (
					$1, GIBBON_CLIP_KIBITZES,
					GIBBON_CLIP_KIBITZES))
				YYABORT;
	      }
	    GSTRING
	    GSTRING
            ;

clip_you_say: CLIP_YOU_SAY
	      {
		if (!gibbon_clip_parser_fixup_uint (
					$1, GIBBON_CLIP_YOU_SAY,
					GIBBON_CLIP_YOU_SAY))
				YYABORT;
	      }
	    GSTRING
	    GSTRING
            ;

clip_you_shout: CLIP_YOU_SHOUT
	      {
		if (!gibbon_clip_parser_fixup_uint (
					$1, GIBBON_CLIP_YOU_SHOUT,
					GIBBON_CLIP_YOU_SHOUT))
				YYABORT;
	      }
	    GSTRING
            ;

clip_you_whisper: CLIP_YOU_WHISPER
	      {
		if (!gibbon_clip_parser_fixup_uint (
					$1, GIBBON_CLIP_YOU_WHISPER,
					GIBBON_CLIP_YOU_WHISPER))
				YYABORT;
	      }
	    GSTRING
            ;

clip_you_kibitz: CLIP_YOU_KIBITZ
	      {
		if (!gibbon_clip_parser_fixup_uint (
					$1, GIBBON_CLIP_YOU_KIBITZ,
					GIBBON_CLIP_YOU_KIBITZ))
				YYABORT;
	      }
	    GSTRING
            ;

clip_alerts: CLIP_ALERTS
	      {
		if (!gibbon_clip_parser_fixup_uint (
					$1, GIBBON_CLIP_ALERTS,
					GIBBON_CLIP_ALERTS))
				YYABORT;
	      }
	    GSTRING
	    GSTRING
            ;

clip_error: CLIP_ERROR
	    GSTRING
	      {
	      	gibbon_clip_reader_prepend_code (reader, GIBBON_CLIP_ERROR);
	      }
            ;

clip_error_no_user: 
            CLIP_ERROR
            CLIP_ERROR_NO_USER
	      {
		if (!gibbon_clip_parser_fixup_user ($2))
			YYABORT;
	      	gibbon_clip_reader_prepend_code (reader, 
	      	                                 GIBBON_CLIP_ERROR_NO_USER);
	      }
            ;

clip_board:
	    CLIP_BOARD
	      {
		if (!gibbon_clip_parser_fixup_uint (
					$1, GIBBON_CLIP_BOARD,
					GIBBON_CLIP_BOARD))
				YYABORT;
	      }
	    /* Player 1.  */
            ':' GSTRING
	      {
		if (!gibbon_clip_parser_fixup_maybe_you ($4))
		        YYABORT;
	      }
	    /* Player 2.  */
            ':' GSTRING
	      {
		if (!gibbon_clip_parser_fixup_user ($7))
		        YYABORT;
	      }
	    /* Match length.  */
	    ':' GINT64
	    /* Both scores.  */
	    ':' GINT64
	    ':' GINT64
	      {
		if (!gibbon_clip_parser_fixup_match_scores ($10, $12, $14))
			YYABORT;
	      }
	    /* 26 points.  */
	    ':' GINT64
	      {
		if (!gibbon_clip_parser_fixup_int ($17, -15, 15))
				YYABORT;
	      }
	    ':' GINT64
	      {
		if (!gibbon_clip_parser_fixup_int ($20, -15, 15))
				YYABORT;
	      }
	    ':' GINT64
	      {
		if (!gibbon_clip_parser_fixup_int ($23, -15, 15))
				YYABORT;
	      }
	    ':' GINT64
	      {
		if (!gibbon_clip_parser_fixup_int ($26, -15, 15))
				YYABORT;
	      }
	    ':' GINT64
	      {
		if (!gibbon_clip_parser_fixup_int ($29, -15, 15))
				YYABORT;
	      }
	    ':' GINT64
	      {
		if (!gibbon_clip_parser_fixup_int ($32, -15, 15))
				YYABORT;
	      }
	    ':' GINT64
	      {
		if (!gibbon_clip_parser_fixup_int ($35, -15, 15))
				YYABORT;
	      }
	    ':' GINT64
	      {
		if (!gibbon_clip_parser_fixup_int ($38, -15, 15))
				YYABORT;
	      }
	    ':' GINT64
	      {
		if (!gibbon_clip_parser_fixup_int ($41, -15, 15))
				YYABORT;
	      }
	    ':' GINT64
	      {
		if (!gibbon_clip_parser_fixup_int ($44, -15, 15))
				YYABORT;
	      }
	    ':' GINT64
	      {
		if (!gibbon_clip_parser_fixup_int ($47, -15, 15))
				YYABORT;
	      }
	    ':' GINT64
	      {
		if (!gibbon_clip_parser_fixup_int ($50, -15, 15))
				YYABORT;
	      }
	    ':' GINT64
	      {
		if (!gibbon_clip_parser_fixup_int ($53, -15, 15))
				YYABORT;
	      }
	    ':' GINT64
	      {
		if (!gibbon_clip_parser_fixup_int ($56, -15, 15))
				YYABORT;
	      }
	    ':' GINT64
	      {
		if (!gibbon_clip_parser_fixup_int ($59, -15, 15))
				YYABORT;
	      }
	    ':' GINT64
	      {
		if (!gibbon_clip_parser_fixup_int ($62, -15, 15))
				YYABORT;
	      }
	    ':' GINT64
	      {
		if (!gibbon_clip_parser_fixup_int ($65, -15, 15))
				YYABORT;
	      }
	    ':' GINT64
	      {
		if (!gibbon_clip_parser_fixup_int ($68, -15, 15))
				YYABORT;
	      }
	    ':' GINT64
	      {
		if (!gibbon_clip_parser_fixup_int ($71, -15, 15))
				YYABORT;
	      }
	    ':' GINT64
	      {
		if (!gibbon_clip_parser_fixup_int ($74, -15, 15))
				YYABORT;
	      }
	    ':' GINT64
	      {
		if (!gibbon_clip_parser_fixup_int ($77, -15, 15))
				YYABORT;
	      }
	    ':' GINT64
	      {
		if (!gibbon_clip_parser_fixup_int ($80, -15, 15))
				YYABORT;
	      }
	    ':' GINT64
	      {
		if (!gibbon_clip_parser_fixup_int ($83, -15, 15))
				YYABORT;
	      }
	    ':' GINT64
	      {
		if (!gibbon_clip_parser_fixup_int ($86, -15, 15))
				YYABORT;
	      }
	    ':' GINT64
	      {
		if (!gibbon_clip_parser_fixup_int ($89, -15, 15))
				YYABORT;
	      }
	    ':' GINT64
	      {
		if (!gibbon_clip_parser_fixup_int ($92, -15, 15))
				YYABORT;
	      }
	    /* Turn.  */
	    ':' GINT64
	      {
		if (!gibbon_clip_parser_fixup_int ($95, -1, 1))
				YYABORT;
	      }
	    /* Player's dice.  */
	    ':' GINT64
	      {
		if (!gibbon_clip_parser_fixup_int ($98, 0, 6))
				YYABORT;
	      }
	    ':' GINT64
	      {
		if (!gibbon_clip_parser_fixup_int ($101, 0, 6))
				YYABORT;
	      }
	    /* Opponent's dice.  */
	    ':' GINT64
	      {
		if (!gibbon_clip_parser_fixup_int ($104, 0, 6))
				YYABORT;
	      }
	    ':' GINT64
	      {
		if (!gibbon_clip_parser_fixup_int ($107, 0, 6))
				YYABORT;
	      }
	    /* Doubling cube.  */
	    ':' GINT64
	      {
	        /*
	         * FIXME! Check that only one bit is set in the value.
	         */
		if (!gibbon_clip_parser_fixup_int64 ($110, 1, G_MAXINT64))
				YYABORT;
	      }
	    /* Player may-double.  */
	    ':' GINT64
	      {
		if (!gibbon_clip_parser_fixup_boolean ($113))
				YYABORT;
	      }
	    /* Opponent may-double.  */
	    ':' GINT64
	      {
		if (!gibbon_clip_parser_fixup_boolean ($116))
				YYABORT;
	      }
	    /* Was doubled.  */
	    ':' GINT64
	      {
		if (!gibbon_clip_parser_fixup_boolean ($119))
				YYABORT;
	      }
	    /* Color.  */
	    ':' GINT64
	      {
		if (!gibbon_clip_parser_fixup_color ($122))
				YYABORT;
	      }
	    /* Direction.  */
	    ':' GINT64
	      {
		if (!gibbon_clip_parser_fixup_color ($125))
				YYABORT;
	      }
	    /* Home or bar.  */
	    ':' GINT64
	      {
		if (!gibbon_clip_parser_fixup_home_or_bar ($128))
				YYABORT;
	      }
	    ':' GINT64
	      {
		if (!gibbon_clip_parser_fixup_home_or_bar ($131))
				YYABORT;
	      }
	    /* On home.  */
	    ':' GINT64
	      {
		if (!gibbon_clip_parser_fixup_uint ($134, 0, 15))
				YYABORT;
	      }
	    ':' GINT64
	      {
		if (!gibbon_clip_parser_fixup_uint ($137, 0, 15))
				YYABORT;
	      }
	    /* On bar.  */
	    ':' GINT64
	      {
		if (!gibbon_clip_parser_fixup_uint ($140, 0, 15))
				YYABORT;
	      }
	    ':' GINT64
	      {
		if (!gibbon_clip_parser_fixup_uint ($143, 0, 15))
				YYABORT;
	      }
	    /* Number of pieces that can be moved.  In reality bogus.  */
	    ':' GINT64
	      {
		if (!gibbon_clip_parser_fixup_uint ($146, 0, G_MAXINT))
				YYABORT;
	      }
	    ':' GINT64
	      {
		if (!gibbon_clip_parser_fixup_boolean ($149))
				YYABORT;
	      }
	    /* Did Crawford.  Really "post-Crawford".  */
	    ':' GINT64
	      {
		if (!gibbon_clip_parser_fixup_boolean ($152))
				YYABORT;
	      }
	    /* Number of redoubles.  */
	    ':' GINT64
	      {
		if (!gibbon_clip_parser_fixup_int ($155, 0, G_MAXINT))
				YYABORT;
	      }
	      {
	      	if (!gibbon_clip_reader_fixup_board (reader))
	      		YYABORT;
	      }
	    ;

clip_rolls: 
            CLIP_ROLLS 
	      {
		if (!gibbon_clip_parser_fixup_maybe_you ($1))
		        YYABORT;
	      }
	    GINT64
	      {
		if (!gibbon_clip_parser_fixup_uint ($3, 1, 6))
		        YYABORT;
	      }
	    TOKEN_AND
	    GINT64
	      {
		if (!gibbon_clip_parser_fixup_uint ($6, 1, 6))
		        YYABORT;
		gibbon_clip_reader_prepend_code (reader, GIBBON_CLIP_ROLLS);
	      }
            ;

clip_moves: 
            CLIP_MOVES
	      {
		if (!gibbon_clip_parser_fixup_user ($1))
		        YYABORT;
	      }
	    moves
	      {
		gibbon_clip_reader_prepend_code (reader, GIBBON_CLIP_MOVES);
	      }
            ;

moves: move
     | move move
     | move move move
     | move move move move
     ;
     
move: GINT64 GINT64
        {
		if (!gibbon_clip_parser_fixup_move ($1, $2))
		        YYABORT;
        }
    ;
    
clip_start_game: 
            CLIP_START_GAME
            GSTRING
	      {
		if (!gibbon_clip_parser_fixup_user ($2))
		        YYABORT;
		gibbon_clip_reader_prepend_code (reader, 
		                                 GIBBON_CLIP_START_GAME);
	      }
            ;

clip_left_game: 
            CLIP_ERROR
            GSTRING
	      {
		if (!gibbon_clip_parser_fixup_maybe_you ($2))
		        YYABORT;
	      }
            CLIP_LEFT_GAME
	      {
		gibbon_clip_reader_prepend_code (reader, 
		                                 GIBBON_CLIP_LEFT_GAME);
	      }
            ;

clip_drops_game: 
            CLIP_DROPS_GAME
	      {
		if (!gibbon_clip_parser_fixup_maybe_you ($1))
		        YYABORT;
		gibbon_clip_reader_prepend_code (reader, 
		                                 GIBBON_CLIP_DROPS_GAME);
	      }
            ;

clip_cannot_move: 
            CLIP_CANNOT_MOVE
	      {
		if (!gibbon_clip_parser_fixup_maybe_you ($1))
		        YYABORT;
		gibbon_clip_reader_prepend_code (reader, 
		                                 GIBBON_CLIP_CANNOT_MOVE);
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
gibbon_clip_parser_fixup_int (void *raw, gint64 lower, gint64 upper)
{
	GValue *value = (GValue *) raw;
	gint64 i64 = g_value_get_int64 (value);

	if (i64 < lower) return FALSE;
	if (i64 > upper) return FALSE;
	
	g_value_unset (value);
	g_value_init (value, G_TYPE_INT);
	g_value_set_int (value, (gint) i64);
	
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


static gboolean
gibbon_clip_parser_fixup_maybe_you (void *raw)
{
	/*
	 * We accept all user names as long as they do not contain whitespace
	 * or a colon and that is already enforced by the scanner.
	 */
	return TRUE;
}

static gboolean
gibbon_clip_parser_fixup_match_scores (void *length_raw, 
                                       void *score1_raw,
                                       void *score2_raw)
{
	GValue *length_value = (GValue *) length_raw;
	gint64 length_i64 = g_value_get_int64 (length_value);
	GValue *score1_value = (GValue *) score1_raw;
	gint64 score1_i64 = g_value_get_int64 (score1_value);
	GValue *score2_value = (GValue *) score2_raw;
	gint64 score2_i64 = g_value_get_int64 (score2_value);
	
	if (length_i64 < 0) return FALSE;
	if (length_i64 >= 9999) {
		length_i64 = 0;
		g_value_set_int64 (length_value, length_i64);
	} else if (score1_i64 >= length_i64 && score2_i64 >= length_i64) {
		/*
		 * Impossible score.
		 */
		return FALSE;
	}

	g_value_unset (length_value);
	g_value_unset (score1_value);
	g_value_unset (score2_value);
	g_value_init (length_value, G_TYPE_UINT);
	g_value_init (score1_value, G_TYPE_UINT);
	g_value_init (score2_value, G_TYPE_UINT);
	g_value_set_uint (length_value, length_i64);
	g_value_set_uint (score1_value, score1_i64);
	g_value_set_uint (score2_value, score2_i64);
	
	return TRUE;
}

static gboolean
gibbon_clip_parser_fixup_color (void *raw)
{
	GValue *value = (GValue *) raw;
	gint64 i64 = g_value_get_int64 (value);
	
	if (i64 == 0 || i64 > 1 || i64 < -1) return FALSE;
	
	g_value_unset (value);
	g_value_init (value, G_TYPE_INT);
	g_value_set_int (value, i64);
	
	return TRUE;
}

static gboolean
gibbon_clip_parser_fixup_home_or_bar (void *raw)
{
	GValue *value = (GValue *) raw;
	gint64 i64 = g_value_get_int64 (value);
	
	if (i64 != 0 && i64 != 25) return FALSE;
	
	return TRUE;
}

/*
 * Transform the raw integers that came from the lexer into points.  This
 * does two things:
 *
 * - The lexer will usually return -N for the destination point in a move
 *   because it cannot distinguish between the separating hyphen and a minus
 *   sign.
 * - It translates a 0 into a 25 where necessary.  The lexer returns 0 for
 *   both "bar" and "off".  We decide what was meant by looking at the other
 *   point.
 *
 * Additionally, we also check the difference between the two points not
 * being 0 or greater than 6.
 */
static gboolean
gibbon_clip_parser_fixup_move (void *from, void *to)
{
	GValue *from_value = (GValue *) from;
	gint64 from_i64 = ABS (g_value_get_int64 (from_value));
	GValue *to_value = (GValue *) to;
	gint64 to_i64 = ABS (g_value_get_int64 (to_value));
	
	if (from_i64 > 24)
	    return FALSE;
	if (to_i64 > 24)
	    return FALSE;
	    
	if (from_i64 == 0) {
            if (to_i64 > 18)
                from_i64 = 25;
	}
	if (to_i64 == 0) {
            if (from_i64 > 18)
                to_i64 = 25;
	}
	
	if (from_i64 == to_i64)
	    return FALSE;
	if (ABS (from_i64 - to_i64) > 6)
	    return FALSE;

	g_value_unset (from_value);
	g_value_unset (to_value);
	g_value_init (from_value, G_TYPE_UINT);
	g_value_init (to_value, G_TYPE_UINT);
	g_value_set_uint (from_value, from_i64);
	g_value_set_uint (to_value, to_i64);
	
	return TRUE;
}
