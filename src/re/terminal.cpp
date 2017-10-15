#include "re/terminal.h"

struct termios orig_termios;

void disableRawMode() {
  if (tcsetattr(STDIN_FILENO, TCSANOW, &orig_termios) == -1)
  throw "tcsetattr";
}

void Terminal::enableRawMode() {
  struct termios tattr;

  // Save terminal attributes so we can restore them later
  tcgetattr(STDIN_FILENO, &orig_termios);
  atexit(disableRawMode);

  // Set custom terminal attributes
  tattr.c_iflag &= ~(BRKINT | ICRNL | INPCK | ISTRIP | IXON);
  tattr.c_oflag &= ~(OPOST);
  tattr.c_cflag &= ~(CS8);
  tattr.c_lflag &= ~(ECHO | ICANON | ISIG | IEXTEN);
  tattr.c_cc[VMIN] = 0;
  tattr.c_cc[VTIME] = 1;

  if (tcsetattr(STDIN_FILENO, TCSAFLUSH, &tattr) == -1) throw "tcsetattr";
}

int Terminal::getCursorPosition(int *rows, int *cols) {
  char buf[32];
  unsigned int i = 0;

  if (write(STDOUT_FILENO, "\x1b[6n", 4) != 4) return -1;

  while (i < sizeof(buf) - 1) {
    if (read(STDIN_FILENO, &buf[i], 1) != 1) break;
    if (buf[i] == 'R') break;
    i++;
  }
  buf[i] = '\0';

  if (buf[0] != '\x1b' || buf[1] != '[') return -1;
  if (sscanf(&buf[2], "%d;%d", rows, cols) != 2) return -1;

  return 0;
}

int Terminal::getWindowSize(int *rows, int *cols) {
  struct winsize ws;

  if (ioctl(STDOUT_FILENO, TIOCGWINSZ, &ws) == -1 || ws.ws_col == 0) {
    if (write(STDOUT_FILENO, "\x1b[999C\x1b[999B", 12) != 12) return -1;
    return getCursorPosition(rows, cols);
  } else {
    *cols = ws.ws_col;
    *rows = ws.ws_row;
    return 0;
  }
}
