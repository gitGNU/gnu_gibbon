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
 * SECTION:gibbon-geo-ip-updater
 * @short_description: Class for updating the Gibbon GeoIP database.
 *
 * Since: 0.1.0
 *
 * Class for updating the Gibbon GeoIP database.
 */

#include <errno.h>

#include <gtk/gtk.h>
#include <glib/gi18n.h>

#include "gibbon-app.h"
#include "gibbon-geo-ip-updater.h"
#include "gibbon-geo-ip-data.h"

typedef struct _GibbonGeoIPUpdaterPrivate GibbonGeoIPUpdaterPrivate;
struct _GibbonGeoIPUpdaterPrivate {
        GibbonDatabase *database;

        GtkWidget *dialog;
        GtkWidget *label;
        GtkWidget *progress_bar;
        gulong response_handler;

        gchar *uri;
        GFile *file;
        GCancellable *cancellable;
        GInputStream *stream;

        gchar *buf;
#define GIBBON_GEO_IP_UPDATER_CHUNK_SIZE 8192
        gsize buf_size;
        gsize buf_fill;
        gsize lineno;
};

#define GIBBON_GEO_IP_UPDATER_PRIVATE(obj) (G_TYPE_INSTANCE_GET_PRIVATE ((obj), \
        GIBBON_TYPE_GEO_IP_UPDATER, GibbonGeoIPUpdaterPrivate))

G_DEFINE_TYPE (GibbonGeoIPUpdater, gibbon_geo_ip_updater, G_TYPE_OBJECT)

static void gibbon_geo_ip_updater_on_response (GibbonGeoIPUpdater *self,
                                               gint response_id);
static void gibbon_geo_ip_updater_on_open (GFile *file, GAsyncResult *res,
                                           GibbonGeoIPUpdater *self);
static void gibbon_geo_ip_updater_on_read (GInputStream *stream,
                                           GAsyncResult *res,
                                           GibbonGeoIPUpdater *self);
static gboolean gibbon_geo_ip_updater_parse (GibbonGeoIPUpdater *self,
                                             const gchar *line_start,
                                             gsize length);

static void 
gibbon_geo_ip_updater_init (GibbonGeoIPUpdater *self)
{
        self->priv = G_TYPE_INSTANCE_GET_PRIVATE (self,
                GIBBON_TYPE_GEO_IP_UPDATER, GibbonGeoIPUpdaterPrivate);

        self->priv->database = NULL;

        self->priv->dialog = NULL;
        self->priv->label = NULL;
        self->priv->progress_bar = NULL;
        self->priv->response_handler = 0;

        self->priv->uri = NULL;
        self->priv->file = NULL;
        self->priv->cancellable = NULL;
        self->priv->stream = NULL;

        self->priv->buf = NULL;
        self->priv->buf_size = 0;
        self->priv->buf_fill = 0;
        self->priv->lineno = 0;
}

static void
gibbon_geo_ip_updater_finalize (GObject *object)
{
        GibbonGeoIPUpdater *self = GIBBON_GEO_IP_UPDATER (object);

        if (self->priv->cancellable)
                g_object_unref (self->priv->cancellable);

        if (self->priv->stream)
                g_object_unref (self->priv->stream);

        if (self->priv->buf)
                g_free (self->priv->buf);

        if (self->priv->uri)
                g_free (self->priv->uri);

        if (self->priv->file)
                g_object_unref (self->priv->file);

        if (self->priv->dialog) {
                if (self->priv->response_handler)
                        g_signal_handler_disconnect (self->priv->dialog,
                                                  self->priv->response_handler);
                gtk_widget_destroy (self->priv->dialog);
        }

        G_OBJECT_CLASS (gibbon_geo_ip_updater_parent_class)->finalize(object);
}

static void
gibbon_geo_ip_updater_class_init (GibbonGeoIPUpdaterClass *klass)
{
        GObjectClass *object_class = G_OBJECT_CLASS (klass);
        
        g_type_class_add_private (klass, sizeof (GibbonGeoIPUpdaterPrivate));

        object_class->finalize = gibbon_geo_ip_updater_finalize;
}

/**
 * gibbon_geo_ip_updater_new:
 * @database: The #GibbonDatabase.
 *
 * Creates a new #GibbonGeoIPUpdater.
 *
 * Returns: The newly created #GibbonGeoIPUpdater or %NULL in case of failure.
 */
