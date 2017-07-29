#include <stdlib.h>
#include <errno.h>
#include <unistd.h>
#include <termios.h>
#include <ctype.h>
#include <stdio.h>
#include <sys/ioctl.h>
#include <iostream>

using namespace std;

#define RE_VERSION "0.0.1"
#define CTRL_KEY(k) ((k) & 0x1f)

struct editorConfig {
  int screenrows;
  int screencols;
  struct termios orig_termios;
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

char editorReadKey() {
  int nread;
  char c;

  while((nread = read(STDIN_FILENO, &c, 1)) != 1) {
    if (nread == -1 && errno != EAGAIN) die("read");
  }

  return c;
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
  if (getWindowSize(&E.screenrows, &E.screencols) == -1) die("getWindowSize");
}

void editorProcessKeypress() {
  char c = editorReadKey();

  switch(c) {
    case CTRL_KEY('q'):
      write(STDOUT_FILENO, "\x1b[2J", 4);
      write(STDOUT_FILENO, "\x1b[H", 3);
      exit(0);
      break;
  }
}

void editorDrawRows(string &sbuf) {
  int y;
  for (y = 0; y < E.screenrows; y++) {
    if (y == E.screenrows - 1) {
      char welcome[80];
      int welcomelen = snprintf(welcome, sizeof(welcome),
          "RickEdit -- version %s", RE_VERSION);
      if (welcomelen > E.screencols) welcomelen = E.screencols;

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

  sbuf += "\x1b[H"; // reset cursor position
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
