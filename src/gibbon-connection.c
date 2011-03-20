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

#include <stdlib.h>
#include <glib/gi18n.h>
#include <gio/gio.h>
#include <gtk/gtk.h>

#include <string.h>

#include "gibbon-connection.h"
#include "gibbon-connector.h"
#include "gibbon-session.h"
#include "gibbon-prefs.h"
#include "gibbon-server-console.h"
#include "gibbon-fibs-command.h"

enum gibbon_connection_signals {
        RESOLVING,
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
        guint port;
        gchar *login;
        gchar *password;
        
        enum GibbonConnectorState connector_state;
        enum GibbonConnectionState state;
        
        GibbonConnector *connector;
        gchar *error;
        
        GIOChannel *io;
        guint in_watcher;
        gchar *in_buffer;
        guint out_watcher;
        GList *out_queue;
        
        GibbonSession *session;
};

#define GIBBON_CONNECTION_DEFAULT_PORT 4321
#define GIBBON_CONNECTION_DEFAULT_HOST "fibs.com"

#define GIBBON_CONNECTION_PRIVATE(o) (G_TYPE_INSTANCE_GET_PRIVATE ((o), \
                                      GIBBON_TYPE_CONNECTION,           \
                                      GibbonConnectionPrivate))
G_DEFINE_TYPE (GibbonConnection, gibbon_connection, G_TYPE_OBJECT);

static gboolean gibbon_connection_on_input (GIOChannel *channel,
                                            GIOCondition condition,
                                            GibbonConnection *self);
static gboolean gibbon_connection_on_output (GIOChannel *channel,
                                             GIOCondition condition,
                                             GibbonConnection *self);
static gboolean gibbon_connection_wait_connect (GibbonConnection *self);

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
        
        conn->priv->state = WAIT_LOGIN_PROMPT;
        
        conn->priv->connector = NULL;
        conn->priv->connector_state = GIBBON_CONNECTOR_INITIAL;
        
        conn->priv->error = NULL;
        
        conn->priv->io = NULL;

        conn->priv->in_watcher = 0;
        conn->priv->in_buffer = g_strconcat ("", NULL);
        
        conn->priv->out_watcher = 0;
        conn->priv->out_queue = NULL;
        
        conn->priv->session = NULL;
}

static void
gibbon_connection_finalize (GObject *object)
{
        GibbonConnection *self = GIBBON_CONNECTION (object);

        if (self->priv->connector) {
                gibbon_connector_cancel (self->priv->connector);
                g_object_unref (self->priv->connector);
        }

        self->priv->connector = NULL;
        self->priv->connector_state = GIBBON_CONNECTOR_INITIAL;

        if (self->priv->in_watcher)
                g_source_remove (self->priv->in_watcher);
        self->priv->in_watcher = 0;

        if (self->priv->in_buffer)
                g_free (self->priv->in_buffer);
        self->priv->in_buffer = NULL;
        
        if (self->priv->out_watcher)
                g_source_remove (self->priv->out_watcher);
        self->priv->out_watcher = 0;

        if (self->priv->out_queue) {
                g_list_foreach (self->priv->out_queue, (GFunc) g_object_unref,
                                NULL);
                g_list_free (self->priv->out_queue);
        }
        self->priv->out_queue = NULL;

        if (self->priv->io) {
                g_io_channel_close (self->priv->io);
                g_io_channel_unref (self->priv->io);
        }
        self->priv->io = NULL;

        if (self->priv->hostname)
                g_free (self->priv->hostname);
        self->priv->hostname = NULL;

        self->priv->port = 0;

        if (self->priv->password)
                g_free (self->priv->password);
        self->priv->password = NULL;

        if (self->priv->login)
                g_free (self->priv->login);
        self->priv->login = NULL;

        if (self->priv->session)
                g_object_unref (self->priv->session);
        self->priv->session = NULL;
        
        G_OBJECT_CLASS (gibbon_connection_parent_class)->finalize (object);
}

