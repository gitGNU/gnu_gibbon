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
 * Parser for the internal format of JavaFIBS.  This grammar describes that
 * format only loosely.  This is sufficient for parsing because we will
 * ignore all redundant date, while building the syntax tree.
 */

%{
#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include <glib.h>
#include <glib/gi18n.h>

#include "gibbon-java-fibs-parser.h"
#include "gibbon-java-fibs-reader-priv.h"

#define reader gibbon_java_fibs_lexer_get_extra(scanner)

/*
 * Remap normal yacc parser interface names (yyparse, yylex, yyerror, etc),
 * as well as gratuitiously global symbol names, so we can have multiple
 * yacc generated parsers in the same program.  Note that these are only
 * the variables produced by yacc.  If other parser generators (bison,
 * byacc, etc) produce additional global names that conflict at link time,
 * then those parser generators need to be fixed instead of adding those
 * names to this list. 
 */

#define yymaxdepth gibbon_java_fibs_parser_maxdepth
#define yyparse(s)    gibbon_java_fibs_parser_parse(s)
#define yylex      gibbon_java_fibs_lexer_lex
extern int gibbon_java_fibs_lexer_lex (YYSTYPE * lvalp, void *scanner);
#define yyerror    gibbon_java_fibs_reader_yyerror
#define yylval     gibbon_java_fibs_parser_lval
#define yychar     gibbon_java_fibs_parser_char
#define yydebug    gibbon_java_fibs_parser_debug
#define yypact     gibbon_java_fibs_parser_pact
#define yyr1       gibbon_java_fibs_parser_r1
#define yyr2       gibbon_java_fibs_parser_r2
#define yydef      gibbon_java_fibs_parser_def
#define yychk      gibbon_java_fibs_parser_chk
#define yypgo      gibbon_java_fibs_parser_pgo
#define yyact      gibbon_java_fibs_parser_act
#define yyexca     gibbon_java_fibs_parser_exca
#define yyerrflag  gibbon_java_fibs_parser_errflag
#define yynerrs    gibbon_java_fibs_parser_nerrs
#define yyps       gibbon_java_fibs_parser_ps
#define yypv       gibbon_java_fibs_parser_pv
#define yys        gibbon_java_fibs_parser_s
#define yy_yys     gibbon_java_fibs_parser_yys
#define yystate    gibbon_java_fibs_parser_state
#define yytmp      gibbon_java_fibs_parser_tmp
#define yyv        gibbon_java_fibs_parser_v
#define yy_yyv     gibbon_java_fibs_parser_yyv
#define yyval      gibbon_java_fibs_parser_val
#define yylloc     gibbon_java_fibs_parser_lloc
#define yyreds     gibbon_java_fibs_parser_reds    /* With YYDEBUG defined */
#define yytoks     gibbon_java_fibs_parser_toks    /* With YYDEBUG defined */
#define yylhs      gibbon_java_fibs_parser_yylhs
#define yylen      gibbon_java_fibs_parser_yylen
#define yydefred   gibbon_java_fibs_parser_yydefred
#define yysdgoto    gibbon_java_fibs_parser_yydgoto
#define yysindex   gibbon_java_fibs_parser_yysindex
#define yyrindex   gibbon_java_fibs_parser_yyrindex
#define yygindex   gibbon_java_fibs_parser_yygindex
#define yytable    gibbon_java_fibs_parser_yytable
#define yycheck    gibbon_java_fibs_parser_yycheck

#define YYDEBUG 42

static guint gibbon_java_fibs_parser_encode_movement (guint64 from, guint64 to);
%}

%union {
	guint64 num;
	gchar *name;
}

%token PROLOG
%token COLON
%token HYPHEN
%token <num> INTEGER
%token <name> PLAYER
%token ROLL
%token MOVE
%token CUBE
%token RESIGN
%token TAKE
%token DROP
%token START_OF_GAME
%token WIN_GAME
%token START_OF_MATCH
%token WIN_MATCH
%token REJECT_RESIGN
%token OPPONENTS
%token SCORE
%token BAR
%token OFF

%type <num>point
%type <num>movement
%type <num>movements

%pure-parser
%lex-param {void *scanner}
%parse-param {void *scanner}

%%

java_fibs_file
	: /* empty */
		{
			yyerror (scanner,
			         _("Empty file!"));
			YYABORT;
	        }
        | { yydebug = 0; } 
          PROLOG match  { gibbon_java_fibs_reader_free_names (reader); }
        ;

match
	: START_OF_MATCH COLON PLAYER 
		{
			_gibbon_java_fibs_reader_set_black (reader, $3);
		}
	  COLON INTEGER 
		{
			_gibbon_java_fibs_reader_set_match_length (reader, $6);
		}
	  playing
	;

