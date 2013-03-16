/*
 * This file is part of Gibbon, a graphical frontend to the First Internet 
 * Backgammon Server FIBS.
 * Copyright (C) 2009-2012 Guido Flohr, http://guido-flohr.net/.
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

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <glib-object.h>

#include "gibbon-clip-reader.h"
#include "gibbon-position.h"

struct token_pair {
        GType type;
        const gchar *value;
};

struct test_case {
        const gchar *line;
        struct token_pair values[];
};

static struct test_case test_clip01_0 = {
                "1 GibbonTestA 1306865048 gibbon.example.com",
                {
                                { G_TYPE_INT64, "1" },
                                { G_TYPE_STRING, "GibbonTestA" },
                                { G_TYPE_INT64, "1306865048" },
                                { G_TYPE_STRING, "gibbon.example.com" },
                                { G_TYPE_INVALID }
                }
};

static struct test_case test_clip01_1 = {
                "1 GibbonTestB 1306865049 127.128.129.130",
                {
                                { G_TYPE_INT64, "1" },
                                { G_TYPE_STRING, "GibbonTestB" },
                                { G_TYPE_INT64, "1306865049" },
                                { G_TYPE_STRING, "127.128.129.130" },
                                { G_TYPE_INVALID }
                }
};

static struct test_case test_clip02_0 = {
                "2 GibbonTestA 1 1 0 0 0 0 1 1 2396 0 1 0 1 3457.85 0 0 0 0 0"
                " Europe/Sofia",
                {
                                { G_TYPE_INT64, "2" },
                                { G_TYPE_STRING, "GibbonTestA" },
                                { G_TYPE_BOOLEAN, "TRUE" },
                                { G_TYPE_BOOLEAN, "TRUE" },
                                { G_TYPE_BOOLEAN, "FALSE" },
                                { G_TYPE_BOOLEAN, "FALSE" },
                                { G_TYPE_BOOLEAN, "FALSE" },
                                { G_TYPE_BOOLEAN, "FALSE" },
                                { G_TYPE_BOOLEAN, "TRUE" },
                                { G_TYPE_BOOLEAN, "TRUE" },
                                { G_TYPE_INT64, "2396" },
                                { G_TYPE_BOOLEAN, "FALSE" },
                                { G_TYPE_BOOLEAN, "TRUE" },
                                { G_TYPE_BOOLEAN, "FALSE" },
                                { G_TYPE_BOOLEAN, "TRUE" },
                                { G_TYPE_DOUBLE, "3457.850000" },
                                { G_TYPE_BOOLEAN, "FALSE" },
                                { G_TYPE_BOOLEAN, "FALSE" },
                                { G_TYPE_INT64, "0" },
                                { G_TYPE_BOOLEAN, "FALSE" },
                                { G_TYPE_BOOLEAN, "FALSE" },
                                { G_TYPE_STRING, "Europe/Sofia" },
                                { G_TYPE_INVALID }
                }
};

static struct test_case test_clip02_1 = {
                "2 GibbonTestB 1 1 0 0 0 0 1 1 2396 0 1 0 1 3457.85 0 0 2 0 0"
                " Europe/Sofia",
                {
                                { G_TYPE_INT64, "2" },
                                { G_TYPE_STRING, "GibbonTestB" },
                                { G_TYPE_BOOLEAN, "TRUE" },
                                { G_TYPE_BOOLEAN, "TRUE" },
                                { G_TYPE_BOOLEAN, "FALSE" },
                                { G_TYPE_BOOLEAN, "FALSE" },
                                { G_TYPE_BOOLEAN, "FALSE" },
                                { G_TYPE_BOOLEAN, "FALSE" },
                                { G_TYPE_BOOLEAN, "TRUE" },
                                { G_TYPE_BOOLEAN, "TRUE" },
                                { G_TYPE_INT64, "2396" },
                                { G_TYPE_BOOLEAN, "FALSE" },
                                { G_TYPE_BOOLEAN, "TRUE" },
                                { G_TYPE_BOOLEAN, "FALSE" },
                                { G_TYPE_BOOLEAN, "TRUE" },
                                { G_TYPE_DOUBLE, "3457.850000" },
                                { G_TYPE_BOOLEAN, "FALSE" },
                                { G_TYPE_BOOLEAN, "FALSE" },
                                { G_TYPE_INT64, "2" },
                                { G_TYPE_BOOLEAN, "FALSE" },
                                { G_TYPE_BOOLEAN, "FALSE" },
                                { G_TYPE_STRING, "Europe/Sofia" },
                                { G_TYPE_INVALID }
                }
};

static struct test_case test_clip02_2 = {
                "2 GibbonTestC 1 1 0 0 0 0 1 1 2396 0 1 0 1 3457.85 0 0"
                " unlimited 0 0 Europe/Sofia",
                {
                                { G_TYPE_INT64, "2" },
                                { G_TYPE_STRING, "GibbonTestC" },
                                { G_TYPE_BOOLEAN, "TRUE" },
                                { G_TYPE_BOOLEAN, "TRUE" },
                                { G_TYPE_BOOLEAN, "FALSE" },
                                { G_TYPE_BOOLEAN, "FALSE" },
                                { G_TYPE_BOOLEAN, "FALSE" },
                                { G_TYPE_BOOLEAN, "FALSE" },
                                { G_TYPE_BOOLEAN, "TRUE" },
                                { G_TYPE_BOOLEAN, "TRUE" },
                                { G_TYPE_INT64, "2396" },
                                { G_TYPE_BOOLEAN, "FALSE" },
                                { G_TYPE_BOOLEAN, "TRUE" },
                                { G_TYPE_BOOLEAN, "FALSE" },
                                { G_TYPE_BOOLEAN, "TRUE" },
                                { G_TYPE_DOUBLE, "3457.850000" },
                                { G_TYPE_BOOLEAN, "FALSE" },
                                { G_TYPE_BOOLEAN, "FALSE" },
                                { G_TYPE_INT64, "-1" },
                                { G_TYPE_BOOLEAN, "FALSE" },
                                { G_TYPE_BOOLEAN, "FALSE" },
                                { G_TYPE_STRING, "Europe/Sofia" },
                                { G_TYPE_INVALID }
                }
};

static struct test_case test_clip03 = {
                "3",
                {
                                { G_TYPE_INT64, "3" },
                                { G_TYPE_INVALID }
                }
};

static struct test_case test_clip03a = {
                "+------------------------------+",
                {
                                { G_TYPE_INT64, "413" },
                                { G_TYPE_STRING,
                                  "+------------------------------+" },
                                { G_TYPE_INVALID }
                }
};

static struct test_case test_clip03b = {
                "| This is the motto of the day. |",
                {
                                { G_TYPE_INT64, "413" },
                                { G_TYPE_STRING,
                                  "| This is the motto of the day. |"
                                },
                                { G_TYPE_INVALID }
                }
};

static struct test_case test_clip04 = {
                "4",
                {
                                { G_TYPE_INT64, "4" },
                                { G_TYPE_INVALID }
                }
};


static struct test_case test_clip05_0 = {
                "5 GibbonTestA barrack - 0 0 1418.61 1914 23 1306926526"
                " 173.223.48.110 Gibbon_0.1.1 president@whitehouse.gov",
                {
                                { G_TYPE_INT64, "5" },
                                { G_TYPE_STRING, "GibbonTestA" },
                                { G_TYPE_STRING, "barrack" },
                                { G_TYPE_STRING, "" },
                                { G_TYPE_BOOLEAN, "FALSE" },
                                { G_TYPE_BOOLEAN, "FALSE" },
                                { G_TYPE_DOUBLE, "1418.610000" },
                                { G_TYPE_INT64, "1914" },
                                { G_TYPE_INT64, "23" },
                                { G_TYPE_INT64, "1306926526" },
                                { G_TYPE_STRING, "173.223.48.110" },
                                { G_TYPE_STRING, "Gibbon_0.1.1" },
                                { G_TYPE_STRING,
                                                "president@whitehouse.gov" },
                                { G_TYPE_INVALID }
                }
};

static struct test_case test_clip05_1 = {
                "5 GibbonTestB - - 0 0 -35.67 914 23 1306926526"
                " 44.55.66.77 Gibbon_0.1.1 foo@bar.baz",
                {
                                { G_TYPE_INT64, "5" },
                                { G_TYPE_STRING, "GibbonTestB" },
                                { G_TYPE_STRING, "" },
                                { G_TYPE_STRING, "" },
                                { G_TYPE_BOOLEAN, "FALSE" },
                                { G_TYPE_BOOLEAN, "FALSE" },
                                { G_TYPE_DOUBLE, "-35.670000" },
                                { G_TYPE_INT64, "914" },
                                { G_TYPE_INT64, "23" },
                                { G_TYPE_INT64, "1306926526" },
                                { G_TYPE_STRING, "44.55.66.77" },
                                { G_TYPE_STRING, "Gibbon_0.1.1" },
                                { G_TYPE_STRING,
                                                "foo@bar.baz" },
                                { G_TYPE_INVALID }
                }
};

static struct test_case test_clip05_2 = {
                "5 GibbonTestC barrack - 0 0 1418.61 1914 23 1306926526"
                " 173.223.48.110* Gibbon_0.1.1 president@whitehouse.gov",
                {
                                { G_TYPE_INT64, "5" },
                                { G_TYPE_STRING, "GibbonTestC" },
                                { G_TYPE_STRING, "barrack" },
                                { G_TYPE_STRING, "" },
                                { G_TYPE_BOOLEAN, "FALSE" },
                                { G_TYPE_BOOLEAN, "FALSE" },
                                { G_TYPE_DOUBLE, "1418.610000" },
                                { G_TYPE_INT64, "1914" },
                                { G_TYPE_INT64, "23" },
                                { G_TYPE_INT64, "1306926526" },
                                { G_TYPE_STRING, "173.223.48.110" },
                                { G_TYPE_STRING, "Gibbon_0.1.1" },
                                { G_TYPE_STRING,
                                                "president@whitehouse.gov" },
                                { G_TYPE_INVALID }
                }
};


static struct test_case test_clip06 = {
                "6",
                {
                                { G_TYPE_INT64, "6" },
                                { G_TYPE_INVALID }
                }
};

static struct test_case test_clip07 = {
                "7 gflohr gflohr logs in.",
                {
                                { G_TYPE_INT64, "7" },
                                { G_TYPE_STRING, "gflohr" },
                                { G_TYPE_STRING, "gflohr logs in." },
                                { G_TYPE_INVALID }
                }
};

static struct test_case test_clip08 = {
                "8 gflohr gflohr drops connection.",
                {
                                { G_TYPE_INT64, "8" },
                                { G_TYPE_STRING, "gflohr" },
                                { G_TYPE_STRING, "gflohr drops connection." },
                                { G_TYPE_INVALID }
                }
};

static struct test_case test_clip09 = {
                "9 gflohr -1306935184    Be back at 20 p.m.",
                {
                                { G_TYPE_INT64, "9" },
                                { G_TYPE_STRING, "gflohr" },
                                { G_TYPE_INT64, "-1306935184" },
                                { G_TYPE_STRING, "   Be back at 20 p.m." },
                                { G_TYPE_INVALID }
                }
};

static struct test_case test_clip10 = {
                "10 gflohr",
                {
                                { G_TYPE_INT64, "10" },
                                { G_TYPE_STRING, "gflohr" },
                                { G_TYPE_INVALID }
                }
};

static struct test_case test_clip11 = {
                "11 gflohr",
                {
                                { G_TYPE_INT64, "11" },
                                { G_TYPE_STRING, "gflohr" },
                                { G_TYPE_INVALID }
                }
};

static struct test_case test_clip12 = {
                "12 gflohr    Hello world.",
                {
                                { G_TYPE_INT64, "12" },
                                { G_TYPE_STRING, "gflohr" },
                                { G_TYPE_STRING, "   Hello world." },
                                { G_TYPE_INVALID }
                }
};

static struct test_case test_clip13 = {
                "13 gflohr    Hello world.",
                {
                                { G_TYPE_INT64, "13" },
                                { G_TYPE_STRING, "gflohr" },
                                { G_TYPE_STRING, "   Hello world." },
                                { G_TYPE_INVALID }
                }
};

static struct test_case test_clip14 = {
                "14 gflohr    Hello world.",
                {
                                { G_TYPE_INT64, "14" },
                                { G_TYPE_STRING, "gflohr" },
                                { G_TYPE_STRING, "   Hello world." },
                                { G_TYPE_INVALID }
                }
};

static struct test_case test_clip15 = {
                "15 gflohr    Hello world.",
                {
                                { G_TYPE_INT64, "15" },
                                { G_TYPE_STRING, "gflohr" },
                                { G_TYPE_STRING, "   Hello world." },
                                { G_TYPE_INVALID }
                }
};

static struct test_case test_clip16 = {
                "16 gflohr    Hello world.",
                {
                                { G_TYPE_INT64, "16" },
                                { G_TYPE_STRING, "gflohr" },
                                { G_TYPE_STRING, "   Hello world." },
                                { G_TYPE_INVALID }
                }
};

static struct test_case test_clip17 = {
                "17    Hello world.",
                {
                                { G_TYPE_INT64, "17" },
                                { G_TYPE_STRING, "   Hello world." },
                                { G_TYPE_INVALID }
                }
};

static struct test_case test_clip18 = {
                "18    Hello world.",
                {
                                { G_TYPE_INT64, "18" },
                                { G_TYPE_STRING, "   Hello world." },
                                { G_TYPE_INVALID }
                }
};

static struct test_case test_clip19 = {
                "19    Hello world.",
                {
                                { G_TYPE_INT64, "19" },
                                { G_TYPE_STRING, "   Hello world." },
                                { G_TYPE_INVALID }
                }
};

static struct test_case test_clip20 = {
                "20 gflohr    Mad world!",
                {
                                { G_TYPE_INT64, "20" },
                                { G_TYPE_STRING, "gflohr" },
                                { G_TYPE_STRING, "   Mad world!" },
                                { G_TYPE_INVALID }
                }
};

static struct test_case test_error00 = {
                "** Funny new message!",
                {
                                { G_TYPE_INT64, "100" },
                                { G_TYPE_INT64, "0" },
                                { G_TYPE_STRING,
                                "Funny new message!"},
                                { G_TYPE_INVALID }
                },
};

static struct test_case test_error01 = {
                "** Error: something new went wrong!",
                {
                                { G_TYPE_INT64, "100" },
                                { G_TYPE_INT64, "0" },
                                { G_TYPE_STRING,
                                "Error: something new went wrong!"},
                                { G_TYPE_INVALID }
                },
};

static struct test_case test_error02 = {
                "** You see a funny new message!",
                {
                                { G_TYPE_INT64, "100" },
                                { G_TYPE_INT64, "0" },
                                { G_TYPE_STRING,
                                "You see a funny new message!"},
                                { G_TYPE_INVALID }
                },
};

static struct test_case test_error03 = {
                "** There is no one called anonymous.",
                {
                                { G_TYPE_INT64, "414" },
                                { G_TYPE_STRING, "anonymous" },
                                { G_TYPE_INVALID }
                },
};

static struct test_case test_board00 =  {
                "board:joe_white:black_jack:7:5:0:0:0:2:-1:0:-1:4:0:2:0:0:0:-2"
                ":4:0:0:0:-3:-2:-4:3:-2:0:0:0:0:-1:0:0:6:6:1:1:1:0:1:-1:0:25:0"
                ":0:0:0:2:6:0:0",
                {
                                { G_TYPE_INT64, "200" },
                                { G_TYPE_STRING, "\
=== Position ===\n\
Opponent: black_jack, 0/7 points, 138 pips\n\
  +-13-14-15-16-17-18-------19-20-21-22-23-24-+ negative: black or X\n\
  | +4          -3 -2 | +0| -4 +3 -2          | May double: yes\n\
 v| dice: -6 : -6     |BAR|                   |  Cube: 1\n\
  | -2          +2    | +0| +4 -1    -1 +2    | May double: yes\n\
  +-12-11-10--9--8--7--------6--5--4--3--2--1-+ positive: white or O\n\
Player: joe_white, 5/7 points, 156 pips\n\
Game info: (null)\n\
Status: (null)\n\
Turn: -1, cube turned: 0, resigned: 0, score: 0\n"},
                                { G_TYPE_BOOLEAN, "TRUE" },
                                { G_TYPE_INVALID }
                }
};

static struct test_case test_board01 =  {
                "board:joe_white:black_jack:1:0:0:0:6:2:0:2:0:0:0:0:0:-1:0:0"
                ":-2:0:0:0:0:0:0:-3:-1:-3:-1:-4:0:1:0:0:6:2:1:1:1:0:-1:1:25:0"
                ":0:5:0:0:2:4:0:0",
                {
                                { G_TYPE_INT64, "200" },
                                { G_TYPE_STRING, "\
=== Position ===\n\
Opponent: black_jack, 0/1 points, 18 pips\n\
  +-13-14-15-16-17-18-------19-20-21-22-23-24-+ negative: black or X\n\
  |       +1          | +0|       -2    -2 -6 | May double: yes\n\
 v| dice: -6 : -2     |BAR|                   |  Cube: 1\n\
  | +2                | +0|    +3 +1 +3 +1 +4 | May double: yes\n\
  +-12-11-10--9--8--7--------6--5--4--3--2--1-+ positive: white or O\n\
Player: joe_white, 0/1 points, 73 pips\n\
Game info: (null)\n\
Status: (null)\n\
Turn: -1, cube turned: 0, resigned: 0, score: 0\n"},
                                { G_TYPE_BOOLEAN, "FALSE" },
                                { G_TYPE_INVALID }
                }
};

static struct test_case test_board02 =  {
                "board:joe_white:black_jack:2:1:1:0:0:0:-1:3:3:2:0:3:0:0:2:0:2"
                ":0:0:0:-1:-1:-2:-2:-2:-4:-2:0:0:1:5:1:0:0:2:0:1:0:1:-1:0:25:0"
                ":0:0:0:2:0:1:0",
                {
                                { G_TYPE_INT64, "200" },
                                { G_TYPE_STRING, "\
=== Position ===\n\
Opponent: black_jack, 1/2 points, 83 pips\n\
  +-13-14-15-16-17-18-------19-20-21-22-23-24-+ negative: black or X\n\
  | +2          -1 -1 | +0| -2 -2 -2 -4 -2    | May double: yes\n\
 v| dice: +5 : +1     |BAR|                   |  Cube: 2\n\
  |    +2       +3    | +0| +2 +3 +3 -1       | May double: no\n\
  +-12-11-10--9--8--7--------6--5--4--3--2--1-+ positive: white or O\n\
Player: joe_white, 1/2 points, 111 pips\n\
Game info: Post-Crawford game\n\
Status: (null)\n\
Turn: 1, cube turned: 0, resigned: 0, score: 0\n"},
                               { G_TYPE_BOOLEAN, "TRUE" },
                               { G_TYPE_INVALID }
                }
};

/*
 * Test case for the Crawford game.
 */
