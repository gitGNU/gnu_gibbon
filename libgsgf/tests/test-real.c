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

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <stdio.h>

#include <glib/gi18n.h>

#include <libgsgf/gsgf.h>
#include <libgsgf/gsgf-private.h>

#include "test.h"

static int test_success();
static int test_garbage();
static int test_trailing_garbage();

int
main(int argc, char *argv[])
{
        int status;

        g_type_init();

        status = test_success();
        if (status)
                return status;

        status = test_garbage();
        if (status)
                return status;

        status = test_trailing_garbage();
        if (status)
                return status;

        return 0;
}

static int
test_success(void)
{
        GSGFReal *r;
        GError *error;
        const gchar *value = "1.0";
        gdouble expect = 1.0;
        gdouble got;

        r = _gsgf_real_new(value, &error);
        if (!r) {
                if (error) {
                        fprintf(stderr, "Could not parse '%s': %s.\n",
                                value, error->message);
                        return -1;
                }
                fprintf(stderr, "Could not parse '%s': No error.\n",
                        value);
                return -1;
        }

        got = gsgf_real_get_value(r);

        if ((got > expect + 0.1) || (got < expect - 0.1)) {
                fprintf(stderr, "Expected %g, got %g.\n",
                        expect, got);
                return -1;
        }
        g_object_unref(r);

        return 0;
}

static int
test_garbage(void)
{
        GSGFReal *r;
        GError *error;
        GError *expect = NULL;
        const gchar *value = "garbage";

        g_set_error(&expect, GSGF_ERROR, GSGF_ERROR_INVALID_NUMBER,
                    _("Invalid number '%s'"), value);

        r = _gsgf_real_new(value, &error);

        return expect_error_conditional(r == NULL,
                                        "Invalid double returns a value",
                                        error, expect);
}

static int
test_trailing_garbage(void)
{
        GSGFReal *r;
        GError *error;
        GError *expect = NULL;
        const gchar *value = "1.0garbage";

        g_set_error(&expect, GSGF_ERROR, GSGF_ERROR_INVALID_NUMBER,
                    _("Invalid number '%s'"), value);

        r = _gsgf_real_new(value, &error);

        return expect_error_conditional(r == NULL,
                                        "Invalid double returns a value",
                                        error, expect);
}
