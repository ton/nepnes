#include "window.h"

/*
 * Creates a window at the given (x, y) screen position, with the given width
 * and height. This will create a window including a border around the window at
 * the give position, and return a window structure that contains the
 * appropriate properties for the *inner* window.
 */
struct Window make_window(int height, int width, int y, int x)
{
  struct Window window = {0};

  /* create the border window */
  window.border_win = newwin(height, width, y, x);
  box(window.border_win, 0, 0);
  wnoutrefresh(window.border_win);

  /* create the inner window */
  window.width = width - 2;
  window.height = height - 2;
  window.win = newwin(window.height, window.width, y + 1, x + 1);

  return window;
}

/*
 * Cleans up the given Window structure, takes care of all curses cleanup that
 * is required.
 */
void destroy_window(struct Window *win)
{
  delwin(win->border_win);
  delwin(win->win);
}
