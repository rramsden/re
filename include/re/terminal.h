#ifndef TERMINAL_H
#define TERMINAL_H

#include <stdio.h>
#include <termios.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/ioctl.h>
#include <sys/types.h>

class Terminal {
  public:
    void enableRawMode();
    int getWindowSize(int *rows, int *cols);

  private:
    int getCursorPosition(int *rows, int *cols);
};

#endif
