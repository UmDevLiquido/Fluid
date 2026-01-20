#include "include/fluid.h"
#include <sqlite3.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Define chunk size for dynamic allocation of file lists */
#define CHUNK_SIZE 32

/* =========================
   Memory management for FileList
   ========================= */

/**
 * Free a FileList and all allocated memory inside it.
 * @param fl Pointer to the FileList to clean. If NULL, does nothing.
 */
void clean_filelist(FileList *fl){
    if (fl == NULL) return;
    for (int i = 0; i < fl->count; i++) free(fl->files[i]);  // free each file string
    free(fl->files);  // free the array of pointers
    free(fl);         // free the FileList struct itself
}

/* =========================
   SQLite error handling
   ========================= */

/**
 * Print a formatted SQLite error message to stderr.
 * @param db Pointer to the SQLite database.
 * @param msg Custom message to prepend.
 */
void sqlite_error(sqlite3 *db, const char *msg) {
    fprintf(stderr, "\033[1;4;31m%s: %s\033[0m\n", msg, sqlite3_errmsg(db));
}

/* =========================
   Database connection and initialization
   ========================= */

/**
 * Connect to the local SQLite database and create tables if they don't exist.
 * @return Pointer to sqlite3 database object, or NULL on failure.
 */
sqlite3* connect_db(){
    sqlite3 *db;
    
    /* Open the database file (must have root permissions if in /var/lib/) */
    if (sqlite3_open("/var/lib/fluid/.cups", &db)){
        sqlite_error(db, "An error ocurred while creating or loading cups file");
        return NULL;
    }

    /* SQL to create the installed packages table */
    const char *installed_sql = "CREATE TABLE IF NOT EXISTS installed ("
                                "package_name VARCHAR(128) PRIMARY KEY);";

    /* SQL to create the package files table */
    const char *files_sql = "CREATE TABLE IF NOT EXISTS files ("
                            "_id INTEGER PRIMARY KEY AUTOINCREMENT,"
                            "file TEXT,"
                            "package_id TEXT);";

    char *errmsg;
    if (sqlite3_exec(db, installed_sql, 0, 0, &errmsg) != SQLITE_OK){
        sqlite_error(db, "An error ocurred creating installed packages table");
        sqlite3_free(errmsg);
        return NULL;
    }

    if (sqlite3_exec(db, files_sql, 0, 0, &errmsg) != SQLITE_OK){
        sqlite_error(db, "An error ocurred creating package files table");
        sqlite3_free(errmsg);
        return NULL;
    }

    return db;
}

/* =========================
   Package management functions
   ========================= */

/**
 * Insert a package into the installed table.
 * @param db Pointer to the SQLite database.
 * @param package_name Name of the package to insert.
 * @return 0 on success, 1 on error.
 */
int insert_package(sqlite3 *db, const char *package_name){
    sqlite3_stmt *stmt;
    const char *sql = "INSERT INTO installed (package_name) VALUES (?);";

    if (sqlite3_prepare_v2(db, sql, -1, &stmt, NULL) != SQLITE_OK){
        sqlite_error(db, "Error registering package entry in local db");
        return 1;
    }

    sqlite3_bind_text(stmt, 1, package_name, -1, SQLITE_TRANSIENT);

    if (sqlite3_step(stmt) != SQLITE_DONE){
        sqlite_error(db, "Error registering package entry in local db");
        sqlite3_finalize(stmt);
        return 1;
    }

    sqlite3_finalize(stmt);
    return 0;
}

/**
 * Delete a package from the installed table.
 * @param db Pointer to the SQLite database.
 * @param package_name Name of the package to delete.
 * @return 0 on success, 1 on error.
 */
int delete_package(sqlite3 *db, const char *package_name){
    sqlite3_stmt *stmt;
    const char *sql = "DELETE FROM installed WHERE package_name = ?;";

    if (sqlite3_prepare_v2(db, sql, -1, &stmt, NULL) != SQLITE_OK){
        sqlite_error(db, "Error deleting package entry in local db");
        return 1;
    }

    sqlite3_bind_text(stmt, 1, package_name, -1, SQLITE_TRANSIENT);

    if (sqlite3_step(stmt) != SQLITE_DONE){
        sqlite_error(db, "Error deleting package entry in local db");
        sqlite3_finalize(stmt);
        return 1;
    }

    sqlite3_finalize(stmt);
    return 0;
}

/**
 * Check if a package is installed.
 * @param db Pointer to the SQLite database.
 * @param packstr Package name to check.
 * @return 1 if installed, 0 if not installed, 2 on SQLite error.
 */
