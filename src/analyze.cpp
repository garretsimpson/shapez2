#include <iostream>
#include <vector>

#include "3ps/ska/bytell_hash_map.hpp"
#include "shapez.hpp"

namespace Shapez {

ska::bytell_hash_set<Shape> halves;
ska::bytell_hash_set<Shape> shapes;

void init(const char* filename) {
  std::cout << "Loading file: " << filename << std::endl;
  ShapeSet set = ShapeSet::load(filename);
  halves = {set.halves.begin(), set.halves.end()};
  shapes = {set.shapes.begin(), set.shapes.end()};
  set.clear();
  std::cout << std::format("halves {}, fulls {}, total {}", halves.size(), shapes.size(), halves.size() + shapes.size())
            << std::endl;
}

// Whether a shape can be constructed by swapping two halves.
bool swappable(Shape shape) {
  constexpr Shape::T mask = repeat<Shape::T>(repeat<Shape::T>(3, 2, Shape::PART / 2), 2 * Shape::PART, Shape::LAYER);
  for (size_t angle = 0; angle < Shape::PART / 2; ++angle) {
    Shape left{shape.rotate(angle).value & mask};
    Shape right{shape.rotate(angle + Shape::PART / 2).value & mask};
    // std::cout << left.toString() << std::endl;
    // std::cout << right.toString() << std::endl;
    // TODO: Use shape.collapse() instead of looking in the halves list
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

bool hasDrop(Shape shape, size_t quad = 0) {
  for (size_t layer = 1; layer < Shape::LAYER; ++layer) {
    if (shape.get(layer - 1, quad) == Type::Empty && shape.get(layer, quad) == Type::Crystal) return true;
  }
  return false;
};

bool hasGapUnderCrystalTop(Shape shape, size_t quad = 0) {
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
  return hasGap && hasCystalTop;
};

void findQuarters() {
  ska::bytell_hash_set<Shape> quarters;

  constexpr Shape::T mask = repeat<Shape::T>(3, 2 * Shape::PART, Shape::LAYER);
  Shape quarter;
  for (Shape shape : halves) {
    for (size_t angle = 0; angle < Shape::PART; ++angle) {
      quarter = shape.rotate(angle) & mask;
      quarters.insert(quarter);
    }
  }

  [[maybe_unused]]
  auto toValue = [](Shape shape) {
    size_t value = 0;
    for (size_t layer = 0; layer < Shape::LAYER; ++layer) {
      value += size_t(shape.get(layer, 0)) << (2 * layer);
    }
    return value;
  };

  auto toCode = [](Shape shape) {
    std::string code = "";
    for (size_t layer = 0; layer < Shape::LAYER; ++layer) {
      code += toChar(shape.get(layer, 0));
    }
    return code;
  };

  auto toShape = [](size_t value) {
    Shape::T corner = 0;
    for (size_t layer = 0; layer < Shape::LAYER; ++layer) {
      corner += (value & 3) << (2 * layer * Shape::PART);
      value >>= 2;
    }
    return Shape(corner);
  };

  [[maybe_unused]]
  auto pinOverGap1 = [](Shape shape) {
    std::optional<size_t> firstGap;
    std::optional<size_t> lastPin;
    Type type;
    for (size_t layer = 0; layer < Shape::LAYER; ++layer) {
      type = shape.get(layer, 0);
      if (!firstGap.has_value() && type == Type::Empty) firstGap = layer;
      if (type == Type::Pin) lastPin = layer;
    }
    bool found = firstGap.has_value() && lastPin.has_value() && (firstGap.value() == lastPin.value() - 1);
    return found;
  };

  // Pin over gap
  auto pinOverGap2 = [](size_t value) {
    // xPGx
    const size_t POG = (size_t(Type::Pin) << 2) + size_t(Type::Empty);
    const size_t MASK = 0xf;
    for (size_t layer = 0; layer < Shape::LAYER - 1; ++layer) {
      if ((MASK & value) == POG) return true;
      value >>= 2;
    }
    return false;
  };

  // Crystal with no base - unsupported
  auto UnCrystal = [](size_t value) {
    // xCG<not solid>
    const size_t UNC2 = (size_t(Type::Crystal) << 2) + size_t(Type::Empty);
    const size_t MASK2 = 0xf;
    if ((MASK2 & value) == UNC2) return true;

    const size_t UNC3 = UNC2 << 2;
    const size_t MASK3 = 0x3f;
    if ((MASK3 & value) == (UNC3 + size_t(Type::Empty))) return true;
    if ((MASK3 & value) == (UNC3 + size_t(Type::Pin))) return true;
    if ((MASK3 & value) == (UNC3 + size_t(Type::Crystal))) return true;

    const size_t UNC4 = UNC3 << 2;
    const size_t MASK4 = 0xff;
    if ((MASK4 & value) == (UNC4 + size_t(Type::Empty))) return true;
    if ((MASK4 & value) == (UNC4 + size_t(Type::Pin))) return true;
    if ((MASK4 & value) == (UNC4 + size_t(Type::Crystal))) return true;

    const size_t MASK42 = 0xfc;
    // TODO: This could be an extension of UNC3.
    if ((MASK42 & value) == (UNC4 + (size_t(Type::Pin) << 2))) return true;
    if ((MASK42 & value) == (UNC4 + (size_t(Type::Crystal) << 2))) return true;

    const size_t UNC5 = UNC4 << 2;
    const size_t MASK5 = 0x3ff;
    if ((MASK5 & value) == (UNC5 + size_t(Type::Empty))) return true;
    if ((MASK5 & value) == (UNC5 + size_t(Type::Pin))) return true;
    if ((MASK5 & value) == (UNC5 + size_t(Type::Crystal))) return true;

    return false;
  };

  // Crystal over pins
  auto CryOnPins = [](size_t value) {
    // xCPx<not pin>
    const size_t COP3 = (size_t(Type::Crystal) << 4) + (size_t(Type::Pin) << 2);
    const size_t MASK3 = 0x3f;
    if ((MASK3 & value) == (COP3 + size_t(Type::Shape))) return true;
    if ((MASK3 & value) == (COP3 + size_t(Type::Crystal))) return true;

    const size_t COP4 = COP3 << 2;
    const size_t MASK4 = 0xff;
    if ((MASK4 & value) == (COP4 + (size_t(Type::Pin) << 2) + size_t(Type::Shape))) return true;
    if ((MASK4 & value) == (COP4 + (size_t(Type::Pin) << 2) + size_t(Type::Crystal))) return true;

    const size_t MASK42 = 0xfc;
    if ((MASK42 & value) == (COP4 + (size_t(Type::Shape) << 2))) return true;
    if ((MASK42 & value) == (COP4 + (size_t(Type::Crystal) << 2))) return true;

    // xCxP<not pin>
    const size_t IMP1 =
        (size_t(Type::Crystal) << 6) + (size_t(Type::Shape) << 4) + (size_t(Type::Pin) << 2) + size_t(Type::Shape);
    const size_t IMP2 =
        (size_t(Type::Crystal) << 6) + (size_t(Type::Shape) << 4) + (size_t(Type::Pin) << 2) + size_t(Type::Crystal);
    if ((MASK4 & value) == IMP1) return true;
    if ((MASK4 & value) == IMP2) return true;

    return false;
  };

  // Impossible drop/break with crystal on bottom
  auto ImpDrop = [](size_t value) {
    // CGSCx
    // CSGCx
    // CxSxC maybe?
    const size_t IMP1 =
        (size_t(Type::Crystal) << 6) + (size_t(Type::Empty) << 4) + (size_t(Type::Shape) << 2) + size_t(Type::Crystal);
    const size_t IMP2 =
        (size_t(Type::Crystal) << 6) + (size_t(Type::Shape) << 4) + (size_t(Type::Empty) << 2) + size_t(Type::Crystal);
    const size_t MASK4 = 0xff;
    if ((MASK4 & value) == IMP1) return true;
    if ((MASK4 & value) == IMP2) return true;
    return false;
  };

  auto possStr = [](bool poss) {
    if (poss == true)
      return "pos";
    else
      return "imp";
  };

  CryOnPins(54);

  const std::string BAD = "BAD";
  const std::string MISS = "MISS";
  size_t total = std::pow(4, Shape::LAYER);
  std::cout << "Total quarters: " << total << std::endl;
  size_t bad = 0;
  size_t miss = 0;
  for (size_t i = 0; i < total; ++i) {
    Shape shape = toShape(i);
    bool possible = quarters.find(shape) != quarters.end();
    bool test1 = pinOverGap2(i);
    bool test2 = UnCrystal(i);
    bool test3 = CryOnPins(i);
    bool test4 = ImpDrop(i);
    bool test = test1 || test2 || test3 || test4;
    std::string result = "";
    if (possible && test) {
      result = BAD;
      bad++;
    }
    if (!possible && !test) {
      result = MISS;
      miss++;
    }
    std::cout << std::format("{:4}  {:3}  {:5}", i, possStr(possible), toCode(shape), result) << std::endl;
    // std::cout << std::format("{:4}  {:3}  {:5}  {:5}  {:5}  {:5}  {:5}  {:4}", i, possStr(possible), toCode(shape),
    //                          test1, test2, test3, test4, result)
    //           << std::endl;
  }
  // std::cout << std::format("BAD: {}  MISS: {}", bad, miss) << std::endl;

  /*
    std::vector<Shape> gapShapes;
    std::vector<Shape> dropShapes;
    // std::copy_if(quarters.begin(), quarters.end(), std::back_inserter(shapes),
    //              [&](Shape shape) { return hasGapUnderCrystalTop(shape); });

    for (auto shape : quarters) {
      if (!hasGapUnderCrystalTop(shape)) continue;
      if (!hasDrop(shape))
        gapShapes.insert(gapShapes.end(), shape);
      else
        dropShapes.insert(dropShapes.end(), shape);
    }

    std::sort(gapShapes.begin(), gapShapes.end());
    std::sort(dropShapes.begin(), dropShapes.end());

    std::cout << "Quarters: " << quarters.size() << std::endl;
    std::cout << "Gap Shapes: " << gapShapes.size() << std::endl;
    for (Shape shape : gapShapes) {
      std::cout << std::format("{:4}  {}", toValue(shape), toCode(shape)) << std::endl;
    }
    std::cout << "Drop Shapes: " << dropShapes.size() << std::endl;
    for (Shape shape : dropShapes) {
      std::cout << std::format("{:4}  {}", toValue(shape), toCode(shape)) << std::endl;
    }
  */
}

// Find halves that have drops on both quarters
void findHalves() {
  std::vector<Shape> shapes0, shapes1;
  std::copy_if(halves.begin(), halves.end(), std::back_inserter(shapes0),
               [&](Shape shape) { return hasDrop(shape, 0); });
  std::copy_if(shapes0.begin(), shapes0.end(), std::back_inserter(shapes1),
               [&](Shape shape) { return hasDrop(shape, 1); });
  std::sort(shapes1.begin(), shapes1.end());

  std::cout << "Halves: " << halves.size() << std::endl;
  std::cout << "Found: " << shapes1.size() << std::endl;
  for (auto shape : shapes1) {
    std::cout << shape.toString() << std::endl;
  }
}

void countShapes() {
  ska::bytell_hash_map<size_t, size_t> distro;

  const size_t MAX_COST = 2 * Shape::LAYER * Shape::PART;
  for (size_t i = 0; i <= MAX_COST; ++i) distro[i] = 0;

  for (Shape shape : shapes) {
    size_t cost = shape.bitCount();
    distro[cost]++;
  }

  for (size_t i = 0; i <= MAX_COST; ++i) {
    std::cout << std::format("{} {}", i, distro[i]) << std::endl;
  }
}

// count number of shapes that have 1,2,3,4 solids/pins on each layer
// Example: how many shapes have less than 2 solids on layer number 2?
void analyzeRos() {
  // First get layer counts
  std::vector<int> layerCount(Shape::LAYER + 1, 0);
  for (Shape shape : shapes) {
    layerCount[shape.layers()]++;
  }
  std::cout << std::format("Layer counts...") << std::endl;
  for (size_t i = 0; i < layerCount.size(); ++i) {
    std::cout << std::format("{}  {:7}", i, layerCount[i]) << std::endl;
  }

  std::cout << std::format("{:7} total shapes", shapes.size()) << std::endl;

  // Find all key shapes
  std::vector<Shape> keyShapes;
  std::copy_if(shapes.begin(), shapes.end(), std::back_inserter(keyShapes),
               [](Shape shape) { return shape.equivalentShapes()[0] == shape; });
  std::cout << std::format("{:7} key shapes", keyShapes.size()) << std::endl;
  std::sort(keyShapes.begin(), keyShapes.end());
  for (Shape shape : keyShapes) {
    std::cout << shape.toString() << std::endl;
  }

  // Find all 5-layer shapes
  std::vector<Shape> shapes5;
  std::copy_if(shapes.begin(), shapes.end(), std::back_inserter(shapes5),
               [](Shape shape) { return shape.layers() == 5; });
  std::cout << std::format("{:7} 5 layer shapes", shapes5.size()) << std::endl;

  // The results table is table[layerNum][partType][numParts]
  std::vector<std::vector<std::vector<int>>> table(
      Shape::LAYER + 1, std::vector<std::vector<int>>(4, std::vector<int>(Shape::PART + 1, 0)));
  for (Shape shape : shapes5) {
    // int numLayers = shape.layers();
    for (size_t layerNum = 0; layerNum < Shape::LAYER; ++layerNum) {
      std::vector<int> partCounts(4, 0);
      for (size_t partNum = 0; partNum < Shape::PART; ++partNum) {
        int partType = (int)shape.get(layerNum, partNum);
        partCounts[partType]++;
      }
      for (int partType = 0; partType < 4; ++partType) {
        table[layerNum][partType][partCounts[partType]]++;
      }
    }
  }

  // Display table of all results
  for (size_t layerNum = 0; layerNum < Shape::LAYER; ++layerNum) {
    std::cout << std::format("Layer {}", layerNum) << std::endl;
    std::cout << std::format(" {:9}{:9}{:9}{:9}{:9}", 0, 1, 2, 3, 4) << std::endl;
    for (int partType = 0; partType < 4; ++partType) {
      std::cout << std::format("{}", toChar((Shapez::Type)partType));
      for (size_t numParts = 0; numParts <= Shape::PART; ++numParts) {
        int value = table[layerNum][partType][numParts];
        std::cout << std::format("{:9}", value);
      }
      std::cout << std::endl;
    }
    std::cout << std::endl;
  }

  // Display percentage of total shapes
  int numShapes = shapes5.size();
  for (size_t layerNum = 0; layerNum < Shape::LAYER; ++layerNum) {
    std::cout << std::format("Layer {}", layerNum + 1) << std::endl;
    std::cout << std::format("{:5}{:5}{:5}{:5}{:5}", 0, 1, 2, 3, 4) << std::endl;
    for (int partType = 0; partType < 4; ++partType) {
      std::cout << std::format("{}", toChar((Shapez::Type)partType));
      int sum = 0;
      for (size_t numParts = 0; numParts <= Shape::PART; ++numParts) {
        int value = table[layerNum][partType][numParts];
        sum += value;
        int percent = (int)(((float)sum / numShapes) * 100.0);
        std::cout << std::format("{:5}", percent);
      }
      std::cout << std::endl;
    }
    std::cout << std::endl;
  }
}

}  // namespace Shapez

int main(int argc, char* argv[]) {
  if (argc != 2) {
    std::cout << "Usage: " << argv[0] << " dump.bin" << std::endl;
    return 1;
  }
  Shapez::init(argv[1]);

  // Shapez::findQuarters();
  // Shapez::findHalves();
  // Shapez::countShapes();
  Shapez::analyzeRos();

  return 0;
}
