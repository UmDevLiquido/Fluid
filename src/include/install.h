#ifndef INSTALL_H
#define INSTALL_H

#include <stdio.h>

/* =========================
   Buffer sizes for XZ decompression
   ========================= */
#define INPUT_SIZE 8192   /* Input buffer for reading compressed data */
#define OUTPUT_SIZE 4096  /* Output buffer for decompressed data */

/* =========================
   Function: extract_xz
   ========================= */
/**
 * Decompresses an .xz file from a given FILE pointer into a .tar file.
 * The resulting tar is stored in /usr/local/packages/<name>.tar.
 * 
 * @param package Pointer to the opened .xz file.
 * @param name Name of the package (used to create output path).
 * @return 0 on success, 1 on error.
 */
int extract_xz(FILE *package, char* name);

/* =========================
   Function: install_tar
   ========================= */
/**
 * Extracts a .tar archive and installs its files to /usr directory.
 * 
 * Filters out unsafe paths (..), handles the ".install" file separately,
 * and writes regular files with proper permissions.
 * 
 * @param name Name of the package (used to locate /usr/local/packages/<name>.tar).
 * @param post_install Pointer to receive the contents of a ".install" file if present.
 *                     The caller is responsible for freeing the buffer.
 * @return 0 on success, 1 on error.
 */
int install_tar(char *name, char **post_install);

/* =========================
   Function: post_mortem
   ========================= */
/**
 * Executes a shell script using /bin/bash in a child process.
 * The parent process waits until the child finishes.
 * 
 * @param script The script commands to execute.
 * @return 0 on success.
 */
int post_mortem(char *script);

#endif /* INSTALL_H */
