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

#ifndef _GIBBON_INVITER_LIST_H
#define _GIBBON_INVITER_LIST_H

#include <glib.h>

G_BEGIN_DECLS

#define GIBBON_TYPE_INVITER_LIST             (gibbon_inviter_list_get_type ())
#define GIBBON_INVITER_LIST(obj)             (G_TYPE_CHECK_INSTANCE_CAST ((obj), GIBBON_TYPE_INVITER_LIST, GibbonInviterList))
#define GIBBON_INVITER_LIST_CLASS(klass)     (G_TYPE_CHECK_CLASS_CAST ((klass), GIBBON_TYPE_INVITER_LIST, GibbonInviterListClass))
#define GIBBON_IS_INVITER_LIST(obj)          (G_TYPE_CHECK_INSTANCE_TYPE ((obj), GIBBON_TYPE_INVITER_LIST))
#define GIBBON_IS_INVITER_LIST_CLASS(klass)  (G_TYPE_CHECK_CLASS_TYPE ((klass), GIBBON_TYPE_INVITER_LIST))
#define GIBBON_INVITER_LIST_GET_CLASS(obj)   (G_TYPE_INSTANCE_GET_CLASS ((obj), GIBBON_TYPE_INVITER_LIST, GibbonInviterListClass))

typedef struct _GibbonInviterListClass   GibbonInviterListClass;
typedef struct _GibbonInviterList        GibbonInviterList;
typedef struct _GibbonInviterListPrivate GibbonInviterListPrivate;

struct _GibbonInviterListClass
{
        GObjectClass parent_class;
};

GType gibbon_inviter_list_get_type (void) G_GNUC_CONST;

struct _GibbonInviterList
{
        GObject parent_instance;
        GibbonInviterListPrivate *priv;
};

enum {
        GIBBON_INVITER_LIST_COL_NAME = 0,
        GIBBON_INVITER_LIST_COL_RATING,
        GIBBON_INVITER_LIST_COL_EXPERIENCE,
        GIBBON_INVITER_LIST_COL_RELIABILITY,
        GIBBON_INVITER_LIST_COL_SAVEDCOUNT,
        GIBBON_INVITER_LIST_COL_CLIENT,
        GIBBON_INVITER_LIST_COL_HOSTNAME,
        GIBBON_INVITER_LIST_COL_EMAIL,
        GIBBON_INVITER_LIST_N_COLUMNS
};

GibbonInviterList *gibbon_inviter_list_new (void);
void gibbon_inviter_list_connect_view (GibbonInviterList *self, 
                                       GtkTreeView *view);

void gibbon_inviter_list_clear (GibbonInviterList *self);
void gibbon_inviter_list_set (GibbonInviterList *self, 
                              const gchar *inviter_name,
                              gdouble rating,
                              guint experience,
                              gdouble reliability,
                              guint confidence,
                              const gchar *client,
                              const gchar *hostname,
                              const gchar *email);
gboolean gibbon_inviter_list_exists (const GibbonInviterList *self,
                                    const gchar *inviter_name);
void gibbon_inviter_list_remove (GibbonInviterList *self,
                                const gchar *inviter_name);

G_END_DECLS

#endif
