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
        GSGFNumber *r;
        GError *error;
        GSGFRaw *value = gsgf_raw_new("2503");
        gint64 expect = 2503;
        gint64 got;

        r = GSGF_NUMBER(gsgf_number_new_from_raw(value, NULL, NULL, &error));
        if (!r) {
                if (error) {
                        fprintf(stderr, "Could not parse '%s': %s.\n",
                                gsgf_raw_get_value(value, 0), error->message);
                        return -1;
                }
                fprintf(stderr, "Could not parse '%s': No error.\n",
                        gsgf_raw_get_value(value, 0));
                return -1;
        }

        got = gsgf_number_get_value(r);

        if (got != expect) {
                fprintf(stderr, "Expected %lld, got %lld.\n",
                        (long long) expect, (long long) got);
                return -1;
        }
        g_object_unref(r);

        return 0;
}

static int
test_garbage(void)
{
        GSGFNumber *r;
        GError *error;
        GError *expect = NULL;
        GSGFRaw *value = gsgf_raw_new("garbage");

        g_set_error(&expect, GSGF_ERROR, GSGF_ERROR_INVALID_NUMBER,
                    _("Invalid number '%s'"), gsgf_raw_get_value(value, 0));

        r = GSGF_NUMBER(gsgf_number_new_from_raw(value, NULL, NULL, &error));

        return expect_error_conditional(r == NULL,
                                        "Invalid number returns a value",
                                        error, expect);
}

static int
test_trailing_garbage(void)
{
        GSGFNumber *r;
        GError *error;
        GError *expect = NULL;
        GSGFRaw *value = gsgf_raw_new("2503garbage");

        g_set_error(&expect, GSGF_ERROR, GSGF_ERROR_INVALID_NUMBER,
                    _("Trailing garbage after number in '%s'"),
                    gsgf_raw_get_value(value, 0));

        r = GSGF_NUMBER(gsgf_number_new_from_raw(value, NULL, NULL, &error));

        return expect_error_conditional(r == NULL,
                                        "Number with trailing garbage returns a value",
                                        error, expect);
}
