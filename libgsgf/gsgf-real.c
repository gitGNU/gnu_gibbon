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

/**
 * SECTION:gsgf-real
 * @short_description: Strong primitive type for SGF floating point numbers.
 *
 * A #GSGFReal encapsulates an SGF integer.  Its main purpose is to allow
 * for type checking when retrieving or setting SGF properties.
 */

#include <glib.h>
#include <glib/gi18n.h>
#include <errno.h>

#include <libgsgf/gsgf.h>

#include <math.h>

typedef struct _GSGFRealPrivate GSGFRealPrivate;
struct _GSGFRealPrivate {
        gdouble value;
};

#define GSGF_REAL_PRIVATE(o) (G_TYPE_INSTANCE_GET_PRIVATE ((o), \
                                      GSGF_TYPE_REAL,           \
                                      GSGFRealPrivate))

G_DEFINE_TYPE (GSGFReal, gsgf_real, GSGF_TYPE_COOKED_VALUE)

static gboolean gsgf_real_write_stream(const GSGFCookedValue *self,
                                       GOutputStream *out, gsize *bytes_written,
                                       GCancellable *cancellable,
                                       GError **error);

static GRegex *double_pattern = NULL;

static void
gsgf_real_init(GSGFReal *self)
{
        self->priv = G_TYPE_INSTANCE_GET_PRIVATE (self,
                        GSGF_TYPE_REAL,
                        GSGFRealPrivate);

        self->priv->value = 0;
}

static void
gsgf_real_finalize(GObject *object)
{
        G_OBJECT_CLASS (gsgf_real_parent_class)->finalize(object);
}

static void
gsgf_real_class_init(GSGFRealClass *klass)
{
        GObjectClass* object_class = G_OBJECT_CLASS (klass);
        GSGFCookedValueClass *cooked_value_class =
                        GSGF_COOKED_VALUE_CLASS (klass);

        cooked_value_class->write_stream = gsgf_real_write_stream;

        double_pattern = g_regex_new("^[+-]?[0-9]+(?:\\.[0-9]+)?$", 0, 0, NULL);

        g_type_class_add_private(klass, sizeof(GSGFRealPrivate));

        object_class->finalize = gsgf_real_finalize;
}

/**
 * gsgf_real_new:
 * @value: The value to store as a string or %NULL.
 *
 * Creates a new #GSGFReal.
 *
 * Returns: The new #GSGFReal.
 */
GSGFReal *
gsgf_real_new(gdouble value)
{
        GSGFReal *self = g_object_new(GSGF_TYPE_REAL, NULL);

        self->priv->value = value;

        return self;
}

GSGFReal *
_gsgf_real_new(const gchar *string, GError **error)
{
        gdouble value;

        g_return_val_if_fail(string != NULL, NULL);

        if (error)
                *error = NULL;

        if (!double_pattern)
                double_pattern = g_regex_new("^[+-]?[0-9]+(?:\\.[0-9]+)?$", 0, 0, NULL);

        if (!g_regex_match(double_pattern, string, 0, NULL)) {
                g_set_error(error, GSGF_ERROR, GSGF_ERROR_INVALID_NUMBER,
                            _("Invalid number '%s'"), string);
                return NULL;
        }

        value = g_ascii_strtod(string, NULL);

        if (errno) {
                g_set_error(error, GSGF_ERROR, GSGF_ERROR_INVALID_NUMBER,
                            _("Invalid number '%s': %s"), string, strerror(errno));
                return NULL;
        }

        return gsgf_real_new(value);
}

/**
 * gsgf_real_new_from_raw:
 * @raw: A #GSGFRaw containing exactly one value that should be stored.
 * @flavor: The #GSGFFlavor of the current #GSGFGameTree.
 * @property: The #GSGFProperty @raw came from.
 * @error: a #GError location to store the error occuring, or %NULL to ignore.
 *
 * Creates a new #GSGFReal from a #GSGFRaw.  This constructor is only
 * interesting for people that write their own #GSGFFlavor.
 *
 * Returns: The new #GSGFReal or %NULL in case of an error.
 */
