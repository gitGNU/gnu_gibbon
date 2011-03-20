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

/**
 * SECTION:gibbon-java-fibs-importer
 * @short_description: Import settings and games from JavaFIBS!
 *
 * Since: 0.1.1
 *
 * Class that handles data import from JavaFIBS.
 **/

#include <gtk/gtk.h>
#include <glib/gi18n.h>

#include "gibbon-java-fibs-importer.h"
#include "gibbon-archive.h"

typedef struct _GibbonJavaFIBSImporterPrivate GibbonJavaFIBSImporterPrivate;
struct _GibbonJavaFIBSImporterPrivate {
        GibbonApp *app;

        GibbonArchive *archive;

        GtkAssistant *assistant;
        GtkFileChooserButton *file_chooser_button;

        gulong apply;
        gulong prepare;
        gulong cancel;
        gulong close;
        gulong directory_selected;

        gboolean signals_connected;
};

#define GIBBON_JAVA_FIBS_IMPORTER_PRIVATE(obj) (G_TYPE_INSTANCE_GET_PRIVATE ((obj), \
        GIBBON_TYPE_JAVA_FIBS_IMPORTER, GibbonJavaFIBSImporterPrivate))

G_DEFINE_TYPE (GibbonJavaFIBSImporter, gibbon_java_fibs_importer, G_TYPE_OBJECT)

static void gibbon_java_fibs_importer_on_cancel (GibbonJavaFIBSImporter *self,
                                                 GtkAssistant *assistant);
static void gibbon_java_fibs_importer_on_close (GibbonJavaFIBSImporter *self,
                                                GtkAssistant *assistant);
static void gibbon_java_fibs_importer_on_apply (GibbonJavaFIBSImporter *self,
                                                GtkAssistant *assistant);
static void gibbon_java_fibs_importer_on_prepare (GibbonJavaFIBSImporter *self,
                                                  GtkAssistant *assistant);
static void gibbon_java_fibs_importer_on_directory_selected (GtkObject *object,
                                                  GibbonJavaFIBSImporter *self);

static void 
gibbon_java_fibs_importer_init (GibbonJavaFIBSImporter *self)
{
        self->priv = G_TYPE_INSTANCE_GET_PRIVATE (self,
                GIBBON_TYPE_JAVA_FIBS_IMPORTER, GibbonJavaFIBSImporterPrivate);

        self->priv->app = NULL;
        self->priv->archive = NULL;
        self->priv->assistant = NULL;
        self->priv->file_chooser_button = NULL;
}

static void
gibbon_java_fibs_importer_finalize (GObject *object)
{
        GibbonJavaFIBSImporter *self = GIBBON_JAVA_FIBS_IMPORTER (object);

        if (self->priv->archive)
                g_object_unref (self->priv->archive);
        self->priv->archive = NULL;

        if (self->priv->assistant) {
                g_signal_handler_disconnect (self->priv->assistant,
                                             self->priv->apply);
                g_signal_handler_disconnect (self->priv->assistant,
                                             self->priv->close);
                g_signal_handler_disconnect (self->priv->assistant,
                                             self->priv->cancel);
                g_signal_handler_disconnect (self->priv->assistant,
                                             self->priv->prepare);
                g_signal_handler_disconnect (
                                self->priv->file_chooser_button,
                                self->priv->directory_selected);
        }

        if (self->priv->file_chooser_button) {
                g_signal_handler_disconnect (self->priv->file_chooser_button,
                                             self->priv->directory_selected);
        }

        self->priv->app = NULL;

        G_OBJECT_CLASS (gibbon_java_fibs_importer_parent_class)->finalize(object);
}

static void
gibbon_java_fibs_importer_class_init (GibbonJavaFIBSImporterClass *klass)
{
        GObjectClass *object_class = G_OBJECT_CLASS (klass);
        
        g_type_class_add_private (klass, sizeof (GibbonJavaFIBSImporterPrivate));

        object_class->finalize = gibbon_java_fibs_importer_finalize;
}

