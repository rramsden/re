#pragma once
#include "fmt/format.h"

using namespace std;

class ANSI {
  public:
    static string clear_screen() {
      return "\x1b[2J";
    }

    static string hide_cursor() {
      return "\x1b[?25l";
    }

    static string show_cursor() {
      return "\x1b[?25h";
    }

    static string set_cursor(int row, int col) {
      return fmt::format("\x1b[{};{}H", row + 1, col + 1);
    }

    static string erase_line() {
      return "\x1b[K";
    }
};
