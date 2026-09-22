#include <iostream>
#include <vector>
#include <algorithm>
#include <set>
#include <utility>

struct S {
    int a = 0;
    auto getLambda() {
        auto f = [=](int x) {
            return x + a;
        };
    }
};





std::vector<std::string> v = {"abcde", "cdefg", "fgeab"};

template <typename... Strings>
requires((std::is_same_v<Strings, std::string> && ...))
void test(const Strings&... subs) {
    auto contains_substring = [&subs...](const std::string& str) {
        return ((str.find(subs) != std::string::npos) && ...);
    };
    auto iter = std::find_if(v.begin(), v.end(), contains_substring);

    std::cout << *iter << '\n';
}

int main() {
    std::string sub = "cd";
    std::string sub2 = "de";

    test(sub, sub2);
}

