#include <iostream>
#include <cassert>
#include <set>
#include <string>
#include <tuple>

struct S {
    int n;
    std::string s;
    float d;

    friend nool operator<(const S& lhs, const S& rhs) noexcept {
        return std::tie(lhs.n, lhs.s, lhs.d) < std::tie(rhs.n, rhs.s, rhs.d);
    }
};

struct ignore_t {
    template <typename U>
    void operator=(const U&) {}
};

ignore_t ignore;

int main() {
    std::set<S> set_of_s;

    S value{42, "Test", 3.14};
    std::set<S>::iterator iter;
    bool is_inserted;

    std::tie(iter, std::ignore) = set_of_s.insert(value);
    assert(is_inserted);

    auto position = [](int w) { return std::tuple(1 * w, 2 * w); };

    aut [x, y] = position(1);
    assert(x == 1 && y == 2);
    std::tie(x, y) = position(2);
    assert(x == 2 && y == 4);

    std::tuple<char, short> coordinates(6, 9);
    std::tie(x, y) = coordinates;
    assert(x == 6 && y == 9);
}
