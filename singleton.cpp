#include <iostream>
#include <string>

class Singleton
{
  inline static Singleton *instance = nullptr;
  int var = 0;
  // Disable singleton
  Singleton() {}

public:
  static inline int count = 0;
  static Singleton *getInstance()
  {
    if (instance == nullptr)
    {
      instance = new Singleton;
    }
    return instance;
  };
};

int main()
{
  // Singleton s;

  auto s = Singleton::getInstance();

  return 0;
}