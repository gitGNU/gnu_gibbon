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

/**
 * SECTION:gibbon-match-tracker
 * @short_description: Track backgammon matches.
 *
 * Since: 0.2.0
 *
 * A #GibbonMatchTracker records all relevant match actions and triggers
 * appropriate actions.  It continually updates the match file in the
 * archive and it adds moves made to the move list view.
 */

#include <glib.h>
#include <glib/gi18n.h>
#include <glib/gstdio.h>

#include "gibbon-match-tracker.h"
#include "gibbon-match.h"
#include "gibbon-app.h"
#include "gibbon-archive.h"
#include "gibbon-gmd-writer.h"
#include "gibbon-connection.h"
#include "gibbon-gmd-reader.h"
#include "gibbon-match-play.h"
#include "gibbon-match-list.h"

typedef struct _GibbonMatchTrackerPrivate GibbonMatchTrackerPrivate;
struct _GibbonMatchTrackerPrivate {
        GibbonMatch *match;
        gchar *outname;
        GibbonGMDWriter *writer;
        GOutputStream *out;
        gchar *wrank;
        gchar *brank;
};

#define GIBBON_MATCH_TRACKER_PRIVATE(obj) (G_TYPE_INSTANCE_GET_PRIVATE ((obj), \
        GIBBON_TYPE_MATCH_TRACKER, GibbonMatchTrackerPrivate))

G_DEFINE_TYPE (GibbonMatchTracker, gibbon_match_tracker, G_TYPE_OBJECT)

static void gibbon_match_tracker_unlink_or_archive (GibbonMatchTracker *self,
                                                    const gchar *player1,
                                                    const gchar *player2);
static void gibbon_match_tracker_resume (GibbonMatchTracker *self,
                                         const gchar *player1,
                                         const gchar *player2);
static void gibbon_match_reader_no_yyerror (const GibbonMatchTracker *self,
                                            const gchar *msg);
static void gibbon_match_tracker_init_match (GibbonMatchTracker *self,
                                             gsize length,
                                             const gchar *player1,
                                             const gchar *player2,
                                             const GibbonPosition *initial);

#define DEBUG_CONTINUATION 1

static void 
gibbon_match_tracker_init (GibbonMatchTracker *self)
{
        self->priv = G_TYPE_INSTANCE_GET_PRIVATE (self,
                GIBBON_TYPE_MATCH_TRACKER, GibbonMatchTrackerPrivate);

        self->priv->match = NULL;
        self->priv->outname = NULL;
        self->priv->writer = NULL;
        self->priv->wrank = self->priv->brank = NULL;
}

static void
gibbon_match_tracker_finalize (GObject *object)
{
        GibbonMatchTracker *self = GIBBON_MATCH_TRACKER (object);
        GError *error = NULL;

        if (self->priv->out && !g_output_stream_close (self->priv->out,
                                                       NULL, &error)) {
                gibbon_app_fatal_error (app, _("Write Error"),
                                        _("Error writing to `%s': %s!\n"),
                                        self->priv->outname,
                                        error->message);
        }

        if (self->priv->match)
                g_object_unref (self->priv->match);

        g_free (self->priv->outname);

        if (self->priv->writer)
                g_object_unref (self->priv->writer);

        g_free (self->priv->wrank);
        g_free (self->priv->brank);

        G_OBJECT_CLASS (gibbon_match_tracker_parent_class)->finalize(object);
}

static void
gibbon_match_tracker_class_init (GibbonMatchTrackerClass *klass)
{
        GObjectClass *object_class = G_OBJECT_CLASS (klass);
        
        g_type_class_add_private (klass, sizeof (GibbonMatchTrackerPrivate));

        object_class->finalize = gibbon_match_tracker_finalize;
}

/**
 * gibbon_match_tracker_new:
 * @player1: White.
 * @player2: Black.
 * @length:  Match length or 0 for unlimited match.
 * @resume:  Try to find the start of the match in the archive and continue
 *           it.
 *
 * Creates a new #GibbonMatchTracker.
 *
 * Returns: The newly created #GibbonMatchTracker or %NULL in case of failure.
 */
