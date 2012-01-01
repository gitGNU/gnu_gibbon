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

#ifndef _GIBBON_FIBS_MESSAGE_H
# define _GIBBON_FIBS_MESSAGE_H

#include <glib.h>
#include <glib-object.h>

#define GIBBON_TYPE_FIBS_MESSAGE (gibbon_fibs_message_get_type ())

/**
 * GibbonFIBSMessage:
 *
 * A boxed type for messages in FIBS.
 **/
typedef struct _GibbonFIBSMessage GibbonFIBSMessage;
struct _GibbonFIBSMessage
{
        gchar *sender;
        gchar *message;
};

GType gibbon_fibs_message_get_type (void) G_GNUC_CONST;

GibbonFIBSMessage *gibbon_fibs_message_new (const gchar *sender,
                                            const gchar *message);
void gibbon_fibs_message_free (GibbonFIBSMessage *self);
gchar *gibbon_fibs_message_formatted (const GibbonFIBSMessage *self);

#endif