static struct test_case test_board03 =  {
                "board:You:GibbonTestA:2:0:1:0:0:0:-1:0:-1:5:0:3:0:0:0:-5:5:0:"
                "0:0:-3:0:-5:1:0:0:0:1:0:1:0:0:0:0:1:1:1:0:-1:1:25:0:0:0:0:0:2:"
                "0:0:0",
                {
                                { G_TYPE_INT64, "200" },
                                { G_TYPE_STRING, "\
=== Position ===\n\
Opponent: GibbonTestA, 1/2 points, 163 pips\n\
  +-13-14-15-16-17-18-------19-20-21-22-23-24-+ negative: black or X\n\
  | +5          -3    | +0| -5 +1    +1       | May double: no\n\
 v| dice: +0 : +0     |BAR|                   |  Cube: 1\n\
  | -5          +3    | +0| +5 -1          -1 | May double: no\n\
  +-12-11-10--9--8--7--------6--5--4--3--2--1-+ positive: white or O\n\
Player: You, 0/2 points, 161 pips\n\
Game info: Crawford game\n\
Status: (null)\n\
Turn: -1, cube turned: 0, resigned: 0, score: 0\n"},
                               { G_TYPE_BOOLEAN, "FALSE" },
                               { G_TYPE_INVALID }
                }
};

