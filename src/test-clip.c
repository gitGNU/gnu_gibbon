/*
 * This file is part of Gibbon, a graphical frontend to the First Internet 
 * Backgammon Server FIBS.
 * Copyright (C) 2009-2011 Guido Flohr, http://guido-flohr.net/.
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

#include "gibbon-clip.h"

struct token_pair {
        enum GibbonClipType type;
        const gchar *value;
};

struct test_case {
        const gchar *line;
        struct token_pair tokens[];
};

static struct test_case test_clip00 = {
                "| This is part of the motto of the day. |",
                {
                                { GIBBON_CLIP_TYPE_END, NULL }
                }
};

static struct test_case test_clip01 = {
                "1 gflohr 1306865048 gibbon.example.com",
                {
                                { GIBBON_CLIP_TYPE_UINT, "1" },
                                { GIBBON_CLIP_TYPE_NAME, "gflohr" },
                                { GIBBON_CLIP_TYPE_TIMESTAMP, "1306865048" },
                                { GIBBON_CLIP_TYPE_STRING,
                                                "gibbon.example.com" },
                                { GIBBON_CLIP_TYPE_END, NULL }
                }
};

static struct test_case test_clip01a = {
                "1 GibbonTestA 1308476373 95.87.204.192",
                {
                                { GIBBON_CLIP_TYPE_UINT, "1" },
                                { GIBBON_CLIP_TYPE_NAME, "GibbonTestA" },
                                { GIBBON_CLIP_TYPE_TIMESTAMP, "1308476373" },
                                { GIBBON_CLIP_TYPE_STRING,
                                                "95.87.204.192" },
                                { GIBBON_CLIP_TYPE_END, NULL }
                }
};

static struct test_case test_clip02 = {
                "2 gflohr 1 1 0 0 0 0 1 1 2396 0 1 0 1 3457.85 0 0 0 0 0"
                " Europe/Sofia",
                {
                                { GIBBON_CLIP_TYPE_UINT, "2" },
                                { GIBBON_CLIP_TYPE_NAME, "gflohr" },
                                { GIBBON_CLIP_TYPE_BOOLEAN, "TRUE" },
                                { GIBBON_CLIP_TYPE_BOOLEAN, "TRUE" },
                                { GIBBON_CLIP_TYPE_BOOLEAN, "FALSE" },
                                { GIBBON_CLIP_TYPE_BOOLEAN, "FALSE" },
                                { GIBBON_CLIP_TYPE_BOOLEAN, "FALSE" },
                                { GIBBON_CLIP_TYPE_BOOLEAN, "FALSE" },
                                { GIBBON_CLIP_TYPE_BOOLEAN, "TRUE" },
                                { GIBBON_CLIP_TYPE_BOOLEAN, "TRUE" },
                                { GIBBON_CLIP_TYPE_UINT, "2396" },
                                { GIBBON_CLIP_TYPE_BOOLEAN, "FALSE" },
                                { GIBBON_CLIP_TYPE_BOOLEAN, "TRUE" },
                                { GIBBON_CLIP_TYPE_BOOLEAN, "FALSE" },
                                { GIBBON_CLIP_TYPE_BOOLEAN, "TRUE" },
                                { GIBBON_CLIP_TYPE_DOUBLE, "3457.850000" },
                                { GIBBON_CLIP_TYPE_BOOLEAN, "FALSE" },
                                { GIBBON_CLIP_TYPE_BOOLEAN, "FALSE" },
                                { GIBBON_CLIP_TYPE_BOOLEAN, "FALSE" },
                                { GIBBON_CLIP_TYPE_BOOLEAN, "FALSE" },
                                { GIBBON_CLIP_TYPE_BOOLEAN, "FALSE" },
                                { GIBBON_CLIP_TYPE_STRING, "Europe/Sofia" },
                                { GIBBON_CLIP_TYPE_END, NULL }
                }
};

static struct test_case test_clip03 = {
                "3",
                {
                                { GIBBON_CLIP_TYPE_UINT, "3" },
                                { GIBBON_CLIP_TYPE_END, NULL }
                }
};

static struct test_case test_clip04 = {
                "4",
                {
                                { GIBBON_CLIP_TYPE_UINT, "4" },
                                { GIBBON_CLIP_TYPE_END, NULL }
                }
};

static struct test_case test_clip05 = {
                "5 gflohr barrack - 0 0 1418.61 1914 23 1306926526"
                " 173.223.48.110 Gibbon_0.1.1 president@whitehouse.gov",
                {
                                { GIBBON_CLIP_TYPE_UINT, "5" },
                                { GIBBON_CLIP_TYPE_NAME, "gflohr" },
                                { GIBBON_CLIP_TYPE_NAME, "barrack" },
                                { GIBBON_CLIP_TYPE_NAME, "" },
                                { GIBBON_CLIP_TYPE_BOOLEAN, "FALSE" },
                                { GIBBON_CLIP_TYPE_BOOLEAN, "FALSE" },
                                { GIBBON_CLIP_TYPE_DOUBLE, "1418.610000" },
                                { GIBBON_CLIP_TYPE_UINT, "1914" },
                                { GIBBON_CLIP_TYPE_UINT, "23" },
                                { GIBBON_CLIP_TYPE_TIMESTAMP, "1306926526" },
                                { GIBBON_CLIP_TYPE_STRING, "173.223.48.110" },
                                { GIBBON_CLIP_TYPE_STRING, "Gibbon_0.1.1" },
                                { GIBBON_CLIP_TYPE_STRING,
                                                "president@whitehouse.gov" },
                                { GIBBON_CLIP_TYPE_END, NULL }
                }
};

static struct test_case test_clip06 = {
                "6",
                {
                                { GIBBON_CLIP_TYPE_UINT, "6" },
                                { GIBBON_CLIP_TYPE_END, NULL }
                }
};

static struct test_case test_clip07 = {
                "7 gflohr gflohr logs in.",
                {
                                { GIBBON_CLIP_TYPE_UINT, "7" },
                                { GIBBON_CLIP_TYPE_NAME, "gflohr" },
                                { GIBBON_CLIP_TYPE_STRING, "gflohr logs in" },
                                { GIBBON_CLIP_TYPE_END, NULL }
                }
};

static struct test_case test_clip08 = {
                "8 gflohr gflohr drops connection.",
                {
                                { GIBBON_CLIP_TYPE_UINT, "8" },
                                { GIBBON_CLIP_TYPE_NAME, "gflohr" },
                                { GIBBON_CLIP_TYPE_STRING,
                                                "gflohr drops connection" },
                                { GIBBON_CLIP_TYPE_END, NULL }
                }
};

static struct test_case test_clip09 = {
                "9 gflohr -1306935184    Be back at 20 p.m.",
                {
                                { GIBBON_CLIP_TYPE_UINT, "9" },
                                { GIBBON_CLIP_TYPE_NAME, "gflohr" },
                                { GIBBON_CLIP_TYPE_TIMESTAMP, "-1306935184" },
                                { GIBBON_CLIP_TYPE_STRING,
                                                "   Be back at 20 p.m." },
                                { GIBBON_CLIP_TYPE_END, NULL }
                }
};

static struct test_case test_clip10 = {
                "10 gflohr",
                {
                                { GIBBON_CLIP_TYPE_UINT, "10" },
                                { GIBBON_CLIP_TYPE_NAME, "gflohr" },
                                { GIBBON_CLIP_TYPE_END, NULL }
                }
};

static struct test_case test_clip11 = {
                "11 gflohr",
                {
                                { GIBBON_CLIP_TYPE_UINT, "11" },
                                { GIBBON_CLIP_TYPE_NAME, "gflohr" },
                                { GIBBON_CLIP_TYPE_END, NULL }
                }
};

static struct test_case test_clip12 = {
                "12 gflohr Hello world.",
                {
                                { GIBBON_CLIP_TYPE_UINT, "12" },
                                { GIBBON_CLIP_TYPE_NAME, "gflohr" },
                                { GIBBON_CLIP_TYPE_STRING,
                                                "Hello world." },
                                { GIBBON_CLIP_TYPE_END, NULL }
                }
};

static struct test_case test_clip13 = {
                "13 gflohr Hello world.",
                {
                                { GIBBON_CLIP_TYPE_UINT, "13" },
                                { GIBBON_CLIP_TYPE_NAME, "gflohr" },
                                { GIBBON_CLIP_TYPE_STRING,
                                                "Hello world." },
                                { GIBBON_CLIP_TYPE_END, NULL }
                }
};

static struct test_case test_clip14 = {
                "14 gflohr Hello world.",
                {
                                { GIBBON_CLIP_TYPE_UINT, "14" },
                                { GIBBON_CLIP_TYPE_NAME, "gflohr" },
                                { GIBBON_CLIP_TYPE_STRING,
                                                "Hello world." },
                                { GIBBON_CLIP_TYPE_END, NULL }
                }
};

static struct test_case test_clip15 = {
                "15 gflohr Hello world.",
                {
                                { GIBBON_CLIP_TYPE_UINT, "15" },
                                { GIBBON_CLIP_TYPE_NAME, "gflohr" },
                                { GIBBON_CLIP_TYPE_STRING,
                                                "Hello world." },
                                { GIBBON_CLIP_TYPE_END, NULL }
                }
};

static struct test_case test_clip16 = {
                "16 gflohr Hello world.",
                {
                                { GIBBON_CLIP_TYPE_UINT, "16" },
                                { GIBBON_CLIP_TYPE_NAME, "gflohr" },
                                { GIBBON_CLIP_TYPE_STRING,
                                                "Hello world." },
                                { GIBBON_CLIP_TYPE_END, NULL }
                }
};

static struct test_case test_clip17 = {
                "17 Hello world.",
                {
                                { GIBBON_CLIP_TYPE_UINT, "17" },
                                { GIBBON_CLIP_TYPE_STRING,
                                                "Hello world." },
                                { GIBBON_CLIP_TYPE_END, NULL }
                }
};

static struct test_case test_clip18 = {
                "18 Hello world.",
                {
                                { GIBBON_CLIP_TYPE_UINT, "18" },
                                { GIBBON_CLIP_TYPE_STRING,
                                                "Hello world." },
                                { GIBBON_CLIP_TYPE_END, NULL }
                }
};

static struct test_case test_clip19 = {
                "19 Hello world.",
                {
                                { GIBBON_CLIP_TYPE_UINT, "19" },
                                { GIBBON_CLIP_TYPE_STRING,
                                                "Hello world." },
                                { GIBBON_CLIP_TYPE_END, NULL }
                }
};

static struct test_case test_error00 = {
                "** Funny new message!",
                {
                                { GIBBON_CLIP_TYPE_UINT, "100" },
                                { GIBBON_CLIP_TYPE_STRING,
                                "Funny new message!"},
                                { GIBBON_CLIP_TYPE_END, NULL }
                },
};

static struct test_case test_error01 = {
                "** Error: something new went wrong!",
                {
                                { GIBBON_CLIP_TYPE_UINT, "100" },
                                { GIBBON_CLIP_TYPE_STRING,
                                "Error: something new went wrong!"},
                                { GIBBON_CLIP_TYPE_END, NULL }
                },
};

static struct test_case test_error02 = {
                "** You see a funny new message!",
                {
                                { GIBBON_CLIP_TYPE_UINT, "100" },
                                { GIBBON_CLIP_TYPE_STRING,
                                "You see a funny new message!"},
                                { GIBBON_CLIP_TYPE_END, NULL }
                },
};

static struct test_case test_board00 =  {
                "board:joe_white:black_jack:7:5:0:0:0:2:-1:0:-1:4:0:2:0:0:0:-2"
                ":4:0:0:0:-3:-2:-4:3:-2:0:0:0:0:-1:0:0:6:6:1:1:1:0:1:-1:0:25:0"
                ":0:0:0:2:6:0:0",
                {
                                /* Board.  */
                                { GIBBON_CLIP_TYPE_UINT, "200" },
                                /* Player and opponent.  */
                                { GIBBON_CLIP_TYPE_NAME, "joe_white" },
                                { GIBBON_CLIP_TYPE_NAME, "black_jack" },
                                /* Match length.  */
                                { GIBBON_CLIP_TYPE_UINT, "7" },
                                /* Score.  */
                                { GIBBON_CLIP_TYPE_UINT, "5" },
                                { GIBBON_CLIP_TYPE_UINT, "0" },
                                /* Position.  */
                                { GIBBON_CLIP_TYPE_INT, "0" },
                                { GIBBON_CLIP_TYPE_INT, "2" },
                                { GIBBON_CLIP_TYPE_INT, "-1" },
                                { GIBBON_CLIP_TYPE_INT, "0" },
                                { GIBBON_CLIP_TYPE_INT, "-1" },
                                { GIBBON_CLIP_TYPE_INT, "4" },
                                { GIBBON_CLIP_TYPE_INT, "0" },
                                { GIBBON_CLIP_TYPE_INT, "2" },
                                { GIBBON_CLIP_TYPE_INT, "0" },
                                { GIBBON_CLIP_TYPE_INT, "0" },
                                { GIBBON_CLIP_TYPE_INT, "0" },
                                { GIBBON_CLIP_TYPE_INT, "-2" },
                                { GIBBON_CLIP_TYPE_INT, "4" },
                                { GIBBON_CLIP_TYPE_INT, "0" },
                                { GIBBON_CLIP_TYPE_INT, "0" },
                                { GIBBON_CLIP_TYPE_INT, "0" },
                                { GIBBON_CLIP_TYPE_INT, "-3" },
                                { GIBBON_CLIP_TYPE_INT, "-2" },
                                { GIBBON_CLIP_TYPE_INT, "-4" },
                                { GIBBON_CLIP_TYPE_INT, "3" },
                                { GIBBON_CLIP_TYPE_INT, "-2" },
                                { GIBBON_CLIP_TYPE_INT, "0" },
                                { GIBBON_CLIP_TYPE_INT, "0" },
                                { GIBBON_CLIP_TYPE_INT, "0" },
                                /* Turn.  */
                                { GIBBON_CLIP_TYPE_INT, "-1" },
                                /* Dice.  */
                                { GIBBON_CLIP_TYPE_INT, "-6" },
                                { GIBBON_CLIP_TYPE_INT, "-6" },
                                /* Cube.  */
                                { GIBBON_CLIP_TYPE_UINT, "1" },
                                /* Player and opponent may double? */
                                { GIBBON_CLIP_TYPE_BOOLEAN, "TRUE" },
                                { GIBBON_CLIP_TYPE_BOOLEAN, "TRUE" },
                                /* Playing direction.  */
                                { GIBBON_CLIP_TYPE_BOOLEAN, "TRUE" },
                                /* Player's and opponent's bar.  */
                                { GIBBON_CLIP_TYPE_UINT, "0" },
                                { GIBBON_CLIP_TYPE_UINT, "0" },
                                /* Post-Crawford? */
                                { GIBBON_CLIP_TYPE_BOOLEAN, "FALSE" },
                                { GIBBON_CLIP_TYPE_END, NULL }
                }
};

