#ifndef CURSOR_H
#define CURSOR_H

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

    static Cursor move(int c_x, int c_y, int clamp_x, int clamp_y);
};

#endif
