#include <stdlib.h>
#include <errno.h>
#include <unistd.h>
#include <ctype.h>
#include <stdio.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <iostream>
#include <fstream>

#include "ansi.h"
#include "terminal.h"
#include "cursor.h"
#include "fmt/format.h"

#define RE_VERSION "0.0.1"

class Editor {
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

  public:
    int screenrows;
    int screencols;
    int rowoff;
    int coloff;
    int cx, cy;
    int numrows;
    std::unique_ptr<Cursor> cursor;
    vector<string> erows;
    Terminal terminal;

    Editor();

    void open(char *filename);
    void append(string line);
    void refresh();
    void move(char key);
    void processKeypress();
  private:
    void scroll();
    void draw_rows(string &sbuf);
    int readKey();
};
