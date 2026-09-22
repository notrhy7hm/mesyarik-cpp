#include <iostream>
#include <array>

constexpr bool isPrime(int value) {
    for (int i = 2; i <= value/2; ++i) {
        if (value % i == 0) {
            return false;
        }
    }
    return value > 1;
}

template <int Num>
consteval std::array<int, Num> primeNumbers() {
    std::array<int, Num> primes;
    int idx = 0;
    for (int val = 1; idx < Num; ++val) {
        if (isPrime(val)) {
            primes[idx++] = val;
        }
    }
    return primes;
}

int main() {
    auto primes = primeNUmbers<100>();
    for (auto v : primes) {
         std::cout << v << '\n';
    }
}
