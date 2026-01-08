#ifndef INSTALL_H
    #define INSTALL_H
    #include <stdio.h>

    #define INPUT_SIZE 8192
    #define OUTPUT_SIZE 4096

    int extract_xz(FILE *package, char* name);
    int install_tar(char *name, char **post_install);
    int post_mortem(char *script);
#else

#endif