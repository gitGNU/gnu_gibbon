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

#include <stdlib.h>
#include <glib/gi18n.h>
#include <gio/gio.h>
#include <gtk/gtk.h>

#include <string.h>

#include "gibbon-connection.h"
#include "gibbon-session.h"
#include "gibbon-server-console.h"
#include "gibbon-fibs-command.h"
#include "gibbon-clip-reader.h"
#include "gibbon-util.h"

enum gibbon_connection_signals {
        CONNECTING,
        CONNECTED,
        LOGIN,
        LOGGED_IN,
        NETWORK_ERROR,
        DISCONNECTED,
        LAST_SIGNAL
};
static guint signals[LAST_SIGNAL] = { 0 };

enum GibbonConnectionState {
        WAIT_LOGIN_PROMPT,
        WAIT_WELCOME,
        WAIT_COMMANDS
};

struct _GibbonConnectionPrivate {
        GibbonApp *app;

        gchar *hostname;
        guint16 port;
        gchar *login;
        gchar *password;

        gboolean guest_login;

        GCancellable *connect_cancellable;
        GCancellable *read_cancellable;
        GCancellable *write_cancellable;
        GSocketClient *socket_client;
        GSocketConnection *socket_connection;
        
        enum GibbonConnectionState state;
        
        gchar *error;
        
#define GIBBON_CONNECTION_CHUNK_SIZE 8192
        guchar read_buf[GIBBON_CONNECTION_CHUNK_SIZE];
        gchar *in_buffer;
        GList *out_queue;
        gboolean out_ready;
        
        GibbonSession *session;

        gboolean debug_input;
        gboolean debug_output;
};

#define GIBBON_CONNECTION_DEFAULT_PORT 4321
#define GIBBON_CONNECTION_DEFAULT_HOST "fibs.com"

#define GIBBON_CONNECTION_PRIVATE(o) (G_TYPE_INSTANCE_GET_PRIVATE ((o), \
                                      GIBBON_TYPE_CONNECTION,           \
                                      GibbonConnectionPrivate))
G_DEFINE_TYPE (GibbonConnection, gibbon_connection, G_TYPE_OBJECT);

static void gibbon_connection_handle_input (GInputStream *stream,
                                            GAsyncResult *result,
                                            GibbonConnection *self);
static void gibbon_connection_handle_output (GOutputStream *stream,
                                             GAsyncResult *result,
                                             GibbonConnection *self);
static void gibbon_connection_send_chunk (GibbonConnection *self);
static void gibbon_connection_on_connect (GObject *src_object,
                                          GAsyncResult *res,
                                          gpointer _self);
static void gibbon_connection_fatal (GibbonConnection *connection,
                                     const gchar *format, ...);

static void
gibbon_connection_init (GibbonConnection *conn)
{
        conn->priv = G_TYPE_INSTANCE_GET_PRIVATE (conn, 
                                                  GIBBON_TYPE_CONNECTION,
                                                  GibbonConnectionPrivate);

        conn->priv->hostname = NULL;
        conn->priv->port = 0;
        conn->priv->password = NULL;
        conn->priv->login = NULL;
        
        conn->priv->connect_cancellable = NULL;
        conn->priv->read_cancellable = NULL;
        conn->priv->write_cancellable = NULL;
        conn->priv->socket_client = NULL;
        conn->priv->socket_connection = NULL;

        conn->priv->state = WAIT_LOGIN_PROMPT;
        
        conn->priv->error = NULL;
        
        conn->priv->in_buffer = g_strconcat ("", NULL);
        
        conn->priv->out_queue = NULL;
        conn->priv->out_ready = FALSE;
        
        conn->priv->session = NULL;

        conn->priv->debug_input = FALSE;
        conn->priv->debug_output = FALSE;
}

