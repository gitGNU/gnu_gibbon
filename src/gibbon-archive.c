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
 * SECTION:gibbon-archive
 * @short_description: Game archive.
 *
 * Since: 0.1.0
 *
 * Handling of archived games.
 *
 * One of the things handled here is the detection and evaluation of the
 * so-called "droppers", i.e. players on FIBS that leave a game, while it
 * is going on.  This is sometimes due to network problems.  Some people
 * hope to save a higher rating.
 *
 * Every player that finishes a match is assigned one point.  For each drop,
 * the dropper gets a malus of one point.  A resume is honored with 1.5
 * points but only if the associated drop happened while our session.
 * Otherwise it is impossible to find out who of the two opponents was
 * the dropper.
 *
 * This gives a malus to both types of droppers which is exactly what we want.
 * It could be argued that only finished matches that happened during our
 * session should be honored, just like only resumes after drops are honored.
 * But the interval between a drop and a resume is necessarily shorter than
 * the interface between start of a match and regular end.  And besides,
 * the result bias from the little injustice does not affect players that
 * stay long on the server.
 *
 * An exception is made for resumes against known bots.  In this case both the
 * drop and the resume is simply discarded as if it had never happened.
 * The rationale behind the exceptions is that it is legitime to play against
 * a bot under circumstances where it is likely that the match cannot be
 * finished and must be continued later.  Note that unresumed drops against
 * bots cause the same malus as against humans.
 *
 * The whole thing results in two values: One is the confidence, which is
 * simply the number of recorded events.  The other is a a rating for the
 * player's reliability, which is the average of all recorded bonusses and
 * malusses.
 **/

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <errno.h>
#include <string.h>
#include <sys/stat.h>
#include <stdlib.h>

#include <glib.h>
#include <glib/gi18n.h>
#include <glib/gstdio.h>
#include <glib/gprintf.h>

#include "gibbon-archive.h"
#include "gibbon-database.h"
#include "gibbon-util.h"
#include "gibbon-country.h"
#include "gibbon-match.h"
#include "gibbon-gmd-reader.h"
#include "gibbon-gmd-writer.h"
#include "gibbon-saved-info.h"

/* We can safely cache that across sessions.  */
static GHashTable *gibbon_archive_countries = NULL;

#define GIBBON_ARCHIVE_RE_OCTET \
        "(1[0-9][0-9]|[1-9][0-9]|2[0-4][0-9]|25[0-5]|[0-9])"
#define GIBBON_ARCHIVE_RE_IP                            \
                GIBBON_ARCHIVE_RE_OCTET                 \
                        "[-.]" GIBBON_ARCHIVE_RE_OCTET  \
                        "[-.]" GIBBON_ARCHIVE_RE_OCTET  \
                        "[-.]" GIBBON_ARCHIVE_RE_OCTET
GRegex *gibbon_archive_re_ip = NULL;

typedef struct _GibbonArchiveLookupInfo {
        gchar *hostname;
        GibbonGeoIPCallback callback;
        GibbonDatabase *database;
        gpointer data;
} GibbonArchiveLookupInfo;

typedef struct _GibbonArchivePrivate GibbonArchivePrivate;
struct _GibbonArchivePrivate {
        gchar *servers_directory;
        gchar *server_directory;
        gchar *session_directory;

        GibbonDatabase *db;

        GHashTable *droppers;
};

#define GIBBON_ARCHIVE_PRIVATE(obj) (G_TYPE_INSTANCE_GET_PRIVATE ((obj), \
        GIBBON_TYPE_ARCHIVE, GibbonArchivePrivate))

G_DEFINE_TYPE (GibbonArchive, gibbon_archive, G_TYPE_OBJECT)

static void gibbon_archive_remove_from_droppers (GibbonArchive *self,
                                                 const gchar *hostname,
                                                 guint port,
                                                 const gchar *player1,
                                                 const gchar *player2);

static void gibbon_archive_on_resolve (GObject *resolver, GAsyncResult *result,
                                       gpointer data);
static void gibbon_archive_on_resolve_ip (GObject *resolver,
                                          GAsyncResult *result,
                                          gpointer data);

static void 
gibbon_archive_init (GibbonArchive *self)
{
        self->priv = G_TYPE_INSTANCE_GET_PRIVATE (self,
                GIBBON_TYPE_ARCHIVE, GibbonArchivePrivate);

        self->priv->servers_directory = NULL;
        self->priv->server_directory = NULL;
        self->priv->session_directory = NULL;
        self->priv->db = NULL;
        self->priv->droppers = NULL;
}

static void
gibbon_archive_finalize (GObject *object)
{
        GibbonArchive *self = GIBBON_ARCHIVE (object);

        g_free (self->priv->servers_directory);
        g_free (self->priv->server_directory);
        g_free (self->priv->session_directory);

        if (self->priv->droppers)
                g_hash_table_destroy (self->priv->droppers);

        if (self->priv->db)
                g_object_unref (self->priv->db);

        G_OBJECT_CLASS (gibbon_archive_parent_class)->finalize(object);
}

static void
gibbon_archive_class_init (GibbonArchiveClass *klass)
{
        GObjectClass *object_class = G_OBJECT_CLASS (klass);
        GError *error = NULL;
        
        g_type_class_add_private (klass, sizeof (GibbonArchivePrivate));

        gibbon_archive_countries = g_hash_table_new_full (g_str_hash,
                                                          g_str_equal,
                                                          g_free, g_free);

        gibbon_archive_re_ip = g_regex_new (GIBBON_ARCHIVE_RE_IP,
                                            G_REGEX_OPTIMIZE, 0, &error);
        if (!gibbon_archive_re_ip) {
                g_error ("Compiling regular expression `%s' failed: %s!",
                         GIBBON_ARCHIVE_RE_IP, error->message);
        }

        object_class->finalize = gibbon_archive_finalize;
}

