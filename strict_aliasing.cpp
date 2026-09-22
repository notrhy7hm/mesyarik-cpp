#include <iostream>

long long foo(volatile long long* a, int* b) {
    if (*a != 11)
        *b = 11;

    return *a;
}

int main() {
    long long a = 22;

    a = foo(&a, (int*)&a);
    std::cout << a;
}
