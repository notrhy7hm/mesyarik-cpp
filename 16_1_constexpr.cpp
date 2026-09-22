#include <iostream>
#include <array>

constexpr int max(int x, int y) {
    return x > y ? x : y;
}

constexpr bool is_prime(int n) {
    for (int i = 2; i * i <= n; ++i) {
        if (n % i == 0) return false;
    }
    return n > 1;
}

constexpr int count_primes() {
    int a[11] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
    int count = 0;
    for (int *p = a; p < a + 11; ++p) {
        count += is_prime(*p);
    }

    return count++ + ++count;
}



int main() {
    static_assert(is_prime(1'000'000'009));

    static_assert(count_primes() == 4);

    int y;
    std::cin >> y;

    std::cout << is_prime(y);



}