int is_package_installed(sqlite3 *db, const char *packstr){
    sqlite3_stmt *stmt;
    const char *sql = "SELECT * FROM installed WHERE package_name = ?;";
    int rc;

    if (sqlite3_prepare_v2(db, sql, -1, &stmt, NULL) != SQLITE_OK){
        sqlite_error(db, "Error checking installed package in local db");
        return 1;
    }

    sqlite3_bind_text(stmt, 1, packstr, -1, SQLITE_TRANSIENT);
    rc = sqlite3_step(stmt);

    if (rc == SQLITE_ROW){
        sqlite3_finalize(stmt);
        return 1;
    } else if (rc == SQLITE_DONE) {
        sqlite3_finalize(stmt);
        return 0;
    } else {
        sqlite3_finalize(stmt);
        return 2;
    }
}

/* =========================
   Package file management functions
   ========================= */

/**
 * Insert a file associated with a package.
 * @param db Pointer to the SQLite database.
 * @param usrpath File path to insert.
 * @param package Package name associated with the file.
 * @return 0 on success, 1 on error.
 */
int insert_file(sqlite3 *db, const char *usrpath, const char *package){
    sqlite3_stmt *stmt;
    const char *sql = "INSERT INTO files (file, package_id) VALUES (?, ?);";

    if (sqlite3_prepare_v2(db, sql, -1, &stmt, NULL) != SQLITE_OK){
        sqlite_error(db, "Error inserting package file in local db");
        return 1;
    }

    sqlite3_bind_text(stmt, 1, usrpath, -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 2, package, -1, SQLITE_TRANSIENT);

    if (sqlite3_step(stmt) != SQLITE_DONE){
        sqlite_error(db, "Error inserting package file in local db");
        sqlite3_finalize(stmt);
        return 1;
    }

    sqlite3_finalize(stmt);
    return 0;
}

/**
 * Delete all files associated with a package.
 * @param db Pointer to the SQLite database.
 * @param package Package name whose files will be deleted.
 * @return 0 on success, 1 on error.
 */
int delete_files(sqlite3 *db, const char *package){
    sqlite3_stmt *stmt;
    const char *sql = "DELETE FROM files WHERE package_id = ?;";

    if (sqlite3_prepare_v2(db, sql, -1, &stmt, NULL) != SQLITE_OK){
        sqlite_error(db, "Error deleting package file in local db");
        return 1;
    }

    sqlite3_bind_text(stmt, 1, package, -1, SQLITE_TRANSIENT);

    if (sqlite3_step(stmt) != SQLITE_DONE){
        sqlite_error(db, "Error deleting package file in local db");
        sqlite3_finalize(stmt);
        return 1;
    }

    sqlite3_finalize(stmt);
    return 0;
}

/**
 * List all files associated with a given package.
 * Allocates a FileList structure that must be freed with clean_filelist().
 * @param db Pointer to the SQLite database.
 * @param packageId Package name to list files for.
 * @return Pointer to FileList. error flag is set if something fails.
 */
FileList *list_package_files(sqlite3 *db, const char *packageId){
    sqlite3_stmt *stmt;
    FileList *result = calloc(1, sizeof(FileList));
    
    /* Allocate initial array for files */
    char **files = malloc(sizeof(char*) * CHUNK_SIZE);
    result->files = files;
    result->_size = CHUNK_SIZE;

    const char *sql = "SELECT file FROM files WHERE package_id = ?;";
    int rc;

    /* Prepare SQL statement */
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, NULL) != SQLITE_OK){
        sqlite_error(db, "Error listing package file in local db");
        result->error = 1;
        sqlite3_finalize(stmt);
        goto exit;
    }

    sqlite3_bind_text(stmt, 1, packageId, -1, SQLITE_TRANSIENT);

    /* Loop through rows and add them to FileList */
    while ((rc = sqlite3_step(stmt)) == SQLITE_ROW){
        int size = sqlite3_column_bytes(stmt, 0);

        /* Expand array if necessary */
        if (result->count >= result->_size){
            char **tmp = realloc(result->files, sizeof(char*) * (result->count + CHUNK_SIZE));
            if (!tmp){
                sqlite_error(db, "Memory Error: Cannot realloc pointer");
                result->error = 1;
                sqlite3_finalize(stmt);
                goto exit;
            }
            result->files = tmp;
            result->_size += CHUNK_SIZE;
            files = tmp;
        }

        /* Allocate and copy string */
        result->files[result->count] = malloc(sizeof(char) * (size + 1));
        const char *row = (const char*)sqlite3_column_text(stmt, 0);
        if (row == NULL){
            sqlite_error(db, "SQLite3 error: ");
            result->error = 1;
            sqlite3_finalize(stmt);
            goto exit;
        }

        memcpy(result->files[result->count], row, size);
        result->files[result->count][size] = '\0';
        result->count += 1;
    }

    /* Check for errors in stepping */
    if (rc == SQLITE_ERROR){
        sqlite_error(db, "Error retrieving package files from local db");
        result->error = 1;
    }
    
    sqlite3_finalize(stmt);

exit:
    return result;
}
