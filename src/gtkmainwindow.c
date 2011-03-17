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

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <glib/gi18n.h>
#include <gtk/gtk.h>

#include "gibbon.h"
#include "gibbon-connection.h"
#include "gui.h"
#include "gibbon-java-fibs-importer.h"

G_MODULE_EXPORT void
on_import_java_fibs_menu_item_activate (GtkObject *object, gpointer user_data)
{
        GibbonJavaFIBSImporter *importer = gibbon_java_fibs_importer_new ();
        gibbon_java_fibs_importer_run (importer);
}
