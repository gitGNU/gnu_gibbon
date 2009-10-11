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

#include "gibbon-board-editor.h"
#include "gibbon-cairoboard.h"
#include "gui.h"

struct _GibbonBoardEditorPrivate {
        GibbonCairoboard *board;
        GibbonDesign *design;
        GtkWidget *dialog;
        GtkWidget *hbox;
};

#define GIBBON_BOARD_EDITOR_PRIVATE(o) (G_TYPE_INSTANCE_GET_PRIVATE ((o), \
                                  GIBBON_TYPE_BOARD_EDITOR,           \
                                  GibbonBoardEditorPrivate))
G_DEFINE_TYPE (GibbonBoardEditor, gibbon_board_editor, G_TYPE_OBJECT);

static void
gibbon_board_editor_init (GibbonBoardEditor *self)
{
        self->priv = G_TYPE_INSTANCE_GET_PRIVATE (self, 
                                                  GIBBON_TYPE_BOARD_EDITOR, 
                                                  GibbonBoardEditorPrivate);
        
        self->priv->dialog = NULL;
        self->priv->board = NULL;
        self->priv->design = NULL;
}


static void
gibbon_board_editor_finalize (GObject *object)
{
        GibbonBoardEditor *self = GIBBON_BOARD_EDITOR (object);
        
        if (self->priv->board)
                g_object_unref (self->priv->board);
        self->priv->board = NULL;
        
        /* The board will unref its design.  */
        self->priv->design = NULL;

        G_OBJECT_CLASS (gibbon_board_editor_parent_class)->finalize (object);
}

static void
gibbon_board_editor_class_init (GibbonBoardEditorClass *klass)
{
        GObjectClass* parent_class = G_OBJECT_CLASS (klass);

        g_type_class_add_private (klass, sizeof (GibbonBoardEditorPrivate));

        G_OBJECT_CLASS (parent_class)->finalize = gibbon_board_editor_finalize;
}

GibbonBoardEditor *
gibbon_board_editor_new (GibbonDesign *design, GtkWidget *dialog)
{
        GibbonBoardEditor *self = g_object_new (GIBBON_TYPE_BOARD_EDITOR, NULL);
        
        self->priv->design = design;
        self->priv->board = gibbon_cairoboard_new (design);
        self->priv->dialog = dialog;
        
        gtk_widget_show (GTK_WIDGET (self->priv->board));
        
        self->priv->hbox = GTK_WIDGET (gtk_builder_get_object (builder, 
                                                   "board_editor_hbox"));

        gtk_box_pack_end (GTK_BOX (self->priv->hbox), 
                          GTK_WIDGET (self->priv->board),
                          TRUE, TRUE, 0);
        
        gtk_dialog_run (GTK_DIALOG (dialog));

        return self;
}
