#ifndef REGISTRY_H
#define REGISTRY_H

#include <sqlite3.h>
#include <stddef.h>

/* Chunk size for dynamic file lists */
#define CHUNK_SIZE 32

/* Struct representing a list of files for a package */
typedef struct {
    size_t count;      /* Number of files currently stored */
    size_t _size;   /* Internal capacity of the files array */
    int error;      /* Error flag: 0 = OK, 1 = error occurred */
    char **files;   /* Array of file paths */
} FileList;

/* =======================
   FileList memory handling
   ======================= */

/* Free a FileList and all its contents */
void clean_filelist(FileList *fl);

/* =======================
   SQLite database utilities
   ======================= */

/* Print SQLite error message to stderr */
void sqlite_error(sqlite3 *db, const char *msg);

/* Connect to the local package database, creating tables if needed */
sqlite3* connect_db(void);

/* =======================
   Package operations
   ======================= */

/* Insert a package name into the installed table */
int insert_package(sqlite3 *db, const char *package_name);

/* Delete a package from the installed table */
int delete_package(sqlite3 *db, const char *package_name);

/* Check if a package is installed
   Returns:
   1 = installed
   0 = not installed
   2 = SQLite error occurred */
int is_package_installed(sqlite3 *db, const char *package_name);

/* =======================
   Package file operations
   ======================= */

/* Insert a file for a package */
int insert_file(sqlite3 *db, const char *usrpath, const char *package);

/* Delete all files associated with a package */
int delete_files(sqlite3 *db, const char *package);

/* List all files for a given package
   Returns a FileList struct (caller must call clean_filelist) */
FileList *list_package_files(sqlite3 *db, const char *packageId);

#endif /* PACKAGE_DB_H */
