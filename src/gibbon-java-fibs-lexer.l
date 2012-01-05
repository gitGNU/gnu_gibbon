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
  /* Not used? */
<OPCODE>10:[^\n]*			; 
<OPCODE>13				{
						BEGIN (WHO); 
						return START_OF_GAME;
					}
  /* This is only used for the opening roll.  But why distinguish? */
<OPCODE>11				{
						BEGIN (WHO); 
						return ROLL;
					}
  /* Dance.  We can treat that like a regular (empty) move. */
<OPCODE>15				{
						BEGIN (WHO); 
						return MOVE;
					}
<OPCODE>0				{
						BEGIN (WHO); 
						return ROLL;
					}
<OPCODE>6				{
						BEGIN (WHO); 
						return OPPONENTS;
					}
<OPCODE>8				{
						BEGIN (WHO); 
						return START_OF_MATCH;
					}
  /* Ignore all unknown opcodes.  */
<OPCODE>16:[^\n]*			;
<OPCODE>17:[^\n]*			;
<OPCODE>18:[^\n]*			;
<OPCODE>19:[^\n]*			;
<OPCODE>[2-9][0-9]:[^\n]*		;
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
