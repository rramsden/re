#include "re/cursor.h"

Cursor::Cursor(int c_x, int c_y, int clamp_x, int clamp_y) {
  cx = c_x;
  cy = c_y;
  clampx = clamp_x;
  clampy = clamp_y;
}

void Cursor::up() {
  if (cy != 0)
    cy--;
}

void Cursor::down() {
  if (cy < clampy)
    cy++;
}

void Cursor::left() {
  if (cx != 0)
    cx--;
}

void Cursor::right() {
  if (cx != clampx - 1)
    cx++;
}

Cursor Cursor::move(int c_x, int c_y, int clamp_x, int clamp_y) {
  return Cursor(c_x, c_y, clamp_x, clamp_y);
}
