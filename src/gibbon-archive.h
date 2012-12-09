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

#ifndef _GIBBON_ARCHIVE_H
# define _GIBBON_ARCHIVE_H

#include <glib.h>
#include <glib-object.h>

#include "gibbon-country.h"
#include "gibbon-match.h"

#define GIBBON_TYPE_ARCHIVE \
        (gibbon_archive_get_type ())
#define GIBBON_ARCHIVE(obj) \
        (G_TYPE_CHECK_INSTANCE_CAST ((obj), GIBBON_TYPE_ARCHIVE, \
                GibbonArchive))
#define GIBBON_ARCHIVE_CLASS(klass) (G_TYPE_CHECK_CLASS_CAST ((klass), \
        GIBBON_TYPE_ARCHIVE, GibbonArchiveClass))
#define GIBBON_IS_ARCHIVE(obj) \
        (G_TYPE_CHECK_INSTANCE_TYPE ((obj), \
                GIBBON_TYPE_ARCHIVE))
#define GIBBON_IS_ARCHIVE_CLASS(klass) \
        (G_TYPE_CHECK_CLASS_TYPE ((klass), \
                GIBBON_TYPE_ARCHIVE))
#define GIBBON_ARCHIVE_GET_CLASS(obj) \
        (G_TYPE_INSTANCE_GET_CLASS ((obj), \
                GIBBON_TYPE_ARCHIVE, GibbonArchiveClass))

/**
 * GibbonArchive:
 *
 * One instance of a #GibbonArchive.  All properties are private.
 **/
typedef struct _GibbonArchive GibbonArchive;
struct _GibbonArchive
{
        GObject parent_instance;

        /*< private >*/
        struct _GibbonArchivePrivate *priv;
};

/**
 * GibbonArchiveClass:
 *
 * Class representing the data that Gibbon saves.
 **/
typedef struct _GibbonArchiveClass GibbonArchiveClass;
struct _GibbonArchiveClass
{
        /* <private >*/
        GObjectClass parent_class;
};

GType gibbon_archive_get_type (void) G_GNUC_CONST;

typedef void (*GibbonGeoIPCallback) (GObject *obj, const gchar *hostname,
                                     const GibbonCountry *country);

GibbonArchive *gibbon_archive_new (GError **error);
/*
 * FIXME! The real purpose of this method is to initialize data structures
 * for the server and user and to set the session_directory private
 * member.  This should all be done as needed instead.
 */
gboolean gibbon_archive_login (GibbonArchive *self,
                               const gchar *hostname, guint port,
                               const gchar *login,
                               GError **error);
void gibbon_archive_update_user (GibbonArchive *self,
                                 const gchar *hostname, guint port,
                                 const gchar *user, gdouble rating,
                                 gint experience);
gboolean gibbon_archive_update_rank (GibbonArchive *self,
                                     const gchar *hostname, guint port,
                                     const gchar *user, gdouble rating,
                                     gint experience, GDateTime *dt,
                                     GError **error);
gboolean gibbon_archive_get_rank (GibbonArchive *self,
                                  const gchar *hostname, guint port,
                                  const gchar *user, gdouble *rating,
                                  guint64 *experience, GError **error);
void gibbon_archive_save_win (GibbonArchive *self,
                              const gchar *hostname, guint port,
                              const gchar *winner, const gchar *loser);
void gibbon_archive_save_drop (GibbonArchive *self,
                               const gchar *hostname, guint port,
                               const gchar *dropper, const gchar *victim);
void gibbon_archive_save_resume (GibbonArchive *self,
                                 const gchar *hostname, guint port,
                                 const gchar *player1, const gchar *player2);
gboolean gibbon_archive_get_reliability (GibbonArchive *self,
                                         const gchar *hostname, guint port,
                                         const gchar *login,
                                         gdouble *value, guint *confidence,
                                         GError **error);
struct _GibbonCountry *gibbon_archive_get_country (const GibbonArchive *self,
                                                   const gchar *hostname,
                                                   GibbonGeoIPCallback
                                                   callback,
                                                   gpointer data);

GSList *gibbon_archive_get_accounts (const GibbonArchive *self,
                                     const gchar *hostname, guint port);
gchar *gibbon_archive_get_saved_name (const GibbonArchive *self,
                                      const gchar *player1,
                                      const gchar *player2,
                                      GError **error);
gchar *gibbon_archive_get_saved_directory (const GibbonArchive *self,
                                           GError **error);
const gchar *gibbon_archive_get_servers_directory (const GibbonArchive *self);
gboolean gibbon_archive_archive_match_file (const GibbonArchive *self,
                                            const GibbonMatch *match,
                                            const gchar *match_file,
                                            GError **error);
gboolean gibbon_archive_save_match (const GibbonArchive *self,
                                    const gchar *hostname,
                                    guint port,
                                    const gchar *login,
                                    const GibbonMatch *match,
                                    GError **error);
gboolean gibbon_archive_create_group (const GibbonArchive *self,
                                      const gchar *hostname, guint port,
                                      const gchar *login, const gchar *group,
                                      GError **error);
gboolean gibbon_archive_create_relation (const GibbonArchive *self,
                                         const gchar *hostname,
                                         guint port,
                                         const gchar *login,
                                         const gchar *group,
                                         const gchar *peer,
                                         GError **error);
GHashTable *gibbon_archive_get_saved (const GibbonArchive *self,
                                      const gchar *hostname,
                                      guint port,
                                      const gchar *login);

#endif
