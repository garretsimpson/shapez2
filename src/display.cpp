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

  std::string filename = argv[1];
  ShapeSet shapeSet = ShapeSet::load(filename);

  std::cout << "Halves: " << shapeSet.halves.size() << std::endl;
  std::cout << "Shapes: " << shapeSet.shapes.size() << std::endl;
  for (auto it : shapeSet.halves) {
    std::cout << it.toString() << std::endl;
  }
  std::cout << std::endl;
  for (auto it : shapeSet.shapes) {
    std::cout << it.toString() << std::endl;
  }
  std::cout << std::endl;
  shapeSet.clear();

  SolutionSet solnSet = SolutionSet::load(filename);
  std::sort(solnSet.solutions.begin(), solnSet.solutions.end());
  for (auto it : solnSet.solutions) {
    std::cout << it.toString() << std::endl;
  }
  solnSet.clear();

  return 0;
}
