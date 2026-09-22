#include <stdio.h>
#include <sys/stat.h>
#include <unistd.h>

int main() {
    const char* filename = "output.txt";

    struct stat fileStat;

    if (stat(filename, &fileStat) == -1) {
        perror("Error using stat");
        return 1;
    }


    printf("File: %s\n", filename);
    printf("Size: %lld bytes\n", (long long)fileStat.st_size);
    printf("Mode: %o\n", fileStat.st_mode);
    printf("Owner UID: %u\n", fileStat.st_uid);
    printf("Group GID: %u\n", fileStat.st_gid);

    return 0;
}
