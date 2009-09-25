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

#include <glib/gi18n.h>
#include <gtk/gtk.h>

#include <unistd.h>
#include <sys/types.h>
#include <netdb.h>
#include <errno.h>
#include <stdlib.h>

#include "gibbon.h"
#include "gibbon-connection.h"
#include "gibbon-session.h"
#include "gui.h"

#define CLIP_WELCOME 1

enum gibbon_connection_signals {
        HTML_SERVER_OUTPUT,
        LAST_SIGNAL
};
static guint signals[LAST_SIGNAL] = { 0 };

static void gibbon_session_send_server_message (GibbonSession *self, 
                                                const gchar *output);
static void gibbon_session_clip_welcome (GibbonSession *self,
                                         const gchar *message,
                                         const gchar *ptr);                                                
struct _GibbonSessionPrivate {
        gint dummy;
};

#define GIBBON_SESSION_PRIVATE(o) (G_TYPE_INSTANCE_GET_PRIVATE ((o), \
                                      GIBBON_TYPE_SESSION,           \
                                      GibbonSessionPrivate))
G_DEFINE_TYPE (GibbonSession, gibbon_session, G_TYPE_OBJECT);

#define GIBBON_SESSION_WHITESPACE " \t\r\v\f"

static void
gibbon_session_init (GibbonSession *self)
{
        self->priv = G_TYPE_INSTANCE_GET_PRIVATE (self, 
                                                  GIBBON_TYPE_SESSION, 
                                                  GibbonSessionPrivate);
}

static void
gibbon_session_finalize (GObject *object)
{
/*        GibbonSession *self = GIBBON_SESSION (object); */

        G_OBJECT_CLASS (gibbon_session_parent_class)->finalize (object);
}

static void
gibbon_session_class_init (GibbonSessionClass *klass)
{
        GObjectClass* object_class = G_OBJECT_CLASS (klass);

        g_type_class_add_private (klass, sizeof (GibbonSessionPrivate));

        object_class->finalize = gibbon_session_finalize;

        signals[HTML_SERVER_OUTPUT] =
                g_signal_new ("html-server-output",
                              G_TYPE_FROM_CLASS (klass),
                              G_SIGNAL_RUN_FIRST,
                              0,
                              NULL, NULL,
                              g_cclosure_marshal_VOID__STRING,
                              G_TYPE_NONE,
                              1,
                              G_TYPE_STRING);
}

GibbonSession *
gibbon_session_new ()
{
        GibbonSession *self = g_object_new (GIBBON_TYPE_SESSION, NULL);

        return self;
}

static void
gibbon_session_clip_welcome (GibbonSession *self, 
                             const gchar *message, const gchar *ptr)
{
        const gchar* login = gibbon_connection_get_login (connection);
        gchar **tokens;
        GTimeVal last_login;
        gchar *last_login_str;
        gchar *reply;
        gchar *mail;
        
        g_return_if_fail (GIBBON_IS_SESSION (self));

        tokens = g_strsplit_set (ptr, GIBBON_SESSION_WHITESPACE, 4);
        
        last_login.tv_usec = 0;
        if (tokens[1])
                last_login.tv_sec = strtol (tokens[1], NULL, 10);
        
        if (!tokens[0] || strcmp (tokens[0], login)) {
                g_print ("Parser expected `%s' as login: %s\n",
                         login, message);
                gibbon_session_send_server_message (self, message);
        } else if (last_login.tv_usec < 0) {
                g_print ("Parser expected timestamp after login `%s': %s\n",
                         login, message);
                gibbon_session_send_server_message (self, message);
        } else if (!tokens[2]) {
                g_print ("Parser expected address after timestamp: %s\n",
                         message);
                gibbon_session_send_server_message (self, message);
        } else if (tokens[3]) {
                g_print ("Trailing garbage after address: %s\n", message);
                gibbon_session_send_server_message (self, message);
        } else {
                /* FIXME! Isn't there a better way to format a date and time
                 * in glib?
                 */
                last_login_str = ctime (&last_login.tv_sec);
                last_login_str[strlen (last_login_str) - 1] = 0;
                
                reply = g_strdup_printf (_("Last login on %s from %s."),
                                         last_login_str, tokens[2]);
                gibbon_session_send_server_message (self, reply);
                g_free (reply);
                
                mail = gconf_client_get_string (conf_client,
                                                GIBBON_GCONF_SERVER_PREFS_PREFIX
                                                "address", NULL);
                if (mail) {
                        gibbon_connection_queue_command (connection, 
                                                         "address %s",
                                                         mail);
                        g_free (mail);
                }
        }
        
        g_strfreev (tokens);
}

static void
gibbon_session_dispatch_clip_message (GibbonSession *self,
                                      const gchar *message)
{
        unsigned long int code;
        gchar *endptr;
        
        g_return_if_fail (GIBBON_IS_SESSION (self));
        
        code = strtoul (message, &endptr, 10);
        
        /* Skip whitespace.  */
        while (*endptr == ' ' || *endptr == '\t' || *endptr == '\r'
               || *endptr == '\f' || *endptr == '\v')
                endptr++;
        
        switch (code) {
                case CLIP_WELCOME:
                        gibbon_session_clip_welcome (self, message, endptr);
                        break;
                default: 
                        gibbon_session_send_server_message (self, message);
        }
}

static void
gibbon_session_send_server_message (GibbonSession *self,
                                    const gchar *output)
{
        /* FIXME! HTML escape the message first and embed it
         * in <em>emphasized</em>.
         */
        g_signal_emit (self, signals[HTML_SERVER_OUTPUT], 0, output);
}

G_MODULE_EXPORT void
gibbon_session_server_output_cb (GibbonSession *self, 
                                 const gchar *output,
                                 GObject *emitter)
{
        g_return_if_fail (GIBBON_IS_SESSION (self));

        if (output[0] >= '0' && output[0] <= '9') {
                gibbon_session_dispatch_clip_message (self, output);
                return;
        }
        
        gibbon_session_send_server_message (self, output);
}
