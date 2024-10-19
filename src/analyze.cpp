#include <iostream>

#include "3ps/ska/bytell_hash_map.hpp"
#include "shapez.hpp"

namespace Shapez {

ShapeSet set;
ska::bytell_hash_set<Shape> halves;

void init(const char* filename) {
  set = ShapeSet::load(filename);
  halves = {set.halves.begin(), set.halves.end()};
}

bool shapeFilter(Shape shape, size_t quad = 0) {
  std::optional<size_t> firstGap;
  std::optional<size_t> lastCrystal;
  std::optional<size_t> lastPart;
  Type type;
  for (size_t layer = 0; layer < Shape::LAYER; ++layer) {
    type = shape.get(layer, quad);
    if (!firstGap.has_value() && type == Type::Empty) firstGap = layer;
    if (type != Type::Empty) lastPart = layer;
    if (type == Type::Crystal) lastCrystal = layer;
  }
  bool hasGap = firstGap.has_value() && lastPart.has_value() && firstGap.value() < lastPart.value();
  bool hasCystalTop = lastCrystal.has_value() && lastPart.has_value() && lastCrystal.value() == lastPart.value();
  bool hasGapUnderCrystal =
      lastCrystal.has_value() && lastCrystal.value() > 0 && shape.get(lastCrystal.value() - 1, quad) == Type::Empty;
  // return hasGap && hasCystalTop;
  return hasGapUnderCrystal;
};

void findQuarters() {
  ska::bytell_hash_set<Shape> quarters;

  constexpr Shape::T mask = repeat<Shape::T>(3, 2 * Shape::PART, Shape::LAYER);
  Shape quarter;
  for (Shape half : halves) {
    for (size_t angle = 0; angle < Shape::PART / 2; ++angle) {
      quarter = half.rotate(angle) & mask;
      quarters.insert(quarter);
    }
  }

  auto toValue = [&](Shape shape) {
    size_t value = 0;
    for (size_t layer = 0; layer < Shape::LAYER; ++layer) {
      value += size_t(shape.get(layer, 0)) << (2 * layer);
    }
    return value;
  };

  auto toCode = [&](Shape shape) {
    std::string code = "";
    for (size_t layer = 0; layer < Shape::LAYER; ++layer) {
      code += toChar(shape.get(layer, 0));
    }
    return code;
  };

  std::vector<Shape> shapes;
  std::copy_if(quarters.begin(), quarters.end(), std::back_inserter(shapes),
               [&](Shape shape) { return shapeFilter(shape); });
  std::sort(shapes.begin(), shapes.end());

  std::cout << "Quarters: " << quarters.size() << std::endl;
  std::cout << "Found: " << shapes.size() << std::endl;
  for (Shape shape : shapes) {
    std::cout << std::format("{:4}  {}", toValue(shape), toCode(shape)) << std::endl;
  }
}

void findHalves() {
  std::vector<Shape> shapes0, shapes1;
  std::copy_if(halves.begin(), halves.end(), std::back_inserter(shapes0),
               [&](Shape shape) { return shapeFilter(shape, 0); });
  std::copy_if(shapes0.begin(), shapes0.end(), std::back_inserter(shapes1),
               [&](Shape shape) { return shapeFilter(shape, 1); });
  std::sort(shapes1.begin(), shapes1.end());

  std::cout << "Halves: " << halves.size() << std::endl;
  std::cout << "Found: " << shapes1.size() << std::endl;
  for (auto shape : shapes1) {
    std::cout << shape.toString() << std::endl;
  }
}

};  // namespace Shapez

int main(int argc, char* argv[]) {
  if (argc != 2) {
    std::cout << "Usage: " << argv[0] << " dump.bin" << std::endl;
    return 1;
  }
  Shapez::init(argv[1]);

  // Shapez::findQuarters();
  Shapez::findHalves();

  return 0;
}