GibbonMatchTracker *
gibbon_match_tracker_new (const gchar *player1, const gchar *player2,
                          gsize length, gboolean resume)
{
        GibbonMatchTracker *self = g_object_new (GIBBON_TYPE_MATCH_TRACKER,
                                                 NULL);

        /*
         * This constructor is a conceptual mess.  If we resume a match, we
         * try to load the old saved state, initialize a couple of variables,
         * and assume success if one of them - notably self->priv->match - is
         * non-NULL.
         *
         * Rather let gibbon_match_tracker_resume() return TRUE/FALSE.
         */
        if (!resume)
                gibbon_match_tracker_unlink_or_archive (self, player1, player2);
        else
                gibbon_match_tracker_resume (self, player1, player2);

        if (!self->priv->match)
                gibbon_match_tracker_init_match (self, length,
                                                 player1, player2,
                                                 NULL);

        return self;
}

void
gibbon_match_tracker_store_rank (const GibbonMatchTracker *self,
                                 const gchar *rank,
                                 GibbonPositionSide side)
{
        GError *error;

        g_return_if_fail (self != NULL);
        g_return_if_fail (rank != NULL);
        g_return_if_fail (side != GIBBON_POSITION_SIDE_NONE);
        g_return_if_fail (GIBBON_IS_MATCH_TRACKER (self));

        if (side < 0) {
                g_free (self->priv->brank);
                self->priv->brank = g_strdup (rank);
        } else {
                g_free (self->priv->wrank);
                self->priv->brank = g_strdup (rank);
        }

        gibbon_match_set_rank (self->priv->match, side, rank);
        if (!gibbon_gmd_writer_update_rank (self->priv->writer,
                                            self->priv->out,
                                            self->priv->match, side,
                                            &error)) {
                gibbon_app_fatal_error (app, _("Write Error"),
                                        _("Error writing to `%s': %s!\n"),
                                        self->priv->outname,
                                        error->message);
        }
        g_output_stream_flush (self->priv->out, NULL, NULL);
}

static void
gibbon_match_tracker_unlink_or_archive (GibbonMatchTracker *self,
                                        const gchar *player1,
                                        const gchar *player2)
{
        gchar *path;
        GibbonArchive *archive = gibbon_app_get_archive (app);
        GibbonMatchReader *reader;
        GibbonMatch *match;
        GibbonMatchReaderErrorFunc yyerror;
        GError *error = NULL;
        gint64 start;

        path = gibbon_archive_get_saved_name (archive, player1, player2);
        if (!g_file_test (path, G_FILE_TEST_EXISTS)) {
                path = gibbon_archive_get_saved_name (archive,
                                                      player2, player1);
                if (!g_file_test (path, G_FILE_TEST_EXISTS))
                        return;
        }

        yyerror = (GibbonMatchReaderErrorFunc) gibbon_match_reader_no_yyerror;
        reader = GIBBON_MATCH_READER (gibbon_gmd_reader_new (yyerror,
                                                             (gpointer) self));
        match = gibbon_match_reader_parse (reader, path);

        if (!match) {
                g_remove (path);
                return;
        }
        start = gibbon_match_get_start_time (match);
        if (start == G_MININT64) {
                /* Either corrupt or only a starting position.  */
                g_remove (path);
                g_object_unref (match);
                return;
        }
        if (!gibbon_match_get_current_game (match)) {
                g_remove (path);
                g_object_unref (match);
                return;
        }

        if (!gibbon_archive_archive_match_file (archive, match, path, &error))
                gibbon_app_fatal_error (app, _("Write Error"),
                                        _("Cannot archive match `%s': %s\n"),
                                        path, error->message);

        g_object_unref (match);
}

static void
gibbon_match_reader_no_yyerror (const GibbonMatchTracker *self,
                                const gchar *msg)
{
        /* Do nothing.  */
}

