#include <curl/curl.h>
#include <string.h>
#include <stdio.h>
#include "fetch.h"

int fetch_package_of_mirror(char *package, char *mirror, FILE *out_file){
	char path[512];
	snprintf(path, sizeof(path), "/usr/local/packages/%s.fpi", package);

	CURL *curl = curl_easy_init();
	
	if (curl){
		char url[512];
		snprintf(url, sizeof(url), "%s/%s.fpi", mirror, package);
		curl_easy_setopt(curl, CURLOPT_URL, url);
		curl_easy_setopt(curl, CURLOPT_WRITEDATA, out_file);
		CURLcode result = curl_easy_perform(curl);
		
		if (result != CURLE_OK){
			curl_easy_cleanup(curl);
			fprintf(stderr, "Cannot download file: %s\n", curl_easy_strerror(result));
			return 1;
		}

		curl_easy_cleanup(curl);
		
		
		return 0;
	}

	return 1;
}