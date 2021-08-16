#include <functional>
#include <iostream>
#include <string>

struct A {
    std::string a, b, c;
    void hash() {
        auto all = a + b + c;
        // hash is a templated function object
        std::cout << std::hash<std::string>{}(all) << '\n';
    }
};

int main() {
    A a{"a", "b", "c"};
    a.hash();
    A b{a};
    b.hash();
    A c{"a", "b", std::string{"c", 200}};
    c.hash();
    return 0;
}