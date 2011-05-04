/*
 * This file is part of gibbon.
 * Gibbon is a Gtk+ frontend for the First Internet Backgammon Server FIBS.
 * Copyright (C) 2009-2011 Guido Flohr, http://guido-flohr.net/.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 3 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with gibbon; if not, see <http://www.gnu.org/licenses/>.
 */

/**
 * SECTION:gibbon-database
 * @short_description: FIXME! Short description missing!
 *
 * Since: 0.1.1
 *
 * Gibbon database.  All data in this database should be either unimportant
 * or redundant.
 */

#include <glib.h>
#include <glib/gi18n.h>

#include <sqlite3.h>

#include "gibbon-database.h"

#define GIBBON_DATABASE_SCHEMA_MAJOR 1
#define GIBBON_DATABASE_SCHEMA_MINOR 0
#define GIBBON_DATABASE_SCHEMA_REVISION 0

typedef struct _GibbonDatabasePrivate GibbonDatabasePrivate;
struct _GibbonDatabasePrivate {
        sqlite3 *dbh;
        sqlite3_stmt *begin_transaction;
        sqlite3_stmt *commit;
        sqlite3_stmt *rollback;

        gboolean in_transaction;

        gchar *path;

        GibbonApp *app;
};

#define GIBBON_DATABASE_PRIVATE(obj) (G_TYPE_INSTANCE_GET_PRIVATE ((obj), \
        GIBBON_TYPE_DATABASE, GibbonDatabasePrivate))

G_DEFINE_TYPE (GibbonDatabase, gibbon_database, G_TYPE_OBJECT)

static gboolean gibbon_database_initialize (GibbonDatabase *self);
static gboolean gibbon_database_exists_table (GibbonDatabase *self,
                                              const gchar *table);
static gboolean gibbon_database_begin_transaction (GibbonDatabase *self);
static gboolean gibbon_database_commit (GibbonDatabase *self);
static gboolean gibbon_database_rollback (GibbonDatabase *self);
static gboolean gibbon_database_display_error (GibbonDatabase *self,
                                               const gchar *msg_fmt, ...);
static gboolean gibbon_database_sql_do (GibbonDatabase *self,
                                        const gchar *sql_fmt, ...);

static void 
gibbon_database_init (GibbonDatabase *self)
{
        self->priv = G_TYPE_INSTANCE_GET_PRIVATE (self,
                GIBBON_TYPE_DATABASE, GibbonDatabasePrivate);

        self->priv->dbh = NULL;
        self->priv->begin_transaction = NULL;
        self->priv->commit = NULL;
        self->priv->rollback = NULL;

        self->priv->in_transaction = FALSE;

        self->priv->path = NULL;
}

static void
gibbon_database_finalize (GObject *object)
{
        GibbonDatabase *self = GIBBON_DATABASE (object);

        if (self->priv->dbh) {
                sqlite3_close (self->priv->dbh);
                if (self->priv->begin_transaction)
                        sqlite3_finalize (self->priv->begin_transaction);
                if (self->priv->commit)
                        sqlite3_finalize (self->priv->commit);
                if (self->priv->rollback)
                        sqlite3_finalize (self->priv->rollback);
        }

        if (self->priv->path)
                g_free (self->priv->path);

        G_OBJECT_CLASS (gibbon_database_parent_class)->finalize(object);
}

static void
gibbon_database_class_init (GibbonDatabaseClass *klass)
{
        GObjectClass *object_class = G_OBJECT_CLASS (klass);
        
        g_type_class_add_private (klass, sizeof (GibbonDatabasePrivate));

        object_class->finalize = gibbon_database_finalize;
}

/**
 * gibbon_database_new:
 * @app: Our application.
 * @path: Path to the sqlitee database.
 *
 * Creates a new #GibbonDatabase.
 *
 * Returns: The newly created #GibbonDatabase or %NULL in case of failure.
 */
GibbonDatabase *
gibbon_database_new (GibbonApp *app, const gchar *path)
{
        GibbonDatabase *self = g_object_new (GIBBON_TYPE_DATABASE, NULL);
        sqlite3 *dbh;

        self->priv->path = g_strdup (path);
        self->priv->app = app;

        if (SQLITE_OK != sqlite3_open (path, &dbh)) {
                if (!dbh) {
                        gibbon_app_display_error (app,
                                                  _("Error opening database"
                                                    " `%s'!"),
                                                  path);
                        g_object_unref (self);
                        return NULL;
                }

                gibbon_database_display_error (self,
                                               _("Error opening database"
                                                 " `%s'"),
                                               path);
                g_object_unref (self);
                return NULL;
        }

        self->priv->dbh = dbh;

        /* FIXME! This does not check anything.  Instead we have to select
         * the property.  We will do that later, once we have our select
         * function coded.
         */
        if (!gibbon_database_sql_do (self, "PRAGMA foreign_keys = ON")) {
                gibbon_app_display_error (app,
                                          _("Your sqlite installation seems"
                                            " to be crippled.  It does not"
                                            " support foreign key constraints. "
                                            " This is not a bug in Gibbon!"));
                g_object_unref (self);
                return NULL;
        }
        if (!gibbon_database_begin_transaction (self)) {
                g_object_unref (self);
                return NULL;
        }
        if (!gibbon_database_initialize (self)) {
                (void) gibbon_database_rollback (self);
                g_object_unref (self);
                return NULL;
        }
        if (!gibbon_database_commit (self)) {
                g_object_unref (self);
                return NULL;
        }

        return self;
}