void
gibbon_match_tracker_update (GibbonMatchTracker *self,
                             const GibbonPosition *target)
{
        GSList *iter = NULL;
        const GibbonPosition *c = NULL;
        GibbonPosition *current;
        GibbonMatchPlay *play;
        GibbonGameAction *action;
        GibbonPositionSide side;
        const GibbonGame *game;
        const GibbonGame *last_game;
        GError *error = NULL;
        GibbonMatchList *list;
        int num_actions;

        g_return_if_fail (self != NULL);
        g_return_if_fail (target != NULL);
        g_return_if_fail (GIBBON_IS_MATCH_TRACKER (self));

        c = gibbon_match_get_current_position (self->priv->match);
        if (c) {
                current = gibbon_position_copy (c);
        } else {
                current = gibbon_position_copy (gibbon_position_initial ());
        }

        if (!gibbon_match_get_missing_actions (self->priv->match, target,
                                               &iter)) {
                /*
                 * This case is not necessarily in error.  It also happens,
                 * if we have a saved match for the two opponents that
                 * has expired on the server.
                 *
                 * Archive the currently saved match, if possible, and
                 * start a new one with the new position as the starting
                 * position.
                 */
#if DEBUG_CONTINUATION
                g_printerr ("Could not guess missing actions from here:\n");
                gibbon_position_dump_position (current);
                g_printerr ("to here:\n");
                gibbon_position_dump_position (target);
                gtk_widget_error_bell (gibbon_app_get_window (app));
#endif
                gibbon_match_tracker_unlink_or_archive (self,
                                                        current->players[0],
                                                        current->players[1]);
                g_object_unref (self->priv->match);
                gibbon_match_tracker_init_match (self, target->match_length,
                                                 target->players[0],
                                                 target->players[1],
                                                 target);
                current = gibbon_position_copy (target);
        }

        list = gibbon_app_get_match_list (app);

        while (iter) {
                play = (GibbonMatchPlay *) iter->data;
                action = play->action;
                side = play->side;
                last_game = gibbon_match_get_current_game (self->priv->match);
                if (!last_game) {
                        if (!gibbon_gmd_writer_add_game (self->priv->writer,
                                                         self->priv->out,
                                                         &error)) {
                                gibbon_app_fatal_error (app, _("Write Error"),
                                                        _("Error writing to"
                                                          " `%s': %s!\n"),
                                                        self->priv->outname,
                                                        error->message);
                        }
                }
                if (!gibbon_match_add_action (self->priv->match, side, action,
                                              G_MININT64, NULL))
                        goto bail_out;
                game = gibbon_match_get_current_game (self->priv->match);
                if (!gibbon_gmd_writer_write_action (self->priv->writer,
                                                     self->priv->out,
                                                     game, action, side,
                                                     g_get_real_time (),
                                                     &error)) {
                        gibbon_app_fatal_error (app, _("Write Error"),
                                                _("Error writing to"
                                                  " `%s': %s!\n"),
                                                self->priv->outname,
                                                error->message);
                }
                if (last_game && last_game != game) {
                        if (!gibbon_gmd_writer_add_game (self->priv->writer,
                                                         self->priv->out,
                                                         &error)) {
                                gibbon_app_fatal_error (app, _("Write Error"),
                                                        _("Error writing to"
                                                          " `%s': %s!\n"),
                                                        self->priv->outname,
                                                        error->message);
                        }
                }
                if (last_game != game) {
                        gibbon_match_list_add_game (list, game);
                } else {
                        /* Show the new game action(s).  */
                        num_actions = gibbon_game_get_num_actions (game);
                        gibbon_match_list_add_action (list, game,
                                                      num_actions - 1);
                }
                last_game = game;
                iter = iter->next;
        }

        gibbon_position_free (current);
        g_slist_free_full (iter, (GDestroyNotify) gibbon_match_play_free);

        return;

        bail_out:

        gibbon_position_free (current);
        g_slist_free_full (iter, (GDestroyNotify) gibbon_match_play_free);

        /*
         * TODO: Archive current match and create a new one with
         * with the current position as the initial setup.
         */
}