static void
gibbon_connection_finalize (GObject *object)
{
        GibbonConnection *self = GIBBON_CONNECTION (object);

        if (self->priv->session)
                g_object_unref (self->priv->session);

        if (self->priv->in_buffer)
                g_free (self->priv->in_buffer);
        
        if (self->priv->out_queue) {
                g_list_foreach (self->priv->out_queue, (GFunc) g_object_unref,
                                NULL);
                g_list_free (self->priv->out_queue);
        }
        self->priv->out_queue = NULL;

        if (self->priv->connect_cancellable) {
                g_cancellable_cancel (self->priv->connect_cancellable);
                g_object_unref (self->priv->connect_cancellable);
        }

        if (self->priv->read_cancellable) {
                g_cancellable_cancel (self->priv->read_cancellable);
                g_object_unref (self->priv->read_cancellable);
        }

        if (self->priv->write_cancellable) {
                g_cancellable_cancel (self->priv->write_cancellable);
                g_object_unref (self->priv->write_cancellable);
        }

        if (self->priv->socket_client)
                g_object_unref (self->priv->socket_client);

        if (self->priv->socket_connection)
                g_object_unref (self->priv->socket_connection);

        if (self->priv->hostname)
                g_free (self->priv->hostname);

        self->priv->port = 0;

        if (self->priv->password)
                g_free (self->priv->password);

        if (self->priv->login)
                g_free (self->priv->login);

        G_OBJECT_CLASS (gibbon_connection_parent_class)->finalize (object);
}

static void
gibbon_connection_class_init (GibbonConnectionClass *klass)
{
        GObjectClass* object_class = G_OBJECT_CLASS (klass);

        g_type_class_add_private (klass, sizeof (GibbonConnectionPrivate));

        object_class->finalize = gibbon_connection_finalize;

        signals[CONNECTING] =
                g_signal_new ("connecting",
                              G_TYPE_FROM_CLASS (klass),
                              G_SIGNAL_RUN_FIRST,
                              0,
                              NULL, NULL,
                              g_cclosure_marshal_VOID__OBJECT,
                              G_TYPE_NONE,
                              1,
                              G_TYPE_OBJECT);
        signals[CONNECTED] =
                g_signal_new ("connected",
                              G_TYPE_FROM_CLASS (klass),
                              G_SIGNAL_RUN_FIRST,
                              0,
                              NULL, NULL,
                              g_cclosure_marshal_VOID__OBJECT,
                              G_TYPE_NONE,
                              1,
                              G_TYPE_OBJECT);
        signals[LOGIN] =
                g_signal_new ("login",
                              G_TYPE_FROM_CLASS (klass),
                              G_SIGNAL_RUN_FIRST,
                              0,
                              NULL, NULL,
                              g_cclosure_marshal_VOID__OBJECT,
                              G_TYPE_NONE,
                              1,
                              G_TYPE_OBJECT);
        signals[LOGGED_IN] =
                g_signal_new ("logged_in",
                              G_TYPE_FROM_CLASS (klass),
                              G_SIGNAL_RUN_FIRST,
                              0,
                              NULL, NULL,
                              g_cclosure_marshal_VOID__OBJECT,
                              G_TYPE_NONE,
                              1,
                              G_TYPE_OBJECT);
        signals[NETWORK_ERROR] =
                g_signal_new ("network-error",
                              G_TYPE_FROM_CLASS (klass),
                              G_SIGNAL_RUN_FIRST,
                              0,
                              NULL, NULL,
                              g_cclosure_marshal_VOID__STRING,
                              G_TYPE_NONE,
                              1,
                              G_TYPE_STRING);
        signals[DISCONNECTED] =
                g_signal_new ("disconnected",
                              G_TYPE_FROM_CLASS (klass),
                              G_SIGNAL_RUN_FIRST,
                              0,
                              NULL, NULL,
                              g_cclosure_marshal_VOID__VOID,
                              G_TYPE_NONE,
                              0);
}

