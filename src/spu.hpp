#include <vector>

namespace Shapez {

struct Spu {
  enum class Op : char {
    Input,
    Output,
    Trash,
    Rotate1,
    Rotate2,
    Rotate3,
    Cut,
    DestroyWest,
    Stack,
    Swap,
    Crystal,
    PinPush,
  };

  static inline char toChar(Op op) {
    switch (op) {
      case Op::Input:
        return 'I';
      case Op::Output:
        return 'O';
      case Op::Trash:
        return 'T';
      case Op::Rotate1:
        return 'L';
      case Op::Rotate2:
        return 'U';
      case Op::Rotate3:
        return 'R';
      case Op::Cut:
        return 'C';
      case Op::DestroyWest:
        return 'D';
      case Op::Stack:
        return 'S';
      case Op::Swap:
        return 'W';
      case Op::PinPush:
        return 'P';
      case Op::Crystal:
        return 'X';
    }
    std::unreachable();
  }

  struct Solution {
    std::vector<Op> ops;
    std::vector<Shape> input;

    void clear() {
      ops.clear();
      input.clear();
    }

    void addOp(Op op) {
      // std::cout << toChar(op) << std::endl;
      ops.push_back(op);
    };

    void addShape(Shape shape) {
      addOp(Op::Input);
      input.push_back(shape);
    };

    std::string toString() {
      std::string result = "";
      std::string code = "";
      for (Op op : ops) {
        code = toChar(op) + code;
      }
      result += std::format("code: {}\n", code);
      result += "input:\n";
      for (Shape shape : input) {
        result += shape.toString() + "\n";
      }
      return result;
    }
  };

  std::vector<Shape> build(Solution solution) {
    std::vector<Shape> result;
    std::vector<Shape> stack;

    std::vector<Op> ops = solution.ops;
    while (!ops.empty()) {
      Op op = ops.back();
      ops.pop_back();
      // std::cout << std::format("Op: {}", toChar(op)) << std::endl;

      Shape shape;
      std::pair<Shape, Shape> shapes;
      if (op == Op::Input) {
        if (solution.input.size() < 1) {
          std::cerr << "ERROR: Input requires at least one shape" << std::endl;
          return result;
        }
        shape = solution.input.back();
        solution.input.pop_back();
        stack.push_back(shape);
        // std::cout << std::format("Shape: {}", shape.toString()) << std::endl;
        continue;
      }
      if (stack.size() < 1) {
        std::cerr << "ERROR: Operation requires at least one shape"
                  << std::endl;
        return result;
      }
      shape = stack.back();
      stack.pop_back();
      if (op == Op::Trash) {
        // do nothing
        std::cout << std::format("Trash: {}", shape.toString()) << std::endl;
        continue;
      }
      if (op == Op::Output) {
        result.push_back(shape);
        continue;
      }
      Shape top, bottom, other;
      std::pair<Shape, Shape> pair;
      switch (op) {
        case Op::Rotate1:
          shape = shape.rotate(1);
          break;
        case Op::Rotate2:
          shape = shape.rotate(2);
          break;
        case Op::Rotate3:
          shape = shape.rotate(3);
          break;
        case Op::Cut:
          shapes = shape.cut();
          stack.push_back(shapes.first);
          shape = shapes.second;
          break;
        case Op::DestroyWest:
          shape = shape.destroyHalf();
          break;
        case Op::Stack:
          if (stack.size() < 1) {
            std::cerr << "ERROR: Stack requires two shapes" << std::endl;
            return result;
          }
          top = shape;
          bottom = stack.back();
          stack.pop_back();
          shape = bottom.stack(top);
          break;
        case Op::Swap:
          if (stack.size() < 1) {
            std::cerr << "ERROR: Swap requires two shapes" << std::endl;
            return result;
          }
          other = stack.back();
          stack.pop_back();
          pair = shape.swap(other);
          stack.push_back(pair.second);
          shape = pair.first;
          break;
        case Op::PinPush:
          shape = shape.pin();
          break;
        case Op::Crystal:
          shape = shape.crystalize();
          break;
      }
      // std::cout << std::format("Shape: {}", shape.toString()) << std::endl;
      stack.push_back(shape);
    }
    return result;
  }
};

}  // namespace Shapez
