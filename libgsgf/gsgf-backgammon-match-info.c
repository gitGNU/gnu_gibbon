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

/**
 * SECTION:gsgf-backgammon-match-info
 * @short_description: Match information for Backgammon.
 *
 * Since: 0.2.0
 *
 * A #GSGFBackgammonMatchInfo holds meta information about a backgammon
 * match.  It resembles a GHashTable.  All keys are of type #GSGFSimpleText,
 * the values have either a qualified cooked type like #GSGFNumber or
 * the default #GSGFSimpleText.
 */

#include <errno.h>
#include <string.h>

#include <glib.h>
#include <glib/gi18n.h>

#include <libgsgf/gsgf.h>

#include <libgsgf/gsgf-backgammon-match-info.h>

#include "gsgf-private.h"

typedef struct _GSGFBackgammonMatchInfoPrivate GSGFBackgammonMatchInfoPrivate;
struct _GSGFBackgammonMatchInfoPrivate {
        GHashTable *hash;
};

#define GSGF_BACKGAMMON_MATCH_INFO_PRIVATE(obj) (G_TYPE_INSTANCE_GET_PRIVATE ((obj), \
        GSGF_TYPE_BACKGAMMON_MATCH_INFO, GSGFBackgammonMatchInfoPrivate))

G_DEFINE_TYPE (GSGFBackgammonMatchInfo, gsgf_backgammon_match_info, \
                GSGF_TYPE_COOKED_VALUE)

static void 
gsgf_backgammon_match_info_init (GSGFBackgammonMatchInfo *self)
{
        self->priv = G_TYPE_INSTANCE_GET_PRIVATE (self,
                GSGF_TYPE_BACKGAMMON_MATCH_INFO, GSGFBackgammonMatchInfoPrivate);

        self->priv->hash = NULL;
}

static void
gsgf_backgammon_match_info_finalize (GObject *object)
{
        GSGFBackgammonMatchInfo *self = GSGF_BACKGAMMON_MATCH_INFO (object);

        if (self->priv->hash)
                g_hash_table_destroy (self->priv->hash);
        self->priv->hash = NULL;

        G_OBJECT_CLASS (gsgf_backgammon_match_info_parent_class)->finalize(object);
}

static void
gsgf_backgammon_match_info_class_init (GSGFBackgammonMatchInfoClass *klass)
{
        GObjectClass *object_class = G_OBJECT_CLASS (klass);
        
        g_type_class_add_private (klass, sizeof (GSGFBackgammonMatchInfoPrivate));

        object_class->finalize = gsgf_backgammon_match_info_finalize;
}

/**
 * gsgf_backgammon_match_info_new:
 *
 * Creates a new #GSGFBackgammonMatchInfo.
 *
 * Returns: The newly created #GSGFBackgammonMatchInfo or %NULL in case of failure.
 */
GSGFBackgammonMatchInfo *
gsgf_backgammon_match_info_new (void)
{
        GSGFBackgammonMatchInfo *self = g_object_new (GSGF_TYPE_BACKGAMMON_MATCH_INFO, NULL);

        self->priv->hash = g_hash_table_new_full (g_str_hash, g_str_equal,
                                                  g_free, g_object_unref);

        return self;
}

/**
 * gsgf_match_info_new_from_raw:
 * @raw: A #GSGFRaw (multi-valued!) containing the data.
 * @flavor: The #GSGFFlavor of the current #GSGFGameTree.
 * @property: The #GSGFProperty @raw came from.
 * @error: a #GError location to store the error occuring, or %NULL to ignore.
 *
 * Creates a new #GSGFBackgammonMatchInfo from a #GSGFRaw.
 *
 * Returns: The new #GSGFBackgammonMatchInfo or %NULL in case of an error.
 */
