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

  Shape shape;
  size_t num;

  std::cout << "Halves: " << set.halves.size() << std::endl;
  num = set.halves.size();
  for (size_t i = 0; i < num; ++i) {
    shape = set.halves[i];
    std::cout << shape.toString() << std::endl;
  }
  std::cout << std::endl;

  std::cout << "Shapes: " << set.shapes.size() << std::endl;
  num = set.shapes.size();
  for (size_t i = 0; i < num; ++i) {
    shape = set.shapes[i];
    std::cout << shape.toString() << std::endl;
  }
  std::cout << std::endl;

  return 0;
}