GibbonGeoIPUpdater *
gibbon_geo_ip_updater_new (GibbonDatabase *database,
                           gint64 _last_update)
{
        GibbonGeoIPUpdater *self = g_object_new (GIBBON_TYPE_GEO_IP_UPDATER, NULL);
        GtkWindow *main_window;
        guint64 diff;
        GtkDialogFlags flags;
        GtkMessageType mtype;
        GtkButtonsType btype;
        guint64 months;
        gchar *question;
        gint64 last_update;
        gint reply;
        gboolean download = FALSE;
#ifdef G_OS_WIN32
        gchar *win32_dir;
#endif

        self->priv->database = database;

        main_window = GTK_WINDOW (gibbon_app_get_window (app));

        if (_last_update == 0)
                last_update = GIBBON_GEO_IP_DATA_UPDATE;
        else
                last_update = _last_update;

        diff = (gint64) time (NULL) - last_update;
        if (diff > 30 * 24 * 60 * 60) {
                months = diff / (30 * 24 * 60 * 60);
                flags = GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT;
                mtype = GTK_MESSAGE_QUESTION;
                btype = GTK_BUTTONS_YES_NO;
                if (_last_update) {
                        question = g_strdup_printf (g_dngettext (GETTEXT_PACKAGE,
                                                    "Your database used for"
                                                    " locating other players"
                                                    " geographically is"
                                                    " more than one month old. "
                                                    " Should a new one be"
                                                    " downloaded (1-2 MB) from"
                                                    " the internet?",
                                                    "Your database used for"
                                                    " locating other players"
                                                    " geographically is"
                                                    " more than %llu months old. "
                                                    " Should a new one be"
                                                    " downloaded (1-2 MB) from"
                                                    " the internet?",
                                                    months),
                                                    (unsigned long long) months);
                } else {
                        question = g_strdup_printf (g_dngettext (GETTEXT_PACKAGE,
                                                    "The database used for"
                                                    " locating other players"
                                                    " geographically that"
                                                    " was installed with this"
                                                    " program is more than"
                                                    " one month old. "
                                                    " Should a new one be"
                                                    " downloaded (1-2 MB) from"
                                                    " the internet?",
                                                    "The database used for"
                                                    " locating other players"
                                                    " geographically that"
                                                    " was installed with this"
                                                    " program is more than"
                                                    " %llu months old. "
                                                    " Should a new one be"
                                                    " downloaded (1-2 MB) from"
                                                    " the internet?",
                                                    months),
                                                    (unsigned long long) months);
                }

                self->priv->dialog = gtk_message_dialog_new (main_window,
                                                             flags, mtype,
                                                             btype,
                                                             "%s", question);
                g_free (question);
                gtk_dialog_set_default_response (GTK_DIALOG (self->priv->dialog),
                                                 GTK_RESPONSE_YES);
                reply = gtk_dialog_run (GTK_DIALOG (self->priv->dialog));
                gtk_widget_destroy (self->priv->dialog);
                self->priv->dialog = NULL;

                if (reply == GTK_RESPONSE_YES) {
                        download = TRUE;
                } else if (_last_update) {
                        g_object_unref (self);
                        return NULL;
                }
        }

#if 0
# define GEO_IP_DEFAULT_URI "http://software77.net/geo-ip/?DL=1"
#else
#define GEO_IP_DEFAULT_URI "http://www.gibbon.bg/ip2country.csv.gz"
#endif
        if (download) {
                self->priv->file = g_file_new_for_uri (GEO_IP_DEFAULT_URI);
                self->priv->uri = g_strdup (GEO_IP_DEFAULT_URI);
        } else {

#ifdef G_OS_WIN32
                win32_dir =
                    g_win32_get_package_installation_directory_of_module (NULL);
                self->priv->uri = g_build_filename (win32_dir, "share", PACKAGE,
                                                    "ip2country.csv.gz", NULL);
#else
                self->priv->uri = g_build_filename (GIBBON_DATADIR, PACKAGE,
                                                    "ip2country.csv.gz", NULL);
#endif                                                    
                self->priv->file = g_file_new_for_path (self->priv->uri);
        }

        return self;
}

static void
gibbon_geo_ip_updater_on_response (GibbonGeoIPUpdater *self,
                                   gint response_id)
{
        if (self->priv->cancellable) {
                g_cancellable_cancel (self->priv->cancellable);
                g_object_unref (self->priv->cancellable);
                self->priv->cancellable = NULL;
        }
        gtk_widget_hide (self->priv->dialog);
        gibbon_app_display_info (app, NULL, "%s",
                                 _("The information about other"
                                   " players' geographic location"
                                   " will not be accurate."));

        if (self->priv->database)
                gibbon_database_cancel_geo_ip_update (self->priv->database);
}

