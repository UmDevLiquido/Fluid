#include "include/fluid.h"
#include <stdio.h>
#include <libtar.h>
#include <lzma.h>
#include <stdlib.h>
#include <fcntl.h>
#include <string.h>
#include <unistd.h>
#define INPUT_SIZE 8192
#define OUTPUT_SIZE 4096

int extract_xz(FILE *package, char *name){
	unsigned char *inbuff = malloc(INPUT_SIZE);
	unsigned char *outbuff = malloc(OUTPUT_SIZE);

	char path[512];
	snprintf(path, sizeof(path), "/usr/local/packages/%s.tar", name);
	FILE *out_file = fopen(path, "wb");
	if (!out_file){
		perror("fopen");
		free(inbuff);
		free(outbuff);
		return 1;
	}

	lzma_stream strm = LZMA_STREAM_INIT;
	lzma_ret ret_stream = lzma_stream_decoder(&strm, UINT64_MAX, 0);

	if (ret_stream != LZMA_OK) {
		fprintf(stderr, "Failed to initialize XZ decoder: %d\n", ret_stream);
		free(inbuff);
		free(outbuff);
    		return 1;
	}

	size_t size;

	while ((size = fread(inbuff, 1, INPUT_SIZE, package)) > 0) {
		strm.next_in = inbuff;
		strm.avail_in = size;

		do {
			strm.next_out = outbuff;
			strm.avail_out = OUTPUT_SIZE;

			lzma_ret ret = lzma_code(&strm, feof(package) ? LZMA_FINISH : LZMA_RUN);

			if (ret != LZMA_OK && ret != LZMA_STREAM_END){
				fprintf(stderr, "Decompression Error 1: %d\n", ret);
				lzma_end(&strm);
				free(inbuff);
				free(outbuff);
				fclose(out_file);
				return 1;
			}

			size_t written = OUTPUT_SIZE - strm.avail_out;
			if (written > 0)
				fwrite(outbuff, 1, written, out_file);

			if (ret == LZMA_STREAM_END) break;
		} while (strm.avail_out == 0);

	}

	free(inbuff);
	free(outbuff);
	fclose(out_file);
	lzma_end(&strm);
	return 0;
}

int install_tar(char *name, char **post_install){
    TAR *handle;

    char path[512];
    snprintf(path, sizeof(path), "/usr/local/packages/%s.tar", name);

    int ret = tar_open(&handle, path, NULL, O_RDONLY, 0, TAR_GNU);

    if (ret == -1) {
        fprintf(stderr, "Cannot extract file: Tarlib Code %d\n", ret);
        return 1;
    }


    while ((ret = th_read(handle)) == 0) {
        char realpath[512];

        // Filtrar
        const char *th_pathname = th_get_pathname(handle);

        if (strstr(th_pathname, "..") != NULL) {
            if (TH_ISREG(handle)) tar_skip_regfile(handle);
            continue; 
        }

        // Escrever
		if (th_pathname != ".install"){
			snprintf(realpath, sizeof(realpath), "/usr/%s", th_pathname);
		} else {
			size_t size = th_get_size(handle);
			char *buf = malloc(size + 1);

			size_t remaining = size;
			size_t offset = 0;

			do {
				char block[512];
				if (tar_block_read(handle, block) != 512)
					break;

				size_t copy = remaining < 512 ? remaining : 512;
				memcpy(buf + offset, block, copy);

				offset += copy;
				remaining -= copy;
			} while (remaining > 0);

			buf[size] = '\0';

			/* ownership of buf is transferred to post_install */
			*post_install = buf;

			if (size % 512) {
				char dummy[512];
				tar_block_read(handle, dummy);
			}

			continue;
		}

		if (TH_ISREG(handle)){
        	if(tar_extract_file(handle, realpath) == -1){
            	perror("tar_extract_file");
            	tar_close(handle);
				continue;
        	}

			sync();

			if (chmod(realpath, 0755) != 0) {
				perror("chmod");
			}
		}
    }

    tar_close(handle);
    return 0;
}

int post_mortem(char *script){
	extern char **environ;
	char *argv[] = {
		"bash",
		"-c",
		script,
		NULL
	};

	pid_t child_pid = fork();

	if (child_pid == -1){
		perror("fork failed");
		exit(EXIT_FAILURE);
	} else if (child_pid == 0){
		execve("/bin/bash",  argv, environ);

		perror("execve failed");
		exit(EXIT_FAILURE);
	} else {
		wait(NULL);
	}

	return 0;
}