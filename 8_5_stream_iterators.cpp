#include <iostream>
#include <iterator>
#include <vector>
#include <algorithm>
#include <fstream>


template <typename T>
class optional {
    char value[sizeof(T)];
    bool initialized = false;
public:
    optional(const T& newvalue): initialized(true) {
        new(value) T(newvalue);
    }
    optional() {}
    ~optional() {
        if (initialized) {
            reinterpret_cast<T*>(value)->~T();
        }
    }
    bool has_value() const {
        return initialized;
    }
    operator bool() const {
        return initialized;
    }
    T& operator*() {
        return reinterpret_cast<T&>(*value);
    }
    const T& value_or(const T& other) const {
        return initialized ? reinterpret_cast<T&>(*value) : other;
    }
};

struct nullopt_t {};
nullopt_t nullopt;

template<typename T>
class istream_iterator {
    std::istream* in = nullptr;
    T value;
public:
    using iterator_category = std::input_iterator_tag;
    using pointer = T*;
    using value_type = T;
    using reference = T&;
    using difference_type = int;

    istream_iterator(std::istream& in): in(&in){
        in >> value;
    }
    istream_iterator() {}

    istream_iterator& operator++() {
        if (!(*in >> value)) {
            *this = istream_iterator();
        }
        return *this;
    }
    T& operator*() {
        return value;
    }
};

bool even(int x) {
    return x % 2 == 0;
}

int main() {

    std::ifstream in("input.txt");
    std::istream_iterator<int> it(in);

    // std::copy_if(it, std::istream_iterator<int>(), std::ostream_iterator<int>(std::cout, " "),
    //        [](int x) {return x % 2 == 0;});

    std::transform(it, std::istream_iterator<int>(), std::ostream_iterator<int>(std::cout, " "),
            [](int x) {return x*x;});
}