static struct test_case test_rolls00 = {
                "gflohr rolls 3 and 1.",
                {
                                { G_TYPE_INT64, "202" },
                                { G_TYPE_STRING, "gflohr" },
                                { G_TYPE_INT64, "3" },
                                { G_TYPE_INT64, "1" },
                                { G_TYPE_INVALID }
                }
};

static struct test_case test_rolls01 = {
                "You roll 6 and 4.",
                {
                                { G_TYPE_INT64, "202" },
                                { G_TYPE_STRING, "You" },
                                { G_TYPE_INT64, "6" },
                                { G_TYPE_INT64, "4" },
                                { G_TYPE_INVALID }
                }
};

static struct test_case test_moves00 = {
                "gflohr moves 8-5 6-5 .",
                {
                                { G_TYPE_INT64, "203" },
                                { G_TYPE_STRING, "gflohr" },
                                { G_TYPE_INT64, "8" },
                                { G_TYPE_INT64, "5" },
                                { G_TYPE_INT64, "6" },
                                { G_TYPE_INT64, "5" },
                                { G_TYPE_INVALID }
                }
};

static struct test_case test_moves01 = {
                "gflohr moves 17-20 19-20 .",
                {
                                { G_TYPE_INT64, "203" },
                                { G_TYPE_STRING, "gflohr" },
                                { G_TYPE_INT64, "17" },
                                { G_TYPE_INT64, "20" },
                                { G_TYPE_INT64, "19" },
                                { G_TYPE_INT64, "20" },
                                { G_TYPE_INVALID }
                }
};

static struct test_case test_moves02 = {
                "gflohr moves 17-20 17-20 19-20 19-20 .",
                {
                                { G_TYPE_INT64, "203" },
                                { G_TYPE_STRING, "gflohr" },
                                { G_TYPE_INT64, "17" },
                                { G_TYPE_INT64, "20" },
                                { G_TYPE_INT64, "17" },
                                { G_TYPE_INT64, "20" },
                                { G_TYPE_INT64, "19" },
                                { G_TYPE_INT64, "20" },
                                { G_TYPE_INT64, "19" },
                                { G_TYPE_INT64, "20" },
                                { G_TYPE_INVALID }
                }
};

static struct test_case test_moves03 = {
                "gflohr moves bar-24 bar-22 .",
                {
                                { G_TYPE_INT64, "203" },
                                { G_TYPE_STRING, "gflohr" },
                                { G_TYPE_INT64, "25" },
                                { G_TYPE_INT64, "24" },
                                { G_TYPE_INT64, "25" },
                                { G_TYPE_INT64, "22" },
                                { G_TYPE_INVALID }
                }
};

static struct test_case test_moves04 = {
                "gflohr moves bar-1 bar-3 .",
                {
                                { G_TYPE_INT64, "203" },
                                { G_TYPE_STRING, "gflohr" },
                                { G_TYPE_INT64, "0" },
                                { G_TYPE_INT64, "1" },
                                { G_TYPE_INT64, "0" },
                                { G_TYPE_INT64, "3" },
                                { G_TYPE_INVALID }
                }
};

static struct test_case test_moves05 = {
                "gflohr moves 24-off 22-off .",
                {
                                { G_TYPE_INT64, "203" },
                                { G_TYPE_STRING, "gflohr" },
                                { G_TYPE_INT64, "24" },
                                { G_TYPE_INT64, "25" },
                                { G_TYPE_INT64, "22" },
                                { G_TYPE_INT64, "25" },
                                { G_TYPE_INVALID }
                }
};

static struct test_case test_moves06 = {
                "gflohr moves 1-off 3-off .",
                {
                                { G_TYPE_INT64, "203" },
                                { G_TYPE_STRING, "gflohr" },
                                { G_TYPE_INT64, "1" },
                                { G_TYPE_INT64, "0" },
                                { G_TYPE_INT64, "3" },
                                { G_TYPE_INT64, "0" },
                                { G_TYPE_INVALID }
                }
};

static struct test_case test_moves07 = {
                "GibbonTestB moves 2-off",
                {
                                { G_TYPE_INT64, "203" },
                                { G_TYPE_STRING, "GibbonTestB" },
                                { G_TYPE_INT64, "2" },
                                { G_TYPE_INT64, "0" },
                                { G_TYPE_INVALID }
                }
};

static struct test_case test_game_start00 = {
                "Starting a new game with gflohr.",
                {
                                { G_TYPE_INT64, "204" },
                                { G_TYPE_STRING, "gflohr" },
                                { G_TYPE_INVALID }
                }
};

static struct test_case test_game_left00 = {
                "** You terminated the game. The game was saved.",
                {
                                { G_TYPE_INT64, "205" },
                                { G_TYPE_STRING, "You" },
                                { G_TYPE_INVALID }
                }
};

static struct test_case test_game_left01 = {
                "** Player GibbonTestA terminated the game."
                " The game was saved.",
                {
                                { G_TYPE_INT64, "205" },
                                { G_TYPE_STRING, "GibbonTestA" },
                                { G_TYPE_INVALID }
                }
};

