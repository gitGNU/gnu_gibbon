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

#ifndef _GIBBON_FIBS_COMMAND_H
# define _GIBBON_FIBS_COMMAND_H

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <glib.h>
#include <glib-object.h>

#define GIBBON_TYPE_FIBS_COMMAND \
        (gibbon_fibs_command_get_type ())
#define GIBBON_FIBS_COMMAND(obj) \
        (G_TYPE_CHECK_INSTANCE_CAST ((obj), GIBBON_TYPE_FIBS_COMMAND, \
                GibbonFIBSCommand))
#define GIBBON_FIBS_COMMAND_CLASS(klass) (G_TYPE_CHECK_CLASS_CAST ((klass), \
        GIBBON_TYPE_FIBS_COMMAND, GibbonFIBSCommandClass))
#define GIBBON_IS_FIBS_COMMAND(obj) \
        (G_TYPE_CHECK_INSTANCE_TYPE ((obj), \
                GIBBON_TYPE_FIBS_COMMAND))
#define GIBBON_IS_FIBS_COMMAND_CLASS(klass) \
        (G_TYPE_CHECK_CLASS_TYPE ((klass), \
                GIBBON_TYPE_FIBS_COMMAND))
#define GIBBON_FIBS_COMMAND_GET_CLASS(obj) \
        (G_TYPE_INSTANCE_GET_CLASS ((obj), \
                GIBBON_TYPE_FIBS_COMMAND, GibbonFIBSCommandClass))

/**
 * GibbonFIBSCommand:
 *
 * One instance of a #GibbonFIBSCommand.  All properties are private.
 **/
typedef struct _GibbonFIBSCommand GibbonFIBSCommand;
struct _GibbonFIBSCommand
{
        GObject parent_instance;

        /*< private >*/
        struct _GibbonFIBSCommandPrivate *priv;
};

/**
 * GibbonFIBSCommandClass:
 *
 * Queued output command for FIBS communcation!
 **/
typedef struct _GibbonFIBSCommandClass GibbonFIBSCommandClass;
struct _GibbonFIBSCommandClass
{
        /* <private >*/
        GObjectClass parent_class;
};

GType gibbon_fibs_command_get_type (void) G_GNUC_CONST;

GibbonFIBSCommand *gibbon_fibs_command_new (const gchar *line,
                                            gboolean is_manual);
const gchar *gibbon_fibs_command_get_line (const GibbonFIBSCommand *self);
void gibbon_fibs_command_write (GibbonFIBSCommand *self, gsize num_bytes);
const gchar *gibbon_fibs_command_get_pointer (const GibbonFIBSCommand *self);
gsize gibbon_fibs_command_get_pending (const GibbonFIBSCommand *self);
gboolean gibbon_fibs_command_is_manual (const GibbonFIBSCommand *self);

#endif