GibbonConnection *
gibbon_connection_new (GibbonApp *app, const gchar *hostname, guint16 port,
                       const gchar *login, const gchar *password)
{
        GibbonConnection *self = g_object_new (GIBBON_TYPE_CONNECTION, NULL);
        gsize i;

        g_return_val_if_fail (GIBBON_IS_APP (app), NULL);

        self->priv->app = app;

        self->priv->hostname = g_strdup (hostname);
        if (!self->priv->hostname)
                self->priv->hostname = g_strdup (GIBBON_CONNECTION_DEFAULT_HOST);
        self->priv->port = port;
        if (!self->priv->port)
                self->priv->port = GIBBON_CONNECTION_DEFAULT_PORT;
        self->priv->login = g_strdup (login);
        if (0 == g_strcmp0 ("guest", login))
                self->priv->guest_login = TRUE;
        else
                self->priv->guest_login = FALSE;

        self->priv->password = g_strdup (password);

        /*
         * Make sure that the hostname is basically canonical.  Maybe this
         * should be done in the binding to the corresponding setting?
         */
        for (i = 0; i < strlen (self->priv->hostname); ++i)
                self->priv->hostname[i] =
                        g_ascii_tolower (self->priv->hostname[i]);

        self->priv->state = WAIT_LOGIN_PROMPT;

        self->priv->session = gibbon_session_new (app, self);

        self->priv->debug_input = gibbon_debug ("connection-in");
        self->priv->debug_output = gibbon_debug ("connection-out");

        return self;
}

gboolean
gibbon_connection_connect (GibbonConnection *self)
{
        g_return_val_if_fail (GIBBON_IS_CONNECTION (self), FALSE);
        g_return_val_if_fail (self->priv->socket_client == NULL, FALSE);

        self->priv->connect_cancellable = g_cancellable_new ();
        self->priv->socket_client = g_socket_client_new ();

        g_signal_emit (self, signals[CONNECTING], 0, self);

        g_socket_client_connect_to_host_async (self->priv->socket_client,
                                               self->priv->hostname,
                                               self->priv->port,
                                               self->priv->connect_cancellable,
                                               gibbon_connection_on_connect,
                                               self);

        return TRUE;
}

const gchar *
gibbon_connection_get_hostname (const GibbonConnection *self)
{
        g_return_val_if_fail (GIBBON_IS_CONNECTION (self), NULL);

        return self->priv->hostname;
}

guint16
gibbon_connection_get_port (const GibbonConnection *self)
{
        g_return_val_if_fail (GIBBON_IS_CONNECTION (self), 0);

        return self->priv->port;
}

const gchar *
gibbon_connection_get_login (const GibbonConnection *self)
{
        g_return_val_if_fail (GIBBON_IS_CONNECTION (self), NULL);
        return self->priv->login;
}

const gchar *
gibbon_connection_get_password (const GibbonConnection *self)
{
        g_return_val_if_fail (GIBBON_IS_CONNECTION (self), NULL);
        return self->priv->password;
}

