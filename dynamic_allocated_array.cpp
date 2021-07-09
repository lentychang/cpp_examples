#include <iostream>
int main()
{
  int n;
  std::cin >> n;
  int *k = new int[n];
  k[1] = 2;
  std::cout << k[1];

  delete[] k;
  return 0;
}