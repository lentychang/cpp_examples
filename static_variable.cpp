#include <iostream>
#include <string>
#include <vector>
#include <memory>

class A{
    public:
    static std::vector<int> vec;
    static std::shared_ptr<std::vector<int>> vecPtr;
};

std::vector<int> A::vec{1,2,3};
static std::shared_ptr<std::vector<int>> vecPtr;
int main(){
    
    std::cout << A::vec[0];
    A::vec.push_back(1);
    A::vec.push_back(1);
    A::vec.push_back(1);
    A::vec.push_back(5);
    A::vec.push_back(1);
    A::vec.push_back(1);
    A::vec.push_back(1);
    std::cout << A::vec[6];
    std::vector v{1,2,3,4,5};
    vecPtr = std::make_shared<std::vector<int>>(v);
    std::cout << (*vecPtr)[1];

    return 0;
}