static void
gibbon_connection_handle_input (GInputStream *input_stream,
                                GAsyncResult *result,
                                GibbonConnection *self)
{
        gsize bytes_read;
        GError *error = NULL;
        gchar *pretty_login;
        gchar *package;
        gint clip_code;
        gchar *head;
        gchar *ptr;
        gchar *line_end;
        gchar *console_output;
        GibbonServerConsole *console;
        GibbonSession *session;
        GibbonApp *app;
        gsize i, eaten = 0;
        
        bytes_read = g_input_stream_read_finish (input_stream, result, &error);
        if (g_error_matches (error, G_IO_ERROR, G_IO_ERROR_CANCELLED) 
            || !self || !GIBBON_IS_CONNECTION (self))
                return;

        app = self->priv->app;
        if (!gibbon_app_get_connection (app))
                return;

        if (self->priv->read_cancellable)
                g_object_unref (self->priv->read_cancellable);
        self->priv->read_cancellable = NULL;

        if (bytes_read < 0) {
                g_signal_emit (self, signals[NETWORK_ERROR], 0,
                               error->message);
                g_error_free (error);
                return;
        } else if (bytes_read == 0) {
                g_signal_emit (self, signals[NETWORK_ERROR], 0,
                               _("End-of-file while receiving data from"
                                 " server."));
                return;
        }
        
        /* The input fifo is not exactly efficient.  */
        head = self->priv->in_buffer;
        self->priv->read_buf[bytes_read] = 0;

        /*
         * Filter out all 8 bit data, for example telnet echo will
         * and echo wont.
         */
        for (i = 0; i < bytes_read; ++i) {
                if (self->priv->read_buf[i] < '\n'
                    || (self->priv->read_buf[i] > '\n' 
                        && self->priv->read_buf[i] < ' ')
                    || self->priv->read_buf[i] >= 127) {
                        ++eaten;
                } else if (eaten) {
                        self->priv->read_buf[i - eaten] = 
                                self->priv->read_buf[i];
                }
        }
        self->priv->read_buf[i - eaten] = 0;

        self->priv->in_buffer = g_strconcat (head,
                                             self->priv->read_buf,
                                             NULL);
        g_free (head);

        #ifndef HAVE_INDEX
#define index(str, c) memchr (str, c, strlen (str))
#endif

        console = gibbon_app_get_server_console (app);

        ptr = self->priv->in_buffer;
        while ((line_end = index (ptr, '\012')) != NULL) {
                *line_end = 0;
                if (line_end > ptr && *(line_end - 1) == '\015')
                        *(line_end - 1) = 0;
                if (self->priv->state != WAIT_LOGIN_PROMPT) {
                        session = self->priv->session;
                        /*
                         * We need a copy of string because it could be
                         * destroyed during handling the server output.
                         */
                        console_output = g_alloca (1 + strlen (ptr));
                        strcpy (console_output, ptr);
                        if (self->priv->debug_input)
                                g_printerr ("<<< %s\n", ptr);
                        clip_code = gibbon_session_process_server_line (session,
                                                                        ptr);
                        if (clip_code >= 0) {
                                gibbon_server_console_print_output (console,
                                                                console_output);
                        } else {
                                gibbon_server_console_print_info (console,
                                                                console_output);
                        }
                        /*
                         * Our handler may have destroyed the connection.
                         */
                        if (!GIBBON_IS_CONNECTION (self))
                                return;
                        if (clip_code == GIBBON_CLIP_WELCOME) {
                                self->priv->state = WAIT_COMMANDS;
                                g_signal_emit (self,
                                               signals[LOGGED_IN],
                                               0, self);
                        }
                        self->priv->out_ready = TRUE;
                        gibbon_connection_send_chunk (self);
                } else {
                        gibbon_server_console_print_info (console, ptr);
                }
                ptr = line_end + 1;
        }

        if (ptr != self->priv->in_buffer) {
                head = self->priv->in_buffer;
                self->priv->in_buffer = g_strdup (ptr);
                g_free (head);
        }

        if (self->priv->state == WAIT_LOGIN_PROMPT) {
                if (g_strcmp0 (self->priv->in_buffer, "login: ") == 0) {
                        gibbon_server_console_print_raw (console,
                                                         self->priv->in_buffer);
                        self->priv->out_ready = TRUE;
                        if (self->priv->guest_login) {
                                gibbon_connection_queue_command (self, FALSE,
                                                                 "guest");
                        } else {
                                package = g_strdup (PACKAGE);
                                if (*package >= 'a' && *package <= 'z')
                                        *package -= 32;
                                gibbon_connection_queue_command (self,
                                                                 FALSE,
                                                                 "login %s_%s"
                                                                 " 1008 %s %s",
                                                                 package,
                                                                 VERSION,
                                                                 self->priv->login,
                                                                 self->priv->password);
                                g_free (package);
                        }
                        g_free (self->priv->in_buffer);
                        self->priv->in_buffer = g_strdup ("");
                        self->priv->state = WAIT_WELCOME;
                        if (self->priv->guest_login) {
                                pretty_login = g_strdup ("guest");
                        } else {
                                pretty_login = g_strdup_printf ("login %s_%s"
                                                                " 1008 %s"
                                                                " ********",
                                                                PACKAGE,
                                                                VERSION,
                                                                self->priv->login);
                        }
                        gibbon_server_console_print_login (console,
                                                           pretty_login);
                        g_free (pretty_login);
                }
        } else if (self->priv->state == WAIT_WELCOME) {
                if (strcmp (self->priv->in_buffer, "login: ") == 0) {
                        gibbon_server_console_print_output (console, "login: ");
                        g_signal_emit (self, signals[NETWORK_ERROR], 0,
                                       _("Authentication failed."));
                        return;
                }
        }

        if (self->priv->guest_login) {
                if ('>' == self->priv->in_buffer[0]
                    && ' ' == self->priv->in_buffer[1]
                    && !self->priv->in_buffer[2]) {
                        gibbon_server_console_print_raw (console,
                                                         self->priv->in_buffer);
                        self->priv->out_ready = TRUE;
                        g_free (self->priv->in_buffer);
                        self->priv->in_buffer = g_strdup ("");
                        gibbon_session_handle_prompt (self->priv->session);
                } else if (0 == g_strcmp0 ("Please give your password: ",
                                           self->priv->in_buffer)) {
                        gibbon_server_console_print_raw (console,
                                                         self->priv->in_buffer);
                        self->priv->out_ready = TRUE;
                        g_free (self->priv->in_buffer);
                        self->priv->in_buffer = g_strdup ("");
                        gibbon_session_handle_pw_prompt (self->priv->session);
                } else if (0 == g_strcmp0 ("Please retype your password: ",
                                           self->priv->in_buffer)) {
                        gibbon_server_console_print_raw (console,
                                                         self->priv->in_buffer);
                        self->priv->out_ready = TRUE;
                        g_free (self->priv->in_buffer);
                        self->priv->in_buffer = g_strdup ("");
                        gibbon_session_handle_pw_prompt (self->priv->session);
                }
        }

        /*
         * Our handler may have destroyed the connection.
         */
        if (!gibbon_app_get_connection (app))
                return;

        self->priv->read_cancellable = g_cancellable_new ();
        g_input_stream_read_async (input_stream,
                                   self->priv->read_buf,
                                   sizeof self->priv->read_buf,
                                   G_PRIORITY_DEFAULT,
                                   self->priv->read_cancellable,
                                   (GAsyncReadyCallback)
                                   gibbon_connection_handle_input,
                                   self);

        return;
}