GibbonArchive *
gibbon_archive_new (GError **error)
{
        GibbonArchive *self;
        const gchar *documents_servers_directory;
        gchar *db_path;
        mode_t mode;

        self = g_object_new (GIBBON_TYPE_ARCHIVE, NULL);

        documents_servers_directory = g_get_user_data_dir ();

        if (!documents_servers_directory) {
                g_set_error_literal (error, GIBBON_ERROR, -1,
                                     _("Cannot determine user data"
                                       " directory!"));
                g_object_unref (self);
                return NULL;
        }

        self->priv->servers_directory = g_build_filename (documents_servers_directory,
                                                  PACKAGE, "servers", NULL);

#ifdef G_OS_WIN32
	mode = S_IRWXU;
#else
        mode = S_IRWXU | (S_IRWXG & ~S_IWGRP) | (S_IRWXO & ~S_IWOTH);
#endif
        if (0 != g_mkdir_with_parents (self->priv->servers_directory, mode)) {
                g_set_error (error, GIBBON_ERROR, -1,
                             _("Failed to create"
                               " server directory `%s': %s!"),
                              self->priv->servers_directory,
                              strerror (errno));
                g_object_unref (self);
                return NULL;
        }

        db_path = g_build_filename (documents_servers_directory,
                                    PACKAGE, "db.sqlite", NULL);
        self->priv->db = gibbon_database_new (db_path, error);
        g_free (db_path);

        if (!self->priv->db) {
                g_object_unref (self);
                return NULL;
        }

        self->priv->droppers = g_hash_table_new_full (g_str_hash, g_str_equal,
                                                      g_free, NULL);

        return self;
}

gboolean
gibbon_archive_login (GibbonArchive *self, const gchar *hostname,
                      guint port, const gchar *login, GError **error)
{
        gchar *session_directory;
        gchar *buf;
        mode_t mode;

        gibbon_return_val_if_fail (GIBBON_IS_ARCHIVE (self), FALSE, 
                                         error);
        gibbon_return_val_if_fail (hostname != NULL, FALSE, error);
        gibbon_return_val_if_fail (port > 0, FALSE, error);
        gibbon_return_val_if_fail (login != NULL, FALSE, error);

        session_directory = g_build_filename (self->priv->servers_directory,
                                              hostname, NULL);
        if (port != 4321) {
                buf = g_strdup_printf ("%s_%u", session_directory, port);
                g_free (session_directory);
                session_directory = buf;
        }

        g_free (self->priv->server_directory);
        self->priv->server_directory = session_directory;

        buf = g_build_filename (session_directory, login, NULL);

        g_free (self->priv->session_directory);
        self->priv->session_directory = buf;

#ifdef G_OS_WIN32
	mode = S_IRWXU;
#else
        mode = S_IRWXU | (S_IRWXG & ~S_IWGRP) | (S_IRWXO & ~S_IWOTH);
#endif
        if (0 != g_mkdir_with_parents (self->priv->session_directory, mode)) {
                g_set_error (error, GIBBON_ERROR, -1,
                             _("Failed to create directory `%s': %s!"),
                             self->priv->servers_directory,
                             strerror (errno));
                return FALSE;
        }

        if (!gibbon_database_get_server_id (self->priv->db, hostname, port,
                                            error))
                return FALSE;

        if (!gibbon_database_get_user_id (self->priv->db,
                                          hostname, port, login,
                                          error))
                return FALSE;

        return TRUE;
}

void
gibbon_archive_update_user (GibbonArchive *self,
                            const gchar *hostname, guint port,
                            const gchar *user, gdouble rating,
                            gint experience)
{
        g_return_if_fail (GIBBON_IS_ARCHIVE (self));
        g_return_if_fail (hostname != NULL);
        g_return_if_fail (user != NULL);

        gibbon_database_update_user_full (self->priv->db,
                                          hostname, port, user,
                                          rating, experience, NULL);
}

