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

#define reader gibbon_clip_lexer_get_extra(scanner)

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

%}

%token CLIP_WELCOME<value>
%token GSTRING<value>
%token GINT64<value>

%union {
	void *value;
}

%pure-parser
%lex-param {void *scanner}
%parse-param {void *scanner}

%%
line: { yydebug = 1; } clip_welcome
    ;

clip_welcome: CLIP_WELCOME GSTRING GINT64 GSTRING
            ;
            
%%
