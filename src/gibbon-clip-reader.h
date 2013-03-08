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

#ifndef _GIBBON_CLIP_READER_H
# define _GIBBON_CLIP_READER_H

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <glib.h>
#include <glib-object.h>

#define GIBBON_TYPE_CLIP_READER \
        (gibbon_clip_reader_get_type ())
#define GIBBON_CLIP_READER(obj) \
        (G_TYPE_CHECK_INSTANCE_CAST ((obj), GIBBON_TYPE_CLIP_READER, \
                GibbonCLIPReader))
#define GIBBON_CLIP_READER_CLASS(klass) (G_TYPE_CHECK_CLASS_CAST ((klass), \
        GIBBON_TYPE_CLIP_READER, GibbonCLIPReaderClass))
#define GIBBON_IS_CLIP_READER(obj) \
        (G_TYPE_CHECK_INSTANCE_TYPE ((obj), \
                GIBBON_TYPE_CLIP_READER))
#define GIBBON_IS_CLIP_READER_CLASS(klass) \
        (G_TYPE_CHECK_CLASS_TYPE ((klass), \
                GIBBON_TYPE_CLIP_READER))
#define GIBBON_CLIP_READER_GET_CLASS(obj) \
        (G_TYPE_INSTANCE_GET_CLASS ((obj), \
                GIBBON_TYPE_CLIP_READER, GibbonCLIPReaderClass))


enum GibbonClipCode {
                        /* Numerical CLIP codes.  */
                        GIBBON_CLIP_UNHANDLED = 0,
                        GIBBON_CLIP_WELCOME = 1,
                        GIBBON_CLIP_OWN_INFO = 2,
                        GIBBON_CLIP_MOTD_START = 3,
                        GIBBON_CLIP_MOTD_END = 4,
                        GIBBON_CLIP_WHO_INFO = 5,
                        GIBBON_CLIP_WHO_INFO_END = 6,
                        GIBBON_CLIP_LOGIN = 7,
                        GIBBON_CLIP_LOGOUT = 8,
                        GIBBON_CLIP_MESSAGE = 9,
                        GIBBON_CLIP_MESSAGE_DELIVERED = 10,
                        GIBBON_CLIP_MESSAGE_SAVED = 11,
                        GIBBON_CLIP_SAYS = 12,
                        GIBBON_CLIP_SHOUTS = 13,
                        GIBBON_CLIP_WHISPERS = 14,
                        GIBBON_CLIP_KIBITZES = 15,
                        GIBBON_CLIP_YOU_SAY = 16,
                        GIBBON_CLIP_YOU_SHOUT = 17,
                        GIBBON_CLIP_YOU_WHISPER = 18,
                        GIBBON_CLIP_YOU_KIBITZ = 19,
                        GIBBON_CLIP_ALERTS = 20,

                        /*
                         * Error messages.  The second uint describes the
                         * error more precisely.  The third one is a human
                         * readable error message.
                         */
                        GIBBON_CLIP_ERROR = 100,
                        /*
                         * FIXME! GIBBON_CLIP_UNKNOWN_MESSAGE is obsolete!
                         */
                        GIBBON_CLIP_UNKNOWN_MESSAGE = 101,

                        /* Game play.  */
                        GIBBON_CLIP_BOARD = 200,
                        GIBBON_CLIP_ROLLS = 202,
                        GIBBON_CLIP_MOVES = 203,
                        GIBBON_CLIP_START_GAME = 204,
                        GIBBON_CLIP_LEFT_GAME = 205,
                        GIBBON_CLIP_CANNOT_MOVE = 206,
                        GIBBON_CLIP_DOUBLES = 207,
                        GIBBON_CLIP_ACCEPTS_DOUBLE = 208,
                        GIBBON_CLIP_RESIGNS = 209,
                        GIBBON_CLIP_REJECTS_RESIGNATION = 210,
                        GIBBON_CLIP_DROPS_GAME = 211,

