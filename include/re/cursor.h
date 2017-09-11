#pragma once

#include <string>
#include <memory>

class Cursor {
  public:
    int cx, cy;
    int clampx;
    int clampy;

    Cursor(int c_x, int c_y, int clamp_x, int clamp_y);

    void up();
    void down();
    void left();
    void right();

    static std::unique_ptr<Cursor> move(int c_x, int c_y, int clamp_x, int clamp_y);
};
