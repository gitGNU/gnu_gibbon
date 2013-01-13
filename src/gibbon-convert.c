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

#include <locale.h>
#include <string.h>

#include <glib/gi18n.h>
#include <gio/gio.h>
#include <gdk/gdk.h>

#include "gibbon-gmd-reader.h"
#include "gibbon-gmd-writer.h"
#include "gibbon-sgf-reader.h"
#include "gibbon-sgf-writer.h"
#include "gibbon-java-fibs-reader.h"
#include "gibbon-java-fibs-writer.h"
#include "gibbon-jelly-fish-reader.h"
#include "gibbon-jelly-fish-writer.h"

typedef enum {
        GIBBON_CONVERT_FORMAT_UNKNOWN = 0,
        GIBBON_CONVERT_FORMAT_SGF = 1,
        GIBBON_CONVERT_FORMAT_GMD = 2,
        GIBBON_CONVERT_FORMAT_JAVA_FIBS = 3,
        GIBBON_CONVERT_FORMAT_JELLY_FISH = 4
} GibbonConvertFormat;

static gchar *program_name;
static gchar *input_filename = NULL;
static gchar *output_filename = NULL;
static gchar *from_format = NULL;
static gchar *to_format = NULL;

GibbonConvertFormat input_format = GIBBON_CONVERT_FORMAT_UNKNOWN;
GibbonConvertFormat output_format = GIBBON_CONVERT_FORMAT_UNKNOWN;

gboolean version = FALSE;

static gchar *debug;

static const GOptionEntry options[] =
{
                { "input-file", 'i', 0, G_OPTION_ARG_FILENAME, &input_filename,
                  N_("input filename or '-' for standard input"),
                  N_("FILENAME")
                },
                { "output-file", 'o', 0, G_OPTION_ARG_FILENAME, &output_filename,
                  N_("output filename or '-' for standard output"),
                  N_("FILENAME")
                },
                { "from-format", 'f', 0, G_OPTION_ARG_STRING, &from_format,
                  N_("input format (omit for automatic detection)"),
                  N_("FORMAT")
                },
                { "to-format", 't', 0, G_OPTION_ARG_STRING, &to_format,
                  N_("output format (omit for automatic detection)"),
                  N_("FORMAT")
                },
                { "debug", 'D', 0, G_OPTION_ARG_STRING, &debug,
                  N_("enable various debugging flags"),
                  NULL
                },
                { "version", 'V', 0, G_OPTION_ARG_NONE, &version,
                  N_("output version information and exit"),
                  NULL
                },
	        { NULL }
};

#ifdef G_OS_WIN32
static void init_i18n (const gchar *installdir);
#else
static void init_i18n (void);
#endif

static void print_version ();
static void usage_error ();
static gboolean parse_command_line (int argc, char *argv[]);
static GibbonConvertFormat guess_format_from_id (const gchar *id);
static GibbonConvertFormat guess_format_from_filename (const gchar *name);