gboolean
gibbon_archive_update_rank (GibbonArchive *self,
                            const gchar *hostname, guint port,
                            const gchar *user, gdouble rating,
                            gint experience, GDateTime *dt,
                            GError **error)
{
        gchar *path;
        gchar *directory;
        mode_t mode;
        GFile *file;
        GFileOutputStream *fout;
        gchar *buffer;
        gchar *year;
        gchar month[3];
        gchar day[3];
        gint y, m, d;
        guint64 now;
        gboolean free_dt;

        gibbon_return_val_if_fail (GIBBON_IS_ARCHIVE (self), FALSE, error);
        gibbon_return_val_if_fail (hostname != NULL, FALSE, error);
        gibbon_return_val_if_fail (user != NULL, FALSE, error);

        if (!dt) {
                dt = g_date_time_new_now_local ();
                if (!dt) {
                        g_set_error_literal (error, GIBBON_ERROR, -1,
                                             _("Error retrieving current time!"));
                        return FALSE;
                }
                free_dt = TRUE;
        } else {
                free_dt = FALSE;
        }

        now = g_get_real_time ();
        g_date_time_get_ymd (dt, &y, &m, &d);
        if (free_dt)
                g_date_time_unref (dt);
        year = g_strdup_printf ("%4d", y);
        month[0] = '0' + m / 10;
        month[1] = '0' + m % 10;
        month[2] = 0;
        day[0] = '0' + d / 10;
        day[1] = '0' + d % 10;
        day[2] = 0;

        directory = g_build_filename (self->priv->session_directory,
                                      year, month, day, NULL);
#ifdef G_OS_WIN32
        mode = S_IRWXU;
#else
        mode = S_IRWXU | (S_IRWXG & ~S_IWGRP) | (S_IRWXO & ~S_IWOTH);
#endif
        if (0 != g_mkdir_with_parents (directory, mode)) {
                g_set_error (error, GIBBON_ERROR, -1,
                             _("Failed to create server directory `%s': %s!"),
                             self->priv->servers_directory,
                             strerror (errno));
                return FALSE;
        }
        g_free (directory);

        path = g_build_filename (self->priv->session_directory,
                                 year, month, day, "ranks", NULL);
        g_free (year);
        file = g_file_new_for_path (path);
        g_free (path);
        fout = g_file_append_to (file, G_FILE_CREATE_NONE, NULL, NULL);
        g_object_unref (file);
        if (fout) {
                buffer = g_strdup_printf ("%lld %f %llu\n",
                                          (long long) g_get_real_time (),
                                          rating,
                                          (unsigned long long) experience);
                g_output_stream_write_all (G_OUTPUT_STREAM (fout),
                                           buffer, strlen (buffer),
                                           NULL, NULL, NULL);
                g_free (buffer);
                g_object_unref (fout);
        }

        gibbon_database_update_rank (self->priv->db,
                                     hostname, port, user,
                                     rating, experience, now, NULL);

        return TRUE;
}

void
gibbon_archive_save_win (GibbonArchive *self,
                         const gchar *hostname, guint port,
                         const gchar *winner, const gchar *loser)
{
        g_return_if_fail (GIBBON_IS_ARCHIVE (self));
        g_return_if_fail (hostname != 0);
        g_return_if_fail (port != 0);
        g_return_if_fail (port <= 65536);
        g_return_if_fail (winner != NULL);
        g_return_if_fail (loser != NULL);

        gibbon_archive_remove_from_droppers (self, hostname, port, winner, loser);

        (void) gibbon_database_insert_activity (self->priv->db,
                                                hostname, port,
                                                winner, 1.0, NULL);
        (void) gibbon_database_insert_activity (self->priv->db,
                                                hostname, port,
                                                loser, 1.0, NULL);
}

void
gibbon_archive_save_drop (GibbonArchive *self,
                          const gchar *hostname, guint port,
                          const gchar *dropper, const gchar *victim)
{
        g_return_if_fail (GIBBON_IS_ARCHIVE (self));
        g_return_if_fail (hostname != 0);
        g_return_if_fail (port != 0);
        g_return_if_fail (port <= 65536);
        g_return_if_fail (dropper != NULL);
        g_return_if_fail (victim != NULL);

        g_hash_table_insert (self->priv->droppers,
                             g_strdup_printf ("%s:%u%s:%s",
                                              hostname, port, dropper, victim),
                             (gpointer) 1);

        (void) gibbon_database_insert_activity (self->priv->db,
                                                hostname, port,
                                                dropper, -1.0, NULL);
}

void
gibbon_archive_save_resume (GibbonArchive *self,
                            const gchar *hostname, guint port,
                            const gchar *player1, const gchar *player2)
{
        gchar *key;
        enum GibbonClientType type;

        g_return_if_fail (GIBBON_IS_ARCHIVE (self));
        g_return_if_fail (hostname != 0);
        g_return_if_fail (port != 0);
        g_return_if_fail (port <= 65536);
        g_return_if_fail (player1 != NULL);
        g_return_if_fail (player2 != NULL);

        key = g_alloca (strlen (hostname) + 5
                        + strlen (player1) + strlen (player2) + 4);

        (void) sprintf (key, "%s:%u:%s:%s", hostname, port, player1, player2);
        if (g_hash_table_remove (self->priv->droppers, key)) {
                type = gibbon_get_client_type ("", player2,
                                               hostname, port);
                if (type == GibbonClientBot) {
                        (void) gibbon_database_void_activity (self->priv->db,
                                                              hostname, port,
                                                              player1, -1.0,
                                                              NULL);
                } else {
                        (void) gibbon_database_insert_activity (self->priv->db,
                                                                hostname, port,
                                                                player1, 1.5,
                                                                NULL);
                }

                return;
        }

        (void) sprintf (key, "%s:%u:%s:%s", hostname, port, player2, player1);
        if (g_hash_table_remove (self->priv->droppers, key)) {
                type = gibbon_get_client_type ("", player1,
                                               hostname, port);
                if (type == GibbonClientBot) {
                        (void) gibbon_database_void_activity (self->priv->db,
                                                              hostname, port,
                                                              player2, -1.0,
                                                              NULL);
                } else {
                        (void) gibbon_database_insert_activity (self->priv->db,
                                                                hostname, port,
                                                                player2, 1.5,
                                                                NULL);
                }

                return;
        }
}

static void
gibbon_archive_remove_from_droppers (GibbonArchive *self,
                                     const gchar *hostname, guint port,
                                     const gchar *player1, const gchar *player2)
{
        gchar *key = g_alloca (strlen (hostname) + 5
                               + strlen (player1) + strlen (player2) + 4);

        (void) sprintf (key, "%s:%u:%s:%s", hostname, port, player1, player2);
        (void) g_hash_table_remove (self->priv->droppers, key);
        (void) sprintf (key, "%s:%u:%s:%s", hostname, port, player2, player1);
        (void) g_hash_table_remove (self->priv->droppers, key);
}

