#include "fetch.h"
#include "install.h"
#include <stdio.h>
#include <sys/stat.h>

int main(int argc, char **argv){
    if (argc == 2){
        struct stat st = {0};
        FILE *package_xz;

        if (stat("/usr/local/packages", &st) == -1){
            mkdir("/usr/local/packages", 755);
        }

        package_xz = fopen("/usr/local/packages/nano.fpi", "a+");
        int fetch = fetch_package_of_mirror("nano", argv[1], package_xz);

        if (fetch > 0) {
            fprintf(stderr, "An error ocurred while downloading package...\n");
            fclose(package_xz);
            return 1;
        }

        fseek(package_xz, 0, SEEK_SET);
        int xzExtract = extract_xz(package_xz, "nano");

        if (xzExtract > 1){
            fprintf(stderr, "An error ocurred while installing package (Step 1)...\n");
            fclose(package_xz);
            return 1;
        };

        char *post_install = NULL;
        int tarInstall = install_tar("nano", &post_install);

        if (tarInstall > 0){
            fprintf(stderr, "An error ocurred while installing package (Step 2)...\n");
            fclose(package_xz);
            return 1;
        }

        printf("Package Installed Successfully, Thx for using (:\n");
        fclose(package_xz);

        if (post_install) {
            int ret = post_mortem(post_install);
            free(post_install);
            post_install = NULL;
            return ret;
        }

        return 0;
    }

    printf("Pass mirror as parameter...");
    return 0;
}