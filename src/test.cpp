#include <iostream>

#include "shapez.hpp"

namespace Shapez {

using T = Shape::T;

void testStack(Shape bot, Shape top, Shape exp) {
  Shape res = bot.stack(top);
  bool pass = res == exp;
  std::string passStr;
  if (pass)
    passStr = "PASS";
  else
    passStr = "FAIL";
  std::cout << std::format("{} Stack {} <- {} {}", passStr, res.toString(), bot.toString(), top.toString())
            << std::endl;
  if (res != exp) {
    std::cout << std::format("  Expected {}", exp.toString()) << std::endl;
  }
}

void test() {
  Shape empty = Shape(0);
  Shape oneLayer = Shape(repeat<T>(T(Type::Shape), 2, Shape::PART));
  std::vector<Shape> full(Shape::LAYER, Shape(0));
  for (int i = 1; i <= Shape::LAYER; ++i) {
    full[i] = Shape(repeat<T>(oneLayer.value, 2 * Shape::PART, i));
  }

  Shape bowtie;
  for (size_t part = 0; part < Shape::PART; part += 2) {
    bowtie.set(0, part, Type::Shape);
  }
  Shape bt0 = bowtie;
  Shape bt1 = bowtie.rotate(1);
  Shape btp0 = bt0.pin();
  Shape btp1 = bt1.pin();

#if CONFIG_LAYER == 4
  testStack(btp0, btp0, Shape("P-P-:S-S-:P-P-:S-S-"));
  testStack(btp0, btp1, Shape("PPPP:SSSS:----:----"));
  testStack(full[Shape::LAYER - 1], btp0, Shape("SSSS:SSSS:SSSS:P-P-"));
  testStack(Shape("S---:----:----:----"), Shape("PPPP:----:----:----"), Shape("SPPP:P---:----:----"));
#endif
}

}  // namespace Shapez

int main(int argc, char* argv[]) {
  Shapez::test();

  return 0;
}
