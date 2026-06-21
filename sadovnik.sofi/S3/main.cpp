#include <iostream>

int main(int argc, char * argv[])
{
  if (argc != 2)
  {
    std::cerr << "filename required\n";
    return 1;
  }

  (void)argv;
  std::cerr << "S3 scaffold: graph loader is not implemented yet\n";
  return 1;
}
