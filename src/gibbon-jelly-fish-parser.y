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

#define reader gibbon_jelly_fish_lexer_get_extra(scanner)

/*
 * Remap normal yacc parser interface names (yyparse, yylex, yyerror, etc),
 * as well as gratuitiously global symbol names, so we can have multiple
 * yacc generated parsers in the same program.  Note that these are only
 * the variables produced by yacc.  If other parser generators (bison,
 * byacc, etc) produce additional global names that conflict at link time,
 * then those parser generators need to be fixed instead of adding those
 * names to this list. 
 */

#define yymaxdepth gibbon_jelly_fish_parser_maxdepth
#define yyparse(s)    gibbon_jelly_fish_parser_parse(s)
#define yylex      gibbon_jelly_fish_lexer_lex
extern int gibbon_jelly_fish_lexer_lex (YYSTYPE * lvalp, void *scanner);
#define yyerror    gibbon_jelly_fish_reader_yyerror
#define yylval     gibbon_jelly_fish_parser_lval
#define yychar     gibbon_jelly_fish_parser_char
#define yydebug    gibbon_jelly_fish_parser_debug
#define yypact     gibbon_jelly_fish_parser_pact
#define yyr1       gibbon_jelly_fish_parser_r1
#define yyr2       gibbon_jelly_fish_parser_r2
#define yydef      gibbon_jelly_fish_parser_def
#define yychk      gibbon_jelly_fish_parser_chk
#define yypgo      gibbon_jelly_fish_parser_pgo
#define yyact      gibbon_jelly_fish_parser_act
#define yyexca     gibbon_jelly_fish_parser_exca
#define yyerrflag  gibbon_jelly_fish_parser_errflag
#define yynerrs    gibbon_jelly_fish_parser_nerrs
#define yyps       gibbon_jelly_fish_parser_ps
#define yypv       gibbon_jelly_fish_parser_pv
#define yys        gibbon_jelly_fish_parser_s
#define yy_yys     gibbon_jelly_fish_parser_yys
#define yystate    gibbon_jelly_fish_parser_state
#define yytmp      gibbon_jelly_fish_parser_tmp
#define yyv        gibbon_jelly_fish_parser_v
#define yy_yyv     gibbon_jelly_fish_parser_yyv
#define yyval      gibbon_jelly_fish_parser_val
#define yylloc     gibbon_jelly_fish_parser_lloc
#define yyreds     gibbon_jelly_fish_parser_reds    /* With YYDEBUG defined */
#define yytoks     gibbon_jelly_fish_parser_toks    /* With YYDEBUG defined */
#define yylhs      gibbon_jelly_fish_parser_yylhs
#define yylen      gibbon_jelly_fish_parser_yylen
#define yydefred   gibbon_jelly_fish_parser_yydefred
#define yysdgoto    gibbon_jelly_fish_parser_yydgoto
#define yysindex   gibbon_jelly_fish_parser_yysindex
#define yyrindex   gibbon_jelly_fish_parser_yyrindex
#define yygindex   gibbon_jelly_fish_parser_yygindex
#define yytable    gibbon_jelly_fish_parser_yytable
#define yycheck    gibbon_jelly_fish_parser_yycheck

static guint gibbon_jelly_fish_parser_encode_movement (guint64 from,
                                                       guint64 to);

%}

%union {
	guint64 num;
	gchar *name;
}

%token <num> INTEGER
%token <name> PLAYER
%token MATCH_LENGTH
%token GAME
%token COLON
%token WHITESPACE
%token PAREN
%token <num> ROLL
%token SLASH
%token <num> POINT
%token DOUBLES
%token DROPS
%token TAKES
%token <num> WINS

%type <name> opponent
%type <num> movement
%type <num> movements

%pure-parser
%lex-param {void *scanner}
%parse-param {void *scanner}

%%

jelly_fish_file
        : prolog games { gibbon_jelly_fish_reader_free_names (reader); }
        ;

