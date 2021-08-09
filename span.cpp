#include <span>
#include <iostream>
#include <algorithm>
#include <vector>
#include <ranges>
#include <random>

typedef int T;

void print_buffer(std::span<T> buffer)
{
  std::cout << "Size of bytes: " << buffer.size_bytes() << "\nNumber of Elements: " << buffer.size() << "\n";
  std::ranges::for_each(buffer, [](int a)
                        { std::cout << a << " "; });
  std::cout << '\n';
}

int main()
{
  // std::random_device seed;
  std::mt19937 generator(0);
  std::uniform_int_distribution<int> distribution(0, 100);

  // C array, assign random number
  T buff[10];
  for (auto i = 0; i < 10; i++)
    buff[i] = distribution(generator);

  // init buffer array and print all value
  print_buffer(buff);

  // Accessing through sub-array sp2 and modify value
  // span stores pointer with size, can be seen as the reference/alias of array
  std::span<T> sp1{buff};
  auto sp2 = sp1.subspan(1, 3);

  std::transform(sp2.begin(), sp2.end(), sp2.begin(), [](int i)
                 { return i * i; });
  print_buffer(buff);

  // span size: 16 bytes
  std::cout << "Size of span: " << sizeof(sp1) << "\n";

  // span can also be used with STL containers
  std::vector<int> v{1, 2, 3};
  std::span<int> sv{v};
  std::ranges::for_each(sv, [](int a)
                        { std::cout << a << " "; });

  return 0;
}