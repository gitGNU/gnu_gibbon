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

#include <stdlib.h>
#include <glib/gi18n.h>
#include <gio/gio.h>
#include <gtk/gtk.h>

#include <netdb.h>
#include <string.h>

#include "gibbon-connection.h"
#include "gibbon-connector.h"
#include "gui.h"

enum gibbon_connection_signals {
        RESOLVING,
        CONNECTING,
        CONNECTED,
        LOGIN,
        LOGGED_IN,
        DISCONNECTED,
        RAW_SERVER_OUTPUT,
        COOKED_SERVER_OUTPUT,
        LAST_SIGNAL
};
static guint signals[LAST_SIGNAL] = { 0 };

enum GibbonConnectionState {
        IDLE,
        WAIT_LOGIN_PROMPT,
        WAIT_WELCOME,
        WAIT_COMMANDS
};

struct _GibbonConnectionPrivate {
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
        gchar *out_buffer;
        
        GArray* command_queue;
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
        
        conn->priv->state = IDLE;
        
        conn->priv->connector = NULL;
        conn->priv->connector_state = GIBBON_CONNECTOR_INITIAL;
        
        conn->priv->error = NULL;
        
        conn->priv->io = NULL;

        conn->priv->in_watcher = 0;
        conn->priv->in_buffer = g_strconcat ("", NULL);
        
        conn->priv->out_watcher = 0;
        conn->priv->out_buffer = g_strconcat ("", NULL);
}

static void
gibbon_connection_finalize (GObject *object)
{
        GibbonConnection *conn = GIBBON_CONNECTION (object);
        
        if (conn->priv->hostname)
                g_free (conn->priv->hostname);
        conn->priv->hostname = NULL;

        conn->priv->port = 0;

        if (conn->priv->password)
                g_free (conn->priv->password);
        conn->priv->password = NULL;

        if (conn->priv->login)
                g_free (conn->priv->login);
        conn->priv->login = NULL;

        if (conn->priv->connector)
                gibbon_connector_cancel (conn->priv->connector);
        conn->priv->connector = NULL;

        if (conn->priv->error)
                g_free (conn->priv->error);
        g_free (conn->priv->error);

        if (conn->priv->in_watcher)
                g_source_remove (conn->priv->in_watcher);
        conn->priv->in_watcher = 0;
        
        if (conn->priv->out_watcher)
                g_source_remove (conn->priv->out_watcher);
        conn->priv->out_watcher = 0;
        
        if (conn->priv->io)
                g_io_channel_shutdown (conn->priv->io, FALSE, NULL);
        conn->priv->io = NULL;
        
        if (conn->priv->in_buffer)
                g_free (conn->priv->in_buffer);
        conn->priv->in_buffer = NULL;
        
        if (conn->priv->out_buffer)
                g_free (conn->priv->out_buffer);
        conn->priv->out_buffer = NULL;
        
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
                              g_cclosure_marshal_VOID__STRING,
                              G_TYPE_NONE,
                              1,
                              G_TYPE_STRING);
        signals[CONNECTED] =
                g_signal_new ("connected",
                              G_TYPE_FROM_CLASS (klass),
                              G_SIGNAL_RUN_FIRST,
                              0,
                              NULL, NULL,
                              g_cclosure_marshal_VOID__STRING,
                              G_TYPE_NONE,
                              1,
                              G_TYPE_STRING);
        signals[LOGIN] =
                g_signal_new ("login",
                              G_TYPE_FROM_CLASS (klass),
                              G_SIGNAL_RUN_FIRST,
                              0,
                              NULL, NULL,
                              g_cclosure_marshal_VOID__STRING,
                              G_TYPE_NONE,
                              1,
                              G_TYPE_STRING);
        signals[LOGGED_IN] =
                g_signal_new ("logged_in",
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
                              g_cclosure_marshal_VOID__STRING,
                              G_TYPE_NONE,
                              1,
                              G_TYPE_STRING);
        signals[RAW_SERVER_OUTPUT] =
                g_signal_new ("raw-server-output",
                              G_TYPE_FROM_CLASS (klass),
                              G_SIGNAL_RUN_FIRST,
                              0,
                              NULL, NULL,
                              g_cclosure_marshal_VOID__STRING,
                              G_TYPE_NONE,
                              1,
                              G_TYPE_STRING);
        signals[COOKED_SERVER_OUTPUT] =
                g_signal_new ("cooked-server-output",
                              G_TYPE_FROM_CLASS (klass),
                              G_SIGNAL_RUN_FIRST,
                              0,
                              NULL, NULL,
                              g_cclosure_marshal_VOID__STRING,
                              G_TYPE_NONE,
                              1,
                              G_TYPE_STRING);
}

