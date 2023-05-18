#include <iostream>

inline void print_indentation(int level) {
  while (level--) {
    std::cout << "    ";
  }
}
