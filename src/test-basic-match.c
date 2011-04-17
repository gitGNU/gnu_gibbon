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

#include <glib.h>

#include <gibbon-match.h>

static GibbonMatch *fill_match (void);
static gboolean check_match (const GibbonMatch *match);
static gboolean serialize_match (const GibbonMatch *match);

int
main(int argc, char *argv[])
{
	int status = 0;
	GibbonMatch *match;

        g_type_init ();

        match = fill_match ();
        if (!match)
                return -1;

        if (!check_match (match))
                status = -1;

        if (!serialize_match (match))
                status = -1;

        g_object_unref (match);

        return status;
}

static GibbonMatch *
fill_match (void)
{
        GibbonMatch *match = gibbon_match_new ();
        GError *error = NULL;

        if (!gibbon_match_set_black_player (match, "Joe Black", &error))
                g_printerr ("Error setting black player: %s.\n",
                            error->message);

        if (!gibbon_match_set_white_player (match, "Snow White", &error))
                g_printerr ("Error setting white player: %s.\n",
                            error->message);

        return match;
}

static gboolean
check_match (const GibbonMatch *match)
{
        gboolean retval = TRUE;
        const gchar *got;
        const gchar *expect;

        got = gibbon_match_get_black_player (match);
        expect = "Joe Black";
        if (g_strcmp0 (expect, got)) {
                g_printerr ("Expected `%s', got `%s'!\n", expect, got);
                retval = FALSE;
        }

        got = gibbon_match_get_white_player (match);
        expect = "Snow White";
        if (g_strcmp0 (expect, got)) {
                g_printerr ("Expected `%s', got `%s'!\n", expect, got);
                retval = FALSE;
        }

        return retval;
}

static gboolean
serialize_match (const GibbonMatch *match)
{
        const GSGFCollection *collection = gibbon_match_get_collection (match);
        GOutputStream *out = g_memory_output_stream_new (NULL, 0,
                                                         g_realloc, g_free);
        GError *error = NULL;
        gsize written;

        if (!gsgf_component_write_stream (GSGF_COMPONENT (collection), out,
                                          &written, NULL, &error)) {
                g_printerr ("Writing basic match failed: %s.\n",
                            error->message);
                return FALSE;
        }

#if (0)
        g_printerr ("%s",
                    (gchar *) g_memory_output_stream_get_data  (
                                    G_MEMORY_OUTPUT_STREAM (out)));
#endif

        g_object_unref (out);

        return TRUE;
}
