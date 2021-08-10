#include <iostream>
#include <string>
#include <thread>
#include <mutex>

// This is not thread safe!!
class singleThreadSingleton
{
  static singleThreadSingleton *instance;
  std::string str;
  // Disable singleton
  singleThreadSingleton(const std::string &s) : str{s} {}

public:
  static singleThreadSingleton *getInstance(const std::string &s)
  {
    if (instance == nullptr)
    {
      instance = new singleThreadSingleton(s);
    }
    return instance;
  };
  void print()
  {
    std::cout << str << "\n";
  }
};

inline singleThreadSingleton *singleThreadSingleton::instance = nullptr;

void singleThreadFoo()
{
  std::this_thread::sleep_for(std::chrono::milliseconds(500));
  auto p = singleThreadSingleton::getInstance("Foo");
  p->print();
}

void singleThreadBar()
{
  std::this_thread::sleep_for(std::chrono::milliseconds(500));
  auto p = singleThreadSingleton::getInstance("Bar");
  p->print();
}

class ThreadSafeSingleton
{
  static ThreadSafeSingleton *instance;
  static std::mutex mx;
  std::string str;
  ThreadSafeSingleton(const std::string &s) : str{s} {}

public:
  static ThreadSafeSingleton *getInstance(const std::string &s)
  {
    std::scoped_lock sl{mx};
    if (ThreadSafeSingleton::instance == nullptr)
    {
      ThreadSafeSingleton::instance = new ThreadSafeSingleton(s);
    }
    return instance;
  }
  void print()
  {
    std::cout << str << "\n";
  }
};

inline ThreadSafeSingleton *ThreadSafeSingleton::instance = nullptr;
inline std::mutex ThreadSafeSingleton::mx{};

void ThreadSafeFoo()
{
  std::this_thread::sleep_for(std::chrono::milliseconds(500));
  auto p = ThreadSafeSingleton::getInstance("Foo");
  p->print();
}

void ThreadSafeBar()
{
  std::this_thread::sleep_for(std::chrono::milliseconds(500));
  auto p = ThreadSafeSingleton::getInstance("Bar");
  p->print();
}

int main()
{
  // singleThreadSingleton

  // If the value is not the same, it means this is not thread safe!
  // Solution: add mutex to block static pointer
  auto th1 = std::thread(&singleThreadFoo);
  auto th2 = std::thread(&singleThreadBar);
  th1.join();
  th2.join();

  // Thread safe singleton

  auto th3 = std::thread(&ThreadSafeFoo);
  auto th4 = std::thread(&ThreadSafeBar);
  th3.join();
  th4.join();

  return 0;
}