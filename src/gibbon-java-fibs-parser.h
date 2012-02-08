/* A Bison parser, made by GNU Bison 2.4.3.  */

/* Skeleton interface for Bison's Yacc-like parsers in C
   
      Copyright (C) 1984, 1989, 1990, 2000, 2001, 2002, 2003, 2004, 2005, 2006,
   2009, 2010 Free Software Foundation, Inc.
   
   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.
   
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.
   
   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>.  */

/* As a special exception, you may create a larger work that contains
   part or all of the Bison parser skeleton and distribute that work
   under terms of your choice, so long as that work isn't itself a
   parser generator using the skeleton or a modified version thereof
   as a parser skeleton.  Alternatively, if you modify or redistribute
   the parser skeleton itself, you may (at your option) remove this
   special exception, which will cause the skeleton and the resulting
   Bison output files to be licensed under the GNU General Public
   License without this special exception.
   
   This special exception was added by the Free Software Foundation in
   version 2.2 of Bison.  */


/* Tokens.  */
#ifndef YYTOKENTYPE
# define YYTOKENTYPE
   /* Put the tokens into the symbol table, so that GDB and other debuggers
      know about them.  */
   enum yytokentype {
     PROLOG = 258,
     COLON = 259,
     HYPHEN = 260,
     INTEGER = 261,
     PLAYER = 262,
     ROLL = 263,
     MOVE = 264,
     CUBE = 265,
     RESIGN = 266,
     TAKE = 267,
     DROP = 268,
     START_OF_GAME = 269,
     WIN_GAME = 270,
     START_OF_MATCH = 271,
     WIN_MATCH = 272,
     REJECT_RESIGN = 273,
     OPPONENTS = 274,
     SCORE = 275,
     BAR = 276,
     OFF = 277,
     JUNK = 278
   };
#endif
/* Tokens.  */
#define PROLOG 258
#define COLON 259
#define HYPHEN 260
#define INTEGER 261
#define PLAYER 262
#define ROLL 263
#define MOVE 264
#define CUBE 265
#define RESIGN 266
#define TAKE 267
#define DROP 268
#define START_OF_GAME 269
#define WIN_GAME 270
#define START_OF_MATCH 271
#define WIN_MATCH 272
#define REJECT_RESIGN 273
#define OPPONENTS 274
#define SCORE 275
#define BAR 276
#define OFF 277
#define JUNK 278




#if ! defined YYSTYPE && ! defined YYSTYPE_IS_DECLARED
typedef union YYSTYPE
{

/* Line 1685 of yacc.c  */
#line 92 "gibbon-java-fibs-parser.y"

	guint64 num;
	gchar *name;



/* Line 1685 of yacc.c  */
#line 104 "gibbon-java-fibs-parser.h"
} YYSTYPE;
# define YYSTYPE_IS_TRIVIAL 1
# define yystype YYSTYPE /* obsolescent; will be withdrawn */
# define YYSTYPE_IS_DECLARED 1
#endif

extern YYSTYPE yylval;


