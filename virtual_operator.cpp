#include <iostream>

struct Base
{
    virtual bool operator()() = 0;
};

struct Derived : public Base
{
    virtual bool operator()()
    {
        return true;
    };
};

int main()
{
    Base *obj = new Derived;
    std::cout << std::boolalpha << (*obj)() << std::endl;

    return 0;
}
