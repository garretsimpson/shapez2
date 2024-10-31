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

  std::vector<Shape> halves;
  std::vector<Shape> shapes;

  std::string filename = argv[1];
  ShapeSet shapeSet = ShapeSet::load(filename);
  // std::copy_if(shapeSet.halves.begin(), shapeSet.halves.end(), std::back_inserter(halves),
  //              [&](Shape shape) { return shape.layers() <= 4; });
  // std::copy_if(shapeSet.shapes.begin(), shapeSet.shapes.end(), std::back_inserter(shapes),
  //              [&](Shape shape) { return shape.layers() <= 4; });
  halves = {shapeSet.halves.begin(), shapeSet.halves.end()};
  shapes = {shapeSet.shapes.begin(), shapeSet.shapes.end()};
  shapeSet.clear();

  std::cout << "Halves: " << halves.size() << std::endl;
  std::cout << "Shapes: " << shapes.size() << std::endl;
  for (auto it : halves) {
    std::cout << it.toString() << std::endl;
  }
  std::cout << std::endl;
  for (auto it : shapes) {
    std::cout << it.toString() << std::endl;
  }
  std::cout << std::endl;

  SolutionSet solnSet = SolutionSet::load(filename);
  std::sort(solnSet.solutions.begin(), solnSet.solutions.end());
  for (auto it : solnSet.solutions) {
    std::cout << it.toString() << std::endl;
  }
  solnSet.clear();

  return 0;
}
