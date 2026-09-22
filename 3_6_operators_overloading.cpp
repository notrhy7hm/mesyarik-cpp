#include <iostream>
#include <compare>

struct Complex {
    double re = 0.0;
    double im = 0.0;
    Complex(double re): re(re) {}
    Complex(double re, double im): re(re), im(im) {}

    std::weak_ordering operator<=>(const Complex& other) const {
        return std::weak_ordering::equivalent;
    }

    Complex& operator+=(const Complex& other) {
        re += other.re;
        im += other.im;
        return *this;
    }
};

std::ostream& operator<<(std::ostream& out, const Complex& str);
std::istream& operator>>(std::istream& in, Complex& str);

Complex operator+(const Complex& a, const Complex& b) {
    Complex result = a;
    result += b;
    return result;
}

bool operator<(const Complex& a, const Complex& b) {
    return a.re < b.re || a.re == b.re && a.im < b.im;
}

bool operator>(const Complex& a, const Complex& b) {
    return b < a;
}

struct UserId {
    int value = 0;

    UserId& operator++() {
        ++value;
        return *this;
    }
    UserId operator++(int) {
        UserId copy = *this;
        ++value;
        return copy;
    }
};

struct Greater {
    

    bool operator()(int x, int y) const {
        return x > y;
    }
};


int main() {
    std::vector<int> v(10);
    std::sort(v.begin(), v.end(), Greater());

    Complex a(1.0);
    Complex b(2.0);
    Complex c(3.0);

}