static struct test_case test_board01 =  {
                "board:joe_white:black_jack:1:0:0:0:6:2:0:2:0:0:0:0:0:-1:0:0"
                ":-2:0:0:0:0:0:0:-3:-1:-3:-1:-4:0:1:0:0:6:2:1:1:1:0:-1:1:25:0"
                ":0:5:0:0:2:4:0:0",
                {
                                /* Board.  */
                                { GIBBON_CLIP_TYPE_UINT, "200" },
                                /* Player and opponent.  */
                                { GIBBON_CLIP_TYPE_NAME, "joe_white" },
                                { GIBBON_CLIP_TYPE_NAME, "black_jack" },
                                /* Match length.  */
                                { GIBBON_CLIP_TYPE_UINT, "1" },
                                /* Score.  */
                                { GIBBON_CLIP_TYPE_UINT, "0" },
                                { GIBBON_CLIP_TYPE_UINT, "0" },
                                /* Position.  */
                                { GIBBON_CLIP_TYPE_INT, "4" },
                                { GIBBON_CLIP_TYPE_INT, "1" },
                                { GIBBON_CLIP_TYPE_INT, "3" },
                                { GIBBON_CLIP_TYPE_INT, "1" },
                                { GIBBON_CLIP_TYPE_INT, "3" },
                                { GIBBON_CLIP_TYPE_INT, "0" },
                                { GIBBON_CLIP_TYPE_INT, "0" },
                                { GIBBON_CLIP_TYPE_INT, "0" },
                                { GIBBON_CLIP_TYPE_INT, "0" },
                                { GIBBON_CLIP_TYPE_INT, "0" },
                                { GIBBON_CLIP_TYPE_INT, "0" },
                                { GIBBON_CLIP_TYPE_INT, "2" },
                                { GIBBON_CLIP_TYPE_INT, "0" },
                                { GIBBON_CLIP_TYPE_INT, "0" },
                                { GIBBON_CLIP_TYPE_INT, "1" },
                                { GIBBON_CLIP_TYPE_INT, "0" },
                                { GIBBON_CLIP_TYPE_INT, "0" },
                                { GIBBON_CLIP_TYPE_INT, "0" },
                                { GIBBON_CLIP_TYPE_INT, "0" },
                                { GIBBON_CLIP_TYPE_INT, "0" },
                                { GIBBON_CLIP_TYPE_INT, "-2" },
                                { GIBBON_CLIP_TYPE_INT, "0" },
                                { GIBBON_CLIP_TYPE_INT, "-2" },
                                { GIBBON_CLIP_TYPE_INT, "-6" },
                                /* Turn.  */
                                { GIBBON_CLIP_TYPE_INT, "-1" },
                                /* Dice.  */
                                { GIBBON_CLIP_TYPE_INT, "-6" },
                                { GIBBON_CLIP_TYPE_INT, "-2" },
                                /* Cube.  */
                                { GIBBON_CLIP_TYPE_UINT, "1" },
                                /* Player and opponent may double? */
                                { GIBBON_CLIP_TYPE_BOOLEAN, "TRUE" },
                                { GIBBON_CLIP_TYPE_BOOLEAN, "TRUE" },
                                /* Playing direction.  */
                                { GIBBON_CLIP_TYPE_BOOLEAN, "FALSE" },
                                /* Player's and opponent's bar.  */
                                { GIBBON_CLIP_TYPE_UINT, "0" },
                                { GIBBON_CLIP_TYPE_UINT, "0" },
                                /* Post-Crawford? */
                                { GIBBON_CLIP_TYPE_BOOLEAN, "FALSE" },
                                { GIBBON_CLIP_TYPE_END, NULL }
                }
};

