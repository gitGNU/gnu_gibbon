/* #define foobarbaz */

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

static int yywrap ();

#include <glib.h>
#include <glib/gi18n.h>

#include "gibbon-java-fibs-parser.h"

%}

%option yylineno
%option prefix="gibbon_java_fibs_lexer_"

%option debug

%x OPCODE
%s WHO
%s WHAT

%%
  /* Ignore all whitespace.  */
[ \t]					;
  /* JavaFIBS seems to spit out gratuitous carriage returns.  */
\r					;
:					{ return COLON; }
<INITIAL>\n				;
JavaFIBS[ \t][ \t]*internal[ \t][ \t]*match[ \t][ \t]*representation[ \t][ \t]*v[0-9.][0-9.]* {
						BEGIN (OPCODE);
						return PROLOG;
					}
<OPCODE>\n
<WHAT>\n				BEGIN (OPCODE);
<OPCODE>13				{
						BEGIN (WHO); 
						return START_OF_GAME;
					}
<OPCODE>6				{
						BEGIN (WHO); 
						return OPPONENTS;
					}
<OPCODE>8				{
						BEGIN (WHO); 
						return START_OF_MATCH;
					}
<WHAT>[1-9][0-9]*			return INTEGER;
<WHO,WHAT>[^-: \t\r\n][^-: \t\r\n]*	{
						BEGIN (WHAT);
						return PLAYER;
					}
%%

static int
yywrap ()
{
	return 1;
}