GSGFCookedValue *
gsgf_real_new_from_raw(const GSGFRaw *raw, const GSGFFlavor *flavor,
                       const GSGFProperty *property, GError **error)
{
        gchar *endptr;
        gdouble value;
        const gchar *string;

        g_return_val_if_fail(GSGF_IS_RAW(raw), NULL);

        if (error)
                *error = NULL;

        if (1 != gsgf_raw_get_number_of_values(raw)) {
                g_set_error(error, GSGF_ERROR, GSGF_ERROR_LIST_TOO_LONG,
                            _("Only one value allowed for property"));
                return NULL;
        }
        string = gsgf_raw_get_value(raw, 0);

        /* _gsgf_real_new implicitely resets errno.  We do the same explicitely.  */
        errno = 0;

        value = g_ascii_strtod(string, &endptr);

        if (errno) {
                g_set_error(error, GSGF_ERROR, GSGF_ERROR_INVALID_NUMBER,
                            _("Invalid number '%s': %s"), string, strerror(errno));
                return NULL;
        }

        if (endptr == string) {
                g_set_error(error, GSGF_ERROR, GSGF_ERROR_INVALID_NUMBER,
                            _("Invalid number '%s'"), string);
                return NULL;
        }

        if (*endptr) {
                g_set_error(error, GSGF_ERROR, GSGF_ERROR_INVALID_NUMBER,
                            _("Trailing garbage after number in '%s'"), string);
                return NULL;
        }

        return GSGF_COOKED_VALUE(gsgf_real_new(value));
}

/**
 * gsgf_real_set_value:
 * @self: The #GSGFReal.
 * @value: The new value to store.
 *
 * Stores a new value in a #GSGFReal.
 */
void
gsgf_real_set_value(GSGFReal *self, gdouble value)
{
        g_return_if_fail(GSGF_IS_REAL(self));

        self->priv->value = value;
}

/**
 * gsgf_real_get_value:
 * @self: The #GSGFReal.
 *
 * Retrieve the value stored in a #GSGFReal.
 *
 * Returns: the value stored.
 */
gdouble
gsgf_real_get_value(const GSGFReal *self)
{
        g_return_val_if_fail(GSGF_IS_REAL(self), 0);

        return self->priv->value;
}

/**
 * gsgf_real_to_string:
 * @self: The #GSGFReal.
 *
 * Converts a #GSGFReal into a textual representation.  This representation
 * meets the specification of the SGF real data type.
 *
 * In SGF, a real number is at least one decimal digit, optionally followed
 * by a decimal point and at least one decimal digit for the fractional
 * part.  There are no locale-specific group separators, no exponentional
 * notation, and the decimal point is always a period.  In other words,
 * this function is kind of a counterpart to g_ascii_strtod().
 *
 * Positive or negative infinity values are handled gracefully and converted
 * to the maximum resp. minimum possible values.
 *
 * If the @self stores NaN, or ir @self is not a #GSGFReal,
 * %NULL is returned.
 *
 * Returns: The string (which has to be freed with g_free()) or %NULL.
 */
gchar *
gsgf_real_to_string (const GSGFReal *self)
{
        GString *string;
        gchar *result;
        gdouble integer;
        gdouble fraction;
        gint exp10;
        gdouble divisor;
        gchar digit;
        gsize valid_length;
        gdouble value;

        g_return_val_if_fail (GSGF_IS_REAL (self), NULL);

        if (isnan (self->priv->value)) {
                return NULL;
        } else if (isinf (self->priv->value) > 0) {
                value = +G_MAXDOUBLE;
        } else if (isinf (self->priv->value) < 0) {
                value = -G_MAXDOUBLE;
        } else {
                value = self->priv->value;
        }

        string = g_string_new ("");

        fraction = modf (value, &integer);

        if (integer < 0) {
                integer = -integer;
                fraction = -fraction;
                g_string_assign (string, "-");
        }

        if (!integer) {
                g_string_assign (string, "0");
        } else {
                for (exp10 = (gint) floor (log10 (integer));
                     exp10 >= 0;
                     --exp10) {
                        divisor = pow (10, exp10);
                        digit = floor (integer / divisor);

                        g_string_append_c (string, '0' + digit);
                        integer -= divisor * digit;
                }
        }

        g_string_append_c (string, '.');

        while (fraction > 0.0000000001) {
                fraction *= 10;
                digit = floor (fraction);
                g_string_append_c (string, digit + '0');
                fraction -= digit;
        }

        for (valid_length = string->len; ; --valid_length) {
                if (string->str[valid_length - 1] == '.') {
                        --valid_length;
                        break;
                }
                if (string->str[valid_length - 1] != '0')
                        break;
        }
        if (valid_length != string->len)
                g_string_truncate (string, valid_length);

        result = string->str;

        g_string_free (string, FALSE);

        return result;
}

static gboolean
gsgf_real_write_stream(const GSGFCookedValue *_self,
                       GOutputStream *out, gsize *bytes_written,
                       GCancellable *cancellable, GError **error)
{
        GSGFNumber *self = GSGF_REAL (_self);
        gchar *value;

        *bytes_written = 0;

        value = gsgf_real_to_string (self);
        if (!value) {
                g_set_error (error, GSGF_ERROR, GSGF_ERROR_NAN,
                             _("Not a number"));
                g_free (value);
                return NULL;
        }

        if (!g_output_stream_write_all (out, value, strlen(value),
                                        bytes_written,
                                        cancellable, error)) {
                g_free (value);
                return FALSE;
        }

        g_free(value);

        return TRUE;
}