static struct test_case test_board02 =  {
                "board:joe_white:black_jack:2:1:1:0:0:0:-1:3:3:2:0:3:0:0:2:0:2"
                ":0:0:0:-1:-1:-2:-2:-2:-4:-2:0:0:1:5:1:0:0:2:0:1:0:1:-1:0:25:0"
                ":0:0:0:2:0:1:0",
                {
                                /* Board.  */
                                { GIBBON_CLIP_TYPE_UINT, "200" },
                                /* Player and opponent.  */
                                { GIBBON_CLIP_TYPE_NAME, "joe_white" },
                                { GIBBON_CLIP_TYPE_NAME, "black_jack" },
                                /* Match length.  */
                                { GIBBON_CLIP_TYPE_UINT, "2" },
                                /* Score.  */
                                { GIBBON_CLIP_TYPE_UINT, "1" },
                                { GIBBON_CLIP_TYPE_UINT, "1" },
                                /* Position.  */
                                { GIBBON_CLIP_TYPE_INT, "0" },
                                { GIBBON_CLIP_TYPE_INT, "0" },
                                { GIBBON_CLIP_TYPE_INT, "-1" },
                                { GIBBON_CLIP_TYPE_INT, "3" },
                                { GIBBON_CLIP_TYPE_INT, "3" },
                                { GIBBON_CLIP_TYPE_INT, "2" },
                                { GIBBON_CLIP_TYPE_INT, "0" },
                                { GIBBON_CLIP_TYPE_INT, "3" },
                                { GIBBON_CLIP_TYPE_INT, "0" },
                                { GIBBON_CLIP_TYPE_INT, "0" },
                                { GIBBON_CLIP_TYPE_INT, "2" },
                                { GIBBON_CLIP_TYPE_INT, "0" },
                                { GIBBON_CLIP_TYPE_INT, "2" },
                                { GIBBON_CLIP_TYPE_INT, "0" },
                                { GIBBON_CLIP_TYPE_INT, "0" },
                                { GIBBON_CLIP_TYPE_INT, "0" },
                                { GIBBON_CLIP_TYPE_INT, "-1" },
                                { GIBBON_CLIP_TYPE_INT, "-1" },
                                { GIBBON_CLIP_TYPE_INT, "-2" },
                                { GIBBON_CLIP_TYPE_INT, "-2" },
                                { GIBBON_CLIP_TYPE_INT, "-2" },
                                { GIBBON_CLIP_TYPE_INT, "-4" },
                                { GIBBON_CLIP_TYPE_INT, "-2" },
                                { GIBBON_CLIP_TYPE_INT, "0" },
                                /* Turn.  */
                                { GIBBON_CLIP_TYPE_INT, "1" },
                                /* Dice.  */
                                { GIBBON_CLIP_TYPE_INT, "5" },
                                { GIBBON_CLIP_TYPE_INT, "1" },
                                /* Cube.  */
                                { GIBBON_CLIP_TYPE_UINT, "2" },
                                /* Player and opponent may double? */
                                { GIBBON_CLIP_TYPE_BOOLEAN, "FALSE" },
                                { GIBBON_CLIP_TYPE_BOOLEAN, "TRUE" },
                                /* Playing direction.  */
                                { GIBBON_CLIP_TYPE_BOOLEAN, "TRUE" },
                                /* Player's and opponent's bar.  */
                                { GIBBON_CLIP_TYPE_UINT, "0" },
                                { GIBBON_CLIP_TYPE_UINT, "0" },
                                /* Post-Crawford? */
                                { GIBBON_CLIP_TYPE_BOOLEAN, "TRUE" },
                                { GIBBON_CLIP_TYPE_END, NULL }
                }
};

static struct test_case test_bad_board00 =  {
                "board:You:someplayer:3:0:0:0:-2:0:0:0:0:5:0:3:0:0:0:-5:5:0:0"
                ":0:-3:0:-5:0:0:0:0:2:0:1:6:2:0:0:1:1:1:0:1:-1:0:25:0:0:0:0:2"
                ":0:0:05 anotherplayer - - 0 0 1439.79 1262 410 1041251697"
                " somehost.com - -",
                {
                                /* Bad Board.  */
                                { GIBBON_CLIP_TYPE_UINT, "201" },
                                { GIBBON_CLIP_TYPE_STRING, "board:You:"
                                                "someplayer:3:0:0:0:-2:0:0:0:0"
                                                ":5:0:3:0:0:0:-5:5:0:0:0:-3:0"
                                                ":-5:0:0:0:0:2:0:1:6:2:0:0:1"
                                                ":1:1:0:1:-1:0:25:0:0:0:0:2:0"
                                                ":0:0" },
                                { GIBBON_CLIP_TYPE_STRING, "5 anotherplayer "
                                                "- - 0 0 1439.79 1262 410 "
                                                "1041251697 somehost.com - -" },
                                { GIBBON_CLIP_TYPE_END, NULL }
                }
};

static struct test_case test_bad_board01 =  {
                "board:You:someplayer:3:0:0:0:-2:0:0:0:0:5:0:3:0:0:0:-5:5:0:0"
                ":0:-3:0:-5:0:0:0:0:2:0:1:6:2:0:0:1:1:1:0:1:-1:0:25:0:0:0:0:2"
                ":0:0:15 anotherplayer - - 0 0 1439.79 1262 410 1041251697"
                " somehost.com - -",
                {
                                /* Bad Board.  */
                                { GIBBON_CLIP_TYPE_UINT, "201" },
                                { GIBBON_CLIP_TYPE_STRING, "board:You:"
                                                "someplayer:3:0:0:0:-2:0:0:0:0"
                                                ":5:0:3:0:0:0:-5:5:0:0:0:-3:0"
                                                ":-5:0:0:0:0:2:0:1:6:2:0:0:1"
                                                ":1:1:0:1:-1:0:25:0:0:0:0:2:0"
                                                ":0:1" },
                                { GIBBON_CLIP_TYPE_STRING, "5 anotherplayer "
                                                "- - 0 0 1439.79 1262 410 "
                                                "1041251697 somehost.com - -" },
                                { GIBBON_CLIP_TYPE_END, NULL }
                }
};

static struct test_case test_bad_board02 =  {
                "board:You:someplayer:3:0:0:0:-2:0:0:0:0:5:0:3:0:0:0:-5:5:0:0"
                ":0:-3:0:-5:0:0:0:0:2:0:1:6:2:0:0:1:1:1:0:1:-1:0:25:0:0:0:0:2"
                ":0:0:1Some arbitrary other message",
                {
                                /* Bad Board.  */
                                { GIBBON_CLIP_TYPE_UINT, "201" },
                                { GIBBON_CLIP_TYPE_STRING, "board:You:"
                                                "someplayer:3:0:0:0:-2:0:0:0:0"
                                                ":5:0:3:0:0:0:-5:5:0:0:0:-3:0"
                                                ":-5:0:0:0:0:2:0:1:6:2:0:0:1"
                                                ":1:1:0:1:-1:0:25:0:0:0:0:2:0"
                                                ":0:1" },
                                { GIBBON_CLIP_TYPE_STRING, "Some arbitrary"
                                                " other message" },
                                { GIBBON_CLIP_TYPE_END, NULL }
                }
};

static struct test_case test_rolls00 = {
                "gflohr rolls 3 and 1.",
                {
                                { GIBBON_CLIP_TYPE_UINT, "202" },
                                { GIBBON_CLIP_TYPE_NAME, "gflohr" },
                                { GIBBON_CLIP_TYPE_UINT, "3" },
                                { GIBBON_CLIP_TYPE_UINT, "1" },
                                { GIBBON_CLIP_TYPE_END, NULL }
                }
};

static struct test_case test_rolls01 = {
                "You roll 6 and 4.",
                {
                                { GIBBON_CLIP_TYPE_UINT, "202" },
                                { GIBBON_CLIP_TYPE_NAME, "You" },
                                { GIBBON_CLIP_TYPE_UINT, "6" },
                                { GIBBON_CLIP_TYPE_UINT, "4" },
                                { GIBBON_CLIP_TYPE_END, NULL }
                }
};

static struct test_case test_moves00 = {
                "gflohr moves 8-5 6-5 .",
                {
                                { GIBBON_CLIP_TYPE_UINT, "203" },
                                { GIBBON_CLIP_TYPE_NAME, "gflohr" },
                                { GIBBON_CLIP_TYPE_UINT, "2" },
                                { GIBBON_CLIP_TYPE_UINT, "8" },
                                { GIBBON_CLIP_TYPE_UINT, "5" },
                                { GIBBON_CLIP_TYPE_UINT, "6" },
                                { GIBBON_CLIP_TYPE_UINT, "5" },
                                { GIBBON_CLIP_TYPE_END, NULL }
                }
};

static struct test_case test_moves01 = {
                "gflohr moves 17-20 19-20 .",
                {
                                { GIBBON_CLIP_TYPE_UINT, "203" },
                                { GIBBON_CLIP_TYPE_NAME, "gflohr" },
                                { GIBBON_CLIP_TYPE_UINT, "2" },
                                { GIBBON_CLIP_TYPE_UINT, "17" },
                                { GIBBON_CLIP_TYPE_UINT, "20" },
                                { GIBBON_CLIP_TYPE_UINT, "19" },
                                { GIBBON_CLIP_TYPE_UINT, "20" },
                                { GIBBON_CLIP_TYPE_END, NULL }
                }
};

static struct test_case test_moves02 = {
                "gflohr moves 17-20 17-20 19-20 19-20 .",
                {
                                { GIBBON_CLIP_TYPE_UINT, "203" },
                                { GIBBON_CLIP_TYPE_NAME, "gflohr" },
                                { GIBBON_CLIP_TYPE_UINT, "4" },
                                { GIBBON_CLIP_TYPE_UINT, "17" },
                                { GIBBON_CLIP_TYPE_UINT, "20" },
                                { GIBBON_CLIP_TYPE_UINT, "17" },
                                { GIBBON_CLIP_TYPE_UINT, "20" },
                                { GIBBON_CLIP_TYPE_UINT, "19" },
                                { GIBBON_CLIP_TYPE_UINT, "20" },
                                { GIBBON_CLIP_TYPE_UINT, "19" },
                                { GIBBON_CLIP_TYPE_UINT, "20" },
                                { GIBBON_CLIP_TYPE_END, NULL }
                }
};

static struct test_case test_moves03 = {
                "gflohr moves bar-24 bar-22 .",
                {
                                { GIBBON_CLIP_TYPE_UINT, "203" },
                                { GIBBON_CLIP_TYPE_NAME, "gflohr" },
                                { GIBBON_CLIP_TYPE_UINT, "2" },
                                { GIBBON_CLIP_TYPE_UINT, "25" },
                                { GIBBON_CLIP_TYPE_UINT, "24" },
                                { GIBBON_CLIP_TYPE_UINT, "25" },
                                { GIBBON_CLIP_TYPE_UINT, "22" },
                                { GIBBON_CLIP_TYPE_END, NULL }
                }
};