void
gibbon_geo_ip_updater_start (GibbonGeoIPUpdater *self)
{
        GtkWidget *vbox;
        GtkWidget *content;
        gchar *message;
        GCallback callback;
        GtkWindow *main_window;
        gchar *uri;

        g_return_if_fail (GIBBON_IS_GEO_IP_UPDATER (self));

        main_window = GTK_WINDOW (gibbon_app_get_window (app));

        self->priv->dialog =
                        gtk_dialog_new_with_buttons (_("Update Geo IP database"),
                                                     main_window,
                                                  GTK_DIALOG_DESTROY_WITH_PARENT,
                                                     GTK_STOCK_CANCEL,
                                                     GTK_RESPONSE_CANCEL,
                                                     NULL);
        callback = G_CALLBACK (gibbon_geo_ip_updater_on_response);
        self->priv->response_handler =
                        g_signal_connect_swapped (G_OBJECT (self->priv->dialog),
                                                  "response",
                                                  callback,
                                                  G_OBJECT (self));
        content = gtk_dialog_get_content_area (GTK_DIALOG (self->priv->dialog));

        vbox = gtk_vbox_new (FALSE, 10);
        gtk_container_add (GTK_CONTAINER (content), vbox);

        uri = g_file_get_uri (self->priv->file);
        message = g_strdup_printf (_("Opening `%s' ..."), uri);
        g_free (uri);
        self->priv->label = gtk_label_new (message);
        g_free (message);
        gtk_box_pack_start (GTK_BOX (vbox), self->priv->label, FALSE, FALSE, 0);

        self->priv->progress_bar = gtk_progress_bar_new ();
        gtk_container_add (GTK_CONTAINER (content), self->priv->progress_bar);
        gtk_progress_bar_set_fraction (
                        GTK_PROGRESS_BAR (self->priv->progress_bar), 0.0);

        gtk_window_set_modal (GTK_WINDOW (self->priv->dialog), TRUE);
        gtk_widget_show_all (self->priv->dialog);
        gtk_widget_hide (self->priv->progress_bar);

        self->priv->cancellable = g_cancellable_new ();
        g_file_read_async (self->priv->file, G_PRIORITY_DEFAULT,
                           self->priv->cancellable,
                           (GAsyncReadyCallback) gibbon_geo_ip_updater_on_open,
                           self);
}

static void
gibbon_geo_ip_updater_on_open (GFile *file, GAsyncResult *res,
                               GibbonGeoIPUpdater *self)
{
        GError *error = NULL;
        GFileInputStream *fstream;
        GInputStream *stream;
        GZlibDecompressor *filter;
        GAsyncReadyCallback callback;
        gchar *msg;

        g_return_if_fail (G_IS_FILE (file));
        g_return_if_fail (G_IS_ASYNC_RESULT (res));
        g_return_if_fail (GIBBON_IS_GEO_IP_UPDATER (self));

        fstream = g_file_read_finish (file, res, &error);
        g_object_unref (file);
        self->priv->file = NULL;
        if (!fstream) {
                gtk_widget_hide (self->priv->dialog);
                gibbon_app_display_error (app, NULL,
                                          _("Cannot open `%s': %s!"),
                                          self->priv->uri, error->message);
                g_error_free (error);
                gibbon_database_cancel_geo_ip_update (self->priv->database);
                return;
        }

        gibbon_database_on_start_geo_ip_update (self->priv->database);

        msg = g_strdup_printf (_("Reading `%s'."), self->priv->uri);
        gtk_label_set_text (GTK_LABEL (self->priv->label), msg);
        g_free (msg);
        gtk_widget_show (GTK_WIDGET (self->priv->progress_bar));

        filter = g_zlib_decompressor_new (G_ZLIB_COMPRESSOR_FORMAT_GZIP);
        stream = g_converter_input_stream_new (G_INPUT_STREAM (fstream),
                                               G_CONVERTER (filter));

        self->priv->stream = G_INPUT_STREAM (stream);

        self->priv->buf = g_realloc (self->priv->buf,
                                     GIBBON_GEO_IP_UPDATER_CHUNK_SIZE);
        self->priv->buf_size = GIBBON_GEO_IP_UPDATER_CHUNK_SIZE;
        self->priv->buf_fill = 0;

        callback = (GAsyncReadyCallback) gibbon_geo_ip_updater_on_read;
        g_input_stream_read_async (stream,
                                   self->priv->buf,
                                   GIBBON_GEO_IP_UPDATER_CHUNK_SIZE,
                                   G_PRIORITY_DEFAULT,
                                   self->priv->cancellable,
                                   callback, self);
}

