#include <iostream>

#include "3ps/ska/bytell_hash_map.hpp"
#include "shapez.hpp"

int main(int argc, char* argv[]) {
  using namespace Shapez;

  if (argc != 2) {
    std::cout << "Usage: " << argv[0] << "dump.bin" << std::endl;
    return 1;
  }

  ShapeSet set = ShapeSet::load(argv[1]);

  ska::bytell_hash_set<Shape> halves;
  ska::bytell_hash_set<Shape> quarters;

  halves = {set.halves.begin(), set.halves.end()};

  constexpr Shape::T mask = repeat<Shape::T>(3, 2 * Shape::PART, Shape::LAYER);
  Shape quarter;
  for (Shape half : halves) {
    for (size_t angle = 0; angle < Shape::PART / 2; ++angle) {
      quarter = half.rotate(angle) & mask;
      quarters.insert(quarter);
    }
  }

  auto shapeFilter = [&](Shape shape) {
    std::optional<size_t> firstGap;
    std::optional<size_t> lastCrystal;
    std::optional<size_t> lastPart;
    Type type;
    for (size_t layer = 0; layer < Shape::LAYER; ++layer) {
      type = shape.get(layer, 0);
      if (!firstGap.has_value() && type == Type::Empty) firstGap = layer;
      if (type != Type::Empty) lastPart = layer;
      if (type == Type::Crystal) lastCrystal = layer;
    }
    // std::cout << shape.toString() << std::endl;
    // if (firstGap.has_value()) std::cout << "fg" << firstGap.value() << " ";
    // if (lastCrystal.has_value())
    //   std::cout << "lc" << lastCrystal.value() << " ";
    // if (lastPart.has_value()) std::cout << "lp" << lastPart.value();
    // std::cout << std::endl;

    bool hasGap = firstGap.has_value() && lastPart.has_value() &&
                  firstGap.value() < lastPart.value();
    bool hasCystalTop = lastCrystal.has_value() && lastPart.has_value() &&
                        lastCrystal.value() == lastPart.value();
    return hasGap && hasCystalTop;
  };

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
               shapeFilter);
  std::sort(shapes.begin(), shapes.end());

  std::cout << "Quarters: " << quarters.size() << std::endl;
  std::cout << "Found: " << shapes.size() << std::endl;
  for (Shape shape : shapes) {
    std::cout << std::format("{:4}  {}", toValue(shape), toCode(shape))
              << std::endl;
  }

  return 0;
}