static struct test_case test_moves04 = {
                "gflohr moves bar-1 bar-3 .",
                {
                                { GIBBON_CLIP_TYPE_UINT, "203" },
                                { GIBBON_CLIP_TYPE_NAME, "gflohr" },
                                { GIBBON_CLIP_TYPE_UINT, "2" },
                                { GIBBON_CLIP_TYPE_UINT, "0" },
                                { GIBBON_CLIP_TYPE_UINT, "1" },
                                { GIBBON_CLIP_TYPE_UINT, "0" },
                                { GIBBON_CLIP_TYPE_UINT, "3" },
                                { GIBBON_CLIP_TYPE_END, NULL }
                }
};

static struct test_case test_moves05 = {
                "gflohr moves 24-off 22-off .",
                {
                                { GIBBON_CLIP_TYPE_UINT, "203" },
                                { GIBBON_CLIP_TYPE_NAME, "gflohr" },
                                { GIBBON_CLIP_TYPE_UINT, "2" },
                                { GIBBON_CLIP_TYPE_UINT, "24" },
                                { GIBBON_CLIP_TYPE_UINT, "25" },
                                { GIBBON_CLIP_TYPE_UINT, "22" },
                                { GIBBON_CLIP_TYPE_UINT, "25" },
                                { GIBBON_CLIP_TYPE_END, NULL }
                }
};

static struct test_case test_moves06 = {
                "gflohr moves 1-off 3-off .",
                {
                                { GIBBON_CLIP_TYPE_UINT, "203" },
                                { GIBBON_CLIP_TYPE_NAME, "gflohr" },
                                { GIBBON_CLIP_TYPE_UINT, "2" },
                                { GIBBON_CLIP_TYPE_UINT, "1" },
                                { GIBBON_CLIP_TYPE_UINT, "0" },
                                { GIBBON_CLIP_TYPE_UINT, "3" },
                                { GIBBON_CLIP_TYPE_UINT, "0" },
                                { GIBBON_CLIP_TYPE_END, NULL }
                }
};

static struct test_case test_moves07 = {
                "GibbonTestB moves 2-off",
                {
                                { GIBBON_CLIP_TYPE_UINT, "203" },
                                { GIBBON_CLIP_TYPE_NAME, "GibbonTestB" },
                                { GIBBON_CLIP_TYPE_UINT, "1" },
                                { GIBBON_CLIP_TYPE_UINT, "2" },
                                { GIBBON_CLIP_TYPE_UINT, "0" },
                                { GIBBON_CLIP_TYPE_END, NULL }
                }
};

static struct test_case test_game_start00 = {
                "Starting a new game with gflohr.",
                {
                                { GIBBON_CLIP_TYPE_UINT, "204" },
                                { GIBBON_CLIP_TYPE_NAME, "gflohr" },
                                { GIBBON_CLIP_TYPE_END, NULL }
                }
};

static struct test_case test_game_left = {
                "** You terminated the game. The game was saved.",
                {
                                { GIBBON_CLIP_TYPE_UINT, "205" },
                                { GIBBON_CLIP_TYPE_END, NULL }
                }
};

static struct test_case test_cannot_move00 = {
                "You can't move.",
                {
                                { GIBBON_CLIP_TYPE_UINT, "206" },
                                { GIBBON_CLIP_TYPE_NAME, "You" },
                                { GIBBON_CLIP_TYPE_END, NULL }
                }
};

static struct test_case test_cannot_move01 = {
                "GibbonTestA can't move.",
                {
                                { GIBBON_CLIP_TYPE_UINT, "206" },
                                { GIBBON_CLIP_TYPE_NAME, "GibbonTestA" },
                                { GIBBON_CLIP_TYPE_END, NULL }
                }
};

static struct test_case test_doubling00 = {
                "You double. Please wait for GibbonTestA to accept or reject.",
                {
                                { GIBBON_CLIP_TYPE_UINT, "207" },
                                { GIBBON_CLIP_TYPE_NAME, "You" },
                                { GIBBON_CLIP_TYPE_END, NULL }
                }
};

static struct test_case test_doubling01 = {
                "GibbonTestA doubles. Type 'accept' or 'reject'.",
                {
                                { GIBBON_CLIP_TYPE_UINT, "207" },
                                { GIBBON_CLIP_TYPE_NAME, "GibbonTestA" },
                                { GIBBON_CLIP_TYPE_END, NULL }
                }
};

static struct test_case test_doubling02 = {
                "You accept the double.  The cube shows 2.",
                {
                                { GIBBON_CLIP_TYPE_UINT, "208" },
                                { GIBBON_CLIP_TYPE_NAME, "You" },
                                { GIBBON_CLIP_TYPE_UINT, "2" },
                                { GIBBON_CLIP_TYPE_END, NULL }
                }
};

static struct test_case test_doubling03 = {
                "GibbonTestA accepts the double.  The cube shows 4.",
                {
                                { GIBBON_CLIP_TYPE_UINT, "208" },
                                { GIBBON_CLIP_TYPE_NAME, "GibbonTestA" },
                                { GIBBON_CLIP_TYPE_UINT, "4" },
                                { GIBBON_CLIP_TYPE_END, NULL }
                }
};

static struct test_case test_resign00 = {
                "GibbonTestA wants to resign. You will win 1 point."
                " Type 'accept' or 'reject'.",
                {
                                { GIBBON_CLIP_TYPE_UINT, "209" },
                                { GIBBON_CLIP_TYPE_NAME, "GibbonTestA" },
                                { GIBBON_CLIP_TYPE_UINT, "1" },
                                { GIBBON_CLIP_TYPE_END, NULL }
                }
};

static struct test_case test_resign01 = {
                "GibbonTestA wants to resign. You will win 6 points."
                " Type 'accept' or 'reject'.",
                {
                                { GIBBON_CLIP_TYPE_UINT, "209" },
                                { GIBBON_CLIP_TYPE_NAME, "GibbonTestA" },
                                { GIBBON_CLIP_TYPE_UINT, "6" },
                                { GIBBON_CLIP_TYPE_END, NULL }
                }
};

static struct test_case test_resign02 = {
                "You want to resign. GibbonTestA will win 1 point."
                " Type 'accept' or 'reject'.",
                {
                                { GIBBON_CLIP_TYPE_UINT, "209" },
                                { GIBBON_CLIP_TYPE_NAME, "You" },
                                { GIBBON_CLIP_TYPE_UINT, "1" },
                                { GIBBON_CLIP_TYPE_END, NULL }
                }
};

static struct test_case test_resign03 = {
                "You want to resign. GibbonTestA will win 4 points."
                " Type 'accept' or 'reject'.",
                {
                                { GIBBON_CLIP_TYPE_UINT, "209" },
                                { GIBBON_CLIP_TYPE_NAME, "You" },
                                { GIBBON_CLIP_TYPE_UINT, "4" },
                                { GIBBON_CLIP_TYPE_END, NULL }
                }
};

static struct test_case test_resign04 = {
                "You reject. The game continues.",
                {
                                { GIBBON_CLIP_TYPE_UINT, "210" },                                { GIBBON_CLIP_TYPE_NAME, "You" },
                                { GIBBON_CLIP_TYPE_END, NULL }
                }
};

static struct test_case test_resign05 = {
                "GibbonTestA rejects. The game continues.",
                {
                                { GIBBON_CLIP_TYPE_UINT, "210" },                                { GIBBON_CLIP_TYPE_NAME, "You" },
                                { GIBBON_CLIP_TYPE_END, NULL }
                }
};

static struct test_case test_between00 = {
                "GibbonTestA wants to play a 5 point match with you.",
                {
                                { GIBBON_CLIP_TYPE_UINT, "300" },
                                { GIBBON_CLIP_TYPE_NAME, "GibbonTestA" },
                                { GIBBON_CLIP_TYPE_INT, "5" },
                                { GIBBON_CLIP_TYPE_END, NULL }
                }
};

static struct test_case test_between01 = {
                "GibbonTestA wants to play an unlimited match with you.",
                {
                                { GIBBON_CLIP_TYPE_UINT, "300" },
                                { GIBBON_CLIP_TYPE_NAME, "GibbonTestA" },
                                { GIBBON_CLIP_TYPE_INT, "0" },
                                { GIBBON_CLIP_TYPE_END, NULL }
                }
};

static struct test_case test_between02 = {
                "GibbonTestA wants to resume a saved match with you.",
                {
                                { GIBBON_CLIP_TYPE_UINT, "300" },
                                { GIBBON_CLIP_TYPE_NAME, "GibbonTestA" },
                                { GIBBON_CLIP_TYPE_INT, "-1" },
                                { GIBBON_CLIP_TYPE_END, NULL }
                }
};

static struct test_case test_between03 = {
                "Type 'join GibbonTestA' to accept.",
                {
                                { GIBBON_CLIP_TYPE_UINT, "301" },
                                { GIBBON_CLIP_TYPE_NAME, "GibbonTestA" },
                                { GIBBON_CLIP_TYPE_END, NULL }
                }
};

static struct test_case test_between04 = {
                "You're now watching gflohr.",
                {
                                { GIBBON_CLIP_TYPE_UINT, "302" },
                                { GIBBON_CLIP_TYPE_NAME, "gflohr" },
                                { GIBBON_CLIP_TYPE_END, NULL }
                }
};

static struct test_case test_between05 = {
                "** You are now playing a 5 point match with gflohr",
                {
                                { GIBBON_CLIP_TYPE_UINT, "303" },
                                { GIBBON_CLIP_TYPE_NAME, "gflohr" },
                                { GIBBON_CLIP_TYPE_UINT, "5" },
                                { GIBBON_CLIP_TYPE_END, NULL }
                }
};

