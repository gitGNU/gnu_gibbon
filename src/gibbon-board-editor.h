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

#ifndef _GIBBON_BOARD_EDITOR_H
#define _GIBBON_BOARD_EDITOR_H

#include <glib.h>

#include "gibbon-design.h"

G_BEGIN_DECLS

#define GIBBON_TYPE_BOARD_EDITOR             (gibbon_board_editor_get_type ())
#define GIBBON_BOARD_EDITOR(obj)             (G_TYPE_CHECK_INSTANCE_CAST ((obj), GIBBON_TYPE_BOARD_EDITOR, GibbonBoardEditor))
#define GIBBON_BOARD_EDITOR_CLASS(klass)     (G_TYPE_CHECK_CLASS_CAST ((klass), GIBBON_TYPE_BOARD_EDITOR, GibbonBoardEditorClass))
#define GIBBON_IS_BOARD_EDITOR(obj)          (G_TYPE_CHECK_INSTANCE_TYPE ((obj), GIBBON_TYPE_BOARD_EDITOR))
#define GIBBON_IS_BOARD_EDITOR_CLASS(klass)  (G_TYPE_CHECK_CLASS_TYPE ((klass), GIBBON_TYPE_BOARD_EDITOR))
#define GIBBON_BOARD_EDITOR_GET_CLASS(obj)   (G_TYPE_INSTANCE_GET_CLASS ((obj), GIBBON_TYPE_BOARD_EDITOR, GibbonBoardEditorClass))

typedef struct _GibbonBoardEditorClass   GibbonBoardEditorClass;
typedef struct _GibbonBoardEditor        GibbonBoardEditor;
typedef struct _GibbonBoardEditorPrivate GibbonBoardEditorPrivate;

struct _GibbonBoardEditorClass
{
        GObjectClass parent_class;
};

GType gibbon_board_editor_get_type (void) G_GNUC_CONST;

struct _GibbonBoardEditor
{
        GObject parent_instance;
        GibbonBoardEditorPrivate *priv;
};

GibbonBoardEditor *gibbon_board_editor_new (GibbonDesign *design,
                                            GtkWidget *dialog);

G_END_DECLS

#endif
