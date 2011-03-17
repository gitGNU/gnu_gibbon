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

#include <glib/gi18n.h>
#include <gtk/gtk.h>

#include <unistd.h>
#include <sys/types.h>
#include <errno.h>

#ifndef G_OS_WIN32
# include <netdb.h>
#else
# include <winsock.h>
# include <windns.h>
#endif

#include "gibbon-connector.h"
#include "gui.h"

struct _GibbonConnectorPrivate {
        gchar *hostname;
        guint port;
        gchar *error;
        enum GibbonConnectorState state;
        void* mutex;
        int socket_fd;
        GThread *worker;
};

#define GIBBON_CONNECTOR_PRIVATE(o) (G_TYPE_INSTANCE_GET_PRIVATE ((o), \
                                      GIBBON_TYPE_CONNECTOR,           \
                                      GibbonConnectorPrivate))
G_DEFINE_TYPE (GibbonConnector, gibbon_connector, G_TYPE_OBJECT);

static gpointer *gibbon_connector_connect_worker (GibbonConnector *self);

#ifdef G_OS_WIN32
static gchar *strerror_msdos (DWORD code);
#endif

static void
gibbon_connector_init (GibbonConnector *conn)
{
        conn->priv = G_TYPE_INSTANCE_GET_PRIVATE (conn, 
                                                        GIBBON_TYPE_CONNECTOR, 
                                                        GibbonConnectorPrivate);
        conn->priv->hostname = NULL;
        conn->priv->error = NULL;
        
        conn->priv->state = GIBBON_CONNECTOR_INITIAL;
        conn->priv->mutex = NULL;
        conn->priv->worker = NULL;
        
        conn->priv->socket_fd = -1;
}

static void
gibbon_connector_finalize (GObject *object)
{
        GibbonConnector *connector = GIBBON_CONNECTOR (object);

        if (connector->priv->hostname)
                g_free (connector->priv->hostname);
        connector->priv->hostname = NULL;

        if (connector->priv->error)
                g_free (connector->priv->error);        
        connector->priv->error = NULL;
        
        if (connector->priv->socket_fd >= 0)
                close (connector->priv->socket_fd);
        connector->priv->socket_fd = -1;
 
        connector->priv->worker = NULL;

        if (connector->priv->mutex)
                g_mutex_free (connector->priv->mutex);
        connector->priv->mutex = NULL;

        G_OBJECT_CLASS (gibbon_connector_parent_class)->finalize (object);
}

static void
gibbon_connector_class_init (GibbonConnectorClass *klass)
{
        GObjectClass* object_class = G_OBJECT_CLASS (klass);

        g_type_class_add_private (klass, sizeof (GibbonConnectorPrivate));

        object_class->finalize = gibbon_connector_finalize;
}

GibbonConnector *
gibbon_connector_new (const gchar *hostname, guint port)
{
         GibbonConnector *self = g_object_new (GIBBON_TYPE_CONNECTOR, NULL);

         self->priv->hostname = g_strdup (hostname);
         self->priv->port = port;
         self->priv->mutex = g_mutex_new ();
         
         return self;
}

/* TODO! When glib 2.22 becomes established, move this code to the more
 * portable GResolver API.
 */