static void
gibbon_connection_handle_output (GOutputStream *output_stream,
                                 GAsyncResult *result,
                                 GibbonConnection *self)
{
        gsize bytes_written;
        GError *error = NULL;
        gchar *line;
        GibbonServerConsole *console;
        GibbonFIBSCommand *command;
        gsize pending;

        if (!self || !GIBBON_IS_CONNECTION (self))
                return;

        if (self->priv->write_cancellable)
                g_object_unref (self->priv->write_cancellable);
        self->priv->write_cancellable = NULL;

        bytes_written = g_output_stream_write_finish (output_stream, result,
                                                      &error);

        if (bytes_written < 0) {
                g_signal_emit (self, signals[NETWORK_ERROR], 0,
                               error->message);
                g_error_free (error);
                return;
        }

        command = g_list_nth_data (self->priv->out_queue, 0);
        gibbon_fibs_command_write (command, bytes_written);
        pending = gibbon_fibs_command_get_pending (command);

        if (pending <= 0) {
                console = gibbon_app_get_server_console (self->priv->app);
                line = g_strdup (
                                gibbon_fibs_command_get_line (command));
                line[strlen (line) - 2] = 0;
                if (gibbon_fibs_command_is_manual (command)) {
                        gibbon_server_console_print_info (console, line);
                } else {
                        gibbon_server_console_print_input (console, line);
                }
                g_free (line);
                g_object_unref (command);
                self->priv->out_queue = g_list_remove (self->priv->out_queue,
                                                       command);
                /*
                 * First wait for a reply from FIBS before sending the next
                 * command.
                 */
                self->priv->out_ready = FALSE;
        }

        if (self->priv->out_queue)
                gibbon_connection_send_chunk (self);
}

