#include <algorithm>
#include <iostream>

#include "shapez.hpp"

namespace Shapez {

struct Solver {
  using T = Shape::T;
  constexpr static size_t PART = Shape::PART;
  constexpr static size_t LAYER = Shape::LAYER;

  ShapeSet set;

  Solver(char* filename) { set = ShapeSet::load(filename); }

  // Whether a shape can be constructed by swapping two halves.
  bool swappable(Shape shape) const {
    constexpr T mask = repeat<T>(repeat<T>(3, 2, PART / 2), 2 * PART, LAYER);
    for (size_t angle = 0; angle < PART / 2; ++angle) {
      Shape left{shape.rotate(angle).value & mask};
      Shape right{shape.rotate(angle + PART / 2).value & mask};
      std::cout << left.toString() << std::endl;
      std::cout << right.toString() << std::endl;
      if (left.value == 0 || right.value == 0) return true;
    }

    return false;
  }

  void verifyShapes() {
    Shape shape;
    size_t num;

    num = set.shapes.size();
    std::cout << "Shapes: " << num << std::endl;
    // for (size_t i = 0; i < num; ++i) {
    //   shape = set.halves[i];
    //   std::cout << shape.toString() << std::endl;
    // }
    shape = set.shapes[0];
    std::cout << shape.toString() << std::endl;
    bool result = swappable(shape);
    std::cout << "result: " << result << std::endl;
  }
};

}  // namespace Shapez

int main(int argc, char* argv[]) {
  // using namespace Shapez;

  if (argc != 2) {
    std::cout << "Usage: solver dump.bin" << std::endl;
    return 1;
  }
  char* filename = argv[1];

  Shapez::Solver solver(filename);
  solver.verifyShapes();

  return 0;
}
