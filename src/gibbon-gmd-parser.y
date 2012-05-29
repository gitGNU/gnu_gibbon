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

static guint gibbon_gmd_parser_encode_movement (guint64 from, guint64 to);
%}

%union {
	guint64 num;
	gdouble dbl;
	gchar *name;
}

%%

gmd_file
	: /* empty */
        ;
%%

static guint
gibbon_gmd_parser_encode_movement (guint64 from, guint64 to)
{
	return (from << 8 | to);
}
