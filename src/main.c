#include <stdio.h>
#include <stdlib.h>
#include "include/fluid.h"  // seu umbrella header incluindo fetch, install, package_db

int main(int argc, char *argv[]) {
    if (argc < 3) {
        fprintf(stderr, "Usage: %s <install/check> <package>\n", argv[0]);
        return 1;
    }

    char *command = argv[1];
    char *package = argv[2];

    /* Connect to the database */
    sqlite3 *db = connect_db();
    if (!db) return 1;

    if (strcmp(command, "install") == 0) {
        char path[512];
        snprintf(path, sizeof(path), "/usr/local/packages/%s.fpi", package);

        /* Open file to store the downloaded package */
        FILE *fpi_file = fopen(path, "wb");
        if (!fpi_file) {
            perror("fopen");
            sqlite3_close(db);
            return 1;
        }

        /* Fetch the package from a mirror (hardcoded for now) */
        if (fetch_package_of_mirror(package, "http://localhost:8000", fpi_file) != 0) {
            fclose(fpi_file);
            sqlite3_close(db);
            return 1;
        }
        fclose(fpi_file);

        /* Extract XZ to TAR */
        fpi_file = fopen(path, "rb");
        if (!fpi_file) {
            perror("fopen");
            sqlite3_close(db);
            return 1;
        }

        char *post_install = NULL;
        if (extract_xz(fpi_file, package) != 0) {
            fclose(fpi_file);
            sqlite3_close(db);
            return 1;
        }
        fclose(fpi_file);

        /* Install the tarball */
        if (install_tar(package, &post_install) != 0) {
            sqlite3_close(db);
            free(post_install);
            return 1;
        }

        /* Insert package into DB */
        insert_package(db, package);

        /* Run post_install script if exists */
        if (post_install) {
            post_mortem(post_install);
            free(post_install);
        }

        printf("Package %s installed successfully!\n", package);

    } else if (strcmp(command, "check") == 0) {
        int installed = is_package_installed(db, package);
        if (installed == 1) {
            printf("Package %s is installed.\n", package);
        } else if (installed == 0) {
            printf("Package %s is not installed.\n", package);
        } else {
            printf("Error checking package %s.\n", package);
        }
    } else {
        fprintf(stderr, "Unknown command: %s\n", command);
    }

    sqlite3_close(db);
    return 0;
}
