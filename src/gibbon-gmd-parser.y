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
 * Parser for the internal format of Gibbon.  The internal format is only
 * used for saving running matches.  They are archived as SGF.
 */

%{
#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include <glib.h>
#include <glib/gi18n.h>

#include "gibbon-gmd-parser.h"
#include "gibbon-gmd-reader-priv.h"

#define reader _gibbon_gmd_reader_instance

/*
 * Remap normal yacc parser interface names (yyparse, yylex, yyerror, etc),
 * as well as gratuitiously global symbol names, so we can have multiple
 * yacc generated parsers in the same program.  Note that these are only
 * the variables produced by yacc.  If other parser generators (bison,
 * byacc, etc) produce additional global names that conflict at link time,
 * then those parser generators need to be fixed instead of adding those
 * names to this list. 
 */

#define yymaxdepth gibbon_gmd_parser_maxdepth
#define yyparse    gibbon_gmd_parser_parse
#define yylex      gibbon_gmd_lexer_lex
extern int gibbon_gmd_lexer_lex (void);
#define yyerror    _gibbon_gmd_reader_yyerror
#define yylval     gibbon_gmd_parser_lval
#define yychar     gibbon_gmd_parser_char
#define yydebug    gibbon_gmd_parser_debug
#define yypact     gibbon_gmd_parser_pact
#define yyr1       gibbon_gmd_parser_r1
#define yyr2       gibbon_gmd_parser_r2
#define yydef      gibbon_gmd_parser_def
#define yychk      gibbon_gmd_parser_chk
#define yypgo      gibbon_gmd_parser_pgo
#define yyact      gibbon_gmd_parser_act
#define yyexca     gibbon_gmd_parser_exca
#define yyerrflag  gibbon_gmd_parser_errflag
#define yynerrs    gibbon_gmd_parser_nerrs
#define yyps       gibbon_gmd_parser_ps
#define yypv       gibbon_gmd_parser_pv
#define yys        gibbon_gmd_parser_s
#define yy_yys     gibbon_gmd_parser_yys
#define yystate    gibbon_gmd_parser_state
#define yytmp      gibbon_gmd_parser_tmp
#define yyv        gibbon_gmd_parser_v
#define yy_yyv     gibbon_gmd_parser_yyv
#define yyval      gibbon_gmd_parser_val
#define yylloc     gibbon_gmd_parser_lloc
#define yyreds     gibbon_gmd_parser_reds    /* With YYDEBUG defined */
#define yytoks     gibbon_gmd_parser_toks    /* With YYDEBUG defined */
#define yylhs      gibbon_gmd_parser_yylhs
#define yylen      gibbon_gmd_parser_yylen
#define yydefred   gibbon_gmd_parser_yydefred
#define yysdgoto    gibbon_gmd_parser_yydgoto
#define yysindex   gibbon_gmd_parser_yysindex
#define yyrindex   gibbon_gmd_parser_yyrindex
#define yygindex   gibbon_gmd_parser_yygindex
#define yytable    gibbon_gmd_parser_yytable
#define yycheck    gibbon_gmd_parser_yycheck

#define YYDEBUG 42
%}

%union {
	guint64 num;
	gdouble dbl;
	gchar *name;
	gint side;
}

%token MAGIC
%token HYPHEN
%token COLON
%token <num> INTEGER
%token <name> NAME
%token LENGTH
%token PLAYER
%token RANK
%token LOCATION
%token RULE
%token CRAWFORD
%token GAME
%token WHITE
%token BLACK
%token NO_COLOR
%token ROLL
%token MOVE
%token SLASH
%token BAR
%token HOME
%token DOUBLE
%token DROP
%token TAKE
%token RESIGN
%token REJ
%token ACCEPT
%token UNKNOWN
%token GARBAGE

%type <side> color
%type <num> movements
%type <num> movement
%type <num> point

%%