GibbonConnection *
gibbon_connection_new (void)
{
        return g_object_new (GIBBON_TYPE_CONNECTION, NULL);
}

void
gibbon_connection_set_hostname (GibbonConnection *self, const gchar *hostname)
{
        const gchar *new_hostname = NULL;
        
        g_return_if_fail (GIBBON_IS_CONNECTION (self));
        
        if (!(hostname && hostname[0])) {
                new_hostname = GIBBON_CONNECTION_DEFAULT_HOST;
        } else {
                new_hostname = hostname;
        }
        
        if (self->priv->hostname)
                g_free (self->priv->hostname);
        
        self->priv->hostname = g_strdup (new_hostname);
}

const gchar *
gibbon_connection_get_hostname (GibbonConnection *self)
{
        g_return_val_if_fail (GIBBON_IS_CONNECTION (self), NULL);
        return self->priv->hostname;
}

guint
gibbon_connection_get_port (GibbonConnection *self)
{
        g_return_val_if_fail (GIBBON_IS_CONNECTION (self), 0);
        return self->priv->port;
}

const gchar *
gibbon_connection_get_login (GibbonConnection *self)
{
        g_return_val_if_fail (GIBBON_IS_CONNECTION (self), NULL);
        return self->priv->login;
}

const gchar *
gibbon_connection_get_password (GibbonConnection *self)
{
        g_return_val_if_fail (GIBBON_IS_CONNECTION (self), NULL);
        return self->priv->password;
}

void
gibbon_connection_set_port (GibbonConnection *self, guint port)
{
        g_return_if_fail (GIBBON_IS_CONNECTION (self));
        
        if (!port) {
                port = GIBBON_CONNECTION_DEFAULT_PORT;
        }
        
        self->priv->port = port;
}

void
gibbon_connection_set_login (GibbonConnection *self, const gchar *login)
{
        g_return_if_fail (GIBBON_IS_CONNECTION (self));
        
        g_assert (login && login[0]);
        
        if (self->priv->login)
                g_free (self->priv->login);
        
        self->priv->login = g_strdup (login);
}