static void
gibbon_geo_ip_updater_on_read (GInputStream *stream, GAsyncResult *res,
                               GibbonGeoIPUpdater *self)
{
        gssize read_bytes;
        GError *error = NULL;
        gchar *start_of_line;
        gchar *end_of_line;
        gsize wanted_size;
        GAsyncReadyCallback callback;

        g_return_if_fail (G_IS_INPUT_STREAM (stream));
        g_return_if_fail (stream == self->priv->stream);
        g_return_if_fail (G_IS_ASYNC_RESULT (res));
        g_return_if_fail (GIBBON_IS_GEO_IP_UPDATER (self));

        read_bytes = g_input_stream_read_finish (stream, res, &error);
        if (read_bytes < 0) {
                gtk_widget_hide (self->priv->dialog);
                gibbon_app_display_error (app, NULL,
                                          _("Error reading `%s': %s!"),
                                          self->priv->uri, error->message);
                g_error_free (error);
                gibbon_database_cancel_geo_ip_update (self->priv->database);
                return;
        }

        self->priv->buf_fill += read_bytes;

        start_of_line = end_of_line = self->priv->buf;
        while (end_of_line < self->priv->buf + self->priv->buf_fill) {
                if (*end_of_line == '\n') {
                        if (!gibbon_geo_ip_updater_parse (self,
                                                          start_of_line,
                                                          end_of_line
                                                          - start_of_line)) {
                                gibbon_database_cancel_geo_ip_update (
                                                self->priv->database);
                                return;
                        }
                        ++end_of_line;
                        start_of_line = end_of_line;
                        continue;
                }
                ++end_of_line;
        }

        self->priv->buf_fill = end_of_line - start_of_line;
        if (self->priv->buf_fill)
                memmove (self->priv->buf, start_of_line, self->priv->buf_fill);

        if (!read_bytes) {
                if (self->priv->buf_fill) {
                        /* Incomplete last line.  */
                        if (!gibbon_geo_ip_updater_parse (self,
                                                          self->priv->buf,
                                                          self->priv->buf_fill)) {
                                gibbon_database_cancel_geo_ip_update (
                                                self->priv->database);
                                return;
                        }
                }
                gibbon_database_close_geo_ip_update (self->priv->database);
                return;
        }

        wanted_size = self->priv->buf_fill + GIBBON_GEO_IP_UPDATER_CHUNK_SIZE;
        if (self->priv->buf_size < wanted_size) {
                self->priv->buf_size = wanted_size;
                self->priv->buf = g_realloc (self->priv->buf, wanted_size);
        }

        callback = (GAsyncReadyCallback) gibbon_geo_ip_updater_on_read;
        g_input_stream_read_async (stream,
                                   self->priv->buf + self->priv->buf_fill,
                                   GIBBON_GEO_IP_UPDATER_CHUNK_SIZE,
                                   G_PRIORITY_DEFAULT,
                                   self->priv->cancellable,
                                   callback, self);
}

/*
 * And now the mother of all spaghetti code routines spiced with some
 * beautiful sugo di goto.
 */