int
main (int argc, char *argv[])
{	
        GibbonMatchReader *reader = NULL;
        GibbonMatchWriter *writer = NULL;
        GibbonMatch *match;
        GFile *file = NULL;
        GFileOutputStream *fout;
        GOutputStream *out;
        GError *error = NULL;
#ifdef G_OS_WIN32
        gchar *win32_dir =
                g_win32_get_package_installation_directory_of_module (NULL);

        init_i18n (win32_dir);                
#else
        init_i18n ();
#endif

        g_type_init ();

        program_name = argv[0];
        if (!parse_command_line (argc, argv))
                return 1;

        if (version) {
                print_version ();
                return 0;
        }

        if (debug)
                g_setenv ("GIBBON_DEBUG", debug, TRUE);

        if (from_format) {
                input_format = guess_format_from_id (from_format);
                if (!input_format)
                        return 1;
        } else if (input_filename) {
                input_format = guess_format_from_filename (input_filename);
                if (!input_format)
                        return 1;
        } else {
                usage_error (_("The option `--from-format' is mandatory,"
                               " when reading standard input."));
                return 1;
        }

        if (to_format) {
                output_format = guess_format_from_id (to_format);
                if (!output_format)
                        return 1;
        } else if (output_filename) {
                output_format = guess_format_from_filename (output_filename);
                if (!output_format)
                        return 1;
        } else {
                usage_error (_("The option `--to-format' is mandatory,"
                               " when writing to standard output."));
                return 1;
        }

        if (input_format == output_format) {
                usage_error (_("The output format is the same as the input"
                               " format."));
                return 1;
        }

        if (!g_thread_supported ()) {
#if (GLIB_MAJOR_VERSION < 2 \
     || (GLIB_MAJOR_VERSION == 2 && GLIB_MINOR_VERSION < 32))
                g_thread_init (NULL);
#endif
                gdk_threads_init ();
        }

        switch (input_format) {
        case GIBBON_CONVERT_FORMAT_UNKNOWN:
                return 1;
        case GIBBON_CONVERT_FORMAT_SGF:
                reader = GIBBON_MATCH_READER (gibbon_sgf_reader_new (NULL,
                                                                     NULL));
                break;
        case GIBBON_CONVERT_FORMAT_GMD:
                reader = GIBBON_MATCH_READER (gibbon_gmd_reader_new (NULL,
                                                                     NULL));
                break;
        case GIBBON_CONVERT_FORMAT_JAVA_FIBS:
                reader =
                   GIBBON_MATCH_READER (gibbon_java_fibs_reader_new (NULL,
                                                                     NULL));
                break;
        case GIBBON_CONVERT_FORMAT_JELLY_FISH:
                reader =
                   GIBBON_MATCH_READER (gibbon_jelly_fish_reader_new (NULL,
                                                                      NULL));
                break;
        }

        match = gibbon_match_reader_parse (reader, input_filename);
        if (!match)
                return 1;

        if (!gibbon_match_get_current_game (match)) {
                g_printerr (_("%s: Empty or incomplete match file!\n"),
                            input_filename
                            ? input_filename : _("standard input"));
                return 1;
        }

        switch (output_format) {
        case GIBBON_CONVERT_FORMAT_UNKNOWN:
                g_object_unref (match);
                return 1;
        case GIBBON_CONVERT_FORMAT_SGF:
                writer = GIBBON_MATCH_WRITER (gibbon_sgf_writer_new ());
                break;
        case GIBBON_CONVERT_FORMAT_GMD:
                writer = GIBBON_MATCH_WRITER (gibbon_gmd_writer_new ());
                break;
        case GIBBON_CONVERT_FORMAT_JAVA_FIBS:
                writer = GIBBON_MATCH_WRITER (gibbon_java_fibs_writer_new ());
                break;
        case GIBBON_CONVERT_FORMAT_JELLY_FISH:
                writer = GIBBON_MATCH_WRITER (gibbon_jelly_fish_writer_new ());
                break;
        }

        if (output_filename) {
                file = g_file_new_for_commandline_arg (output_filename);
                fout = g_file_replace (file, NULL, FALSE, G_FILE_COPY_OVERWRITE,
                                       NULL, &error);
                g_object_unref (file);
                if (!fout) {
                        if (error) {
                                g_printerr (_("%s: Error writing to `%s': %s!\n"),
                                            program_name,
                                            output_filename, error->message);
                                g_error_free (error);
                        }
                        return 1;
                }
                out = G_OUTPUT_STREAM (fout);
        } else {
                /* FIXME! How do we get a GOutputStream for stdout? */
                out = G_OUTPUT_STREAM (g_memory_output_stream_new (NULL, 0,
                                                                   g_realloc,
                                                                   g_free));
        }

        if (!gibbon_match_writer_write_stream (writer, out, match, &error)) {
                if (error) {
                        /*
                         * TRANSLATORS: The first argument is the program
                         * name (gibbon-convert), and the second argument
                         * is the error message.
                         */
                        g_printerr (_("%s: %s.\n"), program_name,
                                    error->message);
                        g_error_free (error);
                }
                return 1;
        }

        if (!output_filename) {
                g_print ("%s",
                        (gchar *) g_memory_output_stream_get_data  (
                                            G_MEMORY_OUTPUT_STREAM (out)));
        } else if (!g_output_stream_close (out, NULL, &error)) {
                if (error) {
                        /*
                         * TRANSLATORS: The first argument is the program
                         * name (gibbon-convert), the second is the
                         * output filename, the third one is the actual
                         * error message.
                         */
                        g_printerr (_("%s: Error closing `%s': %s!\n"),
                                    program_name, output_filename,
                                    error->message);
                        g_error_free (error);
                }
                return 1;
        }

        g_object_unref (out);
        g_object_unref (writer);
        g_object_unref (reader);

        return 0;
}

