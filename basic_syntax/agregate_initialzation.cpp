#include <algorithm>
#include <array>
#include <iostream>
#include <ranges>
#include <span>
#include <string>
#include <vector>

struct A {
    std::string str;
    std::vector<int> vec;
    int arr[10];
};

struct B {
    std::string str;
    std::vector<int> vec;
    std::array<int, 10> arr;
};

class C {
public:
    std::string str;
    std::vector<int> vec;
    std::array<int, 10> arr;
};

class D {  // Non-aggregatable
    std::string str;
    std::vector<int> vec;
    std::array<int, 10> arr;
};

void nested_aggregation() {
    // aggregate
    struct base1 {
        int b1, b2 = 42;
    };
    // non-aggregate
    struct base2 {
        base2() : b3(42) {}
        int b3;
    };
    // aggregate in C++17
    struct derived : base1, base2 {
        int d;
    };
    derived d1{{1, 2}, {}, 4};  // d1.b1 = 1, d1.b2 = 2,  d1.b3 = 42, d1.d = 4
    derived d2{{}, {}, 4};      // d2.b1 = 0, d2.b2 = 42, d2.b3 = 42, d2.d = 4
}

int main(int argc, char* argv[]) {
    // Prefer std::array than c-array
    // Because std::array can use variable in aggregate initialization. see (3)
    std::string str{"string"};
    std::vector<int> vec{1, 2, 3};

    A a{str, {1, 2, 3, 4}, {1, 2, 3, 4, 5, 6, 7, 8, 9, 10}};  // (1)
    A b{"string", vec, {1, 2, 3}};                            // (2)

    std::array<int, 10> arr{1, 2, 3};
    B c{"string", {1, 2, 3}, arr};  // (3)

    std::cout << a.arr[2] << std::endl;

    // This is not aggregate initialzation.
    // It is initializer_list contstructor
    std::vector vec2{1, 2, 3, 4, 5, 6};

    // Here shows nested aggregation
    nested_aggregation();

    return 0;
}