prolog
	: INTEGER MATCH_LENGTH
		{
			_gibbon_jelly_fish_reader_set_match_length (reader, $1);
		}
	;

games
	: game
	| games game
	;
	
game
	: game_prolog opponents 
		{
			_gibbon_jelly_fish_reader_set_side (
				reader,
				GIBBON_POSITION_SIDE_BLACK
			);
		}
		actions
	;

game_prolog
	: GAME INTEGER
		{
			if (!_gibbon_jelly_fish_reader_add_game (reader))
					YYABORT;
		}
	;

opponents
	: opponent opponent
		{
			_gibbon_jelly_fish_reader_set_black (reader, $1);
			_gibbon_jelly_fish_reader_set_white (reader, $2);
			gibbon_jelly_fish_reader_free_names (reader);
		}
	;

opponent
	: PLAYER COLON INTEGER { $$ = $1; }
	;

actions
	: /* empty */
	| action actions
	;

action
	: move
	;
	
move
	: INTEGER PAREN 
		half_move
		{
			_gibbon_jelly_fish_reader_set_side (
				reader,
				GIBBON_POSITION_SIDE_WHITE
			);
		}
		half_move
		{
			_gibbon_jelly_fish_reader_set_side (
				reader,
				GIBBON_POSITION_SIDE_BLACK
			);
		}
	| INTEGER PAREN half_move
		{
			_gibbon_jelly_fish_reader_set_side (
				reader,
				GIBBON_POSITION_SIDE_BLACK
			);
		}
	| INTEGER PAREN WHITESPACE
		{
			_gibbon_jelly_fish_reader_set_side (
				reader,
				GIBBON_POSITION_SIDE_WHITE
			);
		}
		half_move
		{
			_gibbon_jelly_fish_reader_set_side (
				reader,
				GIBBON_POSITION_SIDE_BLACK
			);
		}
	| INTEGER PAREN WHITESPACE 
		{
			_gibbon_jelly_fish_reader_set_side (
				reader,
				GIBBON_POSITION_SIDE_WHITE
			);
		}
		WINS
		{
			if (!_gibbon_jelly_fish_reader_win_game (reader, $5))
				YYABORT;
			_gibbon_jelly_fish_reader_set_side (
				reader,
				GIBBON_POSITION_SIDE_NONE
			);
		}
	| WHITESPACE 
		{
			_gibbon_jelly_fish_reader_set_side (
				reader,
				GIBBON_POSITION_SIDE_WHITE
			);
		}
		WINS
		{
			if (!_gibbon_jelly_fish_reader_win_game (reader, $3))
				YYABORT;
			_gibbon_jelly_fish_reader_set_side (
				reader,
				GIBBON_POSITION_SIDE_NONE
			);
		}
	| WINS
		{
			if (!_gibbon_jelly_fish_reader_win_game (reader, $1))
				YYABORT;
			_gibbon_jelly_fish_reader_set_side (
				reader,
				GIBBON_POSITION_SIDE_NONE
			);
		}
	;

half_move
	: ROLL movements
		{
			if (!_gibbon_jelly_fish_reader_move (reader, $1, $2))
				YYABORT;
		}
	| DOUBLES
		{
			if (!_gibbon_jelly_fish_reader_double (reader))
				YYABORT;
		}
	| TAKES
		{
			if (!_gibbon_jelly_fish_reader_take (reader))
				YYABORT;
		}
	| DROPS
		{
			if (!_gibbon_jelly_fish_reader_drop (reader))
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
	: POINT SLASH POINT
		{
			if ($1 == $3) {
				yyerror (scanner, 
				         _("Start and end point are equal!"));
				YYABORT;
			} 
			$$ = gibbon_jelly_fish_parser_encode_movement ($1, $3);
		}
	;
	
%%

static guint
gibbon_jelly_fish_parser_encode_movement (guint64 from, guint64 to)
{
	return (from << 8 | to);
}
