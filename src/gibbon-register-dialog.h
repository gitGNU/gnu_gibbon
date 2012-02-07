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

#ifndef _GIBBON_REGISTER_DIALOG_H
# define _GIBBON_REGISTER_DIALOG_H

#include <gtk/gtk.h>

#include "gibbon-app.h"

#define GIBBON_TYPE_REGISTER_DIALOG \
        (gibbon_register_dialog_get_type ())
#define GIBBON_REGISTER_DIALOG(obj) \
        (G_TYPE_CHECK_INSTANCE_CAST ((obj), GIBBON_TYPE_REGISTER_DIALOG, \
                GibbonRegisterDialog))
#define GIBBON_REGISTER_DIALOG_CLASS(klass) (G_TYPE_CHECK_CLASS_CAST ((klass), \
        GIBBON_TYPE_REGISTER_DIALOG, GibbonRegisterDialogClass))
#define GIBBON_IS_REGISTER_DIALOG(obj) \
        (G_TYPE_CHECK_INSTANCE_TYPE ((obj), \
                GIBBON_TYPE_REGISTER_DIALOG))
#define GIBBON_IS_REGISTER_DIALOG_CLASS(klass) \
        (G_TYPE_CHECK_CLASS_TYPE ((klass), \
                GIBBON_TYPE_REGISTER_DIALOG))
#define GIBBON_REGISTER_DIALOG_GET_CLASS(obj) \
        (G_TYPE_INSTANCE_GET_CLASS ((obj), \
                GIBBON_TYPE_REGISTER_DIALOG, GibbonRegisterDialogClass))

/**
 * GibbonRegisterDialog:
 *
 * One instance of a #GibbonRegisterDialog.  All properties are private.
 **/
typedef struct _GibbonRegisterDialog GibbonRegisterDialog;
struct _GibbonRegisterDialog
{
        GtkDialog parent_instance;

        /*< private >*/
        struct _GibbonRegisterDialogPrivate *priv;
};

/**
 * GibbonRegisterDialogClass:
 *
 * Class representing the registration dialog for Gibbon.
 **/
typedef struct _GibbonRegisterDialogClass GibbonRegisterDialogClass;
struct _GibbonRegisterDialogClass
{
        /* <private >*/
        GtkDialogClass parent_class;
};

GType gibbon_register_dialog_get_type (void) G_GNUC_CONST;

GibbonRegisterDialog *gibbon_register_dialog_new (GibbonApp *app);
const gchar *gibbon_register_dialog_get_password (const GibbonRegisterDialog
                                                  *self);
gboolean gibbon_register_dialog_okay (const GibbonRegisterDialog *self);

#endif
