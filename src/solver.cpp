#include <algorithm>
#include <iostream>

#include "3ps/ska/bytell_hash_map.hpp"
#include "shapez.hpp"

namespace Shapez {

struct Solver {
  using T = Shape::T;
  constexpr static size_t PART = Shape::PART;
  constexpr static size_t LAYER = Shape::LAYER;

  ShapeSet set;
  ska::bytell_hash_set<Shape> halves;

  Solver(char* filename) {
    set = ShapeSet::load(filename);
    halves = {set.halves.begin(), set.halves.end()};
  }

  // Whether a shape can be constructed by swapping two halves.
  bool swappable(Shape shape) const {
    constexpr T mask = repeat<T>(repeat<T>(3, 2, PART / 2), 2 * PART, LAYER);
    for (size_t angle = 0; angle < PART / 2; ++angle) {
      Shape left{shape.rotate(angle).value & mask};
      Shape right{shape.rotate(angle + PART / 2).value & mask};
      // std::cout << left.toString() << std::endl;
      // std::cout << right.toString() << std::endl;
      // TODO: Use shape.collapse()
      left = left.equivalentHalves()[0];
      right = right.equivalentHalves()[0];
      if (left.value == 0 || right.value == 0) return true;
      if (halves.find(left) != halves.end() &&
          halves.find(right) != halves.end()) {
        return true;
      }
    }

    return false;
  }

  void verifyShapes() {
    size_t num;
    Shape shape;
    size_t found = 0;
    // shape = {"SSS-:----:----:----"};

    num = set.shapes.size();
    std::cout << "Shapes: " << num << std::endl;
    for (size_t i = 0; i < num; ++i) {
      shape = set.shapes[i];
      if (swappable(shape)) {
        std::cout << "Found: " << shape.toString() << std::endl;
        found++;
      }
    }
    if (found == 0)
      std::cout << "None found" << std::endl;
    else
      std::cout << "Found: " << found << std::endl;
  }
};

}  // namespace Shapez

int main(int argc, char* argv[]) {
  if (argc != 2) {
    std::cout << "Usage: solver dump.bin" << std::endl;
    return 1;
  }
  char* filename = argv[1];

  Shapez::Solver solver(filename);
  solver.verifyShapes();

  return 0;
}