gboolean
gibbon_archive_get_reliability (GibbonArchive *self,
                                const gchar *hostname, guint port,
                                const gchar *login,
                                gdouble *value, guint *confidence,
                                GError **error)
{
        gibbon_return_val_if_fail (GIBBON_IS_ARCHIVE (self), FALSE, error);
        gibbon_return_val_if_fail (hostname != NULL, FALSE, error);
        gibbon_return_val_if_fail (port != 0, FALSE, error);
        gibbon_return_val_if_fail (port <= 65536, FALSE, error);
        gibbon_return_val_if_fail (login != NULL, FALSE, error);
        gibbon_return_val_if_fail (value != NULL, FALSE, error);
        gibbon_return_val_if_fail (confidence != NULL, FALSE, error);

        return gibbon_database_get_reliability (self->priv->db,
                                                hostname, port,
                                                login, value, confidence,
                                                error);
}

gboolean
gibbon_archive_get_rank (GibbonArchive *self,
                         const gchar *hostname, guint port,
                         const gchar *login,
                         gdouble *rating, guint64 *experience,
                         GError **error)
{
        gboolean r;

        gibbon_return_val_if_fail (GIBBON_IS_ARCHIVE (self), FALSE, error);
        gibbon_return_val_if_fail (hostname != NULL, FALSE, error);
        gibbon_return_val_if_fail (port != 0, FALSE, error);
        gibbon_return_val_if_fail (port <= 65536, FALSE, error);
        gibbon_return_val_if_fail (login != NULL, FALSE, error);
        gibbon_return_val_if_fail (rating != NULL, FALSE, error);
        gibbon_return_val_if_fail (experience != NULL, FALSE, error);

        r = gibbon_database_get_rank (self->priv->db,
                                      hostname, port,
                                      login, rating, experience,
                                      error);
        if (r)
                return TRUE;

        *rating = 1500.0;
        *experience = 0;

        if (error && !*error)
                return TRUE;

        return FALSE;
}

GibbonCountry *
gibbon_archive_get_country (const GibbonArchive *self,
                            const gchar *_hostname,
                            GibbonGeoIPCallback callback,
                            gpointer data)
{
        const gchar *alpha2;
        GibbonArchiveLookupInfo *info;
        gchar *hostname;
        GInetAddress *address;
        GResolver *resolver;
        gsize l;

        g_return_val_if_fail (GIBBON_IS_ARCHIVE (self), NULL);

        /*
         * FIBS does not necessarily show all logged in users.  Thosse that
         * suddenly pop up will have empty information.  This is hopefully
         * healed later by issuing a rawwho command on that user.
         */
        if (!_hostname || !*_hostname)
                return gibbon_country_new ("xy");

        /*
         * We do not bother normalizing the hostname.  It is the result of a
         * reverse DNS lookup done by FIBS, and it will be at least
         * unambiguous if not canonical.
         *
         * We also do not bother to use the extended lookup function here.
         * We will store "xy" for unknown IPs or unknown locations, and
         * a returned NULL means therefore that this IP is new.
         */

        alpha2 = g_hash_table_lookup (gibbon_archive_countries, _hostname);

        if (!alpha2) {
                /*
                 * We immediately insert the fallback country in the hash
                 * table in order to avoid parallel lookups of the same
                 * hostname.  We rely on the callback updating the
                 * country information of all rows.
                 */
                hostname = g_strdup (_hostname);

                /*
                 * First try to find out if it is a numerical IP address in
                 * one of the private IP ranges.
                 */
                address = g_inet_address_new_from_string (_hostname);
                if (address
                    && (g_inet_address_get_is_site_local (address)
                        || g_inet_address_get_is_loopback (address))) {
                        g_object_unref (address);
                        g_hash_table_insert (gibbon_archive_countries,
                                             hostname, g_strdup ("xl"));
                        return gibbon_country_new ("xl");
                }
                if (address)
                        g_object_unref (address);

                if (0 == g_strcmp0 (_hostname, "localhost")) {
                        g_hash_table_insert (gibbon_archive_countries,
                                             hostname, g_strdup ("xl"));
                        return gibbon_country_new ("xl");
                }

                g_hash_table_insert (gibbon_archive_countries,
                                     hostname, g_strdup ("xy"));
                alpha2 = "xy";

                info = g_malloc (sizeof *info);
                info->hostname = hostname;
                info->callback = callback;
                info->database = self->priv->db;
                info->data = data;

                resolver = g_resolver_get_default ();
                g_resolver_lookup_by_name_async (resolver,
                                                 hostname,
                                                 /* No need to cancel.  */
                                                 NULL,
                                                 gibbon_archive_on_resolve,
                                                 info);

                /*
                 * Assume the tld until the lookup is done, but only if it is
                 * a known one.
                 *
                 * FIXME! gibbon_country_new() should fall back to "xy" for
                 * unknown countries.
                 */
                l = strlen (hostname);
                if (l >= 4 && hostname[l - 3] == '.')
                        alpha2 = hostname + l - 2;
        }

        return gibbon_country_new (alpha2);
}

