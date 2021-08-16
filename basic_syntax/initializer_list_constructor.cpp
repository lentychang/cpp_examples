#include <initializer_list>
#include <iostream>
#include <iterator>
#include <string>
#include <vector>

// Regarding initializaer list
// std::initializer_list references static memory.
// That's what the class is for. You cannot move from static memory, because movement implies changing it.
// You can only copy from it

class InitializerListCtr {
    std::vector<int> vec;
    std::vector<int> vec2;

public:
    InitializerListCtr(std::initializer_list<std::vector<int>> list) : vec{}, vec2{} {
        auto& first_element = *(list.begin());
        auto& second_element = *(list.begin() + 1);

        vec.insert(vec.begin(), first_element.begin(), first_element.end());
        vec2.insert(vec2.begin(), second_element.begin(), second_element.end());
    }
    void print() {
        std::copy(vec.begin(), vec.end(), std::ostream_iterator<int>(std::cout, " "));
        std::cout << '\n';
        std::copy(vec2.begin(), vec2.end(), std::ostream_iterator<int>(std::cout, " "));
        std::cout << "\n\n";
    }
};

int main(int argc, char* argv[]) {
    InitializerListCtr a = InitializerListCtr{{1, 2, 3}, {1, 2, 3}};
    a.print();

    std::vector<int> b1{4, 5, 6}, b2{7, 8, 9};

    InitializerListCtr b = InitializerListCtr{{b1, b2}};

    // InitializerListCtr b = InitializerListCtr{b1, b2}; this will also call initializer_list
    // [Notice] If it is not agregatable struct/union it will be called prior to Class(T1 t1,T2 t2)

    b.print();

    return 0;
}