static gpointer *
gibbon_connector_connect_worker (GibbonConnector *self)
{
        int socket_fd = -1;
        int last_errno = 0;
#ifndef G_OS_WIN32
        struct addrinfo hints;
        struct addrinfo *results;
        struct addrinfo *ai;
        int s;
        gchar *service = NULL;
#else
	DNS_STATUS resolve_status;
	DNS_RECORD *results = NULL;
	DNS_RECORD *ai;
	gchar *errmsg;
	struct sockaddr_in sa_in;
#endif

        g_mutex_lock (self->priv->mutex);
        if (self->priv->state == GIBBON_CONNECTOR_CANCELLED) {
                g_mutex_unlock (self->priv->mutex);
                gibbon_connector_cancel (self);
                return NULL;
        }

#ifndef G_OS_WIN32
        memset (&hints, 0, sizeof hints);       
        hints.ai_family = AF_UNSPEC;
        hints.ai_socktype = SOCK_STREAM;
#endif
        
        self->priv->state = GIBBON_CONNECTOR_RESOLVING;        
        g_mutex_unlock (self->priv->mutex);

#ifndef G_OS_WIN32        
        service = g_strdup_printf ("%u", self->priv->port);
        s = getaddrinfo (self->priv->hostname, service, &hints, &results);
        g_free (service);
#else
	resolve_status = DnsQuery_A (self->priv->hostname, 
				     DNS_TYPE_A,
			             DNS_QUERY_STANDARD,
				     NULL,
                                     &results,
                                     NULL);
#endif
        
        g_mutex_lock (self->priv->mutex);
        if (self->priv->state == GIBBON_CONNECTOR_CANCELLED) {
                g_mutex_unlock (self->priv->mutex);
                gibbon_connector_cancel (self);
                return NULL;
        }

#ifndef G_OS_WIN32
        if (s != 0) {
                self->priv->error = 
                        g_strdup_printf (_("Error resolving address for "
                                           "%s: %s.\n"),
                                         self->priv->hostname, 
                                         gai_strerror (s));
                self->priv->state = GIBBON_CONNECTOR_ERROR;
                g_mutex_unlock (self->priv->mutex);
                return NULL;
        }
#else
	if (resolve_status != NOERROR) {
		errmsg = strerror_msdos (resolve_status);
		self->priv->error = 
			g_strdup_printf (_("Error resolving address for "
                                           "%s: %s.\n"),
                                           self->priv->hostname, errmsg);
		g_free (errmsg);
		self->priv->state = GIBBON_CONNECTOR_ERROR;
		return NULL;
	}
#endif
        
        self->priv->state = GIBBON_CONNECTOR_CONNECTING;

        g_mutex_unlock (self->priv->mutex);

#ifndef G_OS_WIN32
        for (ai = results; ai != NULL; ai = ai->ai_next) {
                socket_fd = socket (ai->ai_family, ai->ai_socktype,
                                    ai->ai_protocol);

                if (socket_fd < 0) {
                        last_errno = errno;
                        continue;
                }
                
                if (connect (socket_fd, ai->ai_addr, ai->ai_addrlen) < 0) {
                        last_errno = errno;
                        (void) close (socket_fd);
                        continue;
                }
                break; /* Success! */
        }

        freeaddrinfo (results);
#else
        for (ai = results; ai != NULL; ai = ai->pNext) {
                if (ai->wType != DNS_TYPE_A)
                        continue;
                if (g_strcmp0 (ai->pName, self->priv->hostname))
                        continue;

                /* MS-DOS does not support IPv6?! */
                socket_fd = socket (PF_INET, SOCK_STREAM, 0);
                if (socket_fd < 0) {
                        last_errno = errno;
                        continue;
                }
                
                sa_in.sin_family = AF_INET;
                sa_in.sin_port = htons (self->priv->port);
                sa_in.sin_addr.s_addr = (unsigned long) ai->Data.A.IpAddress;
                
                if (connect (socket_fd, &sa_in, sizeof (sa_in)) < 0) {
                        last_errno = errno;
                        (void) close (socket_fd);
                        continue;
                }
                break; /* Success! */
        }
        
        DnsRecordListFree (results, DnsFreeRecordList);
#endif
        
        g_mutex_lock (self->priv->mutex);
        if (self->priv->state == GIBBON_CONNECTOR_CANCELLED) {
                g_mutex_unlock (self->priv->mutex);
                gibbon_connector_cancel (self);
                return NULL;
        }

        if (ai == NULL) {
                self->priv->state = GIBBON_CONNECTOR_ERROR;
                self->priv->error = g_strdup (strerror (last_errno));
                g_mutex_unlock (self->priv->mutex);
                return NULL;
        }

        self->priv->socket_fd = socket_fd;
        self->priv->state = GIBBON_CONNECTOR_CONNECTED;

        g_mutex_unlock (self->priv->mutex);
        
        return NULL;
}

gboolean
gibbon_connector_connect (GibbonConnector *self)
{
        GError *error;
        
        g_return_val_if_fail (GIBBON_IS_CONNECTOR (self), FALSE);

        self->priv->worker = 
                g_thread_create ((GThreadFunc) gibbon_connector_connect_worker,
                                 (gpointer) self, FALSE, &error);
        if (!self->priv->worker) {
                self->priv->error = 
                        g_strdup_printf (_("Failed to create connector " 
                                           "thread: %s."),
                                         error->message);
                g_error_free (error);
                return FALSE;
        }
        
        return TRUE;
}

void
gibbon_connector_cancel (GibbonConnector *self)
{
        g_return_if_fail (GIBBON_IS_CONNECTOR (self));

        g_mutex_lock (self->priv->mutex);
        self->priv->state = GIBBON_CONNECTOR_CANCELLED;
        g_mutex_unlock (self->priv->mutex);
}

enum GibbonConnectorState 
gibbon_connector_get_state (GibbonConnector *self)
{
        g_return_val_if_fail (GIBBON_IS_CONNECTOR (self),
                              GIBBON_CONNECTOR_ERROR);
        return self->priv->state;
}

const gchar *
gibbon_connector_error (GibbonConnector *self)
{
        g_return_val_if_fail (GIBBON_IS_CONNECTOR (self), NULL);
        return self->priv->error;
}

gint
gibbon_connector_steal_socket (GibbonConnector *self)
{
        int socket_fd;
        
        g_return_val_if_fail (GIBBON_IS_CONNECTOR (self), -1);
        g_return_val_if_fail (self->priv->socket_fd >= 0, -1);
        
        socket_fd = self->priv->socket_fd;
        self->priv->socket_fd = -1;
        
        return socket_fd;
}

#ifdef G_OS_WIN32
/* No wonder that MS-DOS applications never report errors ...  */
static gchar *
strerror_msdos (DWORD code) 
{
	WCHAR *buffer = NULL;
	gchar *retval;
	size_t length;

	/* This API is so sick that it is no wonder that MS-DOS applications
         * usually take guesses or just give numerical error codes, when it
         * comes to error reporting.  The necessary cast for the pointer to
	 * the output buffer is really weird.
         */
        /* FIXME! Get rid of the warning about de-referencing a type punned
         * pointer.
         */
	if (!FormatMessageW (FORMAT_MESSAGE_FROM_SYSTEM
		             | FORMAT_MESSAGE_ALLOCATE_BUFFER,
                             0,
                             code,
                             0,
                             (LPWSTR) &buffer,
                             0,
                             NULL)) {
		return g_strdup_printf (_("Windows error %u."), 
                                        (unsigned) code);
        }
	/* A UTF-8 character can be at most 6 bytes big, plus the terminating
         * null-byte.
         */
	length = 1 + 6 * wcslen (buffer);
	retval = g_malloc (length);
	WideCharToMultiByte (CP_UTF8, 0, buffer, -1, retval, length, 0, 0);
	LocalFree (buffer);	

	return retval;
}
#endif