static void
gibbon_connection_on_connect (GObject *src_object,
                              GAsyncResult *res,
                              gpointer _self)
{
        GibbonConnection *self;
        GError *error = NULL;
        GInputStream *input_stream;
        
        /*
         * This can happen, when cancelled while eastablishing the
         * connection.
         */
        if (!_self || !GIBBON_IS_CONNECTION (_self))
                return;

        self = GIBBON_CONNECTION (_self);

        if (self->priv->connect_cancellable)
                g_object_unref (self->priv->connect_cancellable);
        self->priv->connect_cancellable = NULL;

        self->priv->socket_connection =
                g_socket_client_connect_to_host_finish (self->priv->socket_client,
                                                        res, &error);
        g_object_unref (self->priv->socket_client);
        self->priv->socket_client = NULL;

        if (!self->priv->socket_connection) {
                gibbon_connection_fatal (self, _("Error connecting to `%s'"
                                                 " port %u: %s.\n"),
                                         self->priv->hostname,
                                         self->priv->port,
                                         error->message);
                g_error_free (error);
                return;
        }

        g_signal_emit (self, signals[CONNECTED], 0, self);

        self->priv->read_cancellable = g_cancellable_new ();
        
        input_stream = g_io_stream_get_input_stream (
                        G_IO_STREAM (self->priv->socket_connection));
        g_input_stream_read_async (input_stream,
                                   self->priv->read_buf,
                                   sizeof self->priv->read_buf,
                                   G_PRIORITY_DEFAULT,
                                   self->priv->read_cancellable,
                                   (GAsyncReadyCallback)
                                   gibbon_connection_handle_input,
                                   self);
}

static void
gibbon_connection_send_chunk (GibbonConnection *self)
{
        GibbonFIBSCommand *command;
        gsize pending;
        const gchar *buffer;
        GIOStream *io_stream;
        GOutputStream *output_stream;

        g_return_if_fail (self->priv->socket_connection != NULL);
        g_return_if_fail (G_IS_SOCKET_CONNECTION (self->priv->socket_connection));

        if (!self->priv->out_queue)
                return;

        if (self->priv->write_cancellable)
                return;

        /*
         * Wait for a reply from FIBS before sending the next command.
         */
        if (!self->priv->out_ready)
                return;

        self->priv->write_cancellable = g_cancellable_new ();

        command = g_list_nth_data (self->priv->out_queue, 0);
        buffer = gibbon_fibs_command_get_pointer (command);
        pending = gibbon_fibs_command_get_pending (command);

        io_stream = G_IO_STREAM (self->priv->socket_connection);
        output_stream = g_io_stream_get_output_stream (io_stream);

        g_output_stream_write_async (output_stream, buffer, pending,
                                     G_PRIORITY_DEFAULT,
                                     self->priv->write_cancellable,
                                     (GAsyncReadyCallback)
                                     gibbon_connection_handle_output,
                                     self);
}

void
gibbon_connection_queue_command (GibbonConnection *self, 
                                 gboolean is_manual,
                                 const gchar *format, ...)
{
        va_list args;
        gchar *formatted;
        gchar *line;
        GibbonFIBSCommand *command;

        g_return_if_fail (GIBBON_IS_CONNECTION (self));
        
        va_start (args, format);
        formatted = g_strdup_vprintf (format, args);        
        va_end (args);

        if (self->priv->debug_output)
                g_printerr (">>> %s\n", formatted);

        line = g_strconcat (formatted, "\015\012", NULL);
        command = gibbon_fibs_command_new (line, is_manual);
        g_free (line);

        self->priv->out_queue = g_list_append (self->priv->out_queue, command);

        if (!self->priv->write_cancellable)
                gibbon_connection_send_chunk (self);
}

static void
gibbon_connection_fatal (GibbonConnection *self,
                         const gchar *message_format, ...)
{
        va_list args;
        gchar *message;

        va_start (args, message_format);
        message = g_strdup_vprintf (message_format, args);
        va_end (args);

        g_signal_emit (self, signals[NETWORK_ERROR], 0, message);

        g_free (message);
}

GibbonSession *
gibbon_connection_get_session (const GibbonConnection *self)
{
        g_return_val_if_fail (GIBBON_IS_CONNECTION (self), NULL);

        return self->priv->session;
}

void
gibbon_connection_send_password (GibbonConnection *self,
                                 gboolean display)
{
        GibbonServerConsole *console;

        g_return_if_fail (GIBBON_IS_CONNECTION (self));

        gibbon_connection_queue_command (self, FALSE, "%s",
                                         self->priv->password);
        if (display) {
                console = gibbon_app_get_server_console (self->priv->app);
                gibbon_server_console_print_login (console, "********");
        }
}