GibbonJavaFIBSImporter *
gibbon_java_fibs_importer_new (GibbonApp *app)
{
        GibbonJavaFIBSImporter *self =
                        g_object_new (GIBBON_TYPE_JAVA_FIBS_IMPORTER, NULL);
        GtkWidget *widget;
        gchar *selected_directory;

        GtkAssistant *assistant =
                GTK_ASSISTANT (gibbon_app_find_object (app,
                                                       "java_fibs_assistant",
                                                       GTK_TYPE_ASSISTANT));

        if (!assistant) {
                g_object_unref (self);
                return NULL;
        }

        self->priv->app = app;

        self->priv->assistant = assistant;

        self->priv->apply =
                g_signal_connect_swapped (G_OBJECT (assistant), "apply",
                        G_CALLBACK (gibbon_java_fibs_importer_on_apply), self);
        self->priv->close =
                g_signal_connect_swapped (G_OBJECT (assistant), "close",
                        G_CALLBACK (gibbon_java_fibs_importer_on_close), self);
        self->priv->cancel =
                g_signal_connect_swapped (G_OBJECT (assistant), "cancel",
                        G_CALLBACK (gibbon_java_fibs_importer_on_cancel), self);
        self->priv->prepare =
                g_signal_connect_swapped (G_OBJECT (assistant), "prepare",
                        G_CALLBACK (gibbon_java_fibs_importer_on_prepare), self);

        widget = GTK_WIDGET (gibbon_app_find_object (app,
                                                     "java_fibs_file_chooser_button",
                                                     GTK_TYPE_FILE_CHOOSER_BUTTON));
        self->priv->file_chooser_button = (GTK_FILE_CHOOSER_BUTTON (widget));

        selected_directory =
            gtk_file_chooser_get_filename (GTK_FILE_CHOOSER (widget));
        if (selected_directory) {
                /* TODO: Write a validation function that checks for
                 * the required directory structure.
                 */
                g_printerr ("Currently selected: %s\n", selected_directory);
                g_free (selected_directory);
        } else {
                g_printerr ("No filename selected????\n");
        }
        self->priv->directory_selected =
                g_signal_connect_swapped (G_OBJECT (widget), "file-set",
                        G_CALLBACK (gibbon_java_fibs_importer_on_directory_selected),
                        self);

        /* Set the first page to complete.  */
        widget = gtk_assistant_get_nth_page (assistant, 0);
        gtk_assistant_set_page_complete (assistant, widget, TRUE);

        /* Same for the second page.  */
        widget = gtk_assistant_get_nth_page (assistant, 1);
        gtk_assistant_set_page_complete (assistant, widget, TRUE);

        return self;
}

void
gibbon_java_fibs_importer_run (GibbonJavaFIBSImporter *self)
{
        g_return_if_fail (GIBBON_IS_JAVA_FIBS_IMPORTER (self));

        gtk_widget_show_all (GTK_WIDGET (self->priv->assistant));
}

static void
gibbon_java_fibs_importer_on_cancel (GibbonJavaFIBSImporter *self,
                                     GtkAssistant *assistant)
{
        gtk_widget_hide (GTK_WIDGET (assistant));
        g_object_unref (G_OBJECT (self));
}

static void
gibbon_java_fibs_importer_on_close (GibbonJavaFIBSImporter *self,
                                    GtkAssistant *assistant)
{
        g_printerr ("Close ...\n");
}

static void
gibbon_java_fibs_importer_on_apply (GibbonJavaFIBSImporter *self,
                                    GtkAssistant *assistant)
{
        g_printerr ("Apply ...\n");
}

static void
gibbon_java_fibs_importer_on_prepare (GibbonJavaFIBSImporter *self,
                                      GtkAssistant *assistant)
{
        g_printerr ("Prepare ...\n");
}

static void
gibbon_java_fibs_importer_on_directory_selected (GtkObject *object,
                                                 GibbonJavaFIBSImporter *self)
{
        g_printerr ("Directory selected ...\n");
}