static struct test_case test_game_left02 = {
                "GibbonTestA logs out. The game was saved.",
                {
                                { G_TYPE_INT64, "211" },
                                { G_TYPE_STRING, "GibbonTestA" },
                                { G_TYPE_INVALID }
                }
};

static struct test_case test_game_left03 = {
                "GibbonTestA drops connection. The game was saved.",
                {
                                { G_TYPE_INT64, "211" },
                                { G_TYPE_STRING, "GibbonTestA" },
                                { G_TYPE_INVALID }
                }
};

static struct test_case test_game_left04 = {
                "Connection with GibbonTestA timed out. The game was saved.",
                {
                                { G_TYPE_INT64, "211" },
                                { G_TYPE_STRING, "GibbonTestA" },
                                { G_TYPE_INVALID }
                }
};

static struct test_case test_cannot_move00 = {
                "You can't move.",
                {
                                { G_TYPE_INT64, "206" },
                                { G_TYPE_STRING, "You" },
                                { G_TYPE_INVALID }
                }
};

static struct test_case test_cannot_move01 = {
                "GibbonTestA can't move.",
                {
                                { G_TYPE_INT64, "206" },
                                { G_TYPE_STRING, "GibbonTestA" },
                                { G_TYPE_INVALID }
                }
};

static struct test_case test_doubling00 = {
                "You double. Please wait for GibbonTestA to accept or reject.",
                {
                                { G_TYPE_INT64, "207" },
                                { G_TYPE_STRING, "You" },
                                { G_TYPE_INVALID }
                }
};

static struct test_case test_doubling01 = {
                "GibbonTestA doubles. Type 'accept' or 'reject'.",
                {
                                { G_TYPE_INT64, "207" },
                                { G_TYPE_STRING, "GibbonTestA" },
                                { G_TYPE_INVALID }
                }
};

static struct test_case test_doubling02 = {
                "You accept the double.  The cube shows 2.",
                {
                                { G_TYPE_INT64, "208" },
                                { G_TYPE_STRING, "You" },
                                { G_TYPE_INT64, "2" },
                                { G_TYPE_INVALID }
                }
};

static struct test_case test_doubling03 = {
                "GibbonTestA accepts the double.  The cube shows 4.",
                {
                                { G_TYPE_INT64, "208" },
                                { G_TYPE_STRING, "GibbonTestA" },
                                { G_TYPE_INT64, "4" },
                                { G_TYPE_INVALID }
                }
};

static struct test_case test_resign00 = {
                "GibbonTestA wants to resign. You will win 1 point."
                " Type 'accept' or 'reject'.",
                {
                                { G_TYPE_INT64, "209" },
                                { G_TYPE_STRING, "GibbonTestA" },
                                { G_TYPE_INT64, "1" },
                                { G_TYPE_INVALID }
                }
};

static struct test_case test_resign01 = {
                "GibbonTestA wants to resign. You will win 6 points."
                " Type 'accept' or 'reject'.",
                {
                                { G_TYPE_INT64, "209" },
                                { G_TYPE_STRING, "GibbonTestA" },
                                { G_TYPE_INT64, "6" },
                                { G_TYPE_INVALID }
                }
};

static struct test_case test_resign02 = {
                "You want to resign. GibbonTestA will win 1 point.",
                {
                                { G_TYPE_INT64, "209" },
                                { G_TYPE_STRING, "You" },
                                { G_TYPE_INT64, "1" },
                                { G_TYPE_INVALID }
                }
};

static struct test_case test_resign03 = {
                "You want to resign. GibbonTestA will win 4 points.",
                {
                                { G_TYPE_INT64, "209" },
                                { G_TYPE_STRING, "You" },
                                { G_TYPE_INT64, "4" },
                                { G_TYPE_INVALID }
                }
};

static struct test_case test_resign04 = {
                "You reject. The game continues.",
                {
                                { G_TYPE_INT64, "210" },
                                { G_TYPE_INVALID }
                }
};

static struct test_case test_resign05 = {
                "GibbonTestA rejects. The game continues.",
                {
                                { G_TYPE_INT64, "210" },
                                { G_TYPE_INVALID }
                }
};

static struct test_case test_between00 = {
                "GibbonTestA wants to play a 5 point match with you.",
                {
                                { G_TYPE_INT64, "300" },
                                { G_TYPE_STRING, "GibbonTestA" },
                                { G_TYPE_INT64, "5" },
                                { G_TYPE_INVALID }
                }
};

static struct test_case test_between01 = {
                "GibbonTestA wants to play an unlimited match with you.",
                {
                                { G_TYPE_INT64, "300" },
                                { G_TYPE_STRING, "GibbonTestA" },
                                { G_TYPE_INT64, "0" },
                                { G_TYPE_INVALID }
                }
};

static struct test_case test_between02 = {
                "GibbonTestA wants to resume a saved match with you.",
                {
                                { G_TYPE_INT64, "300" },
                                { G_TYPE_STRING, "GibbonTestA" },
                                { G_TYPE_INT64, "-1" },
                                { G_TYPE_INVALID }
                }
};

static struct test_case test_between03 = {
                "Type 'join GibbonTestA' to accept.",
                {
                                { G_TYPE_INT64, "301" },
                                { G_TYPE_STRING, "GibbonTestA" },
                                { G_TYPE_INVALID }
                }
};

static struct test_case test_between04 = {
                "You're now watching gflohr.",
                {
                                { G_TYPE_INT64, "302" },
                                { G_TYPE_STRING, "gflohr" },
                                { G_TYPE_INVALID }
                }
};

static struct test_case test_between05 = {
                "** You are now playing a 5 point match with gflohr",
                {
                                { G_TYPE_INT64, "303" },
                                { G_TYPE_INT64, "5" },
                                { G_TYPE_STRING, "gflohr" },
                                { G_TYPE_INVALID }
                }
};

static struct test_case test_between06 = {
                "** You are now playing an unlimited match with gflohr",
                {
                                { G_TYPE_INT64, "303" },
                                { G_TYPE_INT64, "0" },
                                { G_TYPE_STRING, "gflohr" },
                                { G_TYPE_INVALID }
                }
};

static struct test_case test_between07 = {
                "** gflohr is refusing games.",
                {
                                { G_TYPE_INT64, "304" },
                                { G_TYPE_STRING, "gflohr" },
                                { G_TYPE_STRING,
                                  "Player `gflohr' is now refusing to play!"},
                                { G_TYPE_INVALID }
                }
};

static struct test_case test_between08 = {
                "** Error: gflohr is already playing with someone else.",
                {
                                { G_TYPE_INT64, "304" },
                                { G_TYPE_STRING, "gflohr" },
                                { G_TYPE_STRING,
                                  "Player `gflohr' is already playing with"
                                  " someone else!"},
                                { G_TYPE_INVALID }
                }
};

static struct test_case test_between09 = {
                "** gflohr didn't invite you.",
                {
                                { G_TYPE_INT64, "304" },
                                { G_TYPE_STRING, "gflohr" },
                                { G_TYPE_STRING,
                                  "Player `gflohr' is already playing with"
                                  " someone else!"},
                                { G_TYPE_INVALID }
                }
};

static struct test_case test_between10 = {
                "** Error: can't find player gflohr",
                {
                                { G_TYPE_INT64, "304" },
                                { G_TYPE_STRING, "gflohr" },
                                { G_TYPE_STRING,
                                  "Player `gflohr' has logged out!" },
                                { G_TYPE_INVALID }
                }
};

static struct test_case test_between11 = {
                "** You can't play two games at once",
                {
                                { G_TYPE_INT64, "100" },
                                { G_TYPE_INT64, "3" },
                                { G_TYPE_STRING,
                                  "You cannot play two games at once!"},
                                { G_TYPE_INVALID }
                }
};

static struct test_case test_between12 = {
                "** Player gflohr has joined you for a 5 point match.",
                {
                                { G_TYPE_INT64, "303" },
                                { G_TYPE_STRING, "gflohr" },
                                { G_TYPE_INT64, "5" },
                                { G_TYPE_INVALID }
                }
};

static struct test_case test_between13 = {
                "Player gflohr has joined you for an unlimited match.",
                {
                                { G_TYPE_INT64, "303" },
                                { G_TYPE_STRING, "gflohr" },
                                { G_TYPE_INT64, "0" },
                                { G_TYPE_INVALID }
                }
};