void
gibbon_connection_set_password (GibbonConnection *self, const gchar *password)
{
        g_return_if_fail (GIBBON_IS_CONNECTION (self));
        
        g_assert (password && password[0]);
        
        if (self->priv->password)
                g_free (self->priv->password);
        
        self->priv->password = g_strdup (password);
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
        guint signal;
                
        g_return_val_if_fail (GIBBON_IS_CONNECTION (self), FALSE);
        
        if (self->priv->state == WAIT_COMMANDS)
                signal = COOKED_SERVER_OUTPUT;
        else
                signal = RAW_SERVER_OUTPUT; 
        
        status = g_io_channel_read_chars (channel, buf, -1 + sizeof buf, 
                                          &bytes_read, &error);
        if (status != G_IO_STATUS_NORMAL) {
                gdk_threads_enter ();
                display_error (_("Error receiving data from server: %s.\n"),
                               error->message);
                gdk_threads_leave ();
                g_error_free (error);
                
                gibbon_connection_disconnect (self);
                return FALSE;
        }
        
        /* The input fifo is not exactly efficient.  */
        head = self->priv->in_buffer;
        buf[bytes_read] = 0;

        self->priv->in_buffer = g_strconcat (head, buf, NULL);
        g_free (head);
        
        ptr = self->priv->in_buffer;
        while ((line_end = index (ptr, '\012')) != NULL) {
                *line_end = 0;
                if (line_end > ptr && *(line_end - 1) == '\015')
                        *(line_end - 1) = 0;
                gdk_threads_enter ();
                if (self->priv->state == WAIT_WELCOME
                    && '1' == ptr[0]
                    && ' ' == ptr[1]
                    && (0 == strncmp (ptr + 2, self->priv->login,
                                      strlen (self->priv->login)))) {
                        self->priv->state = WAIT_COMMANDS;
                        signal = COOKED_SERVER_OUTPUT;
                        g_signal_emit (self, signals[LOGGED_IN], 0,
                                       self->priv->hostname);
                }
                g_signal_emit (self, signals[signal], 0, ptr);
                gdk_threads_leave ();
                ptr = line_end + 1;
        }
        if (ptr != self->priv->in_buffer) {
                head = self->priv->in_buffer;
                self->priv->in_buffer = g_strdup (ptr);
                g_free (head);
        }

        /* Only while not logged in: */
        if (self->priv->state == WAIT_LOGIN_PROMPT 
           && strcmp (self->priv->in_buffer, "login: ") == 0) {
                gdk_threads_enter ();
                g_signal_emit (self, signals[RAW_SERVER_OUTPUT], 0,
                               self->priv->in_buffer);
                gdk_threads_leave ();
                gibbon_connection_queue_command (self,
                                                 "login %s_%s 9999 %s %s",
                                                 PACKAGE,
                                                 VERSION,
                                                 self->priv->login,
                                                 self->priv->password);
                g_free (self->priv->in_buffer);
                self->priv->in_buffer = g_strdup ("");
                self->priv->state = WAIT_WELCOME;
        } else if (self->priv->state == WAIT_WELCOME
                   && strcmp (self->priv->in_buffer, "login: ") == 0) {
                gdk_threads_enter ();
                display_error (_("Authentication failed!"));
                gdk_threads_leave ();
                gibbon_connection_disconnect (self);
                return FALSE;
        }
        
        return TRUE;
}

static gboolean
gibbon_connection_on_input (GIOChannel *channel,
                            GIOCondition condition,
                            GibbonConnection *self)
{
        g_return_val_if_fail (GIBBON_IS_CONNECTION (self), TRUE);

        if (G_IO_IN & condition) {
                return gibbon_connection_handle_input (self, channel);
        }
        
        return TRUE;
}

static gboolean
gibbon_connection_on_output (GIOChannel *channel,
                             GIOCondition condition,
                             GibbonConnection *self)
{
        gchar *buffer;
        gsize bytes_written;
        GError *error = NULL;
        
        g_return_val_if_fail (GIBBON_IS_CONNECTION (self), TRUE);
        g_return_val_if_fail (G_IO_OUT & condition, TRUE);
        
        buffer = self->priv->out_buffer;
        
        if (G_IO_STATUS_NORMAL != g_io_channel_write_chars (self->priv->io,
                                                            buffer,
                                                            strlen (buffer),
                                                            &bytes_written,
                                                            &error)) {
                gdk_threads_enter ();
                display_error (_("Error while sending data to server: %s.\n"),
                               error->message);
                gdk_threads_leave ();
                g_error_free (error);
                gibbon_connection_disconnect (self);
                return FALSE;
        }
        
        if (bytes_written >= strlen (buffer)) {
                g_source_remove (self->priv->out_watcher);
                return FALSE;
        }
                        
        /* Not exactly efficient to only write one byte at a time
         * but it avoids blocking and our strings are short.  */
        strcpy (buffer, buffer + bytes_written);
        
        return TRUE;
}