static struct test_case test_between06 = {
                "** You are now playing an unlimited match with gflohr",
                {
                                { GIBBON_CLIP_TYPE_UINT, "303" },
                                { GIBBON_CLIP_TYPE_NAME, "gflohr" },
                                { GIBBON_CLIP_TYPE_UINT, "0" },
                                { GIBBON_CLIP_TYPE_END, NULL }
                }
};

static struct test_case test_between07 = {
                "** gflohr is refusing games.",
                {
                                { GIBBON_CLIP_TYPE_UINT, "304" },
                                { GIBBON_CLIP_TYPE_NAME, "gflohr" },
                                { GIBBON_CLIP_TYPE_STRING,
                                  "Player `gflohr' is now refusing matches!"},
                                { GIBBON_CLIP_TYPE_END, NULL }
                }
};

static struct test_case test_between08 = {
                "** Error: gflohr is already playing with someone else.",
                {
                                { GIBBON_CLIP_TYPE_UINT, "304" },
                                { GIBBON_CLIP_TYPE_NAME, "gflohr" },
                                { GIBBON_CLIP_TYPE_STRING,
                                  "Player `gflohr' is already playing with"
                                  " someone else!"},
                                { GIBBON_CLIP_TYPE_END, NULL }
                }
};

static struct test_case test_between09 = {
                "** gflohr didn't invite you.",
                {
                                { GIBBON_CLIP_TYPE_UINT, "304" },
                                { GIBBON_CLIP_TYPE_NAME, "gflohr" },
                                { GIBBON_CLIP_TYPE_STRING,
                                  "Player `gflohr' is already playing with"
                                  " someone else!"},
                                { GIBBON_CLIP_TYPE_END, NULL }
                }
};

static struct test_case test_between10 = {
                "** Error: can't find player gflohr",
                {
                                { GIBBON_CLIP_TYPE_UINT, "304" },
                                { GIBBON_CLIP_TYPE_NAME, "gflohr" },
                                { GIBBON_CLIP_TYPE_STRING,
                                  "Player `gflohr' is not logged in!" },
                                { GIBBON_CLIP_TYPE_END, NULL }
                }
};

static struct test_case test_between11 = {
                "** You can't play two games at once",
                {
                                { GIBBON_CLIP_TYPE_UINT, "101" },
                                { GIBBON_CLIP_TYPE_STRING,
                                  "You cannot play two matches at once!"},
                                { GIBBON_CLIP_TYPE_END, NULL }
                }
};

static struct test_case test_between12 = {
                "** Player gflohr has joined you for a 5 point match.",
                {
                                { GIBBON_CLIP_TYPE_UINT, "303" },
                                { GIBBON_CLIP_TYPE_NAME, "gflohr" },
                                { GIBBON_CLIP_TYPE_UINT, "5" },
                                { GIBBON_CLIP_TYPE_END, NULL }
                }
};

static struct test_case test_between13 = {
                "Player gflohr has joined you for an unlimited match.",
                {
                                { GIBBON_CLIP_TYPE_UINT, "303" },
                                { GIBBON_CLIP_TYPE_NAME, "gflohr" },
                                { GIBBON_CLIP_TYPE_UINT, "0" },
                                { GIBBON_CLIP_TYPE_END, NULL }
                }
};

static struct test_case test_resume00 = {
                "You are now playing with gflohr."
                " Your running match was loaded.",
                {
                                { GIBBON_CLIP_TYPE_UINT, "305" },
                                { GIBBON_CLIP_TYPE_NAME, "gflohr" },
                                { GIBBON_CLIP_TYPE_END, NULL }
                }
};

static struct test_case test_resume01 = {
                "gflohr has joined you."
                " Your running match was loaded.",
                {
                                { GIBBON_CLIP_TYPE_UINT, "305" },
                                { GIBBON_CLIP_TYPE_NAME, "gflohr" },
                                { GIBBON_CLIP_TYPE_END, NULL }
                }
};

static struct test_case test_resume02 = {
                "turn: gflohr",
                {
                                { GIBBON_CLIP_TYPE_UINT, "306" },
                                { GIBBON_CLIP_TYPE_NAME, "gflohr" },
                                { GIBBON_CLIP_TYPE_END, NULL }
                }
};

static struct test_case test_resume03 = {
                "match length: 42",
                {
                                { GIBBON_CLIP_TYPE_UINT, "307" },
                                { GIBBON_CLIP_TYPE_UINT, "42" },
                                { GIBBON_CLIP_TYPE_END, NULL }
                }
};

static struct test_case test_resume04 = {
                "unlimited match",
                {
                                { GIBBON_CLIP_TYPE_UINT, "307" },
                                { GIBBON_CLIP_TYPE_UINT, "0" },
                                { GIBBON_CLIP_TYPE_END, NULL }
                }
};

static struct test_case test_resume05 = {
                "points for user gflohr: 3",
                {
                                { GIBBON_CLIP_TYPE_UINT, "308" },
                                { GIBBON_CLIP_TYPE_NAME, "gflohr" },
                                { GIBBON_CLIP_TYPE_UINT, "3" },
                                { GIBBON_CLIP_TYPE_END, NULL }
                }
};

static struct test_case test_between14 = {
                "Type 'join' if you want to play the next game,"
                " type 'leave' if you don't.",
                {
                                { GIBBON_CLIP_TYPE_UINT, "309" },
                                { GIBBON_CLIP_TYPE_END, NULL }
                }
};

static struct test_case test_between15 = {
                "You win the game and get 1 point. Congratulations!",
                {
                                { GIBBON_CLIP_TYPE_UINT, "310" },
                                { GIBBON_CLIP_TYPE_NAME, "You" },
                                { GIBBON_CLIP_TYPE_UINT, "1" },
                                { GIBBON_CLIP_TYPE_END, NULL }
                }
};

static struct test_case test_between16 = {
                "You win the game and get 4 points. Congratulations!",
                {
                                { GIBBON_CLIP_TYPE_UINT, "310" },
                                { GIBBON_CLIP_TYPE_NAME, "You" },
                                { GIBBON_CLIP_TYPE_UINT, "4" },
                                { GIBBON_CLIP_TYPE_END, NULL }
                }
};

static struct test_case test_between17 = {
                "GibbonTestA wins the game and gets 1 point. Sorry.",
                {
                                { GIBBON_CLIP_TYPE_UINT, "310" },
                                { GIBBON_CLIP_TYPE_NAME, "GibbonTestA" },
                                { GIBBON_CLIP_TYPE_UINT, "1" },
                                { GIBBON_CLIP_TYPE_END, NULL }
                }
};

static struct test_case test_between18 = {
                "GibbonTestA wins the game and gets 4 points. Sorry.",
                {
                                { GIBBON_CLIP_TYPE_UINT, "310" },
                                { GIBBON_CLIP_TYPE_NAME, "GibbonTestA" },
                                { GIBBON_CLIP_TYPE_UINT, "4" },
                                { GIBBON_CLIP_TYPE_END, NULL }
                }
};

static struct test_case test_between19 = {
                "GibbonTestA gives up. You win 1 point.",
                {
                                { GIBBON_CLIP_TYPE_UINT, "310" },
                                { GIBBON_CLIP_TYPE_NAME, "You" },
                                { GIBBON_CLIP_TYPE_UINT, "1" },
                                { GIBBON_CLIP_TYPE_END, NULL }
                }
};

static struct test_case test_between20 = {
                "GibbonTestA gives up. You win 2 points.",
                {
                                { GIBBON_CLIP_TYPE_UINT, "310" },
                                { GIBBON_CLIP_TYPE_NAME, "You" },
                                { GIBBON_CLIP_TYPE_UINT, "2" },
                                { GIBBON_CLIP_TYPE_END, NULL }
                }
};

static struct test_case test_between21 = {
                "You give up. GibbonTestA wins 1 point.",
                {
                                { GIBBON_CLIP_TYPE_UINT, "310" },
                                { GIBBON_CLIP_TYPE_NAME, "GibbonTestA" },
                                { GIBBON_CLIP_TYPE_UINT, "1" },
                                { GIBBON_CLIP_TYPE_END, NULL }
                }
};

static struct test_case test_between22 = {
                "You give up. GibbonTestA wins 2 points.",
                {
                                { GIBBON_CLIP_TYPE_UINT, "310" },
                                { GIBBON_CLIP_TYPE_NAME, "GibbonTestA" },
                                { GIBBON_CLIP_TYPE_UINT, "2" },
                                { GIBBON_CLIP_TYPE_END, NULL }
                }
};

static struct test_case test_between23 = {
                "You accept and win 1 point.",
                {
                                { GIBBON_CLIP_TYPE_UINT, "310" },
                                { GIBBON_CLIP_TYPE_NAME, "You" },
                                { GIBBON_CLIP_TYPE_UINT, "1" },
                                { GIBBON_CLIP_TYPE_END, NULL }
                }
};

static struct test_case test_between24 = {
                "You accept and win 2 points.",
                {
                                { GIBBON_CLIP_TYPE_UINT, "310" },
                                { GIBBON_CLIP_TYPE_NAME, "You" },
                                { GIBBON_CLIP_TYPE_UINT, "2" },
                                { GIBBON_CLIP_TYPE_END, NULL }
                }
};

static struct test_case test_between25 = {
                "GibbonTestA accepts and wins 1 point.",
                {
                                { GIBBON_CLIP_TYPE_UINT, "310" },
                                { GIBBON_CLIP_TYPE_NAME, "GibbonTestA" },
                                { GIBBON_CLIP_TYPE_UINT, "1" },
                                { GIBBON_CLIP_TYPE_END, NULL }
                }
};

static struct test_case test_between26 = {
                "GibbonTestA accepts and wins 2 points.",
                {
                                { GIBBON_CLIP_TYPE_UINT, "310" },
                                { GIBBON_CLIP_TYPE_NAME, "GibbonTestA" },
                                { GIBBON_CLIP_TYPE_UINT, "2" },
                                { GIBBON_CLIP_TYPE_END, NULL }
                }
};

