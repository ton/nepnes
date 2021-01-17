#include "cpu.h"
#include "dbg.h"
#include "instruction.h"
#include "io.h"
#include "options.h"
#include "rom.h"
#include "util.h"

#include <assert.h>
#include <locale.h>
#include <notcurses/notcurses.h>
#include <stdlib.h>
#include <string.h>

#define KEY_CTRL_B '\2'
#define KEY_CTRL_F '\6'

/*
 * Constructs the plane that contains a view on the memory of the machine, where
 * the values in memory are interpreted as instructions. This will also draw a
 * rounded border around the assembly plane on the standard notcurses plane.
 */
static struct ncplane *make_assembly_plane(const int lines,
                                           const int term_lines,
                                           const int term_cols,
                                           struct ncplane *std_plane)
{
  struct ncplane_options opts = {0};
  opts.x = 1;
  opts.y = 1;
  opts.rows = lines;
  opts.cols = 100;

  struct ncplane *assembly_plane = ncplane_create(std_plane, &opts);

  ncplane_rounded_box(std_plane, 0, 0, term_lines - 2,
                      ncplane_dim_x(assembly_plane), 0);

  return assembly_plane;
}

/*
 * Constructs the plane that contains a view of the CPU state (register values,
 * program counter, etc.). This will also draw a rounded border around the newly
 * created plane on the standard notcurses plane.
 */
static struct ncplane *make_cpu_state_plane(const int cols,
                                            struct ncplane *std_plane)
{
  const int box_width = 13;
  const int box_height = 7;
  const int box_left = cols - 1 - box_width;
  const int box_top = 0;

  struct ncplane_options opts = {0};
  opts.rows = box_height - 2;
  opts.cols = box_width - 2;
  opts.y = box_top + 1;
  opts.x = box_left + 2;

  ncplane_cursor_move_yx(std_plane, box_top, box_left);
  ncplane_rounded_box(std_plane, 0, 0, box_top + box_height - 1,
                      box_left + box_width, 0);

  return ncplane_create(std_plane, &opts);
}

/*
 * Creates the plane that displays the status line.
 */
static struct ncplane *make_status_line_plane(struct ncplane *std_plane)
{
  struct ncplane_options opts = {0};
  opts.rows = 1;
  opts.cols = ncplane_dim_x(std_plane);
  opts.y = ncplane_dim_y(std_plane) - 1;
  opts.x = 0;

  nccell c = CELL_CHAR_INITIALIZER(' ');
  cell_set_bg_rgb8(&c, 0x20, 0x20, 0x20);

  struct ncplane *status_line_plane = ncplane_create(std_plane, &opts);
  ncplane_set_base_cell(status_line_plane, &c);
  return status_line_plane;
}

/*
 * Prints all relevant CPU state data in the given CPU assembly plane.
 */
static void print_cpu_state(struct Cpu *cpu, struct ncplane *plane)
{
  ncplane_printf_yx(plane, 0, 0, " A:  $%02X", cpu->A);
  ncplane_printf_yx(plane, 1, 0, " X:  $%02X", cpu->X);
  ncplane_printf_yx(plane, 2, 0, " Y:  $%02X", cpu->Y);
  ncplane_printf_yx(plane, 3, 0, " S:  $%02X", cpu->S);
  ncplane_printf_yx(plane, 4, 0, "PC:  $%04X", cpu->PC);
}

/*
 * Prints the assembly currently loaded in memory to the given notcurses plane.
 */
static void print_assembly(struct Cpu *cpu, struct ncplane *plane)
{
  uint8_t *pc = cpu->ram;
  const uint8_t *end = cpu->ram + sizeof cpu->ram;

  int line = 0;
  while (line < ncplane_dim_y(plane) && pc < end)
  {
    struct Instruction ins = make_instruction(*pc);

    /*
     * In case of an unknown instruction, the calculated size of the instruction
     * will be zero.
     */
    if (ins.bytes == 0)
    {
      ncplane_printf_yx(plane, line, 0, "$%04X: %*s (%02X)",
                        (unsigned)(pc - cpu->ram), INSTRUCTION_BUFSIZE, "",
                        *pc);
      ++pc;
    }
    else
    {
      uint32_t encoding = *pc;
      for (int i = 1; i < ins.bytes; ++i)
      {
        encoding = (encoding << 8) + (pc + i < end ? *(pc + i) : 0);
      }

      ncplane_printf_yx(plane, line, 0, "$%04X: %-*s (%0*X)",
                        (unsigned)(pc - cpu->ram), INSTRUCTION_BUFSIZE,
                        Instruction_print(&ins, encoding), ins.bytes * 2,
                        encoding);

      pc += ins.bytes;
    }

    ++line;
  }
}

/*
 * Prints the status line.
 */
static void print_status_line(struct Debugger *debugger, struct ncplane *plane)
{
  const int width = ncplane_dim_x(plane);
  const char *help = "?: help";
  ncplane_printf_yx(plane, 0, width - 8, help);
}

/*
 * Prints a help screen in the status line.
 */
static void print_help(struct ncplane *plane)
{
  const int width = ncplane_dim_x(plane);
  ncplane_erase(plane);
  ncplane_putstr_aligned(
      plane, 0, NCALIGN_LEFT,
      " j/k: scroll up/down   C-B/C-F: page up/down   q: quit");
  ncplane_putstr_aligned(plane, 0, NCALIGN_RIGHT,
                         "press any key to close help ");
}

