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

  ska::bytell_hash_set<Shape> allShapes;

  // working stack
  std::vector<Shape> stack;

  Solver(char* filename) {
    ShapeSet set = ShapeSet::load(filename);
    findAllShapes(set);
    set.clear();
  }

  void testFlip() {
    Shape shape("PS--:----:----:----");
    std::cout << shape.toString() << std::endl;
    shape = shape.flip();
    shape = shape.rotate(2);
    std::cout << shape.toString() << std::endl;
  }

  void findAllShapes(ShapeSet& set) {
    std::cout << "Find all shapes..." << std::endl;
    std::vector<Shape> shapes;
    for (Shape shape : set.halves) {
      if (shape.value == 0) continue;
      shapes = shape.equivalentShapes();
      allShapes.insert(shapes.begin(), shapes.end());
    }
    std::cout << allShapes.size() << std::endl;
    for (Shape shape1 : set.halves) {
      for (Shape shape2 : set.halves) {
        if (shape1.value == 0 || shape2.value == 0) continue;
        shapes = (shape1 | shape2.rotate(2)).equivalentShapes();
        // std::cout << "rotate" << std::endl;
        // displayShapes(shapes);
        allShapes.insert(shapes.begin(), shapes.end());
        shapes = (shape1 | shape2.flip()).equivalentShapes();
        // std::cout << "flip" << std::endl;
        // displayShapes(shapes);
        allShapes.insert(shapes.begin(), shapes.end());
      }
      std::cout << allShapes.size() << std::endl;
    }
    for (Shape shape : set.shapes) {
      shapes = shape.equivalentShapes();
      allShapes.insert(shapes.begin(), shapes.end());
    }
    std::cout << std::format("Found {} shapes...", allShapes.size()) << std::endl;
  }

  void clear() { stack.clear(); }

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

  Shape removeCrystals(Shape shape) {
    shape.value &= ~shape.find<Type::Crystal>();
    return shape;
  }

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
  bool solveStack(Spu::Solution& solution) {
    Shape goalShape = stack.back();
    stack.pop_back();
    // std::cout << format("Goal: {}", shape.toString());
    if (hasCrystal(goalShape)) {
      return false;
    }
    size_t numLayers = goalShape.layers();
    if (numLayers == 0) {
      return false;
    }
    while (numLayers > 1) {
      solution.addOp(Spu::Op::Stack);
      solution.addShape(goalShape.getLayer(--numLayers));
    }
    solution.addShape(goalShape.getLayer(--numLayers));
    return true;
  }

  // Attempt to solve using CROS method...
  // For each layer:
  // - construct a 1-layer of solids and pins
  // - stack it on the existing shape
  // - optionally crystalize the whole shape
  bool solveCROS(Spu::Solution& solution) {
    Shape goalShape = stack.back();
    stack.pop_back();
    // std::cout << format("Goal: {}", shape.toString());
    size_t numLayers = goalShape.layers();
    if (numLayers == 0) {
      return false;
    }
    Shape shape;
    while (numLayers > 1) {
      shape = goalShape.getLayer(--numLayers);
      if (hasCrystal(shape)) {
        shape = removeCrystals(shape);
        solution.addOp(Spu::Op::Crystal);
      }
      solution.addOp(Spu::Op::Stack);
      solution.addShape(shape);
    }
    shape = goalShape.getLayer(--numLayers);
    if (hasCrystal(shape)) {
      shape = removeCrystals(shape);
      solution.addOp(Spu::Op::Crystal);
    }
    solution.addShape(shape);
    return true;
  }

  // This solver only handles half-shapes (for now).
  // The West half is empty.
  bool solveHalf(Spu::Solution& solution) {
    Shape goalShape = stack.back();
    stack.pop_back();
    std::cout << format("Goal: {}", goalShape.toString());

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
        return false;
      }
    }

    return true;
  }

  bool tryBuild(const Spu::Solution& solution, const Shape goalShape) {
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
    bool pass = (output[0] == goalShape);
    // if (pass)
    //   std::cout << " PASS";
    // else
    //   std::cout << std::format(" FAIL {}", output[0].toString());
    // std::cout << std::endl;

    return pass;
  }

  size_t countShapes(const std::vector<Shape> shapes) {
    size_t result = 0;
    for (auto shape : shapes) {
      result += shape.equivalentShapes().size();
    }
    return result;
  }

  bool solvePass(Spu::Solution& solution) {
    Shape goalShape = stack.back();
    stack.pop_back();
    solution.addShape(goalShape);
    return true;
  }

  void run() {
    std::cout << "Solving shapes..." << std::endl;

    std::vector<Shape> knowns;
    std::vector<Shape> unknowns;
    Spu::Solution solution;
    for (const Shape goalShape : allShapes) {
      solution.addOp(Spu::Op::Output);
      stack.push_back(goalShape);
      if (solveCROS(solution) && tryBuild(solution, goalShape)) {
        knowns.push_back(goalShape);
      } else {
        unknowns.push_back(goalShape);
      }
      solution.clear();
      stack.clear();
    }

    std::cout << std::format("all {}, knowns {}, unknowns {}", allShapes.size(), knowns.size(), unknowns.size())
              << std::endl;
    std::cout << "Knowns" << std::endl;
    std::sort(knowns.begin(), knowns.end());
    displayShapes(knowns);
    std::cout << "Unknowns" << std::endl;
    std::sort(unknowns.begin(), unknowns.end());
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
  Shape goalShape{"SSSS:SSSS:SSSS:cScS"};
  solver.stack.push_back(goalShape);
  std::cout << "Goal shape: " << goalShape.toString() << std::endl;

  Spu::Solution solution;
  solution.addOp(Spu::Op::Output);
  bool found = solver.solveCROS(solution);
  if (!found) {
    std::cout << "Unable to find solution" << std::endl;
    return;
  }
  std::cout << "Found solution:" << std::endl;
  std::cout << solution.toString() << std::endl;

  bool passed = solver.tryBuild(solution, goalShape);
  if (passed) {
    std::cout << "Build PASSED" << std::endl;
  } else {
    std::cout << "Build FAILED" << std::endl;
  }
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
