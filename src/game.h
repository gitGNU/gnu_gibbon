/*
 * This file is part of Gibbon, a graphical frontend to the First Internet 
 * Backgammon Server FIBS.
 * Copyright (C) 2009 Guido Flohr, http://guido-flohr.net/.
 *
 * Gibbon is free software: you can redistribute it and/or modify 
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Gibbon is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Gibbon.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef _GIBBON_GAME_H
#define _GIBBON_GAME_H

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

/* Whenever a number of checkers has to be given, a negative number
 * is used for player 1's checkers, a positive for player 2's checkers.
 */
struct GibbonPosition {
        /* Number of checkers on the bar for player 1.  */
        gint bar0;
        gint bar1;
        gint checkers[24];
        gint bear_off0;
        gint bear_off1;
};

#endif