/*
 * Asks the user to input an address.
 */
static int user_query_address(struct notcurses *nc, struct ncplane *plane,
                              const char *question, Address *address)
{
  /* Convert plane coordinates to global coordinates to show the cursor at the
   * right position. */
  int y = 0;
  int x = strlen(question) + 1;
  ncplane_translate(plane, NULL, &y, &x);

  ncplane_printf_yx(plane, 0, 1, question);
  notcurses_cursor_enable(nc, y, x);

  /*
   * Create reader plane.
   */
  struct ncplane_options plane_opts = {0};
  plane_opts.rows = 1;
  plane_opts.cols = 4;
  plane_opts.y = 0;
  plane_opts.x = strlen(question) + 1;

  struct ncplane *reader_plane = ncplane_create(plane, &plane_opts);

  /*
   * Create reader itself.
   */
  struct ncreader_options reader_opts = {0};
  reader_opts.flags = NCREADER_OPTION_CURSOR;

  struct ncreader *reader = ncreader_create(reader_plane, &reader_opts);
  notcurses_render(nc);

  /*
   * Query user input, only hexadecimal characters and Enter/Escape are allowed.
   */
  char32_t c;
  struct ncinput input;
  while ((c = notcurses_getc_blocking(nc, &input)) != NCKEY_ENTER &&
         c != NCKEY_ESC)
  {
    if (('0' <= c && c <= '9') || ('a' <= c && c <= 'f') ||
        ('A' <= c && c <= 'F') || c == NCKEY_BACKSPACE)
    {
      ncreader_offer_input(reader, &input);
      notcurses_render(nc);
    }
  }

  /*
   * Retrieve user input in case the user did not press escape, and try to
   * convert to some hexadecimal address.
   */
  char *contents;
  ncreader_destroy(reader, &contents);
  const int is_success =
      c != NCKEY_ESC && sscanf(contents, "%4hx", address) == 1 ? 0 : -1;

  free(contents);
  ncplane_erase(plane);

  return is_success;
}

int main(int argc, char **argv)
{
  setlocale(LC_ALL, "");

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

  notcurses_options opts = {0};
  struct notcurses *nc;
  if ((nc = notcurses_init(&opts, NULL)) == NULL)
  {
    return EXIT_FAILURE;
  }

  /* Create curses windows. */
  int term_lines;
  int term_cols;
  notcurses_term_dim_yx(nc, &term_lines, &term_cols);

  struct ncplane *std_plane = notcurses_stdplane(nc);
  struct ncplane *assembly_plane =
      make_assembly_plane(sizeof(cpu.ram), term_lines, term_cols, std_plane);
  struct ncplane *cpu_state_plane = make_cpu_state_plane(term_cols, std_plane);
  struct ncplane *status_line_plane = make_status_line_plane(std_plane);

  /* The standard plane is on top; easier with box drawing. */
  ncplane_move_top(std_plane);
  ncplane_move_top(status_line_plane);

  /* Generate data for the assembly plane. */
  print_assembly(&cpu, assembly_plane);

  /* Set the program counter to the address where the ROM is loaded. */
  cpu.PC = options.address;
  dbg_scroll_assembly_to_address(&debugger, &cpu, cpu.PC);

  // Event loop; wait for user input.
  struct ncinput input = {0};
  bool quit = false;

  while (!quit)
  {
    ncplane_move_yx(assembly_plane, -debugger.line + 1, 1);
    print_cpu_state(&cpu, cpu_state_plane);
    print_status_line(&debugger, status_line_plane);
    notcurses_render(nc);
    notcurses_getc_blocking(nc, &input);
    switch (input.id)
    {
      case 'L':
        if (input.ctrl)
        {
          notcurses_refresh(nc, NULL, NULL);
        }
        break;
      case 'B':
        if (input.ctrl)
        {
          dbg_scroll_assembly(&debugger, -term_lines);
        }
        break;
      case 'F':
        if (input.ctrl)
        {
          dbg_scroll_assembly(&debugger, term_lines);
        }
        break;
      case 'G':
        dbg_scroll_assembly_to_address(&debugger, &cpu, 0xffff);
        break;
      case 'g':
      {
        struct timespec ts = {0};
        ts.tv_sec = 1;
        if (notcurses_getc(nc, &ts, NULL, NULL) == 'g')
        {
          dbg_scroll_assembly_to_address(&debugger, &cpu, 0x0);
        }
        break;
      }
      case ':':
      {
        Address address;
        if (user_query_address(nc, status_line_plane, "Jump to address: $",
                               &address) == 0)
        {
          dbg_scroll_assembly_to_address(&debugger, &cpu, address);
        }
      }
      break;
      case 'j':
        dbg_scroll_assembly(&debugger, 1);
        break;
      case 'k':
        dbg_scroll_assembly(&debugger, -1);
        break;
      case 'q':
        quit = true;
        break;
      case '?':
      {
        print_help(status_line_plane);
        notcurses_render(nc);
        notcurses_getc_blocking(nc, &input);
        ncplane_erase(status_line_plane);
      }
      break;
    }
  }

  notcurses_stop(nc);
}
