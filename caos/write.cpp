#include <unistd.h>
#include <cstring>
#include <cstdlib>
#include <errno.h>
#include <string>


int main() {
    char str[100] = { 'H', 'e', 'l', 'l', 'o', ' '};
    read(0, str + 6, 5);

    const int len = 20;


    for (int i = len; i > 0; ) {
        ssize_t res = write(3, str + len - i, i);
        if (res >= 0) {
            i -= res;
            continue;
        }

        auto err_code = std::to_string(errno);
        if (errno != EBADF) {
        write(2, err_code.data(), 5);
        }
        exit(1);
    }

    
}