GSGFBackgammonMatchInfo *
gsgf_backgammon_match_info_new_from_raw (const GSGFRaw *raw,
                                         const GSGFFlavor *flavor,
                                         const struct _GSGFProperty *property,
                                         GError **error)
{
        GSGFBackgammonMatchInfo *self;
        gsize num, i;
        gchar *colon;
        gchar *raw_value;
        gint64 number;
        gchar *endptr;

        gsgf_return_val_if_fail (GSGF_IS_RAW (raw), NULL, error);
        gsgf_return_val_if_fail (GSGF_IS_FLAVOR (flavor), NULL, error);
        gsgf_return_val_if_fail (GSGF_IS_PROPERTY (property), NULL, error);

        self = gsgf_backgammon_match_info_new ();

        num = gsgf_raw_get_number_of_values (raw);
        for (i = 0; i < num; ++i) {
                raw_value = gsgf_raw_get_value (raw, i);
                if (!raw_value) {
                        g_set_error (error, GSGF_ERROR,
                                     GSGF_ERROR_SEMANTIC_ERROR,
                                     _("Backgammon match information does not"
                                       " contain a colon!"));
                        g_object_unref (self);
                        return NULL;
                }
                raw_value = g_strdup (raw_value);
                colon = strchr (raw_value, ':');
                if (!colon) {
                        g_set_error (error, GSGF_ERROR,
                                     GSGF_ERROR_SEMANTIC_ERROR,
                                     _("Backgammon match information does not"
                                       " contain a colon!"));
                        g_object_unref (self);
                        g_free (raw_value);
                        return NULL;
                }
                *colon = 0;
                ++colon;
                if (!*raw_value) {
                        g_set_error (error, GSGF_ERROR,
                                     GSGF_ERROR_SEMANTIC_ERROR,
                                     _("Empty key in backgammon match"
                                       " information!"));
                        g_object_unref (self);
                        g_free (raw_value);
                        return NULL;
                }

                /* FIXME! The raw value has to be unescaped! */

                /* Qualify known types.  Actually, all of our own know types
                 * are a GSGFNumber.
                 */
                if (0 == g_strcmp0 ("length", raw_value)
                    || 0 == g_strcmp0 ("game", raw_value)
                    || 0 == g_strcmp0 ("ws", raw_value)
                    || 0 == g_strcmp0 ("bs", raw_value)) {
                        errno = 0;

                        number = g_ascii_strtoll (colon, &endptr, 012);

                        if (errno) {
                                g_set_error (error, GSGF_ERROR,
                                             GSGF_ERROR_INVALID_NUMBER,
                                             _("Invalid number '%s': %s"),
                                             colon, strerror (errno));
                                g_object_unref (self);
                                g_free (raw_value);
                                return NULL;
                        }

                        if (endptr == colon) {
                                g_set_error (error, GSGF_ERROR,
                                             GSGF_ERROR_INVALID_NUMBER,
                                             _("Invalid number '%s'"), colon);
                                g_object_unref (self);
                                g_free (raw_value);
                                return NULL;
                        }

                        if (*endptr) {
                                g_set_error (error, GSGF_ERROR,
                                            GSGF_ERROR_INVALID_NUMBER,
                                            _("Trailing garbage after number"
                                              " in '%s'"), colon);
                                g_object_unref (self);
                                g_free (raw_value);
                                return NULL;
                        }

                        if (number < 0) {
                                g_set_error (error, GSGF_ERROR,
                                             GSGF_ERROR_SEMANTIC_ERROR,
                                             _("Negative number for backgammon"
                                               " match information property"
                                               " `%s' not allowed!"),
                                               raw_value);
                                g_object_unref (self);
                                g_free (raw_value);
                                return NULL;
                        }

                        g_hash_table_insert (self->priv->hash, raw_value,
                                             gsgf_number_new (number));
                } else {
                        g_hash_table_insert (self->priv->hash, raw_value,
                                             gsgf_simple_text_new (colon));
                }
        }

        return self;
}