                        /* Between game action.  */
                        GIBBON_CLIP_INVITATION = 300,
                        GIBBON_CLIP_TYPE_JOIN = 301,
                        GIBBON_CLIP_YOURE_WATCHING = 302,
                        GIBBON_CLIP_NOW_PLAYING = 303,
                        GIBBON_CLIP_INVITE_ERROR = 304,
                        GIBBON_CLIP_RESUME = 305,
                        GIBBON_CLIP_RESUME_INFO_TURN = 306,
                        GIBBON_CLIP_RESUME_INFO_MATCH_LENGTH = 307,
                        GIBBON_CLIP_RESUME_INFO_POINTS = 308,
                        GIBBON_CLIP_JOIN_OR_LEAVE = 309,
                        GIBBON_CLIP_WIN_GAME = 310,
                        GIBBON_CLIP_GAME_SCORE = 311,
                        GIBBON_CLIP_WAIT_JOIN_TOO = 312,
                        GIBBON_CLIP_INVITE_CONFIRMATION = 313,
                        GIBBON_CLIP_WIN_MATCH = 314,
                        GIBBON_CLIP_STOP_WATCHING = 315,

                        /* Various.  */
                        GIBBON_CLIP_START_MATCH = 400,
                        GIBBON_CLIP_ASYNC_WIN_MATCH = 401,
                        GIBBON_CLIP_RESUME_MATCH = 402,
                        GIBBON_CLIP_EMPTY = 403,
                        GIBBON_CLIP_START_SETTINGS = 404,
                        GIBBON_CLIP_SHOW_SETTING = 405,
                        GIBBON_CLIP_START_TOGGLES = 406,
                        GIBBON_CLIP_SHOW_TOGGLE = 407,
                        GIBBON_CLIP_SHOW_START_SAVED = 408,
                        GIBBON_CLIP_SHOW_SAVED = 409,
                        GIBBON_CLIP_SHOW_SAVED_NONE = 410,
                        GIBBON_CLIP_SHOW_SAVED_COUNT = 411,
                        GIBBON_CLIP_SHOW_ADDRESS = 412,
                        GIBBON_CLIP_MOTD = 413,
                        GIBBON_CLIP_NO_SUCH_USER = 414,
                        GIBBON_CLIP_INVALID_ADDRESS = 415,

                        /* Less important messages with two leading stars.  */
                        GIBBON_CLIP_HEARD_YOU = 500
};

/*
 * This is a bogus concept and should be given up.  Either we must qualify
 * an error anyway, or a generic message is enough.  No need to distinguish
 * further.
 */
enum GibbonCLIPErrorCode {
        GIBBON_CLIP_ERROR_UNKNOWN = 0,
        GIBBON_CLIP_ERROR_NO_TWO_MATCHES = 3,
        GIBBON_CLIP_ERROR_SAVED_CORRUPT = 4
};

/**
 * GibbonCLIPReader:
 *
 * One instance of a #GibbonCLIPReader, a stateful line-based parser for
 * FIBS output.
 */
typedef struct _GibbonCLIPReader GibbonCLIPReader;
struct _GibbonCLIPReader
{
        GObject parent_instance;

        /*< private >*/
        struct _GibbonCLIPReaderPrivate *priv;
};

/**
 * GibbonCLIPReaderClass:
 *
 * Parse FIBS server output.
 */
typedef struct _GibbonCLIPReaderClass GibbonCLIPReaderClass;
struct _GibbonCLIPReaderClass
{
        /* <private >*/
        GObjectClass parent_class;
};

GType gibbon_clip_reader_get_type (void) G_GNUC_CONST;

GibbonCLIPReader *gibbon_clip_reader_new ();
GSList *gibbon_clip_reader_parse (GibbonCLIPReader *self, const gchar *line);
void gibbon_clip_reader_free_result (GibbonCLIPReader *self, GSList *values);
gboolean gibbon_clip_reader_get_int64 (const GibbonCLIPReader *self,
                                       GSList **iter, gint64 *i);
gboolean gibbon_clip_reader_get_int (const GibbonCLIPReader *self,
                                     GSList **iter, gint *i);
gboolean gibbon_clip_reader_get_string (const GibbonCLIPReader *self,
                                        GSList **iter, const gchar **s);
gboolean gibbon_clip_reader_get_boolean (const GibbonCLIPReader *self,
                                         GSList **iter, gboolean *b);
gboolean gibbon_clip_reader_get_double (const GibbonCLIPReader *self,
                                        GSList **iter, gdouble *d);
#endif