static struct test_case test_between27 = {
                "GibbonTestA wins the game and gets 1 point.",
                {
                                { GIBBON_CLIP_TYPE_UINT, "310" },
                                { GIBBON_CLIP_TYPE_NAME, "GibbonTestA" },
                                { GIBBON_CLIP_TYPE_UINT, "1" },
                                { GIBBON_CLIP_TYPE_END, NULL }
                }
};

static struct test_case test_between28 = {
                "GibbonTestA wins the game and gets 4 points.",
                {
                                { GIBBON_CLIP_TYPE_UINT, "310" },
                                { GIBBON_CLIP_TYPE_NAME, "GibbonTestA" },
                                { GIBBON_CLIP_TYPE_UINT, "4" },
                                { GIBBON_CLIP_TYPE_END, NULL }
                }
};

static struct test_case test_between29 = {
                "GibbonTestB gives up. GibbonTestA wins 1 point.",
                {
                                { GIBBON_CLIP_TYPE_UINT, "310" },
                                { GIBBON_CLIP_TYPE_NAME, "GibbonTestA" },
                                { GIBBON_CLIP_TYPE_UINT, "1" },
                                { GIBBON_CLIP_TYPE_END, NULL }
                }
};

static struct test_case test_between30 = {
                "GibbonTestB gives up. GibbonTestA win 2 points.",
                {
                                { GIBBON_CLIP_TYPE_UINT, "310" },
                                { GIBBON_CLIP_TYPE_NAME, "GibbonTestA" },
                                { GIBBON_CLIP_TYPE_UINT, "2" },
                                { GIBBON_CLIP_TYPE_END, NULL }
                }
};

static struct test_case test_between31 = {
                "score in 7 point match: GibbonTestA-3 GibbonTestB-5",
                {
                                { GIBBON_CLIP_TYPE_UINT, "311" },
                                { GIBBON_CLIP_TYPE_UINT, "7" },
                                { GIBBON_CLIP_TYPE_NAME, "GibbonTestA" },
                                { GIBBON_CLIP_TYPE_UINT, "3" },
                                { GIBBON_CLIP_TYPE_NAME, "GibbonTestB" },
                                { GIBBON_CLIP_TYPE_UINT, "5" },
                                { GIBBON_CLIP_TYPE_END, NULL }
                }
};

static struct test_case test_between32 = {
                "score in unlimited match: GibbonTestA-3 GibbonTestB-5",
                {
                                { GIBBON_CLIP_TYPE_UINT, "311" },
                                { GIBBON_CLIP_TYPE_UINT, "0" },
                                { GIBBON_CLIP_TYPE_NAME, "GibbonTestA" },
                                { GIBBON_CLIP_TYPE_UINT, "3" },
                                { GIBBON_CLIP_TYPE_NAME, "GibbonTestB" },
                                { GIBBON_CLIP_TYPE_UINT, "5" },
                                { GIBBON_CLIP_TYPE_END, NULL }
                }
};

static struct test_case test_various00 = {
                "gflohr and GibbonTestA start a 7 point match.",
                {
                                { GIBBON_CLIP_TYPE_UINT, "400" },
                                { GIBBON_CLIP_TYPE_NAME, "gflohr" },
                                { GIBBON_CLIP_TYPE_NAME, "GibbonTestA" },
                                { GIBBON_CLIP_TYPE_UINT, "7" },
                                { GIBBON_CLIP_TYPE_END, NULL }
                }
};

static struct test_case test_various01 = {
                "gflohr and GibbonTestA start an unlimited match.",
                {
                                { GIBBON_CLIP_TYPE_UINT, "400" },
                                { GIBBON_CLIP_TYPE_NAME, "gflohr" },
                                { GIBBON_CLIP_TYPE_NAME, "GibbonTestA" },
                                { GIBBON_CLIP_TYPE_UINT, "0" },
                                { GIBBON_CLIP_TYPE_END, NULL }
                }
};

static struct test_case test_various02 = {
                "gflohr wins a 7 point match against GibbonTestA  8-3 .",
                {
                                { GIBBON_CLIP_TYPE_UINT, "401" },
                                { GIBBON_CLIP_TYPE_NAME, "gflohr" },
                                { GIBBON_CLIP_TYPE_NAME, "GibbonTestA" },
                                { GIBBON_CLIP_TYPE_UINT, "7" },
                                { GIBBON_CLIP_TYPE_UINT, "8" },
                                { GIBBON_CLIP_TYPE_UINT, "3" },
                                { GIBBON_CLIP_TYPE_END, NULL }
                }
};

static struct test_case test_various02a = {
                "gflohr wins a 1 point match against GibbonTestA  1-0 .",
                {
                                { GIBBON_CLIP_TYPE_UINT, "401" },
                                { GIBBON_CLIP_TYPE_NAME, "gflohr" },
                                { GIBBON_CLIP_TYPE_NAME, "GibbonTestA" },
                                { GIBBON_CLIP_TYPE_UINT, "1" },
                                { GIBBON_CLIP_TYPE_UINT, "1" },
                                { GIBBON_CLIP_TYPE_UINT, "0" },
                                { GIBBON_CLIP_TYPE_END, NULL }
                }
};

static struct test_case test_various03 = {
                "gflohr and GibbonTestA are resuming their 7-point match.",
                {
                                { GIBBON_CLIP_TYPE_UINT, "402" },
                                { GIBBON_CLIP_TYPE_NAME, "gflohr" },
                                { GIBBON_CLIP_TYPE_NAME, "GibbonTestA" },
                                { GIBBON_CLIP_TYPE_UINT, "7" },
                                { GIBBON_CLIP_TYPE_END, NULL }
                }
};

static struct test_case test_various04 = {
                "gflohr and GibbonTestA are resuming their unlimited match.",
                {
                                { GIBBON_CLIP_TYPE_UINT, "402" },
                                { GIBBON_CLIP_TYPE_NAME, "gflohr" },
                                { GIBBON_CLIP_TYPE_NAME, "GibbonTestA" },
                                { GIBBON_CLIP_TYPE_UINT, "0" },
                                { GIBBON_CLIP_TYPE_END, NULL }
                }
};

static struct test_case test_various05 = {
                "   ",
                {
                                { GIBBON_CLIP_TYPE_UINT, "403" },
                                { GIBBON_CLIP_TYPE_END, NULL }
                }
};

static struct test_case test_settings00 = {
                "Settings of variables:",
                {
                                { GIBBON_CLIP_TYPE_UINT, "404" },
                                { GIBBON_CLIP_TYPE_END, NULL }
                }
};

static struct test_case test_settings01 = {
                "boardstyle: 3",
                {
                                { GIBBON_CLIP_TYPE_UINT, "405" },
                                { GIBBON_CLIP_TYPE_STRING, "boardstyle" },
                                { GIBBON_CLIP_TYPE_STRING, "3" },
                                { GIBBON_CLIP_TYPE_END, NULL }
                }
};

static struct test_case test_settings02 = {
                "linelength: 0",
                {
                                { GIBBON_CLIP_TYPE_UINT, "405" },
                                { GIBBON_CLIP_TYPE_STRING, "linelength" },
                                { GIBBON_CLIP_TYPE_STRING, "0" },
                                { GIBBON_CLIP_TYPE_END, NULL }
                }
};

static struct test_case test_settings03 = {
                "pagelength: 0",
                {
                                { GIBBON_CLIP_TYPE_UINT, "405" },
                                { GIBBON_CLIP_TYPE_STRING, "pagelength" },
                                { GIBBON_CLIP_TYPE_STRING, "0" },
                                { GIBBON_CLIP_TYPE_END, NULL }
                }
};

static struct test_case test_settings04 = {
                "redoubles: none",
                {
                                { GIBBON_CLIP_TYPE_UINT, "405" },
                                { GIBBON_CLIP_TYPE_STRING, "redoubles" },
                                { GIBBON_CLIP_TYPE_STRING, "none" },
                                { GIBBON_CLIP_TYPE_END, NULL }
                }
};

static struct test_case test_settings05 = {
                "sortwho: login",
                {
                                { GIBBON_CLIP_TYPE_UINT, "405" },
                                { GIBBON_CLIP_TYPE_STRING, "sortwho" },
                                { GIBBON_CLIP_TYPE_STRING, "login" },
                                { GIBBON_CLIP_TYPE_END, NULL }
                }
};

static struct test_case test_settings06 = {
                "timezone: UTC",
                {
                                { GIBBON_CLIP_TYPE_UINT, "405" },
                                { GIBBON_CLIP_TYPE_STRING, "timezone" },
                                { GIBBON_CLIP_TYPE_STRING, "UTC" },
                                { GIBBON_CLIP_TYPE_END, NULL }
                }
};

static struct test_case test_settings07 = {
                "Value of 'boardstyle' set to 3.",
                {
                                { GIBBON_CLIP_TYPE_UINT, "405" },
                                { GIBBON_CLIP_TYPE_STRING, "boardstyle" },
                                { GIBBON_CLIP_TYPE_STRING, "3" },
                                { GIBBON_CLIP_TYPE_END, NULL }
                }
};

static struct test_case test_settings08 = {
                "Value of 'linelength' set to 0.",
                {
                                { GIBBON_CLIP_TYPE_UINT, "405" },
                                { GIBBON_CLIP_TYPE_STRING, "linelength" },
                                { GIBBON_CLIP_TYPE_STRING, "0" },
                                { GIBBON_CLIP_TYPE_END, NULL }
                }
};

static struct test_case test_settings09 = {
                "Value of 'pagelength' set to 0.",
                {
                                { GIBBON_CLIP_TYPE_UINT, "405" },
                                { GIBBON_CLIP_TYPE_STRING, "pagelength" },
                                { GIBBON_CLIP_TYPE_STRING, "0" },
                                { GIBBON_CLIP_TYPE_END, NULL }
                }
};

