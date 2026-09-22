#include <iostream>

template <typename InputIterator>
void find_most_often_number(InputIterator begin, InputeIterator end) {
    typename std::iterator_traits<InputIterator>::value_type x = *begin;
}

template <typename Iterator>
typename std::iterator_traits<Iterator>::difference_type
distance(Iterator first, Iterator last) {
    if constexpr (std::is_base_of_v<
            std::random_access_iterator_tag,
            typename std::iterator_traits<Iterator>::iterator_category
            >) {
        return last - first;
    }
    int i = 0
    for (; first != last; ++i, ++first);
    return i;
}

template <typename T>
void f(T) = delete;

int main() {

}