static void
#ifdef G_OS_WIN32
init_i18n (const gchar *installdir)
#else
init_i18n (void)
#endif
{
        gchar *locale_dir;

        setlocale(LC_ALL, "");

#ifdef G_OS_WIN32
        locale_dir = g_build_filename (installdir, "share", "locale", NULL);
#else
        locale_dir = g_build_filename (GIBBON_DATADIR, "locale", NULL);
#endif
        bindtextdomain (GETTEXT_PACKAGE, locale_dir);
        g_free (locale_dir);
        bind_textdomain_codeset (GETTEXT_PACKAGE, "UTF-8");
        textdomain (GETTEXT_PACKAGE);
}
 
static gboolean
parse_command_line (int argc, char *argv[])
{
        GOptionContext *context;
        GError *error = NULL;
        gchar *description;
        gchar *alt_usage;

        context = g_option_context_new (_("- Gibbon match converter"));
        g_option_context_set_summary (context, _("Convert popular backgammon"
                                                 " match formats."));
        alt_usage = g_strdup_printf (
                        _("Alternative usages:\n"
                          " %s [OPTION ...] INPUTFILE\n"
                          " %s [OPTION ...] INPUTFILE OUTPUTFILE\n"),
                          program_name, program_name);
        description = g_strdup_printf ("%s\n%s\n%s\n%s\n",
                        alt_usage,
                        _("Recognized values for FORMAT are 'SGF' for"
                          " the Smart Game format (used by\n"
                          "Gibbon and GNU backgammon), 'GMD' for"
                          " the Gibbon Match Dump format, 'Jellyfish' and"
                          " 'JavaFIBS'.  Formats are\n"
                          "case-insensitve and you can abbreviate"
                          " them as long as they are unique.\n"),
                        _("For automatic detection, '.sgf' is expected for"
                          " SGF, '.mat' for Jellyfish, '.gmd' for the"
                          " Gibbon match format, and\n"
                          "'.match' for the internal format of JavaFIBS.\n"),
                        _("Report bugs at"
                          " <https://savannah.nongnu.org/projects/gibbon>!"));
        g_free (alt_usage);
        g_option_context_set_description (context, description);
        g_free (description);
        g_option_context_add_main_entries (context, options, PACKAGE);
        g_option_context_parse (context, &argc, &argv, &error);

        g_option_context_free (context);

        if (error) {
                g_printerr ("%s\n", error->message);
                g_printerr (_("Run `%s --help' for more information!\n"),
                            program_name);
                g_error_free (error);
                return FALSE;
        }

        if (argc == 2) {
                if (input_filename) {
                        usage_error (_("Either use the option `--input-file'"
                                       " or specify the input file as an"
                                       " argument."));
                        return FALSE;
                }
                input_filename = argv[1];
        } else if (argc == 3) {
                if (input_filename) {
                        usage_error (_("Either use the option `--input-file'"
                                       " or specify the input file as an"
                                       " argument."));
                        return FALSE;
                }
                input_filename = argv[1];
                if (output_filename) {
                        usage_error (_("Either use the option `--output-file'"
                                       " or specify the output file as an"
                                       " argument."));
                        return FALSE;
                }
                output_filename = argv[2];
        } else if (argc > 3) {
                usage_error (_("Too many arguments!"));
                return FALSE;
        }

        return TRUE;
}

