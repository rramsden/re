#include <stdlib.h>
#include <errno.h>
#include <unistd.h>
#include <termios.h>
#include <ctype.h>
#include <stdio.h>
#include <sys/ioctl.h>
#include <iostream>

#include "vendor/fmt/fmt/format.h"

using namespace std;

#define RE_VERSION "0.0.1"
#define CTRL_KEY(k) ((k) & 0x1f)

struct editorConfig {
  int cx, cy;
  int screenrows;
  int screencols;
  struct termios orig_termios;
};

enum editorKey {
  ARROW_LEFT = 'h',
  ARROW_RIGHT = 'l',
  ARROW_UP = 'k',
  ARROW_DOWN = 'j',
  PAGE_UP = CTRL_KEY('b'),
  PAGE_DOWN = CTRL_KEY('f'),
  HOME_KEY = '^',
  END_KEY = '$',
  DEL_KEY = 'x'
};

struct editorConfig E;

void die(const char *s) {
  write(STDOUT_FILENO, "\x1b[2J", 4);
  write(STDOUT_FILENO, "\x1b[H", 3);
  perror(s);
  exit(1);
}

struct termios orig_termios;

void enableRawMode() {
  // Be a nice citizen and restore users original terminal settings.
  if (tcgetattr(STDIN_FILENO, &E.orig_termios) == -1) die("tcgetattr");

  atexit([] {
    if (tcsetattr(STDIN_FILENO, TCSAFLUSH, &E.orig_termios) == -1)
    die("tcsetattr");
  });

  struct termios raw = E.orig_termios;

  raw.c_iflag &= ~(BRKINT | ICRNL | INPCK | ISTRIP | IXON);
  raw.c_oflag &= ~(OPOST);
  raw.c_cflag &= ~(CS8);
  raw.c_lflag &= ~(ECHO | ICANON | ISIG | IEXTEN);
  raw.c_cc[VMIN] = 0;
  raw.c_cc[VTIME] = 1;

  if (tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw) == -1) die("tcsetattr");
}

int editorReadKey() {
  int nread;
  char c;

  while((nread = read(STDIN_FILENO, &c, 1)) != 1) {
    if (nread == -1 && errno != EAGAIN) die("read");
  }

  // handle arrow keys
  if (c == '\x1b') {
    char seq[3];

    if (read(STDIN_FILENO, &seq[0], 1) != 1) return '\x1b';
    if (read(STDIN_FILENO, &seq[1], 1) != 1) return '\x1b';

    if (seq[0] == '[') {
      if (seq[1] >= '0' && seq[1] <= '9') {
        if (read(STDIN_FILENO, &seq[2], 1) != 1) return '\x1b';
        if (seq[2] == '~') {
          switch(seq[1]) {
            case '1': return HOME_KEY;
            case '3': return DEL_KEY;
            case '4': return END_KEY;
            case '5': return PAGE_UP;
            case '6': return PAGE_DOWN;
            case '7': return HOME_KEY;
            case '8': return END_KEY;
          }
        }
      } else {
        switch (seq[1]) {
          case 'A': return ARROW_UP;
          case 'B': return ARROW_DOWN;
          case 'C': return ARROW_RIGHT;
          case 'D': return ARROW_LEFT;
        }
      }
    } else if (seq[0] == 'O') {
      switch(seq[1]) {
        case 'H': return HOME_KEY;
        case 'F': return END_KEY;
      }
    }

    return '\x1b';
  } else {
    return c;
  }
}

void editorMoveCursor(char key) {
  switch(key) {
    case ARROW_LEFT:
      if (E.cx != 0)
        E.cx--;
      break;
    case ARROW_RIGHT:
      if (E.cx != E.screencols - 1)
        E.cx++;
      break;
    case ARROW_DOWN:
      if (E.cy != E.screenrows - 1)
        E.cy++;
      break;
    case ARROW_UP:
      if (E.cy != 0)
        E.cy--;
      break;
  }
}

int getCursorPosition(int *rows, int *cols) {
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

int getWindowSize(int *rows, int *cols) {
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

void initEditor() {
  E.cx = 0;
  E.cy = 0;

  if (getWindowSize(&E.screenrows, &E.screencols) == -1) die("getWindowSize");
}

void editorProcessKeypress() {
  int c = editorReadKey();

  switch(c) {
    case CTRL_KEY('q'):
      write(STDOUT_FILENO, "\x1b[2J", 4);
      write(STDOUT_FILENO, "\x1b[H", 3);
      exit(0);
      break;
      
    case HOME_KEY:
      E.cx = 0;
      break;

    case END_KEY:
      E.cx = E.screencols - 1;
      break;

    case PAGE_UP:
    case PAGE_DOWN:
      {
        int times = E.screenrows;
        while (times--)
          editorMoveCursor(c == PAGE_UP ? ARROW_UP : ARROW_DOWN);
      }
      break;

    case ARROW_UP:
    case ARROW_DOWN:
    case ARROW_LEFT:
    case ARROW_RIGHT:
      editorMoveCursor(c);
      break;
  }
}

void editorDrawRows(string &sbuf) {
  int y;
  for (y = 0; y < E.screenrows; y++) {
    if (y == (E.screenrows / 2)) {
      string welcome = fmt::format("RickEdit -- version {}", RE_VERSION);
      int padding = ((E.screencols - welcome.size()) / 2);
      if (padding) {
        sbuf += "~";
      }
      while (padding--) sbuf += " ";

      sbuf += welcome;
    } else {
      sbuf += "~";
    }

    sbuf += "\x1b[K"; // erase in line

    if (y < E.screenrows - 1) {
      sbuf += "\r\n";
    }
  }
}

void editorRefreshScreen() {
  string sbuf = "";

  sbuf += "\x1b[?25l"; // hide cursor
  sbuf += "\x1b[H"; // reset cursor position

  editorDrawRows(sbuf);

  // position the cursor
  sbuf += fmt::format("\x1b[{};{}H", E.cy + 1, E.cx + 1);

  sbuf += "\x1b[?25h"; // show cursor

  write(STDOUT_FILENO, sbuf.c_str(), sbuf.size());
}

int main() {
  enableRawMode();
  initEditor();

  while(1) {
    editorRefreshScreen();
    editorProcessKeypress();
  }

  return 0;
}
