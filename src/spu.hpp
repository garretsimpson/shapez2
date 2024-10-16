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
        return 'R';
      case Op::Rotate2:
        return 'U';
      case Op::Rotate3:
        return 'L';
      case Op::Cut:
        return 'C';
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
      std::cout << toChar(op) << std::endl;
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
    Shape shape, top, bottom;

    std::vector<Op> ops = solution.ops;
    while (!ops.empty()) {
      switch (ops.back()) {
        case Op::Input:
          shape = solution.input.back();
          solution.input.pop_back();
          stack.push_back(shape);
          std::cout << std::format("DEBUG Input {}", shape.toString())
                    << std::endl;
          break;
        case Op::Output:
          shape = stack.back();
          stack.pop_back();
          result.push_back(shape);
          std::cout << std::format("DEBUG Output {}", shape.toString())
                    << std::endl;
          break;
        case Op::Trash:
          stack.pop_back();
          break;
        case Op::Rotate1:
          break;
        case Op::Rotate2:
          break;
        case Op::Rotate3:
          break;
        case Op::Cut:
          break;
        case Op::Stack:
          if (stack.size() < 2) {
            std::cerr << "ERROR: Stack requires two shapes" << std::endl;
            return result;
          }
          top = stack.back();
          stack.pop_back();
          bottom = stack.back();
          stack.pop_back();
          shape = Shape(top.value | bottom.value);
          stack.push_back(shape);
          std::cout << std::format("DEBUG Stack {} {} -> {}", top.toString(),
                                   bottom.toString(), shape.toString())
                    << std::endl;
          break;
        case Op::Swap:
          break;
        case Op::PinPush:
          break;
        case Op::Crystal:
          break;
      }
      ops.pop_back();
    }
    return result;
  }
};

}  // namespace Shapez