static struct test_case test_resume00 = {
                "You are now playing with gflohr."
                " Your running match was loaded.",
                {
                                { G_TYPE_INT64, "305" },
                                { G_TYPE_STRING, "gflohr" },
                                { G_TYPE_INVALID }
                }
};

static struct test_case test_resume01 = {
                "gflohr has joined you."
                " Your running match was loaded.",
                {
                                { G_TYPE_INT64, "305" },
                                { G_TYPE_STRING, "gflohr" },
                                { G_TYPE_INVALID }
                }
};

static struct test_case test_resume02 = {
                "turn: gflohr",
                {
                                { G_TYPE_INT64, "306" },
                                { G_TYPE_STRING, "gflohr" },
                                { G_TYPE_INVALID }
                }
};

static struct test_case test_resume03 = {
                "match length: 42",
                {
                                { G_TYPE_INT64, "307" },
                                { G_TYPE_INT64, "42" },
                                { G_TYPE_INVALID }
                }
};

static struct test_case test_resume04 = {
                "unlimited match",
                {
                                { G_TYPE_INT64, "307" },
                                { G_TYPE_INT64, "0" },
                                { G_TYPE_INVALID }
                }
};

static struct test_case test_resume05 = {
                "points for user gflohr: 3",
                {
                                { G_TYPE_INT64, "308" },
                                { G_TYPE_STRING, "gflohr" },
                                { G_TYPE_INT64, "3" },
                                { G_TYPE_INVALID }
                }
};

static struct test_case test_between14 = {
                "Type 'join' if you want to play the next game,"
                " type 'leave' if you don't.",
                {
                                { G_TYPE_INT64, "309" },
                                { G_TYPE_INVALID }
                }
};

static struct test_case test_between15 = {
                "You win the game and get 1 point. Congratulations!",
                {
                                { G_TYPE_INT64, "310" },
                                { G_TYPE_STRING, "You" },
                                { G_TYPE_INT64, "1" },
                                { G_TYPE_INVALID }
                }
};

static struct test_case test_between16 = {
                "You win the game and get 4 points. Congratulations!",
                {
                                { G_TYPE_INT64, "310" },
                                { G_TYPE_STRING, "You" },
                                { G_TYPE_INT64, "4" },
                                { G_TYPE_INVALID }
                }
};

static struct test_case test_between17 = {
                "GibbonTestA wins the game and gets 1 point. Sorry.",
                {
                                { G_TYPE_INT64, "310" },
                                { G_TYPE_STRING, "GibbonTestA" },
                                { G_TYPE_INT64, "1" },
                                { G_TYPE_INVALID }
                }
};

static struct test_case test_between18 = {
                "GibbonTestA wins the game and gets 4 points. Sorry.",
                {
                                { G_TYPE_INT64, "310" },
                                { G_TYPE_STRING, "GibbonTestA" },
                                { G_TYPE_INT64, "4" },
                                { G_TYPE_INVALID }
                }
};

static struct test_case test_between19 = {
                "GibbonTestA gives up. You win 1 point.",
                {
                                { G_TYPE_INT64, "310" },
                                { G_TYPE_STRING, "You" },
                                { G_TYPE_INT64, "1" },
                                { G_TYPE_INVALID }
                }
};

static struct test_case test_between20 = {
                "GibbonTestA gives up. You win 2 points.",
                {
                                { G_TYPE_INT64, "310" },
                                { G_TYPE_STRING, "You" },
                                { G_TYPE_INT64, "2" },
                                { G_TYPE_INVALID }
                }
};

static struct test_case test_between21 = {
                "You give up. GibbonTestA wins 1 point.",
                {
                                { G_TYPE_INT64, "310" },
                                { G_TYPE_STRING, "GibbonTestA" },
                                { G_TYPE_INT64, "1" },
                                { G_TYPE_INVALID }
                }
};

static struct test_case test_between22 = {
                "You give up. GibbonTestA wins 2 points.",
                {
                                { G_TYPE_INT64, "310" },
                                { G_TYPE_STRING, "GibbonTestA" },
                                { G_TYPE_INT64, "2" },
                                { G_TYPE_INVALID }
                }
};

static struct test_case test_between23 = {
                "You accept and win 1 point.",
                {
                                { G_TYPE_INT64, "310" },
                                { G_TYPE_STRING, "You" },
                                { G_TYPE_INT64, "1" },
                                { G_TYPE_INVALID }
                }
};

static struct test_case test_between24 = {
                "You accept and win 2 points.",
                {
                                { G_TYPE_INT64, "310" },
                                { G_TYPE_STRING, "You" },
                                { G_TYPE_INT64, "2" },
                                { G_TYPE_INVALID }
                }
};

static struct test_case test_between25 = {
                "GibbonTestA accepts and wins 1 point.",
                {
                                { G_TYPE_INT64, "310" },
                                { G_TYPE_STRING, "GibbonTestA" },
                                { G_TYPE_INT64, "1" },
                                { G_TYPE_INVALID }
                }
};

static struct test_case test_between26 = {
                "GibbonTestA accepts and wins 2 points.",
                {
                                { G_TYPE_INT64, "310" },
                                { G_TYPE_STRING, "GibbonTestA" },
                                { G_TYPE_INT64, "2" },
                                { G_TYPE_INVALID }
                }
};

static struct test_case test_between27 = {
                "GibbonTestA wins the game and gets 1 point.",
                {
                                { G_TYPE_INT64, "310" },
                                { G_TYPE_STRING, "GibbonTestA" },
                                { G_TYPE_INT64, "1" },
                                { G_TYPE_INVALID }
                }
};

static struct test_case test_between28 = {
                "GibbonTestA wins the game and gets 4 points.",
                {
                                { G_TYPE_INT64, "310" },
                                { G_TYPE_STRING, "GibbonTestA" },
                                { G_TYPE_INT64, "4" },
                                { G_TYPE_INVALID }
                }
};

static struct test_case test_between29 = {
                "GibbonTestB gives up. GibbonTestA wins 1 point.",
                {
                                { G_TYPE_INT64, "310" },
                                { G_TYPE_STRING, "GibbonTestA" },
                                { G_TYPE_INT64, "1" },
                                { G_TYPE_INVALID }
                }
};

static struct test_case test_between30 = {
                "GibbonTestB gives up. GibbonTestA win 2 points.",
                {
                                { G_TYPE_INT64, "310" },
                                { G_TYPE_STRING, "GibbonTestA" },
                                { G_TYPE_INT64, "2" },
                                { G_TYPE_INVALID }
                }
};

static struct test_case test_between31 = {
                "score in 7 point match: GibbonTestA-3 GibbonTestB-5",
                {
                                { G_TYPE_INT64, "311" },
                                { G_TYPE_INT64, "7" },
                                { G_TYPE_STRING, "GibbonTestA" },
                                { G_TYPE_INT64, "3" },
                                { G_TYPE_STRING, "GibbonTestB" },
                                { G_TYPE_INT64, "5" },
                                { G_TYPE_INVALID }
                }
};

static struct test_case test_between32 = {
                "score in unlimited match: GibbonTestA-3 GibbonTestB-5",
                {
                                { G_TYPE_INT64, "311" },
                                { G_TYPE_INT64, "0" },
                                { G_TYPE_STRING, "GibbonTestA" },
                                { G_TYPE_INT64, "3" },
                                { G_TYPE_STRING, "GibbonTestB" },
                                { G_TYPE_INT64, "5" },
                                { G_TYPE_INVALID }
                }
};

static struct test_case test_between33 = {
                "** Please wait for GibbonTestA to join too.",
                {
                                { G_TYPE_INT64, "312" },
                                { G_TYPE_STRING, "GibbonTestA" },
                                { G_TYPE_INVALID }
                }
};

static struct test_case test_between34 = {
                "** You invited GibbonTestA to a 3 point match.",
                {
                                { G_TYPE_INT64, "313" },
                                { G_TYPE_STRING, "GibbonTestA" },
                                { G_TYPE_INT64, "3" },
                                { G_TYPE_INVALID }
                }
};

static struct test_case test_between35 = {
                "** You invited GibbonTestA to an unlimited match.",
                {
                                { G_TYPE_INT64, "313" },
                                { G_TYPE_STRING, "GibbonTestA" },
                                { G_TYPE_INT64, "0" },
                                { G_TYPE_INVALID }
                }
};

static struct test_case test_between36 = {
                "** You invited GibbonTestA to resume a saved match.",
                {
                                { G_TYPE_INT64, "313" },
                                { G_TYPE_STRING, "GibbonTestA" },
                                { G_TYPE_INT64, "-1" },
                                { G_TYPE_INVALID }
                }
};

