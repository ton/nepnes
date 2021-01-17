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
static struct ncplane *make_debugger_plane(const int lines,
                                           struct ncplane *std_plane)
{
  struct ncplane_options opts = {0};
  opts.rows = lines - 2;
  opts.cols = 100;

  struct ncplane *debugger_plane = ncplane_create(std_plane, &opts);

  ncplane_rounded_box(std_plane, 0, 0, ncplane_dim_y(debugger_plane),
                      ncplane_dim_x(debugger_plane), 0);

  return debugger_plane;
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
 * Creates the plane where user input is requested for certain actions. It has
 * a height of one line and is always attached to the debug view plane.
 */
struct ncplane *make_user_input_plane(struct ncplane *std_plane,
                                      struct ncplane *debugger_plane)
{
  struct ncplane_options opts = {0};
  opts.rows = 1;
  opts.cols = ncplane_dim_x(debugger_plane);
  /*
   * TODO(ton): Inconvenient that we need to do +1 here, what is the canonical
   * way to deal with planes and boxes, include the box in the plane or not?
   */
  opts.y = ncplane_dim_y(debugger_plane) + 1;
  opts.x = 0;

  return ncplane_create(std_plane, &opts);
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
 * Prints the current state of the debugger in the main debugger view.
 */
static void print_assembly(struct Debugger *debugger, struct Cpu *cpu,
                           struct ncplane *plane)
{
  uint8_t *pc = cpu->ram + debugger->assembly_address;
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
      ncplane_printf_yx(plane, line + 1, 1, "$%04X: %*s (%02X)",
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

      ncplane_printf_yx(plane, line + 1, 1, "$%04X: %-*s (%0*X)",
                        (unsigned)(pc - cpu->ram), INSTRUCTION_BUFSIZE,
                        Instruction_print(&ins, encoding), ins.bytes * 2,
                        encoding);

      pc += ins.bytes;
    }

    ++line;
  }
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
  plane_opts.cols = 5;
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
        ('A' <= c && c <= 'F'))
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
  int lines;
  int cols;
  notcurses_term_dim_yx(nc, &lines, &cols);

  struct ncplane *std_plane = notcurses_stdplane(nc);
  struct ncplane *debugger_plane = make_debugger_plane(lines, std_plane);
  struct ncplane *cpu_state_plane = make_cpu_state_plane(cols, std_plane);
  struct ncplane *input_plane =
      make_user_input_plane(std_plane, debugger_plane);

  // Event loop; wait for user input.
  struct ncinput input = {0};
  bool quit = false;
  while (!quit)
  {
    print_assembly(&debugger, &cpu, debugger_plane);
    print_cpu_state(&cpu, cpu_state_plane);
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
          dbg_scroll_assembly(&debugger, &cpu, -ncplane_dim_y(debugger_plane));
          print_assembly(&debugger, &cpu, debugger_plane);
        }
        break;
      case 'F':
        if (input.ctrl)
        {
          dbg_scroll_assembly(&debugger, &cpu, ncplane_dim_y(debugger_plane));
          print_assembly(&debugger, &cpu, debugger_plane);
        }
        break;
      case 'G':
        dbg_scroll_assembly_to_address(&debugger, &cpu, 0xffff);
        ncplane_erase(debugger_plane);
        print_assembly(&debugger, &cpu, debugger_plane);
        break;
      case 'g':
      {
        struct timespec ts = {0};
        ts.tv_sec = 1;
        if (notcurses_getc(nc, &ts, NULL, NULL) == 'g')
        {
          dbg_scroll_assembly_to_address(&debugger, &cpu, 0);
          print_assembly(&debugger, &cpu, debugger_plane);
        }
        break;
      }
      case ':':
      {
        Address address;
        if (user_query_address(nc, input_plane, "Jump to address: $",
                               &address) == 0)
        {
          dbg_scroll_assembly_to_address(&debugger, &cpu, address);
          print_assembly(&debugger, &cpu, debugger_plane);
        }
      }
      break;
      case 'j':
        dbg_scroll_assembly(&debugger, &cpu, 1);
        print_assembly(&debugger, &cpu, debugger_plane);
        break;
      case 'k':
        dbg_scroll_assembly(&debugger, &cpu, -1);
        print_assembly(&debugger, &cpu, debugger_plane);
        break;
      case 'q':
        quit = true;
        break;
    }
  }

  notcurses_stop(nc);
}
