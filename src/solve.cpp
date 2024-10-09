#include <iostream>

#include "3ps/ska/bytell_hash_map.hpp"
#include "shapez.hpp"

namespace Shapez {

struct Solve {
  ShapeSet set;
  ska::bytell_hash_set<Shape> halves;
  ska::bytell_hash_map<Shape, Build> builds;

  Solve(char* filename) {
    set = ShapeSet::load(filename);

    halves = {set.halves.begin(), set.halves.end()};
    for (auto it : set.solutions) {
      builds[it.shape] = it.build;
    }
  }

  std::optional<std::pair<Shape, Shape>> findSwap(Shape shape) {
    constexpr Shape::T mask = repeat<Shape::T>(
        repeat<Shape::T>(3, 2, Shape::PART / 2), 2 * Shape::PART, Shape::LAYER);
    for (size_t angle = 0; angle < Shape::PART / 2; ++angle) {
      Shape left{shape.rotate(angle).value & mask};
      Shape right{shape.rotate(angle + Shape::PART / 2).value & mask};
      left = left.equivalentHalves()[0];
      right = right.equivalentHalves()[0];
      if (halves.find(left) != halves.end() &&
          halves.find(right) != halves.end()) {
        return std::make_pair(left, right);
      }
    }
    return std::nullopt;
  }

  std::string run(Shape shape, size_t indent = 0) {
    std::string result = "";
    Solution solution;

    for (auto i = 0; i < indent; ++i) {
      result += "  ";
    }

    shape = shape.equivalentShapes()[0];
    auto swapShapes = findSwap(shape);
    if (swapShapes.has_value()) {
      if (swapShapes->first.value == 0 || swapShapes->second.value == 0) {
        // one half shape
        result += shape.toString() + "\n";
      } else {
        // two half shapes that can swapped
        Build build{Op::Swap, swapShapes->first, swapShapes->second};
        solution = {shape, build};
        result += solution.toString() + "\n";
      }
    } else if (auto it = builds.find(shape); it != builds.end()) {
      // complex shape
      solution = {it->first, it->second};
      Shape shape1 = solution.build.shape1;
      Shape shape2 = solution.build.shape2;
      result += solution.toString() + "\n";
      if (shape1.value != 0) result += run(shape1, ++indent);
      if (shape2.value != 0) result += run(shape2, ++indent);
    } else {
      result = "Not found";
    }
    return result;
  }
};

}  // namespace Shapez

int main(int argc, char* argv[]) {
  if (argc != 3) {
    std::cout << "Usage: solve dump.bin shape" << std::endl;
    return 1;
  }

  Shapez::Solve solve{argv[1]};
  Shapez::Shape shape{argv[2]};
  std::cout << "Input: " << shape.toString() << std::endl;

  std::string result = solve.run(shape);
  std::cout << result;

  return 0;
}