static gboolean
gibbon_geo_ip_updater_parse (GibbonGeoIPUpdater *self, const gchar *line_start,
                             gsize length)
{
        gchar *ptr = g_alloca (length + 1);
        gchar *from_ip;
        gchar *to_ip;
        guint max_ip = 4294967295U;
        guint num;
        gchar *timestamp;
        gchar *alpha2;
        gchar *alpha3;
        gdouble fraction;

        ++self->priv->lineno;

        strncpy (ptr, line_start, length);
        ptr[length] = 0;

        while (*ptr == ' ' || *ptr == '\t') {
                if (!*ptr)
                        return TRUE;
        }

        if (*ptr == '#')
                return TRUE;

        if (*ptr != '"')
                goto parse_error;
        ++ptr;

        from_ip = ptr;
        while (*ptr) {
                if ('"' == *ptr)
                        break;
                if (*ptr < '0' || *ptr > '9')
                        goto parse_error;
                ++ptr;
        }

        *ptr = 0;
        ++ptr;

        /* Bail out on leading zeros.  */
        if (!*from_ip && from_ip[1])
                goto parse_error;

        if (*ptr != ',')
                goto parse_error;
        ++ptr;
        if (*ptr != '"')
                goto parse_error;
        ++ptr;

        to_ip = ptr;
        while (*ptr) {
                if ('"' == *ptr)
                        break;
                if (*ptr < '0' || *ptr > '9')
                        goto parse_error;
                ++ptr;
        }

        *ptr = 0;
        ++ptr;

        /* Bail out on leading zeros.  */
        if (!*to_ip && to_ip[1])
                goto parse_error;

        if (!(strlen (to_ip) >= strlen (from_ip)
              || g_strcmp0 (from_ip, to_ip) < 0))
                goto parse_error;

        errno = 0;
        num = g_ascii_strtoull (to_ip, NULL, 10);
        if (errno)
                goto parse_error;
        if (num > max_ip)
                goto parse_error;
        fraction = (gdouble) num / (gdouble) max_ip;

        if (*ptr != ',')
                goto parse_error;
        ++ptr;
        if (*ptr != '"')
                goto parse_error;
        ++ptr;

        while (*ptr) {
                if ('"' == *ptr)
                        break;
                /* We are not prepared for escaping ... */
                if (*ptr == '\\')
                        goto parse_error;
                ++ptr;
        }
        if (*ptr != '"')
                goto parse_error;
        *ptr = 0;
        ++ptr;
        if (*ptr != ',')
                goto parse_error;
        ++ptr;
        if (*ptr != '"')
                goto parse_error;
        ++ptr;

        timestamp = ptr;
        while (*ptr) {
                if ('"' == *ptr)
                        break;
                if (*ptr < '0' || *ptr > '9')
                        goto parse_error;
                ++ptr;
        }

        *ptr = 0;
        ++ptr;

        /* Bail out on leading zeros.  */
        if (!*timestamp)
                goto parse_error;

        if (*ptr != ',')
                goto parse_error;
        ++ptr;
        if (*ptr != '"')
                goto parse_error;
        ++ptr;

        alpha2 = ptr;
        while (*ptr) {
                if ('"' == *ptr)
                        break;
                if (*ptr < 'A' || *ptr > 'Z')
                        goto parse_error;
                *ptr -= 'Q' - 'q';
                ++ptr;
        }
        if (*ptr != '"')
                goto parse_error;
        *ptr = 0;
        if (2 != strlen (alpha2))
                goto parse_error;
        ++ptr;
        if (*ptr != ',')
                goto parse_error;
        ++ptr;
        if (*ptr != '"')
                goto parse_error;
        ++ptr;

        alpha3 = ptr;
        while (*ptr) {
                if ('"' == *ptr)
                        break;
                if (*ptr < 'A' || *ptr > 'Z')
                        goto parse_error;
                ++ptr;
        }
        if (*ptr != '"')
                goto parse_error;
        *ptr = 0;
        /* 3-letter code for EU is EU.  */
        if (3 != strlen (alpha3) && 2 != strlen (alpha3))
                goto parse_error;
        ++ptr;
        if (*ptr != ',')
                goto parse_error;
        ++ptr;
        if (*ptr != '"')
                goto parse_error;
        ++ptr;

        while (*ptr) {
                if ('"' == *ptr)
                        break;
                /* We are not prepared for escaping ... */
                if (*ptr == '\\')
                        goto parse_error;
                ++ptr;
        }
        if (*ptr != '"')
                goto parse_error;
        *ptr = 0;
        ++ptr;

        while (*ptr) {
                if ('#' == *ptr)
                        break;
                if (' ' == *ptr || '\t' == *ptr)
                        continue;
                ++ptr;
        }

        gibbon_database_set_geo_ip (self->priv->database,
                                    from_ip, to_ip, alpha2);

        gtk_progress_bar_set_fraction (
                        GTK_PROGRESS_BAR (self->priv->progress_bar), fraction);

        return TRUE;

parse_error:
        gtk_widget_hide (self->priv->dialog);
        if (*ptr)
                gibbon_app_display_error (app, NULL,
                                          _("%s: line %u: Parse error near"
                                            " `%s'!"),
                                           self->priv->uri,
                                           (unsigned) self->priv->lineno,
                                           ptr);
        else
                gibbon_app_display_error (app, NULL,
                                          _("%s: line %u: Unexpected end of"
                                            " line!"),
                                           self->priv->uri,
                                           (unsigned) self->priv->lineno);

        return FALSE;
}