static struct test_case test_between37 = {
                "You win the 7 point match 8-4.",
                {
                                { G_TYPE_INT64, "314" },
                                { G_TYPE_STRING, "You" },
                                { G_TYPE_INT64, "7" },
                                { G_TYPE_INT64, "8" },
                                { G_TYPE_INT64, "4" },
                                { G_TYPE_INVALID }
                }
};

static struct test_case test_between38 = {
                "GibbonTestA wins the 7 point match 8-4.",
                {
                                { G_TYPE_INT64, "314" },
                                { G_TYPE_STRING, "GibbonTestA" },
                                { G_TYPE_INT64, "7" },
                                { G_TYPE_INT64, "8" },
                                { G_TYPE_INT64, "4" },
                                { G_TYPE_INVALID }
                }
};

static struct test_case test_between39 = {
                "You stop watching gflohr.",
                {
                                { G_TYPE_INT64, "315" },
                                { G_TYPE_STRING, "gflohr" },
                                { G_TYPE_INVALID }
                }
};

static struct test_case test_various00 = {
                "gflohr and GibbonTestA start a 7 point match.",
                {
                                { G_TYPE_INT64, "400" },
                                { G_TYPE_STRING, "gflohr" },
                                { G_TYPE_STRING, "GibbonTestA" },
                                { G_TYPE_INT64, "7" },
                                { G_TYPE_INVALID }
                }
};

static struct test_case test_various01 = {
                "gflohr and GibbonTestA start an unlimited match.",
                {
                                { G_TYPE_INT64, "400" },
                                { G_TYPE_STRING, "gflohr" },
                                { G_TYPE_STRING, "GibbonTestA" },
                                { G_TYPE_INT64, "0" },
                                { G_TYPE_INVALID }
                }
};

static struct test_case test_various02 = {
                "gflohr wins a 7 point match against GibbonTestA  8-3 .",
                {
                                { G_TYPE_INT64, "401" },
                                { G_TYPE_STRING, "gflohr" },
                                { G_TYPE_STRING, "GibbonTestA" },
                                { G_TYPE_INT64, "7" },
                                { G_TYPE_INT64, "8" },
                                { G_TYPE_INT64, "3" },
                                { G_TYPE_INVALID }
                }
};

static struct test_case test_various02a = {
                "gflohr wins a 1 point match against GibbonTestA  1-0 .",
                {
                                { G_TYPE_INT64, "401" },
                                { G_TYPE_STRING, "gflohr" },
                                { G_TYPE_STRING, "GibbonTestA" },
                                { G_TYPE_INT64, "1" },
                                { G_TYPE_INT64, "1" },
                                { G_TYPE_INT64, "0" },
                                { G_TYPE_INVALID }
                }
};

static struct test_case test_various03 = {
                "gflohr and GibbonTestA are resuming their 7-point match.",
                {
                                { G_TYPE_INT64, "402" },
                                { G_TYPE_STRING, "gflohr" },
                                { G_TYPE_STRING, "GibbonTestA" },
                                { G_TYPE_INT64, "7" },
                                { G_TYPE_INVALID }
                }
};

static struct test_case test_various04 = {
                "gflohr and GibbonTestA are resuming their unlimited match.",
                {
                                { G_TYPE_INT64, "402" },
                                { G_TYPE_STRING, "gflohr" },
                                { G_TYPE_STRING, "GibbonTestA" },
                                { G_TYPE_INT64, "0" },
                                { G_TYPE_INVALID }
                }
};

static struct test_case test_various05 = {
                "   ",
                {
                                { G_TYPE_INT64, "403" },
                                { G_TYPE_INVALID }
                }
};

static struct test_case test_settings00 = {
                "Settings of variables:",
                {
                                { G_TYPE_INT64, "404" },
                                { G_TYPE_INVALID }
                }
};

static struct test_case test_settings01 = {
                "boardstyle: 3",
                {
                                { G_TYPE_INT64, "405" },
                                { G_TYPE_STRING, "boardstyle" },
                                { G_TYPE_STRING, "3" },
                                { G_TYPE_INVALID }
                }
};

static struct test_case test_settings02 = {
                "linelength: 0",
                {
                                { G_TYPE_INT64, "405" },
                                { G_TYPE_STRING, "linelength" },
                                { G_TYPE_STRING, "0" },
                                { G_TYPE_INVALID }
                }
};

static struct test_case test_settings03 = {
                "pagelength: 0",
                {
                                { G_TYPE_INT64, "405" },
                                { G_TYPE_STRING, "pagelength" },
                                { G_TYPE_STRING, "0" },
                                { G_TYPE_INVALID }
                }
};

static struct test_case test_settings04 = {
                "redoubles: none",
                {
                                { G_TYPE_INT64, "405" },
                                { G_TYPE_STRING, "redoubles" },
                                { G_TYPE_STRING, "none" },
                                { G_TYPE_INVALID }
                }
};

static struct test_case test_settings05 = {
                "sortwho: login",
                {
                                { G_TYPE_INT64, "405" },
                                { G_TYPE_STRING, "sortwho" },
                                { G_TYPE_STRING, "login" },
                                { G_TYPE_INVALID }
                }
};

static struct test_case test_settings06 = {
                "timezone: UTC",
                {
                                { G_TYPE_INT64, "405" },
                                { G_TYPE_STRING, "timezone" },
                                { G_TYPE_STRING, "UTC" },
                                { G_TYPE_INVALID }
                }
};

static struct test_case test_settings07 = {
                "Value of 'boardstyle' set to 3.",
                {
                                { G_TYPE_INT64, "405" },
                                { G_TYPE_STRING, "boardstyle" },
                                { G_TYPE_STRING, "3" },
                                { G_TYPE_INVALID }
                }
};

static struct test_case test_settings08 = {
                "Value of 'linelength' set to 0.",
                {
                                { G_TYPE_INT64, "405" },
                                { G_TYPE_STRING, "linelength" },
                                { G_TYPE_STRING, "0" },
                                { G_TYPE_INVALID }
                }
};

static struct test_case test_settings09 = {
                "Value of 'pagelength' set to 0.",
                {
                                { G_TYPE_INT64, "405" },
                                { G_TYPE_STRING, "pagelength" },
                                { G_TYPE_STRING, "0" },
                                { G_TYPE_INVALID }
                }
};

static struct test_case test_settings10 = {
                "Value of 'redoubles' set to none.",
                {
                                { G_TYPE_INT64, "405" },
                                { G_TYPE_STRING, "redoubles" },
                                { G_TYPE_STRING, "none" },
                                { G_TYPE_INVALID }
                }
};

static struct test_case test_settings11 = {
                "Value of 'sortwho' set to login.",
                {
                                { G_TYPE_INT64, "405" },
                                { G_TYPE_STRING, "sortwho" },
                                { G_TYPE_STRING, "login" },
                                { G_TYPE_INVALID }
                }
};

static struct test_case test_settings12 = {
                "Value of 'timezone' set to UTC.",
                {
                                { G_TYPE_INT64, "405" },
                                { G_TYPE_STRING, "timezone" },
                                { G_TYPE_STRING, "UTC" },
                                { G_TYPE_INVALID }
                }
};

static struct test_case test_toggles00 = {
                "The current settings are:",
                {
                                { G_TYPE_INT64, "406" },
                                { G_TYPE_INVALID }
                }
};

static struct test_case test_toggles01 = {
                "allowpip   YES",
                {
                                { G_TYPE_INT64, "407" },
                                { G_TYPE_STRING, "allowpip" },
                                { G_TYPE_BOOLEAN, "TRUE" },
                                { G_TYPE_INVALID }
                }
};

static struct test_case test_toggles02 = {
                "autoboard   YES",
                {
                                { G_TYPE_INT64, "407" },
                                { G_TYPE_STRING, "autoboard" },
                                { G_TYPE_BOOLEAN, "TRUE" },
                                { G_TYPE_INVALID }
                }
};

static struct test_case test_toggles03 = {
                "autodouble   NO",
                {
                                { G_TYPE_INT64, "407" },
                                { G_TYPE_STRING, "autodouble" },
                                { G_TYPE_BOOLEAN, "FALSE" },
                                { G_TYPE_INVALID }
                }
};

static struct test_case test_toggles04 = {
                "automove   YES",
                {
                                { G_TYPE_INT64, "407" },
                                { G_TYPE_STRING, "automove" },
                                { G_TYPE_BOOLEAN, "TRUE" },
                                { G_TYPE_INVALID }
                }
};

