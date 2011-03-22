/*
 * This file is part of gibbon.
 * Gibbon is a Gtk+ frontend for the First Internet Backgammon Server FIBS.
 * Copyright (C) 2009-2011 Guido Flohr, http://guido-flohr.net/.
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

#ifndef _GIBBON_ACCOUNT_DIALOG_H
# define _GIBBON_ACCOUNT_DIALOG_H

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <glib.h>
#include <glib-object.h>

#include "gibbon-app.h"

#define GIBBON_TYPE_ACCOUNT_DIALOG \
        (gibbon_account_dialog_get_type ())
#define GIBBON_ACCOUNT_DIALOG(obj) \
        (G_TYPE_CHECK_INSTANCE_CAST ((obj), GIBBON_TYPE_ACCOUNT_DIALOG, \
                GibbonAccountDialog))
#define GIBBON_ACCOUNT_DIALOG_CLASS(klass) (G_TYPE_CHECK_CLASS_CAST ((klass), \
        GIBBON_TYPE_ACCOUNT_DIALOG, GibbonAccountDialogClass))
#define GIBBON_IS_ACCOUNT_DIALOG(obj) \
        (G_TYPE_CHECK_INSTANCE_TYPE ((obj), \
                GIBBON_TYPE_ACCOUNT_DIALOG))
#define GIBBON_IS_ACCOUNT_DIALOG_CLASS(klass) \
        (G_TYPE_CHECK_CLASS_TYPE ((klass), \
                GIBBON_TYPE_ACCOUNT_DIALOG))
#define GIBBON_ACCOUNT_DIALOG_GET_CLASS(obj) \
        (G_TYPE_INSTANCE_GET_CLASS ((obj), \
                GIBBON_TYPE_ACCOUNT_DIALOG, GibbonAccountDialogClass))

/**
 * GibbonAccountDialog:
 *
 * One instance of a #GibbonAccountDialog.  All properties are private.
 */
typedef struct _GibbonAccountDialog GibbonAccountDialog;
struct _GibbonAccountDialog
{
        GObject parent_instance;

        /*< private >*/
        struct _GibbonAccountDialogPrivate *priv;
};

/**
 * GibbonAccountDialogClass:
 *
 * The dialog for the account settings.
 */
typedef struct _GibbonAccountDialogClass GibbonAccountDialogClass;
struct _GibbonAccountDialogClass
{
        /* <private >*/
        GObjectClass parent_class;
};

GType gibbon_account_dialog_get_type (void) G_GNUC_CONST;

GibbonAccountDialog *gibbon_account_dialog_new (GibbonApp *app);
void gibbon_account_dialog_show (GibbonAccountDialog *self);

#endif