static gboolean
gibbon_database_initialize (GibbonDatabase *self)
{
        int major = 0;
        int minor = 0;
        sqlite3_stmt *stmt;
        gchar *sql;
        int status;
        gchar *msg;
        GtkWidget *main_window;
        GtkWidget *dialog;
        gint reply;
        sqlite3 *dbh;
        gboolean drop_first = FALSE;

        dbh = self->priv->dbh;

        /* Version check.  */
        if (gibbon_database_exists_table (self, "version")) {
                sql = g_strdup_printf ("SELECT major, minor FROM version");
                status = sqlite3_prepare_v2 (dbh, sql, -1, &stmt, NULL);
                g_free (sql);

                if (status != SQLITE_OK)
                        return FALSE;

                status = sqlite3_step (stmt);
                if (status == SQLITE_ROW) {
                        major = sqlite3_column_int (stmt, 0);
                        minor = sqlite3_column_int (stmt, 1);
                }
                sqlite3_finalize (stmt);
        }

        if (major == GIBBON_DATABASE_SCHEMA_MAJOR
            && minor == GIBBON_DATABASE_SCHEMA_MINOR)
                return TRUE;

        if (major) {
                if (major > GIBBON_DATABASE_SCHEMA_MAJOR
                    || (major == GIBBON_DATABASE_SCHEMA_MAJOR
                        && minor < GIBBON_DATABASE_SCHEMA_MINOR)) {
                        msg = N_("Your database was created"
                                 " with a more recent version"
                                 " of Gibbon.  With a downgrade"
                                 " you will not lose data but"
                                 " you must repopulate your database"
                                 " which can be a time-consuming"
                                 " process. "
                                 " It could be easier to just"
                                 " upgrade Gibbon instead."
                                 "\n\nDo you want to downgrade the database?");
                } else if (major < GIBBON_DATABASE_SCHEMA_MAJOR) {
                        msg = N_("Your database was created with an"
                                 " older version of Gibbon.  With"
                                 " the upgrade you will not lose"
                                 " data but you must repopulate your"
                                 " database which can be a"
                                 " time-consuming process."
                                 "\n\nProceed anyway?");
                } else {
                        /* We upgrade on the fly without asking.  */
                        msg = NULL;
                }

                if (msg) {
                        main_window = gibbon_app_get_window (self->priv->app);
                        dialog = gtk_message_dialog_new (GTK_WINDOW (main_window),
                                                 GTK_DIALOG_DESTROY_WITH_PARENT,
                                                 GTK_MESSAGE_QUESTION,
                                                 GTK_BUTTONS_YES_NO,
                                                 "%s", _(msg));
                        reply = gtk_dialog_run (GTK_DIALOG (dialog));
                        gtk_widget_destroy (dialog);

                        if (reply != GTK_RESPONSE_YES)
                                return FALSE;
                }
        }

        if (major < GIBBON_DATABASE_SCHEMA_MAJOR)
                drop_first = TRUE;

        if (drop_first
            && !gibbon_database_sql_do (self, "DROP TABLE IF EXISTS version"))
                return FALSE;
        if (!gibbon_database_sql_do (self,
                                     "CREATE TABLE IF NOT EXISTS version ("
                                     "  major INTEGER,"
                                     "  minor INTEGER,"
                                     "  revision INTEGER,"
                                     "  app_version TEXT"
                                     ")"))
                return FALSE;

        if (drop_first
            && !gibbon_database_sql_do (self, "DROP TABLE IF EXISTS server"))
                return FALSE;
        if (!gibbon_database_sql_do (self,
                                     "CREATE TABLE IF NOT EXISTS server ("
                                     "  id INTEGER PRIMARY KEY,"
                                     "  name TEXT,"
                                     "  port INTEGER,"
                                     "  UNIQUE (name, port)"
                                     ")"))
                return FALSE;

        if (drop_first
            && !gibbon_database_sql_do (self, "DROP TABLE IF EXISTS user"))
                return FALSE;
        if (!gibbon_database_sql_do (self,
                                     "CREATE TABLE IF NOT EXISTS user ("
                                     "  id INTEGER PRIMARY KEY,"
                                     "  name TEXT,"
                                     "  server_id INTEGER NOT NULL,"
                                     "  UNIQUE (name, server_id),"
                                     "  FOREIGN KEY (server_id)"
                                     "    REFERENCES server(id)"
                                     "    ON DELETE CASCADE"
                                     ")"))
                return FALSE;

        if (!gibbon_database_sql_do (self, "DELETE FROM version"))
                return FALSE;

        if (!gibbon_database_sql_do (self,
                                     "INSERT INTO version (major, minor,"
                                     " revision, app_version)"
                                     " VALUES(%d, %d, %d, '%s %s')",
                                     GIBBON_DATABASE_SCHEMA_MAJOR,
                                     GIBBON_DATABASE_SCHEMA_MINOR,
                                     GIBBON_DATABASE_SCHEMA_REVISION,
                                     PACKAGE, VERSION))
                return FALSE;

        if (major)
                gibbon_app_display_info (self->priv->app,
                                         _("You should now repopulate your"
                                           " database (menu `Options')."));

        return TRUE;
}

