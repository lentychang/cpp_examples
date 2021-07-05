#include <iostream>
#include <string>

// #include <source_location> this is supported only since gcc11, clang doesn't support currently

template<typename T>
void func(T k)
{
    // standard C++11
    std::cout << __DATE__ << " " << __TIME__ << " File: " << __FILE__ << " Function: "  << __func__ << " Line: " <<__LINE__ << std::endl;
    // __PRETTY_FUNCTION__ is only gcc/clang specific, not standard
    std::cout << __DATE__ << " " << __TIME__ << " File: " << __FILE__ << " Function: "  << __PRETTY_FUNCTION__ << " Line: " <<__LINE__ << std::endl;
}

int main()
{

    std::cout << __DATE__ << " " << __TIME__ << " File: " << __FILE__ << " Line: " <<__LINE__ << std::endl;

    func(1);
    
    return 0;
}