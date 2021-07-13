#include <iostream>
#include <fstream>

int main()
{
  std::ifstream f{"/home/lenty/what.JPG", std::ios::ate | std::ios::in};
  auto size = f.tellg();
  std::cout << "size: " << size;

  f.seekg(0, std::ios::beg);

  char *buffer = new char[size];
  f.read(buffer, size);

  uint8_t *ubffer = new uint8_t[size];

  for (size_t i = 0; i < size; ++i)
  {
    ubffer[i] = static_cast<uint8_t>(buffer[i]);
  }
  std::ofstream g{"/home/lenty/what3.JPG"};
  g.write(reinterpret_cast<char *>(&ubffer[0]), size);
  g.close();

  return 0;
}