static void
gibbon_connection_establish (GibbonConnection *self)
{
        int socket_fd;
        
        g_return_if_fail (GIBBON_IS_CONNECTION (self));

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
        
        g_return_val_if_fail (GIBBON_IS_CONNECTION (self), FALSE);
        
        connector = self->priv->connector;
        if (!connector)
                return FALSE;
                
        last_state = self->priv->connector_state;
        self->priv->connector_state = 
                gibbon_connector_get_state (connector);
        
        if (last_state == self->priv->connector_state)
                return TRUE;

        switch (self->priv->connector_state) {
                case GIBBON_CONNECTOR_INITIAL:
                        return FALSE;
                case GIBBON_CONNECTOR_RESOLVING:
                        g_signal_emit (self, signals[RESOLVING], 0, 
                                       self->priv->hostname);
                        break;
                case GIBBON_CONNECTOR_CONNECTING:
                        g_signal_emit (self, signals[CONNECTING], 0, 
                                       self->priv->hostname);
                        break;
                case GIBBON_CONNECTOR_CANCELLED:
                        return FALSE;
                case GIBBON_CONNECTOR_ERROR:
                        gibbon_connection_disconnect (self);
                        return FALSE;
                case GIBBON_CONNECTOR_CONNECTED:
                        gibbon_connection_establish (self);
                        return FALSE;
        }
        
        return TRUE;
}

void
gibbon_connection_connect (GibbonConnection *self)
{
        g_return_if_fail (GIBBON_IS_CONNECTION (self));

        if (self->priv->connector)
                g_object_unref (self->priv->connector);

        self->priv->connector = gibbon_connector_new (self->priv->hostname,
                                                      self->priv->port);
        self->priv->state = WAIT_LOGIN_PROMPT; 
        self->priv->connector_state = GIBBON_CONNECTOR_INITIAL;
        
        if (!gibbon_connector_connect (self->priv->connector)) {
                display_error (gibbon_connector_error (self->priv->connector));
                gibbon_connection_disconnect (self);
                return;
        }
        
        g_timeout_add (100, (GSourceFunc) gibbon_connection_wait_connect, self);
}

void
gibbon_connection_queue_command (GibbonConnection *self, 
                                 const gchar *format, ...)
{
        va_list args;
        gchar *formatted;
        gchar *new_buf;
        
        g_return_if_fail (GIBBON_IS_CONNECTION (self));
        
        va_start (args, format);
        formatted = g_strdup_vprintf (format, args);        
        va_end (args);
        
        new_buf = g_strconcat (self->priv->out_buffer, formatted, 
                               "\015\012", NULL);
        g_free (self->priv->out_buffer);
        self->priv->out_buffer = new_buf;

        g_signal_emit (G_OBJECT (self), signals[LOGIN], 0, NULL);

        if (!self->priv->out_watcher) {
                self->priv->out_watcher = 
                        g_io_add_watch (self->priv->io, 
                                        G_IO_OUT,
                                        (GIOFunc) gibbon_connection_on_output,
                                        self);
        }        
}

void
gibbon_connection_disconnect (GibbonConnection *self)
{
        gchar *error= NULL;
        
        g_return_if_fail (GIBBON_IS_CONNECTION (self));

        self->priv->state = IDLE;

        if (self->priv->connector) {
                error = g_strdup (gibbon_connector_error (self->priv->connector));
                gibbon_connector_cancel (self->priv->connector);
        }
        self->priv->connector = NULL;
        self->priv->connector_state = GIBBON_CONNECTOR_INITIAL;
        
        if (self->priv->in_watcher)
                g_source_remove (self->priv->in_watcher);
        self->priv->in_watcher = 0;
        
        if (self->priv->in_buffer)
                g_free (self->priv->in_buffer);
        self->priv->in_buffer = g_strdup ("");
        
        if (self->priv->out_watcher)
                g_source_remove (self->priv->out_watcher);
        self->priv->out_watcher = 0;
        
        if (self->priv->out_buffer)
                g_free (self->priv->out_buffer);
        self->priv->out_buffer = g_strdup ("");
        
        if (self->priv->io) {
                g_io_channel_close (self->priv->io);
                g_io_channel_unref (self->priv->io);
        }
        self->priv->io = NULL;
        
        g_signal_emit (G_OBJECT (self), signals[DISCONNECTED], 0, 
                       error);
        
        if (error) {
                gdk_threads_enter ();
                display_error (error);
                gdk_threads_leave (); 
                g_free (error);
        }
}