static struct test_case test_settings10 = {
                "Value of 'redoubles' set to none.",
                {
                                { GIBBON_CLIP_TYPE_UINT, "405" },
                                { GIBBON_CLIP_TYPE_STRING, "redoubles" },
                                { GIBBON_CLIP_TYPE_STRING, "none" },
                                { GIBBON_CLIP_TYPE_END, NULL }
                }
};

static struct test_case test_settings11 = {
                "Value of 'sortwho' set to login.",
                {
                                { GIBBON_CLIP_TYPE_UINT, "405" },
                                { GIBBON_CLIP_TYPE_STRING, "sortwho" },
                                { GIBBON_CLIP_TYPE_STRING, "login" },
                                { GIBBON_CLIP_TYPE_END, NULL }
                }
};

static struct test_case test_settings12 = {
                "Value of 'timezone' set to UTC.",
                {
                                { GIBBON_CLIP_TYPE_UINT, "405" },
                                { GIBBON_CLIP_TYPE_STRING, "timezone" },
                                { GIBBON_CLIP_TYPE_STRING, "UTC" },
                                { GIBBON_CLIP_TYPE_END, NULL }
                }
};

static struct test_case test_toggles00 = {
                "The current settings are:",
                {
                                { GIBBON_CLIP_TYPE_UINT, "406" },
                                { GIBBON_CLIP_TYPE_END, NULL }
                }
};

static struct test_case test_toggles01 = {
                "allowpip   YES",
                {
                                { GIBBON_CLIP_TYPE_UINT, "407" },
                                { GIBBON_CLIP_TYPE_STRING, "allowpip" },
                                { GIBBON_CLIP_TYPE_BOOLEAN, "TRUE" },
                                { GIBBON_CLIP_TYPE_END, NULL }
                }
};

static struct test_case test_toggles02 = {
                "autoboard   YES",
                {
                                { GIBBON_CLIP_TYPE_UINT, "407" },
                                { GIBBON_CLIP_TYPE_STRING, "autoboard" },
                                { GIBBON_CLIP_TYPE_BOOLEAN, "TRUE" },
                                { GIBBON_CLIP_TYPE_END, NULL }
                }
};

static struct test_case test_toggles03 = {
                "autodouble   NO",
                {
                                { GIBBON_CLIP_TYPE_UINT, "407" },
                                { GIBBON_CLIP_TYPE_STRING, "autodouble" },
                                { GIBBON_CLIP_TYPE_BOOLEAN, "FALSE" },
                                { GIBBON_CLIP_TYPE_END, NULL }
                }
};

static struct test_case test_toggles04 = {
                "automove   YES",
                {
                                { GIBBON_CLIP_TYPE_UINT, "407" },
                                { GIBBON_CLIP_TYPE_STRING, "automove" },
                                { GIBBON_CLIP_TYPE_BOOLEAN, "TRUE" },
                                { GIBBON_CLIP_TYPE_END, NULL }
                }
};

static struct test_case test_toggles05 = {
                "bell   NO",
                {
                                { GIBBON_CLIP_TYPE_UINT, "407" },
                                { GIBBON_CLIP_TYPE_STRING, "bell" },
                                { GIBBON_CLIP_TYPE_BOOLEAN, "FALSE" },
                                { GIBBON_CLIP_TYPE_END, NULL }
                }
};

static struct test_case test_toggles06 = {
                "crawford   YES",
                {
                                { GIBBON_CLIP_TYPE_UINT, "407" },
                                { GIBBON_CLIP_TYPE_STRING, "crawford" },
                                { GIBBON_CLIP_TYPE_BOOLEAN, "TRUE" },
                                { GIBBON_CLIP_TYPE_END, NULL }
                }
};

static struct test_case test_toggles07 = {
                "double   YES",
                {
                                { GIBBON_CLIP_TYPE_UINT, "407" },
                                { GIBBON_CLIP_TYPE_STRING, "double" },
                                { GIBBON_CLIP_TYPE_BOOLEAN, "TRUE" },
                                { GIBBON_CLIP_TYPE_END, NULL }
                }
};

static struct test_case test_toggles08 = {
                "greedy   NO",
                {
                                { GIBBON_CLIP_TYPE_UINT, "407" },
                                { GIBBON_CLIP_TYPE_STRING, "greedy" },
                                { GIBBON_CLIP_TYPE_BOOLEAN, "FALSE" },
                                { GIBBON_CLIP_TYPE_END, NULL }
                }
};

static struct test_case test_toggles09 = {
                "moreboards   YES",
                {
                                { GIBBON_CLIP_TYPE_UINT, "407" },
                                { GIBBON_CLIP_TYPE_STRING, "moreboards" },
                                { GIBBON_CLIP_TYPE_BOOLEAN, "TRUE" },
                                { GIBBON_CLIP_TYPE_END, NULL }
                }
};

static struct test_case test_toggles10 = {
                "moves   NO",
                {
                                { GIBBON_CLIP_TYPE_UINT, "407" },
                                { GIBBON_CLIP_TYPE_STRING, "moves" },
                                { GIBBON_CLIP_TYPE_BOOLEAN, "FALSE" },
                                { GIBBON_CLIP_TYPE_END, NULL }
                }
};

static struct test_case test_toggles11 = {
                "notify  YES",
                {
                                { GIBBON_CLIP_TYPE_UINT, "407" },
                                { GIBBON_CLIP_TYPE_STRING, "notify" },
                                { GIBBON_CLIP_TYPE_BOOLEAN, "TRUE" },
                                { GIBBON_CLIP_TYPE_END, NULL }
                }
};

static struct test_case test_toggles12 = {
                "ratings   NO",
                {
                                { GIBBON_CLIP_TYPE_UINT, "407" },
                                { GIBBON_CLIP_TYPE_STRING, "ratings" },
                                { GIBBON_CLIP_TYPE_BOOLEAN, "FALSE" },
                                { GIBBON_CLIP_TYPE_END, NULL }
                }
};

static struct test_case test_toggles13 = {
                "ready   NO",
                {
                                { GIBBON_CLIP_TYPE_UINT, "407" },
                                { GIBBON_CLIP_TYPE_STRING, "ready" },
                                { GIBBON_CLIP_TYPE_BOOLEAN, "FALSE" },
                                { GIBBON_CLIP_TYPE_END, NULL }
                }
};

static struct test_case test_toggles13a = {
                "** You're now ready to invite or join someone.",
                {
                                { GIBBON_CLIP_TYPE_UINT, "407" },
                                { GIBBON_CLIP_TYPE_STRING, "ready" },
                                { GIBBON_CLIP_TYPE_BOOLEAN, "TRUE" },
                                { GIBBON_CLIP_TYPE_END, NULL }
                }
};

static struct test_case test_toggles13b = {
                "** You're now refusing to play with someone.",
                {
                                { GIBBON_CLIP_TYPE_UINT, "407" },
                                { GIBBON_CLIP_TYPE_STRING, "ready" },
                                { GIBBON_CLIP_TYPE_BOOLEAN, "FALSE" },
                                { GIBBON_CLIP_TYPE_END, NULL }
                }
};

static struct test_case test_toggles14 = {
                "report   NO",
                {
                                { GIBBON_CLIP_TYPE_UINT, "407" },
                                { GIBBON_CLIP_TYPE_STRING, "report" },
                                { GIBBON_CLIP_TYPE_BOOLEAN, "FALSE" },
                                { GIBBON_CLIP_TYPE_END, NULL }
                }
};

static struct test_case test_toggles15 = {
                "silent   NO",
                {
                                { GIBBON_CLIP_TYPE_UINT, "407" },
                                { GIBBON_CLIP_TYPE_STRING, "silent" },
                                { GIBBON_CLIP_TYPE_BOOLEAN, "FALSE" },
                                { GIBBON_CLIP_TYPE_END, NULL }
                }
};

static struct test_case test_toggles16 = {
                "telnet  YES",
                {
                                { GIBBON_CLIP_TYPE_UINT, "407" },
                                { GIBBON_CLIP_TYPE_STRING, "telnet" },
                                { GIBBON_CLIP_TYPE_BOOLEAN, "TRUE" },
                                { GIBBON_CLIP_TYPE_END, NULL }
                }
};

static struct test_case test_toggles17 = {
                "wrap   NO",
                {
                                { GIBBON_CLIP_TYPE_UINT, "407" },
                                { GIBBON_CLIP_TYPE_STRING, "wrap" },
                                { GIBBON_CLIP_TYPE_BOOLEAN, "FALSE" },
                                { GIBBON_CLIP_TYPE_END, NULL }
                }
};

static struct test_case test_toggles18 = {
                "** You won't be notified when new users log in.",
                {
                                { GIBBON_CLIP_TYPE_UINT, "407" },
                                { GIBBON_CLIP_TYPE_STRING, "notify" },
                                { GIBBON_CLIP_TYPE_BOOLEAN, "FALSE" },
                                { GIBBON_CLIP_TYPE_END, NULL }
                }
};

static struct test_case test_toggles19 = {
                "** You'll be notified when new users log in.",
                {
                                { GIBBON_CLIP_TYPE_UINT, "407" },
                                { GIBBON_CLIP_TYPE_STRING, "notify" },
                                { GIBBON_CLIP_TYPE_BOOLEAN, "TRUE" },
                                { GIBBON_CLIP_TYPE_END, NULL }
                }
};

static struct test_case test_saved00 = {
        "  opponent          matchlength   score (your points first)",
        {
                                { GIBBON_CLIP_TYPE_UINT, "408" },
                                { GIBBON_CLIP_TYPE_END, NULL }
        }
};