static void
gibbon_archive_on_resolve (GObject *oresolver, GAsyncResult *result,
                           gpointer data)
{
        GibbonArchiveLookupInfo info = *(GibbonArchiveLookupInfo *) data;
        GList *ips;
        GInetAddress *address;
        gsize address_size;
        gsize i;
        const guint8 *octets;
        guint32 key;
        gchar *alpha2;
        GibbonCountry *country;
        GMatchInfo *match_info;
        gchar *xoctets[4];
        gchar numerical_ip[16];
        GResolver *resolver;

        /*
         * The hostname pointer is still in use as a hash key, we cannot
         * g_free() it.
         */
        g_free (data);

        ips = g_resolver_lookup_by_name_finish (G_RESOLVER (oresolver),
                                                result, NULL);
        g_object_unref (oresolver);

        /*
         * FIBS does a gratuitous reverse lookup on IP addresses.  But
         * unfortunately a reverse lookup for the resolved IPs fails quite
         * often, especially for IPs assigned by ISPs to their private
         * clients.
         *
         * However, these names contain the numerical IP most of the time.
         * If we can extract such an IP, and it resolves to our original
         * name, we have worked around the problem and can still locate
         * the IP geographically.
         */
        if (!ips) {
                if (!g_regex_match (gibbon_archive_re_ip, info.hostname, 0,
                                   &match_info))
                        return;

                for (i = 0; i < 4; ++i) {
                        xoctets[i] = g_match_info_fetch (match_info, i + 1);
                }
                g_snprintf (numerical_ip, sizeof numerical_ip, "%s.%s.%s.%s",
                            xoctets[0],
                            xoctets[1],
                            xoctets[2],
                            xoctets[3]);
                g_match_info_free (match_info);

                address = g_inet_address_new_from_string (numerical_ip);

                data = g_malloc (sizeof info);
                memcpy (data, &info, sizeof info);
                resolver = g_resolver_get_default ();
                g_resolver_lookup_by_address_async (resolver,
                                                    address,
                                                    /* No need to cancel.  */
                                                    NULL,
                                                   gibbon_archive_on_resolve_ip,
                                                    data);
                return;
        }

        /*
         * The address space stored in our GeoIP database should be
         * exhaustive.  There is no point iterating over the list.  Picking
         * the first, preferred address from the list is accepatble.
         */
        address = G_INET_ADDRESS (ips->data);

        address_size = g_inet_address_get_native_size (address);
        if (4 != address_size) {
                g_resolver_free_addresses (ips);
                g_return_if_fail (4 == address_size);
        }

        octets = g_inet_address_to_bytes (address);
        key = 0;
        for (i = 0; i < 4; ++i) {
                key <<= 8;
                key += octets[i];
        }
        g_resolver_free_addresses (ips);

        alpha2 = gibbon_database_get_country (info.database, key);

        country = gibbon_country_new (alpha2);
        g_hash_table_insert (gibbon_archive_countries,
                             g_strdup (info.hostname),
                             g_strdup (gibbon_country_get_alpha2 (country)));

        info.callback (info.data, info.hostname, country);
}

static void
gibbon_archive_on_resolve_ip (GObject *resolver, GAsyncResult *result,
                              gpointer data)
{
        GibbonArchiveLookupInfo info = *(GibbonArchiveLookupInfo *) data;
        gchar *hostname;
        GMatchInfo *match_info;
        gchar *xoctets[4];
        gchar numerical_ip[16];
        guint32 key;
        GInetAddress *address;
        gint i;
        const guint8 *octets;
        gchar *alpha2;
        GibbonCountry *country;

        /*
         * The hostname pointer is still in use as a hash key, we cannot
         * g_free() it.
         */
        g_free (data);

        hostname = g_resolver_lookup_by_address_finish (G_RESOLVER (resolver),
                                                        result, NULL);
        g_object_unref (resolver);

        if (g_strcmp0 (hostname, info.hostname))
                return;
        g_free (hostname);

        /*
         * FIXME! It is not exactly efficient to match again here!
         */
        if (!g_regex_match (gibbon_archive_re_ip, info.hostname, 0,
                           &match_info)) {
                g_critical ("Could not extract numerical IP from %s",
                            info.hostname);
                return;
        }

        for (i = 0; i < 4; ++i) {
                xoctets[i] = g_match_info_fetch (match_info, i + 1);
        }
        g_snprintf (numerical_ip, sizeof numerical_ip, "%s.%s.%s.%s",
                    xoctets[0],
                    xoctets[1],
                    xoctets[2],
                    xoctets[3]);
        g_match_info_free (match_info);
        address = g_inet_address_new_from_string (numerical_ip);

        octets = g_inet_address_to_bytes (address);
        g_object_unref (address);

        key = 0;
        for (i = 0; i < 4; ++i) {
                key <<= 8;
                key += octets[i];
        }

        alpha2 = gibbon_database_get_country (info.database, key);

        country = gibbon_country_new (alpha2);
        g_hash_table_insert (gibbon_archive_countries,
                             g_strdup (info.hostname),
                             g_strdup (gibbon_country_get_alpha2 (country)));

        info.callback (info.data, info.hostname, country);
}

GSList *
gibbon_archive_get_accounts (const GibbonArchive *self,
                             const gchar *hostname, guint port)
{
        gchar *server_directory;
        gchar *buf;
        GDir *dir;
        GSList *accounts = NULL;
        const gchar *account;

        g_return_val_if_fail (GIBBON_IS_ARCHIVE (self), NULL);
        g_return_val_if_fail (hostname != NULL, NULL);
        g_return_val_if_fail (port > 0, NULL);

        server_directory = g_build_filename (self->priv->servers_directory,
                                             hostname, NULL);
        if (port != 4321) {
                buf = g_strdup_printf ("%s_%u", server_directory, port);
                g_free (server_directory);
                server_directory = buf;
        }

        dir = g_dir_open (server_directory, 0, NULL);

        if (!dir)
                return NULL;

        account = g_dir_read_name (dir);
        while (account) {
                accounts = g_slist_append (accounts, g_strdup (account));
                account = g_dir_read_name (dir);
        }

        return accounts;
}

