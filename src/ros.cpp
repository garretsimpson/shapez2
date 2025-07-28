#include <algorithm>
#include <format>
#include <iostream>

#include "3ps/ska/bytell_hash_map.hpp"
#include "shapez.hpp"

#ifndef CROS
#define CROS false
#endif

namespace Shapez {

struct Ros {
  using T = Shape::T;
  // baseShapes - list of shapes used to build ROSes
  std::vector<Shape> baseShapes;
  // pinShapes - pin versions of baseShapes
  std::vector<Shape> pinShapes;
  // newly found shapes
  std::vector<Shape> newShapes;
  // all shapes
  ska::bytell_hash_set<Shape> allShapes;

  Ros() {
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
    if (!CROS) baseShapes.push_back(Shape(repeat<T>(T(Type::Shape), 2, Shape::PART)));

    // pin all the baseShapes
    for (Shape shape : baseShapes) {
      pinShapes.push_back(shape.pin());
    }
  }

  void displayShapes() {
    std::cout << "Base shapes..." << std::endl;
    for (Shape shape : baseShapes) {
      std::cout << shape.toString() << std::endl;
    }
    std::cout << "Pin shapes..." << std::endl;
    for (Shape shape : pinShapes) {
      std::cout << shape.toString() << std::endl;
    }
  }

  void enqueue(Shape shape) {
    if (allShapes.emplace(shape).second) {
      newShapes.push_back(shape);
    }
  }

  void process(Shape shape) {
    // stack another shape without pins
    for (Shape top : baseShapes) {
      Shape newShape = shape.stack(top);
      enqueue(newShape);
      if (CROS) enqueue(newShape.crystalize());
    }
    // stack another shape with pins
    for (Shape top : pinShapes) {
      Shape newShape = shape.stack(top);
      enqueue(newShape);
      if (CROS) enqueue(newShape.crystalize());
    }
  }

  // - Make a list of initial shapes
  // - Add to newShapes and allShapes
  // - Repeat LAYER-1 times...
  //   - Copy newShapes to working queue
  //   - Clear newShapes
  //   - Process each shape in the queue
  //   - Add newly found shapes to newShapes and allShapes
  void run() {
    std::cout << "Running..." << std::endl;

    // Add base shapes as first layer
    for (Shape shape : baseShapes) {
      if (CROS) shape = shape.crystalize();
      allShapes.insert(shape);
      newShapes.push_back(shape);
    }

    for (size_t i = 1; i < Shape::LAYER; ++i) {
      std::vector<Shape> queue(newShapes);
      newShapes.clear();
      for (Shape shape : queue) {
        process(shape);
      }
    }
  }
};

}  // namespace Shapez

int main(int argc, char *argv[]) {
  Shapez::Ros ros;

  std::cout << "ROS Finder" << std::endl;
  // ros.test();
  ros.run();

  std::vector<Shapez::Shape> keyShapes;
  std::copy_if(ros.allShapes.begin(), ros.allShapes.end(), std::back_inserter(keyShapes),
               [](Shapez::Shape shape) { return shape.equivalentShapes()[0] == shape; });
  std::cout << std::format("{} total shapes", ros.allShapes.size()) << std::endl;
  std::cout << std::format("{} key shapes", keyShapes.size()) << std::endl;

  // Save shapes to data file
  if (argc >= 2) {
    Shapez::ShapeSet shapeSet;
    // shapeSet.shapes.insert(shapeSet.shapes.end(), ros.allShapes.begin(), ros.allShapes.end());
    shapeSet.shapes.insert(shapeSet.shapes.end(), keyShapes.begin(), keyShapes.end());
    std::sort(shapeSet.shapes.begin(), shapeSet.shapes.end());
    std::string filename = argv[1];
    shapeSet.save(filename);
    shapeSet.clear();
  }
  return 0;
}
