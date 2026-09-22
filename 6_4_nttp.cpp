#include <iostream>
#include <array>
#include <cstddef>
#include <vector>

template<typename T, size_t N>
class Array {
    T arr[N];
};

template <size_t M, size_t N, typename Field = double>
class Matrix {};

template <size_t N, typename Field = double>
using SquareMatrix = Matrix<N, N, Field>;

template <size_t M, size_t K, size_t N, typename Field>
Matrix<M, N, Field> operator*(const Matrix<M, K, Field>& a, const Matrix<K, N, Field>& b);

template <typename T, template<typename, typename> class Container = std::vector>
class Stack {
    Container<T, std::allocator<T>> container;
};

int main() {
    std::array<int, 100> a;

    const int x = 5;
    Matrix<x, x> m;

    Stack<int, std::vector> s;
}

