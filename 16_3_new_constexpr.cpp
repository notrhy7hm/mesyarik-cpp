#include <iostream>
#include <numeric>

constexpr bool is_prime(int n) {
    for (int i = 2; i * i <= n; ++i) {
        if (n % i == 0) return false;
    }
    return n > 1;
}

constexpr int count_primes(int n) {
    int* a = new int[n];
    std::iota(a, a + n, 0);
    int count = 0;
    for (int *p = a; p < a + n; ++p) {
        count += is_prime(*p);
    }
    delete[] a;
    return count;
}


int main() {

    constexpr int countPrimesTo100 = count_primes(100);

    static_assert(countPrimesTo100 == 25);
}
