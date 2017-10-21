#include "re/editor.h"

using namespace std;

Editor::Editor() {
  rowoff = 0;
  coloff = 0;
  numrows = 0;
  filename = "";

  terminal.enableRawMode();
  if (terminal.getWindowSize(&screenrows, &screencols) == -1) throw("getWindowSize");

  screenrows -= 1; // statusbar
  cursor = make_unique<Cursor>(0, 0, screenrows, screencols);
}

void Editor::open(char *fname) {
  string line;
  filename = fname;

  ifstream input(fname);
  if (!input) {
    perror("ifstream");
    exit(errno);
  }

  while (getline(input, line)) {
    append(line);
  }

  // set maximum cursor scroll
  cursor->clampy = erows.size();

  input.close();
}

int Editor::readKey() {
  int nread;
  char c;

  while((nread = read(STDIN_FILENO, &c, 1)) != 1) {
    if (nread == -1 && errno != EAGAIN) throw("read");
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

void Editor::processKeypress() {
  int c = readKey();

  switch(c) {
    case CTRL_KEY('q'):
      write(STDOUT_FILENO, "\x1b[2J", 4);
      write(STDOUT_FILENO, "\x1b[H", 3);
      exit(0);
      break;
    case HOME_KEY:
      cursor = Cursor::move(0, cursor->cy, cursor->clampx, cursor->clampy);
      break;

    case END_KEY:
      cursor->cx = screencols - 1;
      break;

    case PAGE_UP:
    case PAGE_DOWN:
      {
        int times = screenrows;
        while (times--)
          move(c == PAGE_UP ? ARROW_UP : ARROW_DOWN);
      }
      break;

    case ARROW_UP:
    case ARROW_DOWN:
    case ARROW_LEFT:
    case ARROW_RIGHT:
      move(c);
      break;
  }
}

void Editor::scroll() {
  // vertical scrolling
  if (cursor->cy < rowoff) {
    rowoff = cursor->cy;
  }
  if (cursor->cy >= rowoff + screenrows) {
    rowoff = cursor->cy - screenrows + 1;
  }

  // horizontal scrolling
  if (cursor->cx < coloff) {
    coloff = cursor->cx;
  }
  if (cursor->cx >= coloff + screencols) {
    coloff = cursor->cx - screencols + 1;
  }
}

void Editor::draw_rows(string &sbuf) {
  for (int y = 0; y < screenrows; y++) {
    int filerow = y + rowoff;
    int filecol = 0;

    if (filerow >= numrows) {
      // Only display welcome message when there is no file
      if (numrows == 0 && y == (screenrows / 2)) {
        string welcome = fmt::format("RickEdit -- version {}", RE_VERSION);
        int padding = ((screencols - welcome.size()) / 2);
        if (padding) {
          sbuf += "~";
          padding--;
        }
        while (padding--) sbuf += " ";

        sbuf += welcome;
      } else {
        sbuf += "~";
      }
    } else {
      int len = erows[filerow].size() - coloff;
      if (len < 0) {
        sbuf += "";
      } else {
        sbuf += erows[filerow].substr(coloff, screencols);
      }
    }

    sbuf += ANSI::erase_line();
    sbuf += "\r\n";
  }
}

void Editor::draw_status_bar(string &sbuf) {
  string str = fmt::format("{} ({}:{})", (filename != "" ? filename : "[No Name]"), cursor->cy, cursor->cx);
  int len = str.size();

  for (int i = 0; i < screencols - len; ++i) {
    str += " ";
  }

  sbuf += "\e[100m" + str + "\e[49m";
}

void Editor::refresh() {
  scroll();
  string sbuf = "";

  sbuf += ANSI::hide_cursor();
  sbuf += ANSI::set_cursor(0, 0);

  draw_rows(sbuf);
  draw_status_bar(sbuf);

  sbuf += ANSI::set_cursor(cursor->cy - rowoff, cursor->cx - coloff);
  sbuf += ANSI::show_cursor();

  write(STDOUT_FILENO, sbuf.c_str(), sbuf.size());
}

void Editor::move(char key) {
  cursor->clampx = erows[cursor->cy].size();

  switch(key) {
    case ARROW_LEFT:
      cursor->left();
      break;
    case ARROW_RIGHT:
      cursor->right();
      break;
    case ARROW_DOWN:
      cursor->down();
      break;
    case ARROW_UP:
      cursor->up();
      break;
  }
}

void Editor::append(string line) {
  erows.push_back(line);
  numrows++;
}
