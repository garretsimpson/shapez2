#include <algorithm>
#include <deque>
#include <filesystem>
#include <format>
#include <iostream>
#include <optional>

#include "3ps/ska/bytell_hash_map.hpp"
#include "shapez.hpp"

namespace Shapez {

struct Ros {
  using T = Shape::T;
  // baseShapes - list of shapes used to build ROSes
  std::vector<Shape> baseShapes;
  // pinShapes - pin versions of baseShapes
  std::vector<Shape> pinShapes;
  // number of shapes found
  size_t count = 0;
  // all shapes
  ska::bytell_hash_set<Shape> allShapes;
  // working queue
  std::deque<Shape> queue;

  Ros(bool cros = false) {
    // initialize baseShapes with 2, 3, 4 part shapes
    Shape bowtie;
    for (size_t part = 0; part < Shape::PART; part += 2) {
      bowtie.set(0, part, Type::Shape);
    }
    baseShapes.push_back(bowtie);
    baseShapes.push_back(bowtie.rotate(1));
    for (size_t len = 2; len < Shape::PART; ++len) {
      Shape shape;
      for (size_t part = 0; part < len; ++part) {
        shape.set(0, part, Type::Shape);
      }
      for (size_t part = 0; part < Shape::PART; ++part) {
        baseShapes.push_back(shape.rotate(part));
      }
    }
    if (!cros) baseShapes.push_back(Shape(repeat<T>(T(Type::Shape), 2, Shape::PART)));

    // pin all the baseShapes
    for (Shape shape : baseShapes) {
      T empty = shape.find<Type::Empty>();
      T pins = ~empty & repeat<T>(T(Type::Pin), 2, Shape::PART);
      pinShapes.push_back(Shape((shape.value << (2 * Shape::PART)) | pins));
    }

    // std::cout << "Base shapes..." << std::endl;
    // for (Shape shape : baseShapes) {
    //   std::cout << shape.toString() << std::endl;
    // }
    // std::cout << "Pin shapes..." << std::endl;
    // for (Shape shape : pinShapes) {
    //   std::cout << shape.toString() << std::endl;
    // }
  }

  void enqueue(Shape shape) {
    if (allShapes.emplace(shape).second) {
      queue.push_back(shape);
    }
  }

  void process(Shape shape, bool cros = false) {
    // stack another shape without pins
    for (Shape top : baseShapes) {
      Shape newShape = shape.stack(top);
      enqueue(newShape);
      if (cros) {
        enqueue(newShape.crystalize());
      }
    }
    // stack another shape with pins
    for (Shape top : pinShapes) {
      Shape newShape = shape.stack(top);
      enqueue(newShape);
      if (cros) {
        enqueue(newShape.crystalize());
      }
    }
  }

  void run(bool cros = false) {
    std::cout << "Running..." << std::endl;

    // Add base shapes as first layer
    for (Shape shape : baseShapes) {
      if (cros) {
        shape = shape.crystalize();
      }
      queue.push_back(shape);
      allShapes.insert(shape);
    }
    while (!queue.empty()) {
      Shape shape = queue.front();
      queue.pop_front();
      process(shape, cros);
    }
  }

  void test() {
    Shape s0 = baseShapes[0];
    Shape s1 = pinShapes[0];
    Shape s = s0.stack(s1);
    std::cout << "0 " << s0.toString() << std::endl;
    std::cout << "1 " << s1.toString() << std::endl;
    std::cout << "R " << s.toString() << std::endl;
  }
};

}  // namespace Shapez

int main(int argc, char *argv[]) {
  static bool CROS = true;
  Shapez::Ros ros(CROS);

  std::cout << "ROS Finder" << std::endl;
  // ros.test();
  ros.run(CROS);

  std::vector<Shapez::Shape> shapes;
  shapes.insert(shapes.end(), ros.allShapes.begin(), ros.allShapes.end());
  std::sort(shapes.begin(), shapes.end());

  std::cout << "Shapes found: " << shapes.size() << std::endl;
  std::cout << "All shapes..." << std::endl;
  for (Shapez::Shape shape : shapes) {
    std::cout << shape.toString() << std::endl;
  }
}