static struct test_case test_toggles05 = {
                "bell   NO",
                {
                                { G_TYPE_INT64, "407" },
                                { G_TYPE_STRING, "bell" },
                                { G_TYPE_BOOLEAN, "FALSE" },
                                { G_TYPE_INVALID }
                }
};

static struct test_case test_toggles06 = {
                "crawford   YES",
                {
                                { G_TYPE_INT64, "407" },
                                { G_TYPE_STRING, "crawford" },
                                { G_TYPE_BOOLEAN, "TRUE" },
                                { G_TYPE_INVALID }
                }
};

static struct test_case test_toggles07 = {
                "double   YES",
                {
                                { G_TYPE_INT64, "407" },
                                { G_TYPE_STRING, "double" },
                                { G_TYPE_BOOLEAN, "TRUE" },
                                { G_TYPE_INVALID }
                }
};

static struct test_case test_toggles08 = {
                "greedy   NO",
                {
                                { G_TYPE_INT64, "407" },
                                { G_TYPE_STRING, "greedy" },
                                { G_TYPE_BOOLEAN, "FALSE" },
                                { G_TYPE_INVALID }
                }
};

static struct test_case test_toggles09 = {
                "moreboards   YES",
                {
                                { G_TYPE_INT64, "407" },
                                { G_TYPE_STRING, "moreboards" },
                                { G_TYPE_BOOLEAN, "TRUE" },
                                { G_TYPE_INVALID }
                }
};

static struct test_case test_toggles10 = {
                "moves   NO",
                {
                                { G_TYPE_INT64, "407" },
                                { G_TYPE_STRING, "moves" },
                                { G_TYPE_BOOLEAN, "FALSE" },
                                { G_TYPE_INVALID }
                }
};

static struct test_case test_toggles11 = {
                "notify  YES",
                {
                                { G_TYPE_INT64, "407" },
                                { G_TYPE_STRING, "notify" },
                                { G_TYPE_BOOLEAN, "TRUE" },
                                { G_TYPE_INVALID }
                }
};

static struct test_case test_toggles12 = {
                "ratings   NO",
                {
                                { G_TYPE_INT64, "407" },
                                { G_TYPE_STRING, "ratings" },
                                { G_TYPE_BOOLEAN, "FALSE" },
                                { G_TYPE_INVALID }
                }
};

static struct test_case test_toggles13 = {
                "ready   NO",
                {
                                { G_TYPE_INT64, "407" },
                                { G_TYPE_STRING, "ready" },
                                { G_TYPE_BOOLEAN, "FALSE" },
                                { G_TYPE_INVALID }
                }
};

static struct test_case test_toggles14 = {
                "report   NO",
                {
                                { G_TYPE_INT64, "407" },
                                { G_TYPE_STRING, "report" },
                                { G_TYPE_BOOLEAN, "FALSE" },
                                { G_TYPE_INVALID }
                }
};

static struct test_case test_toggles15 = {
                "silent   NO",
                {
                                { G_TYPE_INT64, "407" },
                                { G_TYPE_STRING, "silent" },
                                { G_TYPE_BOOLEAN, "FALSE" },
                                { G_TYPE_INVALID }
                }
};

static struct test_case test_toggles16 = {
                "telnet  YES",
                {
                                { G_TYPE_INT64, "407" },
                                { G_TYPE_STRING, "telnet" },
                                { G_TYPE_BOOLEAN, "TRUE" },
                                { G_TYPE_INVALID }
                }
};

static struct test_case test_toggles17 = {
                "wrap   NO",
                {
                                { G_TYPE_INT64, "407" },
                                { G_TYPE_STRING, "wrap" },
                                { G_TYPE_BOOLEAN, "FALSE" },
                                { G_TYPE_INVALID }
                }
};

static struct test_case test_toggles18 = {
                "** You're now ready to invite or join someone.",
                {
                                { G_TYPE_INT64, "407" },
                                { G_TYPE_STRING, "ready" },
                                { G_TYPE_BOOLEAN, "TRUE" },
                                { G_TYPE_INVALID }
                }
};

static struct test_case test_toggles19 = {
                "** You're now refusing to play with someone.",
                {
                                { G_TYPE_INT64, "407" },
                                { G_TYPE_STRING, "ready" },
                                { G_TYPE_BOOLEAN, "FALSE" },
                                { G_TYPE_INVALID }
                }
};

static struct test_case test_toggles20 = {
                "** You won't be notified when new users log in.",
                {
                                { G_TYPE_INT64, "407" },
                                { G_TYPE_STRING, "notify" },
                                { G_TYPE_BOOLEAN, "FALSE" },
                                { G_TYPE_INVALID }
                }
};

static struct test_case test_toggles21 = {
                "** You'll be notified when new users log in.",
                {
                                { G_TYPE_INT64, "407" },
                                { G_TYPE_STRING, "notify" },
                                { G_TYPE_BOOLEAN, "TRUE" },
                                { G_TYPE_INVALID }
                }
};

static struct test_case test_toggles22 = {
                "** The board will be refreshed after every move.",
                {
                                { G_TYPE_INT64, "407" },
                                { G_TYPE_STRING, "autoboard" },
                                { G_TYPE_BOOLEAN, "TRUE" },
                                { G_TYPE_INVALID }
                }
};

static struct test_case test_toggles23 = {
                "** The board won't be refreshed after every move.",
                {
                                { G_TYPE_INT64, "407" },
                                { G_TYPE_STRING, "autoboard" },
                                { G_TYPE_BOOLEAN, "FALSE" },
                                { G_TYPE_INVALID }
                }
};

static struct test_case test_saved00 = {
        "  opponent          matchlength   score (your points first)",
        {
                                { G_TYPE_INT64, "408" },
                                { G_TYPE_INVALID }
        }
};

static struct test_case test_saved01 = {
        "  GammonBot_VII           5                0 -  0",
        {
                                { G_TYPE_INT64, "409" },
                                { G_TYPE_STRING, "GammonBot_VII" },
                                { G_TYPE_INT64, "5" },
                                { G_TYPE_INT64, "0" },
                                { G_TYPE_INT64, "0" },
                                { G_TYPE_INVALID }
        }
};

static struct test_case test_saved02 = {
        " *BlunderBot_VI           3                1 -  0",
        {
                                { G_TYPE_INT64, "409" },
                                { G_TYPE_STRING, "BlunderBot_VI" },
                                { G_TYPE_INT64, "3" },
                                { G_TYPE_INT64, "1" },
                                { G_TYPE_INT64, "0" },
                                { G_TYPE_INVALID }
        }
};

static struct test_case test_saved03 = {
        "**bonehead                3                1 -  2",
        {
                                { G_TYPE_INT64, "409" },
                                { G_TYPE_STRING, "bonehead" },
                                { G_TYPE_INT64, "3" },
                                { G_TYPE_INT64, "1" },
                                { G_TYPE_INT64, "2" },
                                { G_TYPE_INVALID }
        }
};

static struct test_case test_saved04 = {
        "**bonehead                unlimited                1 -  2",
        {
                                { G_TYPE_INT64, "409" },
                                { G_TYPE_STRING, "bonehead" },
                                { G_TYPE_INT64, "0" },
                                { G_TYPE_INT64, "1" },
                                { G_TYPE_INT64, "2" },
                                { G_TYPE_INVALID }
        }
};

static struct test_case test_saved05 = {
        "  deadbeef                unlimited         1 -  2",
        {
                                { G_TYPE_INT64, "409" },
                                { G_TYPE_STRING, "deadbeef" },
                                { G_TYPE_INT64, "0" },
                                { G_TYPE_INT64, "1" },
                                { G_TYPE_INT64, "2" },
                                { G_TYPE_INVALID }
        }
};

static struct test_case test_saved06 = {
        "no saved games.",
        {
                                { G_TYPE_INT64, "410" },
                                { G_TYPE_INVALID }
        }
};

static struct test_case test_saved07 = {
        "deadbeef has no saved games.",
        {
                                { G_TYPE_INT64, "411" },
                                { G_TYPE_STRING, "deadbeef" },
                                { G_TYPE_INT64, "0" },
                                { G_TYPE_INVALID }
        }
};

static struct test_case test_saved08 = {
        "deadbeef has 12 saved games.",
        {
                                { G_TYPE_INT64, "411" },
                                { G_TYPE_STRING, "deadbeef" },
                                { G_TYPE_INT64, "12" },
                                { G_TYPE_INVALID }
        }
};