gchar *
gibbon_archive_get_saved_directory (const GibbonArchive *self, GError **error)
{
        gchar *saved_directory;
        gint mode;

        g_return_val_if_fail (self->priv->server_directory != NULL, NULL);

        saved_directory = g_build_filename (self->priv->server_directory,
                                            "%saved",
                                            NULL);

        if (!g_file_test (saved_directory, G_FILE_TEST_IS_DIR)) {
#ifdef G_OS_WIN32
                mode = S_IRWXU;
#else
                mode = S_IRWXU | (S_IRWXG & ~S_IWGRP) | (S_IRWXO & ~S_IWOTH);
#endif
                if (0 != g_mkdir_with_parents (saved_directory, mode)) {
                        g_set_error (error, GIBBON_ERROR, -1,
                                     _("Failed to create directory `%s': %s!"),
                                     saved_directory,
                                     strerror (errno));
                        g_free (saved_directory);
                        return NULL;
                }
        }

        return saved_directory;
}

gchar *
gibbon_archive_get_saved_name (const GibbonArchive *self,
                               const gchar *player1,
                               const gchar *player2,
                               GError **error)
{
        gchar *saved_directory;
        gchar *filename, *path;

        gibbon_return_val_if_fail (GIBBON_IS_ARCHIVE (self), NULL, error);
        gibbon_return_val_if_fail (player1 != NULL, NULL, error);
        gibbon_return_val_if_fail (player2 != NULL, NULL, error);

        saved_directory = gibbon_archive_get_saved_directory (self, error);

        filename = g_strdup_printf ("%s%%%s.gmd", player1, player2);
        path = g_build_filename (saved_directory, filename, NULL);
        g_free (filename);
        g_free (saved_directory);

        return path;
}

const gchar *
gibbon_archive_get_servers_directory (const GibbonArchive *self)
{
        g_return_val_if_fail (GIBBON_IS_ARCHIVE (self), NULL);

        return self->priv->servers_directory;
}

gboolean
gibbon_archive_archive_match_file (const GibbonArchive *self,
                                   const GibbonMatch *match,
                                   const gchar *match_file,
                                   GError **error)
{
        gchar *year;
        gchar month[3];
        gchar day[3];
        guint64 start;
        GDateTime *dt;
        gint y, m, d;
        gchar *directory;
        gint mode;
        const gchar *opponent;
        gchar *filename;
        gchar *path;
        GFile *src, *dest;
        gboolean result;
        GibbonMatchReader *reader;

        gibbon_return_val_if_fail (GIBBON_IS_ARCHIVE (self), FALSE, error);

        if (!match) {
                reader = GIBBON_MATCH_READER (gibbon_gmd_reader_new (NULL,
                                                                     NULL));
                match = gibbon_match_reader_parse (reader, match_file);
                if (!match) {
                        /* Ignore errors.  */
                        (void) g_remove (match_file);
                        return TRUE;
                }
        }

        gibbon_return_val_if_fail (GIBBON_IS_MATCH (match), FALSE, error);

        start = gibbon_match_get_start_time (match);
        if (start == G_MININT64)
                start = g_get_real_time ();
        dt = g_date_time_new_from_unix_local (start / 1000000);
        if (!dt) {
                g_set_error (error,
                             GIBBON_ERROR,
                             -1,
                             _("Match start time `%lld' out of range!"),
                             (long long) start);
                return FALSE;
        }
        g_date_time_get_ymd (dt, &y, &m, &d);
        g_date_time_unref (dt);
        year = g_strdup_printf ("%4d", y);
        month[0] = '0' + m / 10;
        month[1] = '0' + m % 10;
        month[2] = 0;
        day[0] = '0' + d / 10;
        day[1] = '0' + d % 10;
        day[2] = 0;

        directory = g_build_filename (self->priv->session_directory,
                                      year, month, day, NULL);
        g_free (year);

#ifdef G_OS_WIN32
        mode = S_IRWXU;
#else
        mode = S_IRWXU | (S_IRWXG & ~S_IWGRP) | (S_IRWXO & ~S_IWOTH);
#endif
        if (0 != g_mkdir_with_parents (directory, mode)) {
                g_set_error (error,
                             GIBBON_ERROR,
                             -1,
                             _("Failed to create directory `%s': %s!"),
                               directory,
                               strerror (errno));
                return FALSE;
        }

        opponent = gibbon_match_get_black (match);
        if (start < 0)
                filename = g_strdup_printf ("-%016llx-%s.gmd",
                                            (unsigned long long) -start,
                                            opponent);
        else
                filename = g_strdup_printf ("%016llx-%s.gmd",
                                            (unsigned long long) start,
                                            opponent);
        path = g_build_filename (directory, filename, NULL);
        g_free (directory);
        g_free (filename);

        src = g_file_new_for_path (match_file);
        dest = g_file_new_for_path (path);
        result = g_file_move (src, dest, G_FILE_COPY_NONE,
                              NULL, NULL, NULL, error);
        g_object_unref (src);
        g_object_unref (dest);
        g_free (path);

        return result;
}