static struct test_case test_saved01 = {
        "  GammonBot_VII           5                0 -  0",
        {
                                { GIBBON_CLIP_TYPE_UINT, "409" },
                                { GIBBON_CLIP_TYPE_NAME, "GammonBot_VII" },
                                { GIBBON_CLIP_TYPE_UINT, "5" },
                                { GIBBON_CLIP_TYPE_UINT, "0" },
                                { GIBBON_CLIP_TYPE_UINT, "0" },
                                { GIBBON_CLIP_TYPE_END, NULL }
        }
};

static struct test_case test_saved02 = {
        " *BlunderBot_VI           3                1 -  0",
        {
                                { GIBBON_CLIP_TYPE_UINT, "409" },
                                { GIBBON_CLIP_TYPE_NAME, "BlunderBot_VI" },
                                { GIBBON_CLIP_TYPE_UINT, "3" },
                                { GIBBON_CLIP_TYPE_UINT, "1" },
                                { GIBBON_CLIP_TYPE_UINT, "0" },
                                { GIBBON_CLIP_TYPE_END, NULL }
        }
};

static struct test_case test_saved03 = {
        "**bonehead                3                1 -  2",
        {
                                { GIBBON_CLIP_TYPE_UINT, "409" },
                                { GIBBON_CLIP_TYPE_NAME, "bonehead" },
                                { GIBBON_CLIP_TYPE_UINT, "3" },
                                { GIBBON_CLIP_TYPE_UINT, "1" },
                                { GIBBON_CLIP_TYPE_UINT, "2" },
                                { GIBBON_CLIP_TYPE_END, NULL }
        }
};

static struct test_case test_saved04 = {
        "no saved games.",
        {
                                { GIBBON_CLIP_TYPE_UINT, "410" },
                                { GIBBON_CLIP_TYPE_END, NULL }
        }
};

static struct test_case test_saved05 = {
        "  deadbeef                unlimited         1 -  2",
        {
                                { GIBBON_CLIP_TYPE_UINT, "409" },
                                { GIBBON_CLIP_TYPE_NAME, "deadbeef" },
                                { GIBBON_CLIP_TYPE_UINT, "0" },
                                { GIBBON_CLIP_TYPE_UINT, "1" },
                                { GIBBON_CLIP_TYPE_UINT, "2" },
                                { GIBBON_CLIP_TYPE_END, NULL }
        }
};

static struct test_case test_saved06 = {
        "deadbeef has no saved games.",
        {
                                { GIBBON_CLIP_TYPE_UINT, "411" },
                                { GIBBON_CLIP_TYPE_NAME, "deadbeef" },
                                { GIBBON_CLIP_TYPE_UINT, "0" },
                                { GIBBON_CLIP_TYPE_END, NULL }
        }
};

static struct test_case test_saved07 = {
        "deadbeef has 12 saved games.",
        {
                                { GIBBON_CLIP_TYPE_UINT, "411" },
                                { GIBBON_CLIP_TYPE_NAME, "deadbeef" },
                                { GIBBON_CLIP_TYPE_UINT, "12" },
                                { GIBBON_CLIP_TYPE_END, NULL }
        }
};

static struct test_case test_saved08 = {
        "deadbeef has 1 saved game.",
        {
                                { GIBBON_CLIP_TYPE_UINT, "411" },
                                { GIBBON_CLIP_TYPE_NAME, "deadbeef" },
                                { GIBBON_CLIP_TYPE_UINT, "1" },
                                { GIBBON_CLIP_TYPE_END, NULL }
        }
};

static struct test_case test_address01 = {
        "Your email address is 'gibbon@example.com'.",
        {
                                { GIBBON_CLIP_TYPE_UINT, "412" },
                                { GIBBON_CLIP_TYPE_STRING, "gibbon@example.com" },
                                { GIBBON_CLIP_TYPE_END, NULL }
        }
};

static struct test_case test_address02 = {
        "** 'http://foobar' is not an email address.",
        {
                                { GIBBON_CLIP_TYPE_UINT, "102" },
                                { GIBBON_CLIP_TYPE_STRING, "http://foobar" },
                                { GIBBON_CLIP_TYPE_END, NULL }
        }
};

static struct test_case test_corrupt = {
                "** ERROR: Saved match is corrupt. Please start another one.",
                {
                                { GIBBON_CLIP_TYPE_UINT, "101" },
                                { GIBBON_CLIP_TYPE_STRING,
                                  "Your saved match was corrupted on server. "
                                  " Please start a new one!"},
                                { GIBBON_CLIP_TYPE_END, NULL }
                }
};

static struct test_case *test_cases[] = {
                &test_clip00,
                &test_clip01,
                &test_clip01a,
                &test_clip02,
                &test_clip03,
                &test_clip04,
                &test_clip05,
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

                &test_error00,
                &test_error01,
                &test_error02,

                &test_board00,
                &test_board01,
                &test_board02,
                &test_bad_board00,
                &test_bad_board01,
                &test_bad_board02,
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
                &test_game_left,
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
                &test_toggles13a,
                &test_toggles13b,
                &test_toggles14,
                &test_toggles15,
                &test_toggles16,
                &test_toggles17,
                &test_toggles18,
                &test_toggles19,

                &test_saved00,
                &test_saved01,
                &test_saved02,
                &test_saved03,
                &test_saved04,
                &test_saved05,
                &test_saved06,
                &test_saved07,
                &test_saved08,

                &test_address01,
                &test_address02,

                &test_corrupt
};

static gboolean test_single_case (struct test_case *test_case);
static gboolean check_result (const gchar *line, gsize num,
                              struct token_pair *token_pair,
                              struct GibbonClipTokenSet *token_set);
static gchar *stringify_token_type (enum GibbonClipType type);
static gchar *stringify_token_value (struct GibbonClipTokenSet *token_set);

int
main (int argc, char *argv[])
{
        gint status = 0;
        gsize i;

        g_type_init ();

        for (i = 0; i < sizeof test_cases / sizeof test_cases[0]; ++i) {
                if (!test_single_case (test_cases[i]))
                        status = -1;
        }

        return status;
}

static gboolean
test_single_case (struct test_case *test_case)
{
        GSList *result = gibbon_clip_parse (test_case->line);
        GSList *iter;
        gboolean retval = TRUE;
        struct token_pair *expect;
        gsize i = 0;
        gchar *expect_type;

        expect = test_case->tokens;

        iter = result;
        while (iter) {
                ++i;
                if (!check_result (test_case->line, i, expect,
                                   (struct GibbonClipTokenSet *) iter->data))
                        retval = FALSE;
                if (expect->type == GIBBON_CLIP_TYPE_END)
                        break;
                iter = iter->next;
                ++expect;
        }

        if (expect->type != GIBBON_CLIP_TYPE_END) {
                expect_type = stringify_token_type (expect->type);
                g_printerr ("%s: token #%llu: expected `%s' (token type %s)"
                            " got nothing.\n",
                            test_case->line, (unsigned long long) i + 1,
                            expect->value, expect_type);
                g_free (expect_type);

                retval = FALSE;
        }

        gibbon_clip_free_result (result);

        return retval;
}

static gboolean
check_result (const gchar *line, gsize num, struct token_pair *token_pair,
              struct GibbonClipTokenSet *token_set)
{
        gchar *got_value;
        gchar *got_type = NULL;
        gchar *expect_type = NULL;
        const gchar *expect_value = NULL;

        gboolean retval = TRUE;

        got_value = stringify_token_value (token_set);
        expect_value = token_pair->value;

        if (token_pair->type != token_set->type
            || g_strcmp0 (got_value, expect_value)) {
                got_type = stringify_token_type (token_set->type);
                expect_type = stringify_token_type (token_pair->type);
                g_printerr ("%s: token #%llu:"
                            " expected `%s' (token type %s),"
                            " got `%s' (token type %s).\n",
                            line, (unsigned long long) num,
                            expect_value, expect_type,
                            got_value, got_type);
                retval = FALSE;
        }

        g_free (got_value);
        g_free (got_type);
        g_free (expect_type);

        return retval;
}

static gchar *
stringify_token_type (enum GibbonClipType type)
{
        switch (type) {
                case GIBBON_CLIP_TYPE_END:
                        return g_strdup ("end");
                case GIBBON_CLIP_TYPE_UINT:
                        return g_strdup ("unsigned integer");
                case GIBBON_CLIP_TYPE_INT:
                        return g_strdup ("integer");
                case GIBBON_CLIP_TYPE_DOUBLE:
                        return g_strdup ("double");
                case GIBBON_CLIP_TYPE_BOOLEAN:
                        return g_strdup ("boolean");
                case GIBBON_CLIP_TYPE_STRING:
                        return g_strdup ("string");
                case GIBBON_CLIP_TYPE_NAME:
                        return g_strdup ("name");
                case GIBBON_CLIP_TYPE_TIMESTAMP:
                        return g_strdup ("timestamp");
        }

        return NULL;
}

static gchar *
stringify_token_value (struct GibbonClipTokenSet *token_set)
{
        switch (token_set->type) {
                case GIBBON_CLIP_TYPE_END:
                        return NULL;
                case GIBBON_CLIP_TYPE_UINT:
                        return g_strdup_printf ("%llu",
                                         (unsigned long long) token_set->v.i64);
                case GIBBON_CLIP_TYPE_INT:
                        return g_strdup_printf ("%lld",
                                                (long long) token_set->v.i64);
                case GIBBON_CLIP_TYPE_DOUBLE:
                        return g_strdup_printf ("%f",
                                                (gdouble) token_set->v.d);
                case GIBBON_CLIP_TYPE_BOOLEAN:
                        if (token_set->v.i64)
                                return g_strdup ("TRUE");
                        else
                                return g_strdup ("FALSE");
                case GIBBON_CLIP_TYPE_STRING:
                case GIBBON_CLIP_TYPE_NAME:
                        return g_strdup (token_set->v.s);
                case GIBBON_CLIP_TYPE_TIMESTAMP:
                        return g_strdup_printf ("%lld",
                                                (long long) token_set->v.i64);
        }

        return NULL;
}