gmd_file
	: { yydebug = 0; } MAGIC HYPHEN INTEGER things
		{
			_gibbon_gmd_reader_free_names (reader);
		}
        ;

things
	: /* empty */
	| things thing
		{
			_gibbon_gmd_reader_free_names (reader);
		}
	;

thing
	: property
	| action
	;

property
	: length | player | rank | location | rule | game | unknown
	;
	
length
	: LENGTH COLON INTEGER
		{
			_gibbon_gmd_reader_set_match_length (reader, $3);
		}
	;

player
	: PLAYER COLON color COLON NAME
		{
			_gibbon_gmd_reader_set_player (reader, $3, $5);
		}
	;

rank
	: RANK COLON color COLON NAME
		{
			_gibbon_gmd_reader_set_rank (reader, $3, $5);
		}
	;

location
	: LOCATION COLON NAME
		{
			_gibbon_gmd_reader_set_location (reader, $3);
		}
	;

game
	: GAME COLON
		{
			if (!_gibbon_gmd_reader_add_game (reader))
				YYABORT;
		}
	;

rule
	: RULE COLON CRAWFORD
		{
			_gibbon_gmd_reader_set_crawford (reader);
		}
	| RULE COLON UNKNOWN
	;

action
	: roll | move | double | drop | take | resign | reject | accept
	;

color
	: BLACK    { $$ = GIBBON_POSITION_SIDE_BLACK; }
	| WHITE    { $$ = GIBBON_POSITION_SIDE_WHITE; }
	| NO_COLOR { $$ = GIBBON_POSITION_SIDE_NONE; }
	;

roll
	: ROLL COLON color COLON INTEGER INTEGER
		{
			if (!_gibbon_gmd_reader_roll (reader, $3, $5, $6))
				YYABORT;
		}
	;

move
	: MOVE COLON color COLON movements
		{
			if (!_gibbon_gmd_reader_move (reader, $3, $5))
				YYABORT;
		}
	;

movements
	: /* empty */
		{
			$$ = 0;
		}
	| movement /* $$ = $1 */
	| movement movement
		{
			$$ = $1 << 16 | $2;
		}
	| movement movement movement
		{
			$$ = $1 << 32 | $2 << 16 | $3;
		}
	| movement movement movement movement
		{
			$$ = $1 << 48 | $2 << 32 | $3 << 16 | $4;
		}
	;

movement
	: point SLASH point 
		{
			$$ = ($1 << 8 | $3);
		}
	;

point
	: INTEGER
		{ 
			if (!$$ || $$ > 24) {
				yyerror (_("Point out of range (1-24)!"));
				YYABORT;
			}
		}
	  /* 
	   * These have to be decoded on a higher level with more
	   * context available.
	   */
	| HOME { $$ = 26; }
	| BAR  { $$ = 27; }
	;

double
	: DOUBLE COLON color
		{
			if (!_gibbon_gmd_reader_double (reader, $3))
				YYABORT;
		}
	;

drop
	: DROP COLON color
		{
			if (!_gibbon_gmd_reader_drop (reader, $3))
				YYABORT;
		}
	;

take
	: TAKE COLON color
		{
			if (!_gibbon_gmd_reader_take (reader, $3))
				YYABORT;
		}
	;

resign
	: RESIGN COLON color COLON INTEGER
		{
			if (!$5) {
				yyerror (_("Resignation value cannot be"
				           " zero!"));
				YYABORT;
			}
			if (!_gibbon_gmd_reader_resign (reader, $3, $5))
				YYABORT;
		}
	;

reject
	: REJ COLON color
		{
			if (!_gibbon_gmd_reader_reject (reader, $3))
				YYABORT;
		}
	;

accept
	: ACCEPT COLON color
		{
			if (!_gibbon_gmd_reader_accept (reader, $3))
				YYABORT;
		}
	;

unknown
	: UNKNOWN COLON GARBAGE
	;
%%