static void
gibbon_match_tracker_resume (GibbonMatchTracker *self,
                             const gchar *player1,
                             const gchar *player2)
{
        gchar *path;
        GibbonArchive *archive = gibbon_app_get_archive (app);
        GibbonMatchReader *reader;
        GibbonMatch *match;
        GibbonMatchReaderErrorFunc yyerror;
        GError *error = NULL;
        GFile *file;
        GFileOutputStream *out;

        path = gibbon_archive_get_saved_name (archive, player1, player2);
        if (!g_file_test (path, G_FILE_TEST_EXISTS)) {
                path = gibbon_archive_get_saved_name (archive,
                                                      player2, player1);
                if (!g_file_test (path, G_FILE_TEST_EXISTS))
                        return;
        }

        yyerror = (GibbonMatchReaderErrorFunc) gibbon_match_reader_no_yyerror;
        reader = GIBBON_MATCH_READER (gibbon_gmd_reader_new (yyerror,
                                                             (gpointer) self));
        match = gibbon_match_reader_parse (reader, path);

        if (!match) {
                g_remove (path);
                return;
        }
        if (!gibbon_match_get_current_game (match)) {
                g_remove (path);
                g_object_unref (match);
                return;
        }

        file = g_file_new_for_path (path);
        out = g_file_append_to (file, G_FILE_CREATE_NONE, NULL, &error);
        g_object_unref (file);
        if (!out) {
                gibbon_app_fatal_error (app, _("Write Error"),
                                        _("Error writing to"
                                          " `%s': %s!\n"),
                                        path,
                                        error->message);
                return;
        }

        self->priv->match = match;
        self->priv->outname = path;
        self->priv->out = G_OUTPUT_STREAM (out);
        self->priv->writer = gibbon_gmd_writer_new ();
}

static void
gibbon_match_tracker_init_match (GibbonMatchTracker *self, gsize length,
                                 const gchar *player1, const gchar *player2,
                                 const GibbonPosition *initial)
{
        GibbonArchive *archive = gibbon_app_get_archive (app);
        GFile *file;
        GFileOutputStream *fout;
        GError *error = NULL;
        GibbonMatchWriter *writer;
        GibbonConnection *connection;
        const gchar *host;
        guint16 port;
        gchar *location;
        GibbonGame *game;

        self->priv->outname = gibbon_archive_get_saved_name (archive,
                                                             player1,
                                                             player2);

        file = g_file_new_for_path (self->priv->outname);
        fout = g_file_replace (file, NULL, FALSE, G_FILE_COPY_OVERWRITE,
                               NULL, &error);
        g_object_unref (file);
        if (!fout) {
                gibbon_app_fatal_error (app, _("Write Error"),
                                        _("Error writing to `%s': %s!\n"),
                                        self->priv->outname,
                                        error->message);
        }
        self->priv->out = G_OUTPUT_STREAM (fout);

        /*
         * We always assume that the Crawford rule applies for
         * fixed-length matches.  We will find out whether this is
         * correct or not later.
         */
        self->priv->match = gibbon_match_new (player1, player2, length,
                                              length);

        if (initial) {
                game = gibbon_match_add_game (self->priv->match, &error);
                if (!game) {
                        gibbon_app_fatal_error (app, _("Internal Error"),
                                                "%s", error->message);
                        /* NOTREACHED */
                        return;
                }
                gibbon_game_set_initial_position (game, initial);
        }

        connection = gibbon_app_get_connection (app);
        host = gibbon_connection_get_hostname (connection);
        port = gibbon_connection_get_port (connection);
        if (port == 4321)
                location = g_strdup_printf ("x-fibs://%s", host);
        else
                location = g_strdup_printf ("x-fibs://%s:%u",
                                            host, port);
        gibbon_match_set_location (self->priv->match, location);
        g_free (location);
        self->priv->writer = gibbon_gmd_writer_new ();
        writer = GIBBON_MATCH_WRITER (self->priv->writer);

        if (!gibbon_match_writer_write_stream (writer, self->priv->out,
                                               self->priv->match, &error)) {
                gibbon_app_fatal_error (app, _("Write Error"),
                                        _("Error writing to `%s': %s!\n"),
                                        self->priv->outname,
                                        error->message);
        }
        g_output_stream_flush (self->priv->out, NULL, NULL);

        gibbon_app_set_match (app, self->priv->match);
}

void
gibbon_match_tracker_set_crawford (GibbonMatchTracker *self, gboolean flag)
{
        GibbonGame *game;

        g_return_if_fail (GIBBON_IS_MATCH_TRACKER (self));

        gibbon_match_set_crawford (self->priv->match, flag);

        if (!flag) {
                game = gibbon_match_get_current_game (self->priv->match);
                if (game)
                        gibbon_game_set_is_crawford (game, FALSE);
        }
}