static struct test_case test_saved09 = {
        "deadbeef has 1 saved game.",
        {
                                { G_TYPE_INT64, "411" },
                                { G_TYPE_STRING, "deadbeef" },
                                { G_TYPE_INT64, "1" },
                                { G_TYPE_INVALID }
        }
};

static struct test_case test_address01 = {
        "Your email address is 'gibbon@example.com'.",
        {
                                { G_TYPE_INT64, "412" },
                                { G_TYPE_STRING, "gibbon@example.com" },
                                { G_TYPE_INVALID }
        }
};

static struct test_case test_address02 = {
        "** 'http://foobar' is not an email address.",
        {
                                { G_TYPE_INT64, "415" },
                                { G_TYPE_STRING, "http://foobar"},
                                { G_TYPE_INVALID }
        }
};

static struct test_case test_corrupt = {
                "** ERROR: Saved match is corrupt. Please start another one.",
                {
                                { G_TYPE_INT64, "100" },
                                { G_TYPE_INT64, "4" },
                                { G_TYPE_STRING,
                                  "Your saved match was corrupted on the"
                                  " server.  Please start a new one!"},
                                { G_TYPE_INVALID }
                }
};

static struct test_case test_heard_you00 = {
                "** 1 user heard you.",
                {
                                { G_TYPE_INT64, "500" },
                                { G_TYPE_INT64, "1" },
                                { G_TYPE_INVALID }
                }
};

static struct test_case test_heard_you01 = {
                "** 42 users heard you.",
                {
                                { G_TYPE_INT64, "500" },
                                { G_TYPE_INT64, "42" },
                                { G_TYPE_INVALID }
                }
};

static struct test_case *test_cases[] = {
                &test_clip01_0,
                &test_clip01_1,
                &test_clip02_0,
                &test_clip02_1,
                &test_clip02_2,
                &test_clip03,
                &test_clip03a,
                &test_clip03b,
                &test_clip04,
                &test_clip05_0,
                &test_clip05_1,
                &test_clip05_2,
                &test_clip06,
                &test_clip07,
                &test_clip08,
                &test_clip09,
                &test_clip10,
                &test_clip11,
                &test_clip12,
                &test_clip13,
                &test_clip14,
                &test_clip15,
                &test_clip16,
                &test_clip17,
                &test_clip18,
                &test_clip19,
                &test_clip20,

                &test_error00,
                &test_error01,
                &test_error02,
                &test_error03,

                &test_board00,
                &test_board01,
                &test_board02,
                &test_board03,
                &test_rolls00,
                &test_rolls01,
                &test_moves00,
                &test_moves01,
                &test_moves02,
                &test_moves03,
                &test_moves04,
                &test_moves05,
                &test_moves06,
                &test_moves07,
                &test_game_start00,
                &test_game_left00,
                &test_game_left01,
                &test_game_left02,
                &test_game_left03,
                &test_game_left04,
                &test_cannot_move00,
                &test_cannot_move01,
                &test_doubling00,
                &test_doubling01,
                &test_doubling02,
                &test_doubling03,

                &test_between00,
                &test_between01,
                &test_between02,
                &test_between03,
                &test_between04,
                &test_between05,
                &test_between06,
                &test_between07,
                &test_between08,
                &test_between09,
                &test_between10,
                &test_between11,
                &test_between12,
                &test_between13,
                &test_between14,
                &test_between15,
                &test_between16,
                &test_between17,
                &test_between18,
                &test_between19,
                &test_between20,
                &test_between21,
                &test_between22,
                &test_between23,
                &test_between24,
                &test_between25,
                &test_between26,
                &test_between27,
                &test_between28,
                &test_between29,
                &test_between30,
                &test_between31,
                &test_between32,
                &test_between33,
                &test_between34,
                &test_between35,
                &test_between36,
                &test_between37,
                &test_between38,
                &test_between39,

                &test_resign00,
                &test_resign01,
                &test_resign02,
                &test_resign03,
                &test_resign04,
                &test_resign05,

                &test_resume00,
                &test_resume01,
                &test_resume02,
                &test_resume03,
                &test_resume04,
                &test_resume05,

                &test_various00,
                &test_various01,
                &test_various02,
                &test_various02a,
                &test_various03,
                &test_various04,
                &test_various05,

                &test_settings00,
                &test_settings01,
                &test_settings02,
                &test_settings03,
                &test_settings04,
                &test_settings05,
                &test_settings06,
                &test_settings07,
                &test_settings08,
                &test_settings09,
                &test_settings10,
                &test_settings11,
                &test_settings12,
                &test_toggles00,
                &test_toggles01,
                &test_toggles02,
                &test_toggles03,
                &test_toggles04,
                &test_toggles05,
                &test_toggles06,
                &test_toggles07,
                &test_toggles08,
                &test_toggles09,
                &test_toggles10,
                &test_toggles11,
                &test_toggles12,
                &test_toggles13,
                &test_toggles14,
                &test_toggles15,
                &test_toggles16,
                &test_toggles17,
                &test_toggles18,
                &test_toggles19,
                &test_toggles20,
                &test_toggles21,
                &test_toggles22,
                &test_toggles23,

                &test_saved00,
                &test_saved01,
                &test_saved02,
                &test_saved03,
                &test_saved04,
                &test_saved05,
                &test_saved06,
                &test_saved07,
                &test_saved08,
                &test_saved09,

                &test_address01,
                &test_address02,

                &test_corrupt,

                &test_heard_you00,
                &test_heard_you01
};

static gboolean test_single_case (GibbonCLIPReader *reader,
                                  struct test_case *test_case);
static gboolean check_result (const gchar *line, gsize num,
                              struct token_pair *token_pair,
                              GValue *value);

int
main (int argc, char *argv[])
{
        gint status = 0;
        gint i;
        GibbonCLIPReader *reader;

        g_type_init ();

        reader = gibbon_clip_reader_new ();

        for (i = 0; i < sizeof test_cases / sizeof test_cases[0]; ++i) {
                if (!test_single_case (reader, test_cases[i]))
                        status = -1;
        }

        return status;
}

static gboolean
test_single_case (GibbonCLIPReader *reader, struct test_case *test_case)
{
        GSList *result = gibbon_clip_reader_parse (reader, test_case->line);
        GSList *iter;
        gboolean retval = TRUE;
        struct token_pair *expect;
        gsize i = 0;

        expect = test_case->values;

        iter = result;
        while (iter) {
                ++i;
                if (!check_result (test_case->line, i, expect,
                                   (GValue *) iter->data))
                        retval = FALSE;
                iter = iter->next;
                if (expect->type != G_TYPE_INVALID)
                        ++expect;
        }

        while (expect->type != G_TYPE_INVALID) {
                retval = FALSE;
                g_printerr ("%s: token #%llu: expected `%s' (token type %s)"
                            " got nothing.\n",
                            test_case->line, (unsigned long long) ++i,
                            expect->value, g_type_name (expect->type));
                ++expect;
        }

        gibbon_clip_reader_free_result (reader, result);

        return retval;
}

static gboolean
check_result (const gchar *line, gsize num,
              struct token_pair *token_pair,
              GValue *value)
{
        gboolean retval = TRUE;
        GValue stringified = G_VALUE_INIT;
        const gchar *got_value;
        const gchar *expect_value;

        g_value_init (&stringified, G_TYPE_STRING);
        g_return_val_if_fail (g_value_transform (value, &stringified), FALSE);

        got_value = g_value_get_string (&stringified);

        if (token_pair->type != G_TYPE_INVALID) {
                expect_value = token_pair->value;
                if (token_pair->type == G_TYPE_STRING
                    && !g_ascii_strncasecmp (expect_value,
                                             "=== Position ===\n", 17))
                        token_pair->type = GIBBON_TYPE_POSITION;
                if (token_pair->type != G_VALUE_TYPE (value)
                    || g_strcmp0 (got_value, expect_value)) {
                        g_printerr ("%s: token #%llu:"
                                    " expected `%s' (token type %s),"
                                    " got `%s' (token type %s).\n",
                                    line, (unsigned long long) num,
                                    expect_value, g_type_name (token_pair->type),
                                    got_value, G_VALUE_TYPE_NAME (value));
                        retval = FALSE;
                }
        } else {
                g_printerr ("%s: token #%llu: expected nothing,"
                            " got `%s' (token type %s).\n",
                            line, (unsigned long long) num,
                            got_value, G_VALUE_TYPE_NAME (value));
                retval = FALSE;
        }

        return retval;
}
