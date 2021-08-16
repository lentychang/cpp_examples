#include <iostream>
#include <string>
#include <vector>

// char const* template
// T1, T2 must be static
template <char const* T1>
struct A {
    void p() { std::cout << T1 << '\n'; }
    template <char const* T2>
    void print() {
        // p();
        std::cout << T2 << '\n';
    }
};
void run_char_ptr_template() {
    static char first[] = "THis is first line";
    static char second[] = "This is second line!!";
    auto a = A<first>{};

    a.p();
    a.print<second>();
}

int main() {
    run_char_ptr_template();
    return 0;
}