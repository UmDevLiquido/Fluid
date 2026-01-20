#ifndef FETCH_H
#define FETCH_H

#include <stdio.h>

/* =========================
   Function: fetch_package_of_mirror
   ========================= */
/**
 * Downloads a package file from a specified mirror using HTTP.
 *
 * @param package Name of the package to download (without extension).
 * @param mirror Base URL of the mirror to fetch from.
 * @param out_file Pointer to an already opened FILE where the package will be written.
 * @return 0 on success, 1 on failure.
 *
 * Notes:
 * - The function constructs the download URL as: <mirror>/<package>.fpi
 * - Uses libcurl to perform the download.
 * - The caller is responsible for opening and closing the FILE pointer.
 */
int fetch_package_of_mirror(char *package, char *mirror, FILE *out_file);

#endif /* FETCH_H */
