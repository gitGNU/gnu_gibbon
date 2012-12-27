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

#ifndef _GIBBON_APP_H
# define _GIBBON_APP_H

#include <glib.h>
#include <glib-object.h>

#include <gtk/gtk.h>

#include "gibbon-fibs-message.h"
#include "gibbon-match.h"

#define GIBBON_TYPE_APP \
        (gibbon_app_get_type ())
#define GIBBON_APP(obj) \
        (G_TYPE_CHECK_INSTANCE_CAST ((obj), GIBBON_TYPE_APP, \
                GibbonApp))
#define GIBBON_APP_CLASS(klass) (G_TYPE_CHECK_CLASS_CAST ((klass), \
        GIBBON_TYPE_APP, GibbonAppClass))
#define GIBBON_IS_APP(obj) \
        (G_TYPE_CHECK_INSTANCE_TYPE ((obj), \
                GIBBON_TYPE_APP))
#define GIBBON_IS_APP_CLASS(klass) \
        (G_TYPE_CHECK_CLASS_TYPE ((klass), \
                GIBBON_TYPE_APP))
#define GIBBON_APP_GET_CLASS(obj) \
        (G_TYPE_INSTANCE_GET_CLASS ((obj), \
                GIBBON_TYPE_APP, GibbonAppClass))

/**
 * GibbonApp:
 *
 * One instance of a #GibbonApp.  All properties are private.
 **/
typedef struct _GibbonApp GibbonApp;
struct _GibbonApp
{
        GObject parent_instance;

        /*< private >*/
        struct _GibbonAppPrivate *priv;
};

/**
 * GibbonAppClass:
 *
 * Class representing the application gibbon!
 **/
typedef struct _GibbonAppClass GibbonAppClass;
struct _GibbonAppClass
{
        /* <private >*/
        GObjectClass parent_class;
};

GType gibbon_app_get_type (void) G_GNUC_CONST;

gchar *gibbon_app_pixmaps_directory;
GibbonApp *app;

GibbonApp *gibbon_app_new (const gchar *builder_name, const gchar *pixmaps_dir,
                           const gchar *data_dir, const gchar *match_file);
void gibbon_app_post_init (const GibbonApp *self);
void gibbon_app_display_error (const GibbonApp *self, const gchar *headline,
                               const gchar *message_format, ...)
                               G_GNUC_PRINTF (3, 4);
void gibbon_app_fatal_error (const GibbonApp *self, const gchar *headline,
                             const gchar *message_format, ...)
                             G_GNUC_PRINTF (3, 4);
void gibbon_app_display_info (const GibbonApp *self, const gchar *headline,
                              const gchar *message_format, ...)
                              G_GNUC_PRINTF (3, 4);
GObject *gibbon_app_find_object (const GibbonApp *self, const gchar *id,
                                 GType type);
#define gibbon_app_find_widget(self, id, type) \
        GTK_WIDGET (gibbon_app_find_object (self, id, type))
GtkWidget *gibbon_app_get_window (const GibbonApp *self);
const gchar *gibbon_app_get_pixmaps_directory (const GibbonApp *app);

const gchar *gibbon_app_get_entry_text (const GibbonApp *self, const gchar *id);
const gchar *gibbon_app_get_trimmed_entry_text (const GibbonApp *self,
                                                const gchar *id);
void gibbon_app_disconnect (GibbonApp *self);
GtkImage *gibbon_app_load_scaled_image (const GibbonApp *self, 
                                        const gchar *path, 
                                        gint width, gint height);

struct _GibbonServerConsole *gibbon_app_get_server_console (const GibbonApp
                                                            *self);
struct _GibbonBoard *gibbon_app_get_board (const GibbonApp *self);
struct _GibbonConnection *gibbon_app_get_connection (const GibbonApp *self);
struct _GibbonSession *gibbon_app_get_session (const GibbonApp *self);
struct _GibbonShouts *gibbon_app_get_shouts (const GibbonApp *self);
struct _GibbonGameChat *gibbon_app_get_game_chat (const GibbonApp *self);
struct _GibbonArchive *gibbon_app_get_archive (const GibbonApp *self);
struct _GibbonPlayerList *gibbon_app_get_player_list (const GibbonApp *self);
struct _GibbonInviterList *gibbon_app_get_inviter_list (const GibbonApp *self);
struct _GibbonMatchList *gibbon_app_get_match_list (const GibbonApp *self);
struct _GibbonClientIcons *gibbon_app_get_client_icons (const GibbonApp *self);
const struct _GibbonMET *gibbon_app_get_met (const GibbonApp *self);

void gibbon_app_start_chat (GibbonApp *self, const gchar *peer);
void gibbon_app_close_chat (GibbonApp *self, const gchar *peer);
void gibbon_app_show_message (GibbonApp *self, const gchar *peer,
                              const GibbonFIBSMessage *message);
void gibbon_app_show_shout (GibbonApp *self, const GibbonFIBSMessage *message);
void gibbon_app_show_game_chat (GibbonApp *self,
                                const GibbonFIBSMessage *message);
void gibbon_app_configure_player_menu (const GibbonApp *self,
                                       const gchar *player,
                                       GtkMenu *menu);
void gibbon_app_activate_player_list (const GibbonApp *self);

/*
 * Getter/setter for the "current" match.
 */
void gibbon_app_set_match (GibbonApp *self, struct _GibbonMatch *match);
GibbonMatch *gibbon_app_get_match (GibbonApp *self);

/*
 * Wrapper around the corresponding method for GibbonMatch but for the
 * current match.  These methods fire signals on success.
 */
gboolean gibbon_app_add_action (GibbonApp *self, GibbonPositionSide side,
                                GibbonGameAction *action,
                                gint64 timestamp, GError **error);

/* State setters.  */
void gibbon_app_set_state_disconnected (GibbonApp *self);
void gibbon_app_set_state_connecting (GibbonApp *self);
void gibbon_app_set_state_available (const GibbonApp *self);
void gibbon_app_set_state_busy (const GibbonApp *self);
void gibbon_app_set_state_playing (const GibbonApp *self);
void gibbon_app_set_state_watching (const GibbonApp *self);
void gibbon_app_set_state_not_playing (const GibbonApp *self);
void gibbon_app_set_state_not_watching (const GibbonApp *self);
void gibbon_app_set_state_expect_response (const GibbonApp *self);
void gibbon_app_set_state_may_double (const GibbonApp *self, gboolean state);

#endif
