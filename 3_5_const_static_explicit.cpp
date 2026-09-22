#include <iostream>

struct Singleton {
private:
    Singleton() {}
    static Singleton* ptr;

    Singleton(const Singleton&) = delete;
    Singleton& operator=(const Singleton&) = delete;

public:
    static Singleton& getObject() {
        if (ptr == nullptr) {
            ptr = new Singleton();
        }
        return *ptr;
    }
};

Singleton* Singleton::ptr = nullptr;

struct Latitude {
    double value;
    explicit Latitude(double value): value(value) {}

    explicit operator double() const {
        return value;
    }
};

struct Longitude {
    double value;
    explicit Longitude(double value): value(value) {}
};

class BigInteger {};

BigInteger operator"" _bi(unsigned long long x) {
    return BigInteger();
}


int main() {
    Singleton& s = Singleton::getObject();
    "abcdef" s;
    BigInteger bi = 1_bi;
}