gboolean
gibbon_archive_save_match (const GibbonArchive *self,
                           const gchar *hostname, guint port,
                           const gchar *login,
                           const GibbonMatch *match,
                           GError **error)
{
        gchar *year;
        gchar month[3];
        gchar day[3];
        guint64 start;
        GDateTime *dt;
        gint y, m, d;
        gchar *directory;
        gint mode;
        gchar *filename;
        gchar *path;
        gboolean result;
        GibbonMatchWriter *writer;
        GFile *file = NULL;
        GFileOutputStream *fout;
        GOutputStream *out;
        gsize match_length;
        const gchar *white;
        const gchar *black;
        guint score1, score2;

        gibbon_return_val_if_fail (GIBBON_IS_ARCHIVE (self), FALSE, error);
        gibbon_return_val_if_fail (GIBBON_IS_MATCH (match), FALSE, error);

        start = gibbon_match_get_start_time (match);
        if (start == G_MININT64)
                start = g_get_real_time ();
        dt = g_date_time_new_from_unix_local (start / 1000000);
        if (!dt) {
                g_set_error (error,
                             GIBBON_ERROR,
                             -1,
                             _("Match start time `%lld' out of range!"),
                             (long long) start);
                return FALSE;
        }
        g_date_time_get_ymd (dt, &y, &m, &d);
        g_date_time_unref (dt);
        year = g_strdup_printf ("%4d", y);
        month[0] = '0' + m / 10;
        month[1] = '0' + m % 10;
        month[2] = 0;
        day[0] = '0' + d / 10;
        day[1] = '0' + d % 10;
        day[2] = 0;

        white = gibbon_match_get_white (match);
        black = gibbon_match_get_black (match);

        directory = g_build_filename (self->priv->server_directory, white,
                                      year, month, day, NULL);
        g_free (year);

#ifdef G_OS_WIN32
        mode = S_IRWXU;
#else
        mode = S_IRWXU | (S_IRWXG & ~S_IWGRP) | (S_IRWXO & ~S_IWOTH);
#endif
        if (0 != g_mkdir_with_parents (directory, mode)) {
                g_set_error (error,
                             GIBBON_ERROR,
                             -1,
                             _("Failed to create directory `%s': %s!"),
                               directory,
                               strerror (errno));
                return FALSE;
        }

        if (start < 0)
                filename = g_strdup_printf ("-%016llx-%s.gmd",
                                            (unsigned long long) -start,
                                            black);
        else
                filename = g_strdup_printf ("%016llx-%s.gmd",
                                            (unsigned long long) start,
                                            black);
        path = g_build_filename (directory, filename, NULL);
        g_free (directory);
        g_free (filename);

        file = g_file_new_for_path (path);
        g_free (path);

        fout = g_file_replace (file, NULL, FALSE, G_FILE_COPY_OVERWRITE,
                               NULL, error);
        g_object_unref (file);
        if (!fout)
                return FALSE;

        out = G_OUTPUT_STREAM (fout);

        writer = GIBBON_MATCH_WRITER (gibbon_gmd_writer_new ());
        result = gibbon_match_writer_write_stream (writer, out, match, error);

        g_object_unref (writer);
        g_object_unref (out);

        if (!result)
                return FALSE;

        match_length = gibbon_match_get_length (match);
        score1 = gibbon_match_get_white_score (match);
        score2 = gibbon_match_get_black_score (match);

        if (!gibbon_database_save_match (self->priv->db, hostname, port,
                                         white, black, start,
                                         match_length, score1, score2,
                                         error))
                return FALSE;

        return TRUE;
}

gboolean
gibbon_archive_create_group (const GibbonArchive *self,
                             const gchar *hostname, guint port,
                             const gchar *login,
                             const gchar *group,
                             GError **error)
{
        gchar *groups_dir;
        gchar *groups_file;
        gint mode;
        GFile *file;
        GFileOutputStream *out;

        gibbon_return_val_if_fail (GIBBON_IS_ARCHIVE (self), FALSE, error);
        gibbon_return_val_if_fail (hostname != NULL, FALSE, error);
        gibbon_return_val_if_fail (port > 0, FALSE, error);
        gibbon_return_val_if_fail (login != NULL, FALSE, error);
        gibbon_return_val_if_fail (group != NULL, FALSE, error);

        groups_dir = g_build_filename (self->priv->session_directory, "groups",
                                       NULL);

        if (!g_file_test (groups_dir, G_FILE_TEST_IS_DIR)) {
#ifdef G_OS_WIN32
                mode = S_IRWXU;
#else
                mode = S_IRWXU | (S_IRWXG & ~S_IWGRP) | (S_IRWXO & ~S_IWOTH);
#endif
                if (0 != g_mkdir_with_parents (groups_dir, mode)) {
                        g_set_error_literal (error, GIBBON_ERROR, -1,
                                             strerror (errno));
                        g_free (groups_dir);
                        return FALSE;
                }
        }

        groups_file = g_build_filename (groups_dir, group, NULL);

        g_free (groups_dir);

        if (!g_file_test (groups_file, G_FILE_TEST_EXISTS)) {
                file = g_file_new_for_path (groups_file);
                out = g_file_replace (file, NULL, FALSE,
                                      G_FILE_CREATE_REPLACE_DESTINATION,
                                      NULL, error);
                g_object_unref (file);
                if (!out) {
                        g_free (groups_file);
                        return FALSE;
                }
                g_object_unref (out);
        }

        g_free (groups_file);

        if (!gibbon_database_create_group (self->priv->db,
                                           hostname, port, login, group, error))
                return FALSE;

        return TRUE;
}

