#include <iostream>
#include <concepts>
#include <vector>
#include <list>

template <typename T>
concept InputIterator = requires(T x) {
    *x;
    ++x;
};

template <typename T>
concept ForwardIterator = InputIterator<T> && requires(T x) {
    x++;
};

template <typename T>
concept BidirectionalIterator = ForwardIterator<T>
&& requires(T x) {
    --x;
    x--;
};

template <typename t>
concept RandomAccessIterator =
requires (T x, T y) {
    x - y;
    x < y; x > y; x<= y; x>= y;
    x += 1; x -= 1;
    x + 1; x - 1; 1 + x;
};


 template <std::input_iterator Iter>
void my_advance(Iter& iter, size_t n) {
    for (size_t i = 0; i < n; ++i) {
        ++iter;
    }
}

template <std::random_access_iterator Iter>
void my_advance(Iter& iter, size_t n) {
    iter += n;
}

int main() {
    std::vector<int> v(10);
    auto vit = v.begin();
    my_advance(vit, 5);

    std::list<int> l(10);
    auto lit = l.begin();
    my_advance(lit, 5);
}
