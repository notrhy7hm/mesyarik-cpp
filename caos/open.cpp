#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>
#include <errno.h>


int main() {
    int fd = open("./output.txt", O_WRONLY|O_CREAT);
    printf("%d\n", fd);

    const char* buf = "Hello World!";
    int res = write(fd, buf, 10);
    printf("%d\e", res);
    printf("%d", errno);
    
    close(fd);
    // write(1, buf, 10);
}
