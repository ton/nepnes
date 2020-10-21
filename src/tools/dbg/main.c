#include "cpu.h"
#include "dbg.h"
#include "instruction.h"
#include "io.h"
#include "options.h"
#include "rom.h"
#include "util.h"
#include "window.h"

#include <assert.h>
#include <ncurses.h>
#include <stdlib.h>
#include <string.h>

#define KEY_CTRL_B '\2'
#define KEY_CTRL_F '\6'

static WINDOW *make_cpu_state_window()
{
  const int height = 7;
  const int width = 12;
  const int x = COLS - width;
  const int y = 0;

  WINDOW *cpu_state_window = newwin(height, width, y, x);
  box(cpu_state_window, 0, 0);
  return cpu_state_window;
}

static void print_cpu_state(struct Cpu *cpu, WINDOW *win)
{
  mvwprintw(win, 1, 1, " A:  $%02X", cpu->A);
  mvwprintw(win, 2, 1, " X:  $%02X", cpu->X);
  mvwprintw(win, 3, 1, " Y:  $%02X", cpu->Y);
  mvwprintw(win, 4, 1, " S:  $%02X", cpu->S);
  mvwprintw(win, 5, 1, "PC:  $%04X", cpu->PC);
}

static struct Window make_assembly_window()
{
  const int height = LINES - 2;
  const int width = 100;

  return make_window(height, width, 0, 0);
}

static void print_assembly(struct Debugger *debugger, struct Cpu *cpu,
                           struct Window *win)
{
  uint8_t *pc = cpu->ram + debugger->assembly_address;
  const uint8_t *end = cpu->ram + sizeof cpu->ram;

  werase(win->win);

  int line = 0;
  while (line < win->height && pc < end)
  {
    struct Instruction ins = make_instruction(*pc);

    /* In case of an unknown instruction, the calculated size of the instruction
     * will be zero. */
    if (ins.bytes == 0)
    {
      mvwprintw(win->win, line, 1, "$%04X: %*s (%02X)", (pc - cpu->ram),
                INSTRUCTION_BUFSIZE, "", *pc);
      ++pc;
    }
    else
    {
      uint32_t encoding = *pc;
      for (int i = 1; i < ins.bytes; ++i)
      {
        encoding = (encoding << 8) + (pc + i < end ? *(pc + i) : 0);
      }

      mvwprintw(win->win, line, 1, "$%04X: %-*s (%0*X)", (pc - cpu->ram),
                INSTRUCTION_BUFSIZE, Instruction_print(&ins, encoding),
                ins.bytes * 2, encoding);

      pc += ins.bytes;
    }

    ++line;
  }
}

/*
 * Asks the user to input an address.
 */
static int user_query_address(const char *question, Address *address)
{
  echo();
  curs_set(1);

  move(LINES - 2, 2);
  clrtoeol();
  printw(question);
  int is_success = scanw("%4hx", address) == 1 ? 0 : -1;
  move(LINES - 2, 2);
  clrtoeol();

  curs_set(0);
  noecho();

  return is_success;
}

int main(int argc, char **argv)
{
  struct Options options = {0};
  parse_options(&options, argc, argv);

  unsigned char *binary_data = NULL;
  size_t binary_size = 0;
  if (read_all(options.binary_file_name, &binary_data, &binary_size) == -1)
  {
    quit_strerror("Could not open the given ROM file '%s' for reading",
                  options.binary_file_name);
  }

  printf("Binary size: %lu bytes\n", binary_size);

  struct Cpu cpu = {0};
  struct Debugger debugger = {0};

  struct RomHeader header = rom_make_header(binary_data);
  if (header.rom_format == RF_UNKNOWN)
  {
    fprintf(stderr,
            "Warning, input binary is not a NES ROM file. Loading binary data "
            "as is into memory.\n");

    memcpy(cpu.ram, binary_data, binary_size);
  }
  else
  {
    uint8_t *prg_data;
    size_t prg_data_size;
    rom_prg_data(&header, binary_data, &prg_data, &prg_data_size);

    printf("PRG ROM size: %lu bytes\n", prg_data_size);
    printf("PRG offset in ROM data: %lu\n", (prg_data - binary_data));
    printf("\n");

    memcpy(cpu.ram + options.address, prg_data, prg_data_size);
  }

  /* Curses initialization. */
  initscr();
  cbreak();
  curs_set(0);
  keypad(stdscr, true);
  noecho();
  refresh();

  /* Create curses windows. */
  WINDOW *cpu_state_window = make_cpu_state_window();
  struct Window assembly_window = make_assembly_window();

  print_assembly(&debugger, &cpu, &assembly_window);
  print_cpu_state(&cpu, cpu_state_window);

  /* Event loop; wait for user input. */
  bool quit = false;
  while (!quit)
  {
    wnoutrefresh(cpu_state_window);
    wnoutrefresh(assembly_window.win);
    doupdate();

    char ch = getch();
    switch (ch)
    {
      case KEY_CTRL_B:
        dbg_scroll_assembly(&debugger, &cpu, -assembly_window.height);
        print_assembly(&debugger, &cpu, &assembly_window);
        break;
      case KEY_CTRL_F:
        dbg_scroll_assembly(&debugger, &cpu, assembly_window.height);
        print_assembly(&debugger, &cpu, &assembly_window);
        break;
      case 'G':
        dbg_scroll_assembly_to_address(&debugger, &cpu, 0xffff);
        print_assembly(&debugger, &cpu, &assembly_window);
        break;
      case 'g':
        if (getch() == 'g')
        {
          dbg_scroll_assembly_to_address(&debugger, &cpu, 0);
          print_assembly(&debugger, &cpu, &assembly_window);
        }
        break;
      case ':':
      {
        /* Ask the user which address to jump to. */
        Address address;
        if (user_query_address("Jump to address: $", &address) == 0)
        {
          dbg_scroll_assembly_to_address(&debugger, &cpu, address);
          print_assembly(&debugger, &cpu, &assembly_window);
        }
      }
      break;
      case 'j':
        dbg_scroll_assembly(&debugger, &cpu, 1);
        print_assembly(&debugger, &cpu, &assembly_window);
        break;
      case 'k':
        dbg_scroll_assembly(&debugger, &cpu, -1);
        print_assembly(&debugger, &cpu, &assembly_window);
        break;
      case 'q':
        quit = true;
        break;
    }
  }

  /* Cleanup curses related stuff. */
  destroy_window(&assembly_window);
  delwin(cpu_state_window);
  endwin();
}
