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

  ska::bytell_hash_set<Shape> halves;
  ska::bytell_hash_set<Shape> shapes;

  Solver(char* filename) {
    ShapeSet set = ShapeSet::load(filename);
    halves = {set.halves.begin(), set.halves.end()};
    shapes = {set.shapes.begin(), set.shapes.end()};
    set.clear();
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
      if (halves.find(left) != halves.end() && halves.find(right) != halves.end()) {
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

    num = shapes.size();
    std::cout << "Shapes: " << num << std::endl;
    for (Shape shape : shapes) {
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
    for (int i = 0; i < size; ++i) {
      std::cout << shapes[i].toString() << std::endl;
    }
  }

  bool oneLayerPin(Shape shape) {
    T mask = repeat<T>(T(Type::Pin), 2, PART);
    return (shape.value & mask) == shape.value;
  }

  bool oneLayerSolid(Shape shape) {
    T mask = repeat<T>(T(Type::Shape), 2, PART);
    return (shape.value & mask) == shape.value;
  }

  bool hasCrystal(Shape shape) { return shape.find<Type::Crystal>() != 0; }

  bool oneCrystalOnTop(Shape shape) {
    Shape testShape;
    testShape.set(0, 0, Type::Crystal);
    return shape.getLayer(shape.layers() - 1) == testShape;
  }

  bool twoCrystal(Shape shape) { return shape.value == repeat<T>(T(Type::Crystal), 2, 2); }

  bool twoParts(Shape shape) { return (shape.get(0, 0) != Type::Empty) && (shape.get(0, 1) != Type::Empty); }

  bool topCrystal(Shape shape) {
    Shape top = shape.getLayer(shape.layers() - 1);
    return top.find<Type::Crystal>() != 0;
  }

  // simple pin push (no breaking)
  bool simplePin(Shape shape) {
    T empty = shape.getLayer(1).find<Type::Empty>();
    T pins = ~empty & repeat<T>(T(Type::Pin), 2, PART);
    return shape.getLayer(0).value == pins;
  }

  // Assumes shape is a simple stack and no crystals
  bool solveStack(Spu::Solution& solution, Shape goal) {
    // std::cout << format("Goal: {}", goal.toString());
    if (hasCrystal(goal)) {
      return false;
    }
    size_t numLayers = goal.layers();
    if (numLayers == 0) {
      return true;
    }
    solution.addOp(Spu::Op::Output);
    while (numLayers > 1) {
      solution.addOp(Spu::Op::Stack);
      solution.addShape(goal.getLayer(--numLayers));
    }
    solution.addShape(goal.getLayer(--numLayers));
    return true;
  }

  // This solver only handles half-shapes (for now).
  // The West half is empty.
  Spu::Solution solveHalf(const Shape goal) {
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
      if (shape.value == 0) {
        // null shape
        solution.addShape(shape);
      } else if (layers == 1) {
        // one layer shapes
        if (twoParts(shape)) {
          // split into two parts, and swap them together
          solution.addOp(Spu::Op::Rotate3);
          solution.addOp(Spu::Op::Trash);  // Trash anything in the second output
          solution.addOp(Spu::Op::Swap);
          solution.addOp(Spu::Op::Rotate1);
          Shape shapeOne, shapeTwo;
          shapeOne.set(0, 0, shape.get(0, 0));
          shapeTwo.set(0, 0, shape.get(0, 1));
          stack.push_back(shapeTwo);
          stack.push_back(shapeOne);
        } else if (hasCrystal(shape)) {
          // make one crystal: import a mold, make crystal, then
          // cut>rotate>cut
          solution.addOp(Spu::Op::DestroyWest);
          solution.addOp(Spu::Op::Rotate1);
          solution.addOp(Spu::Op::DestroyWest);
          solution.addOp(Spu::Op::Crystal);
          // TODO: make the mold at the crystal location
          Shape mold(repeat<T>(T(Type::Shape), 2, PART));
          mold.set(0, 1, Type::Empty);
          solution.addShape(mold);
        } else {
          // one part, not crystal
          solution.addShape(shape);
        }
      } else if (simplePin(shape)) {
        solution.addOp(Spu::Op::PinPush);
        shape = Shape(shape.value >> PART * 2);
        stack.push_back(shape);
      } else if (!topCrystal(shape)) {
        // simple stack
        solution.addOp(Spu::Op::Stack);
        T mask = repeat<T>(3, 2, PART) << (2 * PART * (layers - 1));
        Shape top = Shape(shape.value >> (2 * PART * (layers - 1)));
        Shape bottom = shape & ~mask;
        solution.addShape(top);
        stack.push_back(bottom);
        // TODO: quad swapping
        // } else if (oneCrystalOnTop(shape)) {
        //   Top and bottom need molds
        //   // TODO: make the mold at the crystal location
        //   Shape mold(repeat<T>(T(Type::Shape), 2, PART));
        //   mold.set(0, 1, Type::Empty);
        //   solution.addShape(mold);
        //   solution.addOp(Spu::Op::Crystal);
        //   T mask = repeat<T>(3, 2, PART) << (2 * PART * (layers - 1));
        //   Shape bottom = shape & ~mask;
        //   stack.push_back(bottom);
      } else {
        solution.addShape(Shape(0));
      }
    }

    return solution;
  }

  bool tryBuild(Spu::Solution solution, const Shape goal) {
    if (solution.ops.size() == 0) {
      return false;
    }
    // std::cout << std::endl << solution.toString() << std::endl;
    Spu spu;
    std::vector<Shape> output = spu.build(solution);
    if (output.size() != 1) {
      std::cout << "Invalid build output" << std::endl;
      return false;
    }
    bool pass = (output[0] == goal);
    // if (pass)
    //   std::cout << " PASS";
    // else
    //   std::cout << std::format(" FAIL {}", output[0].toString());
    // std::cout << std::endl;

    return pass;
  }

  Spu::Solution solve(Shape goal) {
    Spu::Solution solution;
    return solution;
  }

  void run() {
    std::vector<Shape> todo;
    std::vector<Shape> knowns;
    std::vector<Shape> unknowns;

    // Make list of shapes to check
    todo = {halves.begin(), halves.end()};
    todo.insert(todo.end(), shapes.begin(), shapes.end());
    std::sort(todo.begin(), todo.end());
    // Shape testShape{"SSSS:SSSS:SSSS:PPPP"};
    // todo.push_back(Shape());
    // todo.push_back(testShape);
    std::cout << std::format("todo {}", todo.size()) << std::endl;

    Spu::Solution solution;
    for (const Shape goalShape : todo) {
      bool found = solveStack(solution, goalShape);
      if (tryBuild(solution, goalShape)) {
        knowns.push_back(goalShape);
      } else {
        unknowns.push_back(goalShape);
      }
      solution.clear();
    }

    std::cout << std::format("knowns {}, unknowns: {}", knowns.size(), unknowns.size()) << std::endl;
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

void test(Solver solver) {
  Shape testShape{"SSSS:SSSS:SSSS:PPPP"};
  Spu::Solution solution;
  bool result = solver.solveStack(solution, testShape);
  std::cout << solution.toString() << std::endl;
  result = solver.tryBuild(solution, testShape);
}

}  // namespace Shapez

int main(int argc, char* argv[]) {
  if (argc != 2) {
    std::cout << "Usage: solver dump.bin" << std::endl;
    return 1;
  }
  char* filename = argv[1];

  Shapez::Solver solver(filename);
  // Shapez::test(solver);
  solver.run();

  std::cout << "DONE" << std::endl;
  return 0;
}