static gboolean
gibbon_database_exists_table (GibbonDatabase *self, const char *name)
{
        sqlite3_stmt *stmt;
        gchar *sql = g_strdup_printf ("SELECT type FROM sqlite_master"
                                      " WHERE name = '%s'"
                                      " AND type = 'table'",
                                      name);
        int status;
        sqlite3 *dbh;

        dbh = self->priv->dbh;
        status = sqlite3_prepare_v2 (dbh, sql, -1, &stmt, NULL);
        g_free (sql);

        if (status != SQLITE_OK)
                return FALSE;

        status = sqlite3_step (stmt);
        sqlite3_finalize (stmt);

        if (status != SQLITE_ROW)
                return FALSE;

        return TRUE;
}

static gboolean
gibbon_database_begin_transaction (GibbonDatabase *self)
{
        if (self->priv->in_transaction) {
                gibbon_app_display_error (self->priv->app,
                                          _("Internal error: Nested"
                                            " transaction."));
                return FALSE;
        }
        self->priv->in_transaction = TRUE;

        if (!self->priv->begin_transaction) {
                if (sqlite3_prepare_v2 (self->priv->dbh,
                                        "BEGIN TRANSACTION",
                                        -1, &self->priv->begin_transaction,
                                         NULL))
                        return gibbon_database_display_error (self,
                                                              "BEGIN"
                                                              " TRANSACTION");
        } else {
                sqlite3_reset (self->priv->begin_transaction);
        }

        return TRUE;
}

static gboolean
gibbon_database_commit (GibbonDatabase *self)
{
        if (!self->priv->in_transaction) {
                gibbon_app_display_error (self->priv->app,
                                          _("Internal error: Commit"
                                            " outside transaction."));
                return FALSE;
        }
        self->priv->in_transaction = FALSE;

        if (!self->priv->commit) {
                if (sqlite3_prepare_v2 (self->priv->dbh,
                                        "COMMIT",
                                        -1, &self->priv->commit,
                                        NULL))
                        return gibbon_database_display_error (self, "COMMIT");
        } else {
                sqlite3_reset (self->priv->commit);
        }

        return TRUE;
}

static gboolean
gibbon_database_rollback (GibbonDatabase *self)
{
        if (!self->priv->in_transaction) {
                gibbon_app_display_error (self->priv->app,
                                          _("Internal error: Rollback"
                                            " outside transaction."));
                return FALSE;
        }
        self->priv->in_transaction = FALSE;

        if (!self->priv->rollback) {
                if (sqlite3_prepare_v2 (self->priv->dbh,
                                        "ROLLBACK",
                                        -1, &self->priv->rollback,
                                        NULL))
                        return gibbon_database_display_error (self,
                                                              "ROLLBACK");
        } else {
                sqlite3_reset (self->priv->rollback);
        }

        return TRUE;
}

static gboolean
gibbon_database_display_error (GibbonDatabase *self, const gchar *msg_fmt, ...)
{
        va_list args;
        gchar *message;

        if (msg_fmt) {
                va_start (args, msg_fmt);
                message = g_strdup_vprintf (msg_fmt, args);
                va_end (args);
        } else {
                message = g_strdup (_("Database failure"));
        }

        gibbon_app_display_error (self->priv->app,
                                  /* TRANSLATORS: The first string is a string
                                   * describing the error, the second one a
                                   * maybe cryptic database error message.
                                   */
                                  (_("%s: %s.")),
                                  message,
                                  sqlite3_errmsg (self->priv->dbh));

        /* Convenience: Return FALSE, so that function call can be used for
         * a return statement.
         */
        return FALSE;
}

static gboolean
gibbon_database_sql_do (GibbonDatabase *self, const gchar *sql_fmt, ...)
{
        va_list args;
        gchar *sql;
        sqlite3_stmt *stmt;
        int status;
        gboolean retval = TRUE;

        va_start (args, sql_fmt);
        sql = g_strdup_vprintf (sql_fmt, args);
        va_end (args);

        status = sqlite3_prepare_v2 (self->priv->dbh, sql,
                                    -1, &stmt,
                                     NULL);

        if (status != SQLITE_OK) {
                (void) gibbon_database_display_error (self, sql);
                g_free (sql);
                return FALSE;
        }

        status = sqlite3_step (stmt);
        if (status != SQLITE_DONE) {
                retval = FALSE;
                (void) gibbon_database_display_error (self, sql);
        }

        g_free (sql);
        sqlite3_finalize (stmt);

        return retval;
}