static void
gibbon_connection_class_init (GibbonConnectionClass *klass)
{
        GObjectClass* object_class = G_OBJECT_CLASS (klass);

        g_type_class_add_private (klass, sizeof (GibbonConnectionPrivate));

        object_class->finalize = gibbon_connection_finalize;

        signals[RESOLVING] =
                g_signal_new ("resolving",
                              G_TYPE_FROM_CLASS (klass),
                              G_SIGNAL_RUN_FIRST,
                              0,
                              NULL, NULL,
                              g_cclosure_marshal_VOID__STRING,
                              G_TYPE_NONE,
                              1,
                              G_TYPE_STRING);
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
gibbon_connection_new (GibbonApp *app)
{
        GibbonConnection *self = g_object_new (GIBBON_TYPE_CONNECTION, NULL);
        gchar *hostname;
        GibbonPrefs *prefs;
        gsize i;

        g_return_val_if_fail (GIBBON_IS_APP (app), NULL);

        self->priv->app = app;
        prefs = gibbon_app_get_prefs (app);

        hostname = gibbon_prefs_get_string (prefs, GIBBON_PREFS_HOST);
        if (hostname && hostname[0])
                self->priv->hostname = g_strdup (hostname);
        else
                self->priv->hostname =
                        g_strdup (GIBBON_CONNECTION_DEFAULT_HOST);

        self->priv->port = gibbon_prefs_get_int (prefs, GIBBON_PREFS_PORT);
        if (!self->priv->port)
                self->priv->port = GIBBON_CONNECTION_DEFAULT_PORT;

        self->priv->login = gibbon_prefs_get_string (prefs, GIBBON_PREFS_LOGIN);

        /* FIXME!  If password is not saved in preferences, then this will
         * be NULL here.
         */
        self->priv->password = gibbon_prefs_get_string (prefs,
                                                        GIBBON_PREFS_PASSWORD);

        /* Make sure that the hostname is basically canonical.  */
        for (i = 0; i < strlen (self->priv->hostname); ++i)
                self->priv->hostname[i] =
                        g_ascii_tolower (self->priv->hostname[i]);

        self->priv->connector = gibbon_connector_new (self->priv->hostname,
                                                      self->priv->port);
        self->priv->state = WAIT_LOGIN_PROMPT;
        self->priv->connector_state = GIBBON_CONNECTOR_INITIAL;

        self->priv->session = gibbon_session_new (app, self);

        return self;
}

gboolean
gibbon_connection_connect (GibbonConnection *self)
{
        g_return_val_if_fail (GIBBON_IS_CONNECTION (self), FALSE);

        if (!gibbon_connector_connect (self->priv->connector)) {
                gibbon_app_display_error (self->priv->app, "%s",
                                          gibbon_connector_error (self->priv->connector));
                return FALSE;
        }

        g_timeout_add (100, (GSourceFunc) gibbon_connection_wait_connect, self);

        return TRUE;
}

const gchar *
gibbon_connection_get_hostname (const GibbonConnection *self)
{
        g_return_val_if_fail (GIBBON_IS_CONNECTION (self), NULL);
        return self->priv->hostname;
}

guint
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

static gboolean
gibbon_connection_handle_input (GibbonConnection *self, GIOChannel *channel)
{
        gchar buf[4096];
        GIOStatus status;
        gsize bytes_read;
        GError *error = NULL;
        gchar *head;
        gchar *ptr;
        gchar *line_end;
        GibbonServerConsole *console;
        gchar *pretty_login;
        gint clip_code;
        GibbonSession *session;

        g_return_val_if_fail (GIBBON_IS_CONNECTION (self), FALSE);
        
        status = g_io_channel_read_chars (channel, buf, -1 + sizeof buf, 
                                          &bytes_read, &error);
        switch (status) {
                case G_IO_STATUS_ERROR:
                        gdk_threads_enter ();
                        g_signal_emit (self, signals[NETWORK_ERROR], 0,
                                       error->message);
                        gdk_threads_leave ();
                        g_error_free (error);

                        return FALSE;
                        
                case G_IO_STATUS_EOF:
                        gdk_threads_enter ();
                        g_signal_emit (self, signals[NETWORK_ERROR], 0,
                                       _("End-of-file while receiving data from"
                                         " server."));
                        gdk_threads_leave ();

                        return FALSE;
                        
                case G_IO_STATUS_AGAIN:
                        /* FIXME! What is the appropriate reaction?  */
                        return TRUE;
                        
                case G_IO_STATUS_NORMAL:
                        break;
        }
        
        /* The input fifo is not exactly efficient.  */
        head = self->priv->in_buffer;
        buf[bytes_read] = 0;

        self->priv->in_buffer = g_strconcat (head, buf, NULL);
        g_free (head);
        
#ifndef HAVE_INDEX
#define index(str, c) memchr (str, c, strlen (str))
#endif

        console = gibbon_app_get_server_console (self->priv->app);

        ptr = self->priv->in_buffer;
        while ((line_end = index (ptr, '\012')) != NULL) {
                *line_end = 0;
                if (line_end > ptr && *(line_end - 1) == '\015')
                        *(line_end - 1) = 0;
                if (self->priv->state != WAIT_LOGIN_PROMPT) {
                        session = self->priv->session;
                        clip_code = gibbon_session_process_server_line (session,
                                                                        ptr);
                        if (clip_code == 1) {
                                self->priv->state = WAIT_COMMANDS;
                                g_signal_emit (self,
                                               signals[LOGGED_IN],
                                               0, self);
                        }
                        gibbon_server_console_print_output (console, ptr);
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
                if (strcmp (self->priv->in_buffer, "login: ") == 0) {
                        gibbon_server_console_print_raw (console,
                                                         self->priv->in_buffer);
                        gibbon_connection_queue_command (self,
                                        FALSE,
                                        "login %s_%s 9999 %s %s",
                                         PACKAGE,
                                         VERSION,
                                         self->priv->login,
                                         self->priv->password);
                        g_free (self->priv->in_buffer);
                        self->priv->in_buffer = g_strdup ("");
                        self->priv->state = WAIT_WELCOME;
                        pretty_login = g_strdup_printf ("login %s_%s 9999 %s"
                                                        " ********",
                                                        PACKAGE, VERSION,
                                                        self->priv->login);
                        gibbon_server_console_print_login (console,
                                                           pretty_login);
                        g_free (pretty_login);
                }
        } else if (self->priv->state == WAIT_WELCOME) {
                if (strcmp (self->priv->in_buffer, "login: ") == 0) {
                        gibbon_server_console_print_output (console, "login: ");
                        gdk_threads_enter ();
                        g_signal_emit (self, signals[NETWORK_ERROR], 0,
                                       _("Authentication failed."));
                        gdk_threads_leave ();
                        return FALSE;
                }
        }

        return TRUE;
}

static gboolean
gibbon_connection_on_input (GIOChannel *channel,
                            GIOCondition condition,
                            GibbonConnection *self)
{
        g_return_val_if_fail (GIBBON_IS_CONNECTION (self), TRUE);

        if (G_IO_IN & condition)
                return gibbon_connection_handle_input (self, channel);
        
        return TRUE;
}

static gboolean
gibbon_connection_on_output (GIOChannel *channel,
                             GIOCondition condition,
                             GibbonConnection *self)
{
        gsize bytes_written;
        GError *error = NULL;
        GibbonFIBSCommand *command;
        gsize pending;
        const gchar *buffer;
        gchar *line;
        GibbonServerConsole *console;

        g_return_val_if_fail (GIBBON_IS_CONNECTION (self), TRUE);
        g_return_val_if_fail (G_IO_OUT & condition, TRUE);

        if (!self->priv->out_queue) {
                g_source_remove (self->priv->out_watcher);
                self->priv->out_watcher = 0;
                return FALSE;
        }
        
        command = g_list_nth_data (self->priv->out_queue, 0);
        buffer = gibbon_fibs_command_get_pointer (command);
        pending = gibbon_fibs_command_get_pending (command);

        if (G_IO_STATUS_NORMAL != g_io_channel_write_chars (self->priv->io,
                                                            buffer,
                                                            pending,
                                                            &bytes_written,
                                                            &error)) {
                gdk_threads_enter ();
                g_signal_emit (self, signals[NETWORK_ERROR], 0,
                               error->message);
                gdk_threads_leave ();
                g_error_free (error);
                g_object_unref (command);
                self->priv->out_queue = g_list_remove (self->priv->out_queue,
                                                       command);
                return FALSE;
        }

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
        }

        if (!self->priv->out_queue) {
                g_source_remove (self->priv->out_watcher);
                self->priv->out_watcher = 0;
                return FALSE;
        }
                        
        return TRUE;
}

static void
gibbon_connection_establish (GibbonConnection *self)
{
        int socket_fd;

        g_signal_emit (self, signals[CONNECTED], 0, self);

        socket_fd = gibbon_connector_steal_socket (self->priv->connector);
        g_object_unref (self->priv->connector);
        self->priv->connector = NULL;

        self->priv->io = g_io_channel_unix_new (socket_fd);
        g_io_channel_set_encoding (self->priv->io, NULL, NULL);
        g_io_channel_set_buffered (self->priv->io, FALSE);
        self->priv->in_watcher = 
                g_io_add_watch (self->priv->io, 
                                G_IO_IN | G_IO_ERR | G_IO_HUP | G_IO_NVAL,
                                (GIOFunc) gibbon_connection_on_input,
                                self);
}

static gboolean
gibbon_connection_wait_connect (GibbonConnection *self)
{
        enum GibbonConnectorState last_state;
        GibbonConnector *connector;

        /* FIXME! Instead of silently returning false here, it would would
         * be cleaner to instead remove the timeout, when finalizing the
         * object.
         */
        if (!GIBBON_IS_CONNECTION (self))
                return FALSE;
        
        connector = self->priv->connector;
        if (!connector) {
                g_signal_emit (self, signals[NETWORK_ERROR], 0,
                               _("Could not initialize network connector."));
                return FALSE;
        }
                
        last_state = self->priv->connector_state;
        self->priv->connector_state = 
                gibbon_connector_get_state (connector);
        if (last_state == self->priv->connector_state)
                return TRUE;

        switch (self->priv->connector_state) {
                case GIBBON_CONNECTOR_INITIAL:
                        return TRUE;
                case GIBBON_CONNECTOR_RESOLVING:
                        g_signal_emit (self, signals[RESOLVING], 0, 
                                       self->priv->hostname);
                        break;
                case GIBBON_CONNECTOR_CONNECTING:
                        g_signal_emit (self, signals[CONNECTING], 0, self);
                        break;
                case GIBBON_CONNECTOR_CANCELLED:
                        g_signal_emit (self, signals[DISCONNECTED], 0,
                                       NULL);
                        return FALSE;
                case GIBBON_CONNECTOR_ERROR:
                        g_signal_emit (self, signals[NETWORK_ERROR], 0,
                                       gibbon_connector_error (connector));
                        return FALSE;
                case GIBBON_CONNECTOR_CONNECTED:
                        gibbon_connection_establish (self);
                        return FALSE;
        }
        
        return TRUE;
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

        line = g_strconcat (formatted, "\015\012", NULL);
        command = gibbon_fibs_command_new (line, is_manual);
        g_free (line);

        self->priv->out_queue = g_list_append (self->priv->out_queue, command);

        if (!self->priv->out_watcher) {
                self->priv->out_watcher = 
                        g_io_add_watch (self->priv->io, 
                                        G_IO_OUT,
                                        (GIOFunc) gibbon_connection_on_output,
                                        self);
        } 
}

void
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
