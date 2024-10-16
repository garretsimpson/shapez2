#include <algorithm>
#include <format>
#include <iostream>

#include "3ps/ska/bytell_hash_map.hpp"
#include "shapez.hpp"
#include "spu.hpp"

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

  void displayShapes(std::vector<Shape> shapes, size_t maxSize = 10) {
    size_t size = std::min(shapes.size(), maxSize);
    for (auto i = 0; i < size; ++i) {
      std::cout << shapes[i].toString() << std::endl;
    }
  }

  Spu::Solution solve(Shape goal) {
    std::cout << format("Goal: {}", goal.toString());

    // working stack
    std::vector<Shape> stack = {goal};

    Spu::Solution solution;
    solution.addOp(Spu::Op::Output);
    while (!stack.empty()) {
      Shape shape = stack.back();
      stack.pop_back();
      // std::cout << format("Shape: {}", shape.toString()) << std::endl;
      size_t layers = shape.layers();
      if (shape.value == 0 || layers == 1) {
        // if shape is one layer, push it
        solution.addShape(shape);
        continue;
      }
      Shape::T mask = 0;
      // extract top layer
      Shape top = shape.getLayer(layers - 1);
      T topCrystal = top.find<Type::Crystal>();
      // std::cout << std::format("topCrystal: {}", topCrystal) << std::endl;

      if (topCrystal == 0) {
        // no crystal on top
        solution.addOp(Spu::Op::Stack);
        solution.addShape(top);
        mask = top.value << (2 * PART * (layers - 1));
        shape = shape & ~mask;
        stack.push_back(shape);
      } else {
        solution.addShape(Shape(0));
      }
      // create new shape and put back on stack
      // Shape::T mask = repeat<Shape::T>(3, 2, Shape::PART * (layers - 1));
    }

    // std::cout << "Solution (before):" << std::endl;
    // std::cout << solution.toString() << std::endl;

    Spu spu;
    std::vector<Shape> output = spu.build(solution);
    bool pass = (output[0] == goal);
    if (pass)
      std::cout << " PASS";
    else
      std::cout << std::format(" FAIL {}", output[0].toString());
    std::cout << std::endl;

    return solution;
  }

  void run() {
    std::vector<Shape> todo;
    std::vector<Shape> knowns;
    std::vector<Shape> unknowns;

    // Make list of shapes to check
    todo = {halves.begin(), halves.end()};
    std::sort(todo.begin(), todo.end());
    std::cout << std::format("todo {}", todo.size()) << std::endl;

    Spu spu;
    for (Shape goalShape : todo) {
      Spu::Solution solution = solve(goalShape);
      Shape newShape = spu.build(solution)[0];
      if (newShape == goalShape) {
        knowns.push_back(goalShape);
      } else {
        unknowns.push_back(goalShape);
      }
    }

    std::cout << std::format("knowns {}, unknowns: {}", knowns.size(),
                             unknowns.size())
              << std::endl;
    displayShapes(unknowns);
  }
};

void testSpu() {
  Spu spu = Spu();
  Spu::Solution solution;

  Shape s1, s2;
  s1.set(0, 0, Type::Shape);
  s2.set(0, 1, Type::Shape);
  solution.addShape(s1);
  solution.addShape(s2);
  solution.addOp(Spu::Op::Input);
  solution.addOp(Spu::Op::Input);
  solution.addOp(Spu::Op::Stack);
  solution.addOp(Spu::Op::Output);

  std::cout << solution.toString() << std::endl;

  spu.build(solution);

  std::cout << solution.toString() << std::endl;
}

}  // namespace Shapez

int main(int argc, char* argv[]) {
  if (argc != 2) {
    std::cout << "Usage: solver dump.bin" << std::endl;
    return 1;
  }
  char* filename = argv[1];

  Shapez::Solver solver(filename);
  // solver.verifyShapes();
  solver.run();

  std::cout << "DONE" << std::endl;
  return 0;
}