gboolean
gibbon_archive_create_relation (const GibbonArchive *self,
                                const gchar *hostname, guint port,
                                const gchar *login, const gchar *group,
                                const gchar *peer, GError **error)
{
        gchar *groups_file;
        GFile *file;
        GFileOutputStream *out;
        gchar *tmp;
        gchar *buffer;

        gibbon_return_val_if_fail (GIBBON_IS_ARCHIVE (self), FALSE, error);
        gibbon_return_val_if_fail (hostname != NULL, FALSE, error);
        gibbon_return_val_if_fail (port > 0, FALSE, error);
        gibbon_return_val_if_fail (login != NULL, FALSE, error);
        gibbon_return_val_if_fail (group != NULL, FALSE, error);
        gibbon_return_val_if_fail (peer != NULL, FALSE, error);

        if (gibbon_database_exists_relation (self->priv->db, hostname, port,
                                             login, group, peer))
                return TRUE;

        groups_file = g_build_filename (self->priv->session_directory, "groups",
                                        group, NULL);

        file = g_file_new_for_path (groups_file);
        out = g_file_append_to (file,
                                G_FILE_CREATE_REPLACE_DESTINATION,
                                NULL, error);
        if (!out) {
                tmp = (*error)->message;
                (*error)->message = g_strdup_printf (_("Error writing `%s': %s"),
                                                     groups_file, tmp);
                g_free (tmp);
                return FALSE;
        }

        buffer = g_strdup_printf ("%s\n", peer);
        if (!g_output_stream_write_all (G_OUTPUT_STREAM (out), buffer,
                                        strlen (buffer), NULL, NULL, error)) {
                tmp = (*error)->message;
                (*error)->message = g_strdup_printf (_("Error writing `%s': %s"),
                                                     groups_file, tmp);
                g_free (tmp);
                g_free (buffer);
                return FALSE;
        }
        g_free (buffer);

        if (!g_output_stream_close (G_OUTPUT_STREAM (out), NULL, error)) {
                tmp = (*error)->message;
                (*error)->message = g_strdup_printf (_("Error closing `%s': %s"),
                                                     groups_file, tmp);
                g_free (tmp);
                return FALSE;
        }

        g_free (groups_file);
        g_object_unref (out);

        if (!gibbon_database_create_relation (self->priv->db, hostname, port,
                                              login, group, peer, error))
                return FALSE;

        return TRUE;
}

GHashTable *
gibbon_archive_get_saved (const GibbonArchive *self,
                          const gchar *hostname,
                          guint port,
                          const gchar *login)
{
        GHashTable *table;
        gchar *saved_directory;
        GDir *dir;
        const gchar *filename;
        gsize login_length;
        const gchar *first_percent;
        gchar *path;
        GibbonMatchReader *reader;
        GibbonMatch *match;
        const gchar *white;
        const gchar *black;
        const gchar *opponent;
        gsize match_length;
        guint white_score, black_score;
        GibbonSavedInfo *saved_info;

        /*
         * The function never fails.  Even in case of errors we still return
         * an empty hash table.
         */
        table = g_hash_table_new_full (g_str_hash,
                                       g_str_equal,
                                       g_free,
                                       (GDestroyNotify) gibbon_saved_info_free);
        g_return_val_if_fail (GIBBON_IS_ARCHIVE (self), table);
        g_return_val_if_fail (hostname != NULL, table);
        g_return_val_if_fail (port != 0, table);
        g_return_val_if_fail (login != NULL, table);

        saved_directory = gibbon_archive_get_saved_directory (self, NULL);
        if (!saved_directory)
                return table;

        dir = g_dir_open (saved_directory, 0, NULL);
        if (!dir) {
                g_free (saved_directory);
                return table;
        }

        login_length = strlen (login);
        while ((filename = g_dir_read_name (dir)) != NULL) {
                first_percent = index (filename, '%');
                if (!first_percent)
                        continue;

                if ((strncmp (filename, login, login_length)
                     || filename[login_length] != '%')
                     && (strncmp (first_percent + 1, login, login_length)
                         || g_strcmp0 (first_percent + 1 + login_length,
                                        ".gmd")))
                        continue;

                reader = GIBBON_MATCH_READER (gibbon_gmd_reader_new (NULL,
                                                                     NULL));
                path = g_build_filename (saved_directory, filename, NULL);
                match = gibbon_match_reader_parse (reader, path);
                g_object_unref (reader);
                if (!match || !gibbon_match_get_current_game (match)) {
                        g_remove (path);
                        g_free (path);
                        continue;
                }
                g_free (path);

                white = gibbon_match_get_white (match);
                black = gibbon_match_get_black (match);
                match_length = gibbon_match_get_length (match);

                if (!g_strcmp0 (white, login)) {
                        opponent = black;
                        white_score = gibbon_match_get_white_score (match);
                        black_score = gibbon_match_get_black_score (match);
                } else if (!g_strcmp0 (black, login)) {
                        opponent = white;
                        white_score = gibbon_match_get_black_score (match);
                        black_score = gibbon_match_get_white_score (match);
                } else {
                        /*
                         * FIXME! Remove matches that are older than N weeks!
                         */
                        g_object_unref (match);
                        continue;
                }

                saved_info = gibbon_saved_info_new (opponent, match_length,
                                                    white_score, black_score);
                g_hash_table_insert (table, g_strdup (filename), saved_info);
                g_object_unref (match);
        }

        g_free (saved_directory);

        return table;
}
