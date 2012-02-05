/*
 * This file is part of Gibbon, a graphical frontend to the First Internet 
 * Backgammon Server FIBS.
 * Copyright (C) 2009-2012 Guido Flohr, http://guido-flohr.net/.
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

#ifndef _LIBGSGF_TEXT_H
# define _LIBGSGF_TEXT_H

#include <glib.h>
#include <gio/gio.h>

#include <libgsgf/gsgf-cooked-value.h>

G_BEGIN_DECLS

#define GSGF_TYPE_TEXT             (gsgf_text_get_type ())
#define GSGF_TEXT(obj)             (G_TYPE_CHECK_INSTANCE_CAST ((obj), GSGF_TYPE_TEXT, GSGFText))
#define GSGF_TEXT_CLASS(klass)     (G_TYPE_CHECK_CLASS_CAST ((klass), GSGF_TYPE_TEXT, GSGFTextClass))
#define GSGF_IS_TEXT(obj)          (G_TYPE_CHECK_INSTANCE_TYPE ((obj), GSGF_TYPE_TEXT))
#define GSGF_IS_TEXT_CLASS(klass)  (G_TYPE_CHECK_CLASS_TYPE ((klass), GSGF_TYPE_TEXT))
#define GSGF_TEXT_GET_CLASS(obj)   (G_TYPE_INSTANCE_GET_CLASS ((obj), GSGF_TYPE_TEXT, GSGFTextClass))

/**
 * GSGFText:
 *
 * One instance of a #GSGFTextClass.  All properties are private.
 **/
typedef struct _GSGFText        GSGFText;
struct _GSGFText
{
        GSGFCookedValue parent_instance;

        /*< private >*/
        struct _GSGFTextPrivate *priv;
};

/**
 * GSGFTextClass:
 *
 * Class representing a text of SGF.
 **/
typedef struct _GSGFTextClass   GSGFTextClass;
struct _GSGFTextClass
{
        /*< private >*/
        GSGFCookedValueClass parent_class;

        gboolean (*set_value) (GSGFText *self, const gchar *value,
                               gboolean copy, GError **error);
        gchar * (*get_value) (const GSGFText *self);
};

GType gsgf_text_get_type(void) G_GNUC_CONST;

GSGFText* gsgf_text_new (const gchar *value);
GSGFCookedValue* gsgf_text_new_from_raw(const GSGFRaw *raw,
                                        const GSGFFlavor *flavor,
                                        const struct _GSGFProperty *property,
                                        GError **error);

gboolean gsgf_text_set_value (GSGFText *self, const gchar *value,
                              gboolean copy, GError **error);
gchar *gsgf_text_get_value(const GSGFText *self);

G_END_DECLS

#endif