static void
print_version ()
{
        g_print ("%s (%s) %s\n", program_name, PACKAGE, VERSION);
        /* xgettext: no-wrap */
        g_print (_("Copyright (C) %s %s.\n\
License GPLv3+: GNU GPL version 3 or later <http://gnu.org/licenses/gpl.html>\n\
This is free software: you are free to change and redistribute it.\n\
There is NO WARRANTY, to the extent permitted by law.\n\
"),
                "2009-2012", _("Guido Flohr"));
        g_print (_("Written by %s.\n"), _("Guido Flohr"));
}

void
usage_error (const gchar *msg)
{
        g_printerr ("%s\n", msg);
        g_printerr (_("Try `%s --help' for more information!\n"),
                    program_name);
}

static GibbonConvertFormat
guess_format_from_id (const gchar *id)
{
        gchar *msg;
        size_t got_length = strlen (id);

        if (got_length) {
                if (got_length <= 3
                    && 0 == g_ascii_strncasecmp ("sgf", id, got_length))
                         return GIBBON_CONVERT_FORMAT_SGF;

                if (got_length <= 3
                    && 0 == g_ascii_strncasecmp ("gmd", id, got_length))
                         return GIBBON_CONVERT_FORMAT_GMD;

                if (0 == g_ascii_strncasecmp ("gibbon", id,
                                              got_length))
                        return GIBBON_CONVERT_FORMAT_JAVA_FIBS;

                if (got_length > 1) {
                        if (0 == g_ascii_strncasecmp ("javafibs", id,
                                                      got_length))
                                return GIBBON_CONVERT_FORMAT_JAVA_FIBS;
                        if (0 == g_ascii_strncasecmp ("jellyfish", id,
                                                      got_length))
                                return GIBBON_CONVERT_FORMAT_JELLY_FISH;
                }

                if ((id[0] == 'j' || id[0] == 'J') && id[1] == 0) {
                        msg = g_strdup_printf (_("The file format `%c' is"
                                                 " ambiguous."),
                                               id[0]);
                        usage_error (msg);
                        g_free (msg);
                        return GIBBON_CONVERT_FORMAT_UNKNOWN;
                }
        }

        msg = g_strdup_printf (_("Unknown file format `%s'!"), id);
        usage_error (msg);
        g_free (msg);

        return GIBBON_CONVERT_FORMAT_UNKNOWN;
}

static GibbonConvertFormat
guess_format_from_filename (const gchar *filename)
{
        const gchar *last_dot;
        gchar *msg;

        last_dot = rindex (filename, '.');

        if (!last_dot) {
                msg = g_strdup_printf (_("Cannot guess format of `%s'!"),
                                       filename);
                usage_error (msg);
                g_free (msg);
                return GIBBON_CONVERT_FORMAT_UNKNOWN;
        } else if (0 == g_ascii_strcasecmp (".sgf", last_dot)) {
                return GIBBON_CONVERT_FORMAT_SGF;
        } else if (0 == g_ascii_strcasecmp (".gmd", last_dot)) {
                return GIBBON_CONVERT_FORMAT_GMD;
        } else if (0 == g_ascii_strcasecmp (".match", last_dot)) {
                return GIBBON_CONVERT_FORMAT_JAVA_FIBS;
        } else if (0 == g_ascii_strcasecmp (".mat", last_dot)) {
                return GIBBON_CONVERT_FORMAT_JELLY_FISH;
        }

        msg = g_strdup_printf (_("Cannot guess format of `%s'!"), filename);
        usage_error (msg);
        g_free (msg);
        return GIBBON_CONVERT_FORMAT_UNKNOWN;
}
