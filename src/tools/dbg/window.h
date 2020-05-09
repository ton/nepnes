#ifndef NEPNES_WINDOW_H
#define NEPNES_WINDOW_H

#include "window.h"

#include <ncurses.h>

/* Wrapper structure around a curses window. */
struct Window
{
  WINDOW *border_win;
  WINDOW *win;

  int width;  /* effective width of the window (excluding the border) */
  int height; /* effective height of the window (excluding the border) */

  int x;      /* x position in screen coordinates */
  int y;      /* y position in screen coordinates */
};

struct Window make_window(int height, int width, int y, int x);
void destroy_window(struct Window* win);

#endif
