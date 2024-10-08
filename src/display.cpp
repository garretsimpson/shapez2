#include <algorithm>
#include <iostream>

// #include "3ps/ska/bytell_hash_map.hpp"
#include "shapez.hpp"

#define NUM 10

int main(int argc, char* argv[]) {
  using namespace Shapez;

  if (argc != 2) {
    std::cout << "Usage: display dump.bin" << std::endl;
    return 1;
  }

  ShapeSet set = ShapeSet::load(argv[1]);
  // ska::bytell_hash_set<Shape> halves{set.halves.begin(), set.halves.end()};

  std::cout << "Halves: " << set.halves.size() << std::endl;
  std::cout << "Shapes: " << set.solutions.size() << std::endl;

  for (auto it : set.halves) {
    std::cout << it.toString() << std::endl;
  }
  std::cout << std::endl;

  for (auto it : set.solutions) {
    std::cout << it.toString() << std::endl;
  }
  std::cout << std::endl;

  return 0;
}
