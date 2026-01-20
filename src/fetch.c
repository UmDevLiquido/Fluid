#include <curl/curl.h>
#include <string.h>
#include <stdio.h>
#include "include/fluid.h"

/* =========================
   Function: fetch_package_of_mirror
   ========================= */
/**
 * Downloads a package file from a specified mirror using HTTP.
 *
 * The downloaded file is saved to the provided FILE pointer.
 *
 * @param package Name of the package to download (without extension).
 * @param mirror Base URL of the mirror to fetch from.
 * @param out_file Pointer to an already opened FILE where the package will be written.
 * @return 0 on success, 1 on failure.
 *
 * The function constructs the download URL as: <mirror>/<package>.fpi
 * It uses libcurl to perform the download and writes directly to out_file.
 */
int fetch_package_of_mirror(char *package, char *mirror, FILE *out_file){
    /* Construct local path (for reference, not used for writing here) */
    char path[512];
    snprintf(path, sizeof(path), "/usr/local/packages/%s.fpi", package);

    /* Initialize CURL session */
    CURL *curl = curl_easy_init();
    
    if (curl){
        /* Construct the URL to download */
        char url[512];
        snprintf(url, sizeof(url), "%s/%s.fpi", mirror, package);

        /* Set URL and output file for CURL */
        curl_easy_setopt(curl, CURLOPT_URL, url);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, out_file);

        /* Perform the download */
        CURLcode result = curl_easy_perform(curl);
        
        /* Check for errors */
        if (result != CURLE_OK){
            curl_easy_cleanup(curl);
            fprintf(stderr, "Cannot download file: %s\n", curl_easy_strerror(result));
            return 1;
        }

        /* Clean up CURL session */
        curl_easy_cleanup(curl);
        return 0;
    }

    /* CURL initialization failed */
    return 1;
}
