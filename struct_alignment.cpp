#include <iostream>
#include <vector>
#include <string>
#include <array>

struct Container1
{
  int integer;
  double d;
  float f;
  std::array<int, 11> arr;
  // std::vector<int> vec;
  // std::string str;
} __attribute__((packed));

struct Container2
{
  int integer;
  double d;
  float f __attribute__((aligned(8)));
  std::array<int, 11> arr;
  // std::vector<int> vec;
  // std::string str;
} __attribute__((aligned(8))); // gcc attribute // defaut

// std keyword
struct alignas(8) Container3
{
  int integer;
  double d;
  float f;
  std::array<int, 11> arr;
  // std::vector<int> vec;
  // std::string str;
};

void print_Container1()
{
  Container1 c{1, 1.4, 0.5f, {1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11}};
  void *cPtr = &c;
  std::cout << "Size of container1: " << sizeof(c) << std::endl;

  std::cout << *(reinterpret_cast<int *>(cPtr)) << std::endl;
  // cPtr = cPtr + sizeof(int) + 4;
  // Although in struct double is priorer to float, but on a 64-bit system, compile try to put int(32bit) and a float(32bit) together

  cPtr = cPtr + sizeof(int);
  std::cout << *(reinterpret_cast<double *>(cPtr)) << std::endl;
  cPtr = cPtr + sizeof(double);
  std::cout << *(reinterpret_cast<float *>(cPtr)) << std::endl;
  cPtr = cPtr + sizeof(float);
  std::cout << (*(reinterpret_cast<std::array<int, 11> *>(cPtr)))[5] << std::endl;
  cPtr = cPtr + sizeof(int) * 10;
}

void print_Container2()
{
  Container2 c{1, 1.4, 0.5f, {1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11}};
  void *cPtr = &c;
  std::cout << "Size of container2: " << sizeof(c) << std::endl;

  std::cout << *(reinterpret_cast<int *>(cPtr)) << std::endl;
  cPtr = cPtr + 8;
  // Although in struct double is priorer to float, but on a 64-bit system, compile try to put int(32bit) and a float(32bit) together
  // see __attribute__(packed)
  std::cout << *(reinterpret_cast<double *>(cPtr)) << std::endl;
  cPtr = cPtr + 8;
  std::cout << *(reinterpret_cast<float *>(cPtr)) << std::endl;
  // since float is followed by int, alignment of 8bypte = float(4byte)+ 1* int(4byte);
  cPtr = cPtr + 4;
  std::cout << (*(reinterpret_cast<std::array<int, 10> *>(cPtr)))[5] << std::endl;

  // vector is store on the heap hence its address is not continuous anymore
  // std::cout << (*(reinterpret_cast<std::vector<int> *>(cPtr)))[3] << std::endl;
  // cPtr = cPtr + sizeof(int) * 4;
}

void print_Container3()
{
  Container3 c{1, 1.4, 0.5f, {1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11}};
  void *cPtr = &c;
  std::cout << "Size of container3: " << sizeof(c) << std::endl;

  std::cout << *(reinterpret_cast<int *>(cPtr)) << std::endl;
  cPtr = cPtr + 8;
  // Although in struct double is priorer to float, but on a 64-bit system, compile try to put int(32bit) and a float(32bit) together
  // see __attribute__(packed)
  std::cout << *(reinterpret_cast<double *>(cPtr)) << std::endl;
  cPtr = cPtr + 8;
  std::cout << *(reinterpret_cast<float *>(cPtr)) << std::endl;
  // since float is followed by int, alignment of 8bypte = float(4byte)+ 1* int(4byte);
  cPtr = cPtr + 4;
  std::cout << (*(reinterpret_cast<std::array<int, 10> *>(cPtr)))[5] << std::endl;

  // vector is store on the heap hence its address is not continuous anymore
  // std::cout << (*(reinterpret_cast<std::vector<int> *>(cPtr)))[3] << std::endl;
  // cPtr = cPtr + sizeof(int) * 4;
}

int main()
{
  std::cout << "container1: \n";
  print_Container1();
  std::cout << "\ncontainer2: \n";
  print_Container2();
  std::cout << "\ncontainer3: \n";
  print_Container3();

  return 0;
}