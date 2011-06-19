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
#include <stdarg.h>
#include <sqlite3.h>
#include <time.h>

#include "gibbon-database.h"

/* Differences in the major schema version require a complete rebuild of the
 * database.
 */
#define GIBBON_DATABASE_SCHEMA_MAJOR 1

/* Differences in the minor schema version require conditional creation of
 * new tables or indexes.
 */
#define GIBBON_DATABASE_SCHEMA_MINOR 1

/* Differences int the schema revision are for cosmetic changes that will
 * not have any impact on existing databases (case, column order, ...).
 */
#define GIBBON_DATABASE_SCHEMA_REVISION 0

typedef struct _GibbonDatabasePrivate GibbonDatabasePrivate;
struct _GibbonDatabasePrivate {
        sqlite3 *dbh;
        sqlite3_stmt *begin_transaction;
        sqlite3_stmt *commit;
        sqlite3_stmt *rollback;

#define GIBBON_DATABASE_SELECT_USER_ID                  \
        "SELECT u.id FROM users u, servers s"           \
        " WHERE s.name = ? AND s.port = ?"              \
        " AND u.name = ? AND u.server_id = s.id"
        sqlite3_stmt *select_user_id;

#define GIBBON_DATABASE_INSERT_USER                                          \
        "INSERT INTO users (server_id, name, last_seen)\n"                   \
        "    VALUES((SELECT id FROM servers WHERE name = ? AND port = ?),"   \
        "           ?, ?)"
        sqlite3_stmt *insert_user;

#define GIBBON_DATABASE_SELECT_SERVER_ID                            \
        "SELECT s.id FROM servers s WHERE s.name = ? AND s.port = ?"
        sqlite3_stmt *select_server_id;

#define GIBBON_DATABASE_INSERT_SERVER                   \
        "INSERT INTO servers (name, port) VALUES (?, ?)"
        sqlite3_stmt *insert_server;

#define GIBBON_DATABASE_UPDATE_USER                                          \
        "INSERT INTO users (server_id, name, last_seen, experience, rating)" \
         " VALUES ((SELECT id FROM servers WHERE name = ? AND port = ?),"    \
         "          ?, ?, ?, ?)"
        sqlite3_stmt *update_user;

#define GIBBON_DATABASE_INSERT_ACTIVITY                                       \
        "INSERT INTO activities (user_id, value, date_time) VALUES (?, ?, ?)"
        sqlite3_stmt *insert_activity;

#define GIBBON_DATABASE_SELECT_ACTIVITY                                      \
        "SELECT AVG(value), COUNT(*) FROM activities a WHERE a.user_id = ?"
        sqlite3_stmt *select_activity;

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
static gboolean gibbon_database_maintain (GibbonDatabase *self);
static gboolean gibbon_database_display_error (GibbonDatabase *self,
                                               const gchar *msg_fmt, ...);
static gboolean gibbon_database_sql_do (GibbonDatabase *self,
                                        const gchar *sql_fmt, ...);
static gboolean gibbon_database_sql_execute (GibbonDatabase *self,
                                             sqlite3_stmt *stmt,
                                             const gchar *sql, ...);
static gboolean gibbon_database_sql_select_row (GibbonDatabase *self,
                                                sqlite3_stmt *stmt,
                                                const gchar *sql, ...);
static gboolean gibbon_database_get_statement (GibbonDatabase *self,
                                               sqlite3_stmt **stmt,
                                               const gchar *sql);
static guint _gibbon_database_get_user_id (GibbonDatabase *self,
                                           const gchar *hostname, guint port,
                                           const gchar *login,
                                           gboolean commit);
static guint _gibbon_database_get_server_id (GibbonDatabase *self,
                                             const gchar *hostname, guint port,
                                             gboolean commit);

static void 
gibbon_database_init (GibbonDatabase *self)
{
        self->priv = G_TYPE_INSTANCE_GET_PRIVATE (self,
                GIBBON_TYPE_DATABASE, GibbonDatabasePrivate);

        self->priv->dbh = NULL;
        self->priv->begin_transaction = NULL;
        self->priv->commit = NULL;
        self->priv->rollback = NULL;
        self->priv->insert_server = NULL;
        self->priv->select_server_id = NULL;
        self->priv->select_user_id = NULL;
        self->priv->insert_user = NULL;
        self->priv->update_user = NULL;
        self->priv->insert_activity = NULL;
        self->priv->select_activity = NULL;

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
                if (self->priv->insert_server)
                        sqlite3_finalize (self->priv->insert_server);
                if (self->priv->select_server_id)
                        sqlite3_finalize (self->priv->select_server_id);
                if (self->priv->select_user_id)
                        sqlite3_finalize (self->priv->select_user_id);
                if (self->priv->insert_user)
                        sqlite3_finalize (self->priv->insert_user);
                if (self->priv->update_user)
                        sqlite3_finalize (self->priv->update_user);
                if (self->priv->insert_activity)
                        sqlite3_finalize (self->priv->insert_activity);
                if (self->priv->select_activity)
                        sqlite3_finalize (self->priv->select_activity);
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
 * @path: Path to the sqlite database.
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

        (void) gibbon_database_maintain (self);

        g_timeout_add_seconds (60 * 60, (GSourceFunc) gibbon_database_maintain,
                               (gpointer) self);

        return self;
}

static gboolean
gibbon_database_initialize (GibbonDatabase *self)
{
        int major = 0;
        int minor = 0;
        sqlite3_stmt *stmt;
        int status;
        gchar *msg;
        GtkWidget *main_window;
        GtkWidget *dialog;
        gint reply;
        sqlite3 *dbh;
        gboolean drop_first = FALSE;
        gboolean new_database = FALSE;
        const gchar *sql_select_version =
                        "SELECT major, minor FROM version";

        dbh = self->priv->dbh;

        /* Version check.  */
        if (gibbon_database_exists_table (self, "version")) {
                status = sqlite3_prepare_v2 (dbh,
                                             sql_select_version,
                                             -1, &stmt, NULL);

                if (status != SQLITE_OK)
                        return FALSE;

                (void) gibbon_database_sql_select_row (self, stmt,
                                                       sql_select_version,
                                                       G_TYPE_INT, &major,
                                                       G_TYPE_INT, &minor,
                                                       -1);
                sqlite3_finalize (stmt);
        } else {
                new_database = TRUE;
        }

        if (major == GIBBON_DATABASE_SCHEMA_MAJOR
            && minor == GIBBON_DATABASE_SCHEMA_MINOR)
                return TRUE;

        if (major) {
                if (major > GIBBON_DATABASE_SCHEMA_MAJOR
                    || (major == GIBBON_DATABASE_SCHEMA_MAJOR
                        && minor > GIBBON_DATABASE_SCHEMA_MINOR)) {
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
            && !gibbon_database_sql_do (self, "DROP TABLE IF EXISTS servers"))
                return FALSE;
        if (!gibbon_database_sql_do (self,
                                     "CREATE TABLE IF NOT EXISTS servers ("
                                     "  id INTEGER PRIMARY KEY,"
                                     "  name TEXT NOT NULL,"
                                     "  port INTEGER NOT NULL,"
                                     "  UNIQUE (name, port)"
                                     ")"))
                return FALSE;

        if (drop_first
            && !gibbon_database_sql_do (self, "DROP TABLE IF EXISTS users"))
                return FALSE;
        if (!gibbon_database_sql_do (self,
                                     "CREATE TABLE IF NOT EXISTS users ("
                                     "  id INTEGER PRIMARY KEY,"
                                     "  name TEXT NOT NULL,"
                                     "  server_id INTEGER NOT NULL,"
                                     "  experience INTEGER,"
                                     "  rating REAL,"
                                     "  last_seen INT64 NOT NULL,"
                                     "  UNIQUE (name, server_id),"
                                     "  FOREIGN KEY (server_id)"
                                     "    REFERENCES servers (id)"
                                     "    ON DELETE CASCADE"
                                     ")"))
                return FALSE;

        if (drop_first
            && !gibbon_database_sql_do (self, "DROP TABLE IF EXISTS activities"))
                return FALSE;
        if (!gibbon_database_sql_do (self,
                                     "CREATE TABLE IF NOT EXISTS activities ("
                                     "  user_id INTEGER NOT NULL,"
                                     "  value REAL NOT NULL,"
                                     "  date_time INT64 NOT NULL,"
                                     "  FOREIGN KEY (user_id)"
                                     "    REFERENCES users (id)"
                                     "    ON DELETE CASCADE"
                                     ")"))
                return FALSE;

        /* Needed for clean-up.  */
        if (!gibbon_database_sql_do (self,
                                     "CREATE INDEX IF NOT EXISTS"
                                     " actitivities_date_time_index"
                                     " ON activities (date_time)"))
                return FALSE;

        if (!gibbon_database_sql_do (self, "DELETE FROM version"))
                return FALSE;

        if (!gibbon_database_sql_do (self,
                                     "INSERT INTO version (major, minor,\n"
                                     "                     revision,\n"
                                     "                     app_version)\n"
                                     "    VALUES(%d, %d, %d, '%s %s')",
                                     GIBBON_DATABASE_SCHEMA_MAJOR,
                                     GIBBON_DATABASE_SCHEMA_MINOR,
                                     GIBBON_DATABASE_SCHEMA_REVISION,
                                     PACKAGE, VERSION))
                return FALSE;

        if (drop_first && !new_database)
                gibbon_app_display_info (self->priv->app,
                                         _("You should now repopulate your"
                                           " database (menu `Options')."));

        return TRUE;
}

static gboolean
gibbon_database_exists_table (GibbonDatabase *self, const char *name)
{
        sqlite3_stmt *stmt;
        gchar *sql = g_strdup_printf ("SELECT type FROM sqlite_master\n"
                                      "    WHERE name = '%s'\n"
                                      "      AND type = 'table'",
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

        if (!gibbon_database_sql_execute (self, self->priv->begin_transaction,
                                          "BEGIN TRANSACTION", -1))
                return FALSE;

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

        if (!gibbon_database_sql_execute (self, self->priv->commit,
                                          "COMMIT", -1))
                return FALSE;

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

        if (!gibbon_database_sql_execute (self, self->priv->rollback,
                                          "ROLLBACK", -1))
                return FALSE;

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

static gboolean
gibbon_database_sql_execute (GibbonDatabase *self,
                             sqlite3_stmt *stmt,
                             const gchar *sql, ...)
{
        gint type;
        gpointer ptr;
        va_list args;
        gint i = 0;

        sqlite3_reset (stmt);

        va_start (args, sql);

        type = va_arg (args, gint);

        while (type != -1) {
                ++i;
                ptr = va_arg (args, gpointer);
                if (!ptr) {
                        if (sqlite3_bind_null (stmt, i))
                                return FALSE;
                        continue;
                }

                switch (type) {
                        case G_TYPE_INT:
                        case G_TYPE_UINT:
                                if (sqlite3_bind_int (stmt, i,
                                                      *((gint *) ptr)))
                                        return gibbon_database_display_error (
                                                        self, sql);
                                break;
                        case G_TYPE_INT64:
                                if (sqlite3_bind_int64 (stmt, i,
                                                        *((sqlite3_int64 *) ptr)))
                                        return gibbon_database_display_error (
                                                        self, sql);
                                break;
                        case G_TYPE_DOUBLE:
                                if (sqlite3_bind_double (stmt, i,
                                                         *((gdouble *) ptr)))
                                        return gibbon_database_display_error (
                                                        self, sql);
                                break;
                        case G_TYPE_STRING:
                                if (sqlite3_bind_text (stmt, i,
                                                       *((gchar **) ptr), -1,
                                                       SQLITE_STATIC
                                                       ))
                                        return gibbon_database_display_error (
                                                        self, sql);
                                break;
                        default:
                                gibbon_app_display_error (self->priv->app,
                                                          _("Unknown data"
                                                            " type %d!"),
                                                            type);
                                return FALSE;
                }

                type = va_arg (args, gint);
        }

        va_end (args);

        if (g_ascii_strncasecmp (sql, "SELECT", 6)) {
                if (!sqlite3_step (stmt))
                        return gibbon_database_display_error (self, sql);
        }

        return TRUE;
}

static gboolean
gibbon_database_sql_select_row (GibbonDatabase *self,
                                sqlite3_stmt *stmt,
                                const gchar *sql, ...)
{
        gint type;
        gpointer ptr;
        va_list args;
        gint i = 0;
        int status;

        status = sqlite3_step (stmt);
        if (SQLITE_DONE == status) {
                return FALSE;
        } else if (SQLITE_ROW != status) {
                return gibbon_database_display_error (self, sql);
        }

        va_start (args, sql);

        type = va_arg (args, gint);

        while (type != -1) {
                ptr = va_arg (args, gpointer);

                switch (type) {
                        case G_TYPE_INT:
                                *((gint *) ptr) = sqlite3_column_int (stmt, i);
                                break;
                        case G_TYPE_UINT:
                                *((guint *) ptr) = sqlite3_column_int (stmt, i);
                                break;
                        case G_TYPE_DOUBLE:
                                *((gdouble *) ptr) =
                                        sqlite3_column_double (stmt, i);
                                break;
                        default:
                                gibbon_app_display_error (self->priv->app,
                                                          _("Unknown data"
                                                            " type %d!"),
                                                            type);
                                return FALSE;
                }

                type = va_arg (args, gint);
                ++i;
        }

        return TRUE;
}

static gboolean
gibbon_database_get_statement (GibbonDatabase *self,
                               sqlite3_stmt **stmt,
                               const gchar *sql)
{
        if (*stmt) {
                sqlite3_reset (*stmt);
                return TRUE;
        }

        if (sqlite3_prepare_v2 (self->priv->dbh, sql, -1, stmt, NULL))
                return gibbon_database_display_error (self, sql);

        return TRUE;
}

gboolean
gibbon_database_update_user_full (GibbonDatabase *self,
                                  const gchar *hostname, guint port,
                                  const gchar *login,
                                  gdouble rating, guint experience)
{
        gint64 now;

        g_return_val_if_fail (GIBBON_IS_DATABASE (self), FALSE);
        g_return_val_if_fail (hostname != NULL, FALSE);
        g_return_val_if_fail (port != 0, FALSE);
        g_return_val_if_fail (login != NULL, FALSE);

        if (!gibbon_database_get_statement (self, &self->priv->update_user,
                                            GIBBON_DATABASE_UPDATE_USER))
                return FALSE;

        if (!gibbon_database_begin_transaction (self))
                return FALSE;

        now = (gint64) time (NULL);
        if (!gibbon_database_sql_execute (self, self->priv->update_user,
                                          GIBBON_DATABASE_UPDATE_USER,
                                          G_TYPE_STRING, &hostname,
                                          G_TYPE_UINT, &port,
                                          G_TYPE_STRING, &login,
                                          G_TYPE_INT64, &now,
                                          G_TYPE_UINT, &experience,
                                          G_TYPE_DOUBLE, &rating,
                                          -1)) {
                gibbon_database_rollback (self);
                return FALSE;
        }

        if (!gibbon_database_commit (self)) {
                gibbon_database_rollback (self);
                return FALSE;
        }

        return TRUE;
}

gboolean
gibbon_database_insert_activity (GibbonDatabase *self,
                                 const gchar *hostname, guint port,
                                 const gchar *login,
                                 gdouble value)
{
        gint64 now;
        guint user_id;

        g_return_val_if_fail (GIBBON_IS_DATABASE (self), FALSE);
        g_return_val_if_fail (login != NULL, FALSE);
        g_return_val_if_fail (port > 0, FALSE);
        g_return_val_if_fail (login != NULL, FALSE);

        if (!gibbon_database_get_statement (self, &self->priv->insert_activity,
                                            GIBBON_DATABASE_INSERT_ACTIVITY))
                return FALSE;

        if (!gibbon_database_begin_transaction (self))
                return FALSE;

        /*
         * A subselect would be more efficient but retrieving the user
         * id implicitly creates unknown users.
         */
        user_id = _gibbon_database_get_user_id (self, hostname, port, login,
                                                FALSE);
        if (!user_id) {
                gibbon_database_rollback (self);
                return FALSE;
        }

        now = (gint64) time (NULL);
        if (!gibbon_database_sql_execute (self, self->priv->insert_activity,
                                          GIBBON_DATABASE_INSERT_ACTIVITY,
                                          G_TYPE_UINT, &user_id,
                                          G_TYPE_DOUBLE, &value,
                                          G_TYPE_INT64, &now,
                                          -1)) {
                gibbon_database_rollback (self);
                return FALSE;
        }

        if (!gibbon_database_commit (self)) {
                gibbon_database_rollback (self);
                return FALSE;
        }

        return TRUE;
}

static gboolean
gibbon_database_maintain (GibbonDatabase *self)
{
        const gchar *sql;
        sqlite3_stmt *stmt;
        gint64 now, then;

        if (!gibbon_database_begin_transaction (self))
                return TRUE;

        /* Delete all activities older than 100 days.  Also handle
         * clock skews gracefully.
         */
        now = time (NULL);
        then = now - 100 * 24 * 60 * 60;
        sql = "DELETE FROM activities WHERE (date_time < ? OR date_time > ?)";
        if (SQLITE_OK == sqlite3_prepare_v2 (self->priv->dbh, sql,
                                             -1, &stmt, NULL)) {
                (void) gibbon_database_sql_execute (self, stmt, sql,
                                                    G_TYPE_INT64, &then,
                                                    G_TYPE_INT64, &now,
                                                    -1);
                sqlite3_finalize (stmt);
        }

        sql = "VACUUM";
        if (SQLITE_OK == sqlite3_prepare_v2 (self->priv->dbh, sql,
                                             -1, &stmt, NULL)) {
                (void) gibbon_database_sql_execute (self, stmt, sql, -1);
                sqlite3_finalize (stmt);
        }

        if (!gibbon_database_commit (self)) {
                gibbon_database_rollback (self);
                return FALSE;
        }

        return TRUE;
}

gboolean
gibbon_database_get_reliability (GibbonDatabase *self,
                                 const gchar *hostname, guint port,
                                 const gchar *login,
                                 gdouble *value, guint *confidence)
{
        guint user_id;

        g_return_val_if_fail (GIBBON_IS_DATABASE (self), FALSE);
        g_return_val_if_fail (login != NULL, FALSE);
        g_return_val_if_fail (value != NULL, FALSE);
        g_return_val_if_fail (confidence != NULL, FALSE);
        g_return_val_if_fail (hostname != NULL, FALSE);
        g_return_val_if_fail (port != 0, FALSE);

        if (!gibbon_database_get_statement (self, &self->priv->select_activity,
                                            GIBBON_DATABASE_SELECT_ACTIVITY))
                return FALSE;

        if (!gibbon_database_begin_transaction (self))
                return FALSE;

        user_id = _gibbon_database_get_user_id (self, hostname, port, login,
                                                FALSE);
        if (!user_id) {
                gibbon_database_rollback (self);
                return FALSE;
        }

        if (!gibbon_database_sql_execute (self, self->priv->select_activity,
                                          GIBBON_DATABASE_SELECT_ACTIVITY,
                                          G_TYPE_UINT, &user_id,
                                          -1)) {
                gibbon_database_rollback (self);
                return -1;
        }

        if (!gibbon_database_sql_select_row (self, self->priv->select_activity,
                                             GIBBON_DATABASE_SELECT_ACTIVITY,
                                             G_TYPE_DOUBLE, value,
                                             G_TYPE_UINT, confidence,
                                            -1)) {
                gibbon_database_rollback (self);
                return -1;
        }

        if (!gibbon_database_commit (self)) {
                gibbon_database_rollback (self);
                return FALSE;
        }

        return TRUE;
}

guint
gibbon_database_get_user_id (GibbonDatabase *self,
                             const gchar *hostname, guint port,
                             const gchar *login)
{
        g_return_val_if_fail (GIBBON_IS_DATABASE (self), 0);
        g_return_val_if_fail (hostname != NULL, 0);
        g_return_val_if_fail (port != 0, 0);
        g_return_val_if_fail (login != NULL, 0);

        return _gibbon_database_get_user_id (self, hostname, port, login,
                                             TRUE);
}

static guint
_gibbon_database_get_user_id (GibbonDatabase *self,
                              const gchar *hostname, guint port,
                              const gchar *login,
                              gboolean commit)
{
        guint user_id;
        gint64 now;

        if (!gibbon_database_get_statement (self, &self->priv->select_user_id,
                                            GIBBON_DATABASE_SELECT_USER_ID))
                return 0;

        if (commit && !gibbon_database_begin_transaction (self))
                return 0;

        if (!gibbon_database_sql_execute (self, self->priv->select_user_id,
                                          GIBBON_DATABASE_SELECT_USER_ID,
                                          G_TYPE_STRING, &hostname,
                                          G_TYPE_UINT, &port,
                                          G_TYPE_STRING, &login,
                                          -1)) {
                if (commit)
                        gibbon_database_rollback (self);
                return 0;
        }

        if (gibbon_database_sql_select_row (self, self->priv->select_user_id,
                                             GIBBON_DATABASE_SELECT_USER_ID,
                                             G_TYPE_UINT, &user_id,
                                            -1)) {
                if (commit && !gibbon_database_commit (self)) {
                        gibbon_database_rollback (self);
                        return 0;
                }

                return user_id;
        }

        /* We have to create a new user.  */
        if (!gibbon_database_get_statement (self, &self->priv->insert_user,
                                            GIBBON_DATABASE_INSERT_USER)) {
                if (commit)
                        gibbon_database_rollback (self);
                return 0;
        }

        now = (gint64) time (NULL);
        if (!gibbon_database_sql_execute (self, self->priv->insert_user,
                                          GIBBON_DATABASE_INSERT_USER,
                                          G_TYPE_STRING, &hostname,
                                          G_TYPE_UINT, &port,
                                          G_TYPE_STRING, &login,
                                          G_TYPE_INT64, &now,
                                          -1)) {
                if (commit)
                        gibbon_database_rollback (self);
                return 0;
        }

        user_id = _gibbon_database_get_user_id (self, hostname, port, login,
                                                FALSE);
        if (!user_id) {
                if (commit)
                        gibbon_database_rollback (self);
                return 0;
        }

        if (commit && !gibbon_database_commit (self)) {
                gibbon_database_rollback (self);
                return 0;
        }

        return user_id;
}


guint
gibbon_database_get_server_id (GibbonDatabase *self,
                               const gchar *hostname, guint port)
{
        g_return_val_if_fail (GIBBON_IS_DATABASE (self), 0);
        g_return_val_if_fail (hostname != NULL, 0);
        g_return_val_if_fail (port != 0, 0);

        return _gibbon_database_get_server_id (self, hostname, port, TRUE);
}

static guint
_gibbon_database_get_server_id (GibbonDatabase *self,
                                const gchar *hostname, guint port,
                                gboolean commit)
{
        guint server_id;

        if (!gibbon_database_get_statement (self, &self->priv->select_server_id,
                                            GIBBON_DATABASE_SELECT_SERVER_ID))
                return 0;

        if (commit && !gibbon_database_begin_transaction (self))
                return 0;

        if (!gibbon_database_sql_execute (self, self->priv->select_server_id,
                                          GIBBON_DATABASE_SELECT_SERVER_ID,
                                          G_TYPE_STRING, &hostname,
                                          G_TYPE_UINT, &port,
                                          -1)) {
                if (commit)
                        gibbon_database_rollback (self);
                return 0;
        }

        if (gibbon_database_sql_select_row (self, self->priv->select_server_id,
                                             GIBBON_DATABASE_SELECT_SERVER_ID,
                                             G_TYPE_UINT, &server_id,
                                            -1)) {
                if (commit && !gibbon_database_commit (self)) {
                        gibbon_database_rollback (self);
                        return 0;
                }

                return server_id;
        }

        /* We have to create a new server.  */
        if (!gibbon_database_get_statement (self, &self->priv->insert_server,
                                            GIBBON_DATABASE_INSERT_SERVER)) {
                if (commit)
                        gibbon_database_rollback (self);
                return 0;
        }

        if (!gibbon_database_sql_execute (self, self->priv->insert_server,
                                          GIBBON_DATABASE_INSERT_SERVER,
                                          G_TYPE_STRING, &hostname,
                                          G_TYPE_UINT, &port,
                                          -1)) {
                if (commit)
                        gibbon_database_rollback (self);
                return 0;
        }

        server_id = _gibbon_database_get_server_id (self, hostname, port,
                                                    FALSE);
        if (!server_id) {
                if (commit)
                        gibbon_database_rollback (self);
                return 0;
        }

        if (commit && !gibbon_database_commit (self)) {
                gibbon_database_rollback (self);
                return 0;
        }

        return server_id;
}