playing
	: games win_match
	| games incomplete_game
	;

games
	: /* empty */
	| games game
	;

game
	: start_of_game opponents actions win_game score
	| start_of_game opponents actions win_game
	;

incomplete_game
	: start_of_game opponents actions
	;

start_of_game
	: START_OF_GAME COLON PLAYER COLON 
		{ 
			_gibbon_java_fibs_reader_set_black (reader, $3);
			if (!_gibbon_java_fibs_reader_add_game (reader))
				YYABORT;
		}
	;

opponents
	: OPPONENTS COLON PLAYER
		{
			_gibbon_java_fibs_reader_set_white (reader, $3);
		}
 	  COLON PLAYER
		{
			_gibbon_java_fibs_reader_set_black (reader, $6);
		}
 	;

actions
	: /* empty */
	| actions action { gibbon_java_fibs_reader_free_names (reader); }
	;
	
action
	: roll | move | cube | drop | take
	| resign | reject_resign
	;

roll
	: ROLL COLON PLAYER COLON INTEGER INTEGER
		{
			if (!_gibbon_java_fibs_reader_roll (reader, $3, $5, $6))
				YYABORT;
		}
	;

move
	: MOVE COLON PLAYER COLON movements
		{
			if (!_gibbon_java_fibs_reader_move (reader, $3, $5))
				YYABORT;
		}
	/*
	 * Sometimes, JavaFIBS appends a gratuitous "xyz-moves ...".  We just
	 * match that exception without being too strict about the exact
	 * semantics.
	 */
	| MOVE COLON PLAYER COLON movements PLAYER HYPHEN PLAYER garbage
		{
			if (!_gibbon_java_fibs_reader_move (reader, $3, $5))
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

garbage
	: /* empty */
	| garbage point
	| garbage HYPHEN
	;
	
movement
	: point HYPHEN point 
		{
			if ($1 == $3) {
				yyerror (scanner,
				         _("Start and end point are equal!"));
				YYABORT;
			} 
			$$ = gibbon_java_fibs_parser_encode_movement ($1, $3);
		}
	;

point
	: INTEGER 
		{ 
			if (!$$ || $$ > 24) {
				yyerror (scanner,
				         _("Point out of range (1-24)!"));
				YYABORT;
			}
		}
	  | BAR { $$ = 25; }
	  | OFF { $$ = 0; }
	;

cube
	: CUBE COLON PLAYER COLON
		{
			if (!_gibbon_java_fibs_reader_double (reader, $3))
				YYABORT;
		}
	;

drop
	: DROP COLON PLAYER COLON
		{
			if (!_gibbon_java_fibs_reader_drop (reader, $3))
				YYABORT;
		}
	;

take
	: TAKE COLON PLAYER COLON
		{
			if (!_gibbon_java_fibs_reader_take (reader, $3))
				YYABORT;
		}
	;
	
win_game
	: WIN_GAME COLON PLAYER COLON INTEGER
		{
			if (!_gibbon_java_fibs_reader_win_game (reader, $3, $5))
				YYABORT;
		}
	;
	
score
	: SCORE COLON PLAYER HYPHEN INTEGER COLON PLAYER HYPHEN INTEGER
		{
			if (!_gibbon_java_fibs_reader_score (reader, 
                                                             $3, $5,
                                                             $7, $9))
				YYABORT;
		}
	;

resign
	: RESIGN COLON PLAYER COLON INTEGER
		{
			if (!_gibbon_java_fibs_reader_resign (reader, $3, $5))
				YYABORT;
		}
	;
	
reject_resign
	: REJECT_RESIGN COLON PLAYER COLON
		{
			if (!_gibbon_java_fibs_reader_reject_resign (reader, 
			                                             $3))
				YYABORT;
		}
	;
	
win_match
	: WIN_MATCH COLON PLAYER COLON INTEGER
	;

%%

static guint
gibbon_java_fibs_parser_encode_movement (guint64 from, guint64 to)
{
	/*
	 * This is the first normalization step for JavaFIBS moves.
	 * Depending on the direction of the move we translate 25 and 0 to
	 * bar and off.
	 */
	if (from >= 25 && to <= 6)
		/* Come in from bar.  */
		from = 0;
	if (to == 0 && from > 6)
		/* Bear-off.  */
		to = 25;

	/*
	 * And now we make sure that we move in descending direction.  That
	 * corresponds to white's move direction in our internal notion.
	 */
	if (from < to) {
		from = -from + 25;
		to = -to + 25;
	}
	
	return (from << 8 | to);
}
