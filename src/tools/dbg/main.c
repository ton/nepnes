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

#define UC_ALPHA "\u03b1"
#define VERSION "0.1" UC_ALPHA

/*
 * Logs the current CPU instruction to the given file, in Nintendulator format
 * so that it can be easily diffed with some verified output.
 */
void log_current_cpu_instruction(FILE *log_file, struct Cpu *cpu)
{
  uint8_t *pc = cpu->ram + cpu->PC;
  const uint8_t *end = cpu->ram + sizeof cpu->ram;

  struct Instruction ins = make_instruction(*pc);

  uint32_t encoding = *pc;
  for (int i = 1; i < ins.bytes; ++i)
  {
    encoding = (encoding << 8) + (pc + i < end ? *(pc + i) : 0);
  }

  char encoding_buf[9];
  switch (ins.bytes)
  {
    case 1:
      sprintf(encoding_buf, "%02X      ", (uint8_t)(encoding & 0x0000ff));
      break;
    case 2:
      sprintf(encoding_buf, "%02X %02X   ",
              (uint8_t)((encoding & 0x00ff00) >> 8),
              (uint8_t)(encoding & 0x0000ff));
      break;
    case 3:
      sprintf(encoding_buf, "%02X %02X %02X",
              (uint8_t)((encoding & 0xff0000) >> 16),
              (uint8_t)((encoding & 0x00ff00) >> 8),
              (uint8_t)(encoding & 0x0000ff));
      break;
  }

  fprintf(log_file,
          "%X  %s  %-31s A:%02X X:%02X Y:%02X P:%02X SP:%02X CYC:%d\n", cpu->PC,
          encoding_buf,
          instruction_print_layout(&ins, encoding, IL_NINTENDULATOR, cpu),
          cpu->A, cpu->X, cpu->Y, cpu->P, cpu->S, cpu->cycle);
  fflush(log_file);
}

/*
 * Draws the border next to the assembly plane. Also called in case of a resize
 * event.
 */
static void draw_assembly_plane_border(struct ncplane *assembly_plane,
                                       struct ncplane *std_plane,
                                       int term_lines)
{
  nccell vline_cell = CELL_TRIVIAL_INITIALIZER;
  cell_load(std_plane, &vline_cell, "\u2502");
  ncplane_cursor_move_yx(std_plane, 0, ncplane_dim_x(assembly_plane));
  ncplane_vline(std_plane, &vline_cell, term_lines - 1);
}

/*
 * Constructs the plane that contains a view on the memory of the machine, where
 * the values in memory are interpreted as instructions. This will also draw a
 * rounded border around the assembly plane on the standard notcurses plane.
 */
static struct ncplane *make_assembly_plane(const int lines, const int y,
                                           const int x,
                                           struct ncplane *std_plane)
{
  struct ncplane_options opts = {0};
  opts.x = x;
  opts.y = y;
  opts.rows = lines;
  opts.cols = 88;

  return ncplane_create(std_plane, &opts);
}

/*
 * Constructs the plane that contains a view of the CPU state (register values,
 * program counter, etc.). This will also draw a rounded border around the newly
 * created plane on the standard notcurses plane.
 */
static struct ncplane *make_cpu_state_plane(const int cols,
                                            struct ncplane *std_plane)
{
  const int box_width = 20;
  const int box_height = 11;
  const int box_left = cols - box_width;
  const int box_top = 0;

  struct ncplane_options opts = {0};
  opts.rows = box_height - 2;
  opts.cols = box_width - 2;
  opts.y = box_top;
  opts.x = box_left + 2;

  struct ncplane *cpu_state_plane = ncplane_create(std_plane, &opts);
  ncplane_perimeter_rounded(cpu_state_plane, 0, 0, 0);

  return cpu_state_plane;
}

/*
 * Function called after a resize event occurs to properly position the status
 * line within the new dimensions of the terminal window.
 */
static void status_line_plane_resize(struct ncplane *status_line_plane,
                                     int lines, int cols)
{
  ncplane_erase(status_line_plane);
  ncplane_resize_simple(status_line_plane, 1, cols);
  ncplane_move_yx(status_line_plane, lines - 1, 0);
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
 * Creates a plane that will highlight the current instruction.
 */
static struct ncplane *make_pc_plane(struct ncplane *assembly_plane,
                                     struct ncplane *std_plane)
{
  struct ncplane_options opts = {0};
  opts.rows = 1;
  opts.cols = ncplane_dim_x(assembly_plane);
  opts.y = 1;
  opts.x = 0;

  nccell c = CELL_TRIVIAL_INITIALIZER;
  cell_set_bg_rgb8(&c, 0xaa, 0xaa, 0xaa);
  cell_set_fg_rgb8(&c, 0x30, 0x30, 0x30);

  struct ncplane *pc_plane = ncplane_create(std_plane, &opts);
  ncplane_set_base_cell(pc_plane, &c);
  return pc_plane;
}

/*
 * Prints all relevant CPU state data in the given CPU assembly plane.
 */
static void print_cpu_state(struct Cpu *cpu, struct ncplane *plane)
{
  const char flags[] = {'-', 'C', 'Z', 'I', 'D', '-', '-', 'O', 'N'};

  ncplane_printf_yx(plane, 1, 1, "   A:  $%02X", cpu->A);
  ncplane_printf_yx(plane, 2, 1, "   X:  $%02X", cpu->X);
  ncplane_printf_yx(plane, 3, 1, "   Y:  $%02X", cpu->Y);
  ncplane_printf_yx(plane, 4, 1, "   S:  $%02X", cpu->S);
  ncplane_printf_yx(plane, 5, 1, "  PC:  $%04X", cpu->PC);
  ncplane_printf_yx(plane, 6, 1, "   P:  %c%c--%c%c%c%c",
                    flags[((cpu->P & FLAGS_NEGATIVE) >> 7) * 8],
                    flags[((cpu->P & FLAGS_OVERFLOW) >> 6) * 7],
                    flags[((cpu->P & FLAGS_DECIMAL) >> 3) * 4],
                    flags[((cpu->P & FLAGS_INTERRUPT_DISABLE) >> 2) * 3],
                    flags[(cpu->P & FLAGS_ZERO)],
                    flags[(cpu->P & FLAGS_CARRY)]);
  ncplane_printf_yx(plane, 7, 1, " CYC:  %d", cpu->cycle);
}

/*
 * Prints the assembly currently loaded in memory to the given notcurses plane.
 */
static void print_assembly(struct Debugger *debugger, struct Cpu *cpu,
                           struct ncplane *plane)
{
  ncplane_erase(plane);

  uint16_t address = dbg_line_to_address(debugger, cpu, debugger->line);

  int y = 0;
  int line = debugger->line;
  const int last_line = MIN(line + ncplane_dim_y(plane), debugger->last_line);

  while (line < last_line)
  {
    struct Instruction ins = {0};

    uint32_t prg_last_address = debugger->prg_offset + debugger->prg_size;
    if (debugger->prg_offset <= address && address < prg_last_address)
    {
      ins = make_instruction(cpu->ram[address]);
    }

    /*
     * In case of an unknown instruction, the calculated size of the instruction
     * will be zero.
     */
    if (ins.bytes == 0)
    {
      ncplane_printf_yx(plane, y, 0, "$%04X: %*s (%02X)", address,
                        INSTRUCTION_BUFSIZE, "", cpu->ram[address]);
      ++address;
    }
    else
    {
      uint32_t encoding = cpu->ram[address];
      for (int i = 1; i < ins.bytes; ++i)
      {
        encoding = (encoding << 8) + cpu->ram[address + i];
      }

      ncplane_printf_yx(plane, y, 0, "$%04X: %-*s (%0*X)", address,
                        INSTRUCTION_BUFSIZE, instruction_print(&ins, encoding),
                        ins.bytes * 2, encoding);

      address += ins.bytes;
    }

    ++line;
    ++y;
  }
}

/*
 * Prints the status line.
 */
static void print_status_line(struct ncplane *plane)
{
  ncplane_putstr_aligned(plane, 0, NCALIGN_RIGHT, "nepnes dbg v" VERSION " ");
  ncplane_putstr_aligned(plane, 0, NCALIGN_LEFT, " ?: help");
}

/*
 * Prints a help screen in the status line.
 */
static void print_help(struct ncplane *plane)
{
  ncplane_erase(plane);
  ncplane_putstr_aligned(plane, 0, NCALIGN_LEFT,
                         " j/k: scroll up/down   "
                         "C-B/C-F: page up/down   "
                         "f: focus PC   "
                         "r: run    "
                         "n: next instruction   "
                         "q: quit");
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
      (c != NCKEY_ESC && sscanf(contents, "%4hx", address) == 1) ? 0 : -1;

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

  /* Create the CPU and debugger state. Set the program counter to the address
   * where the ROM is loaded. */
  struct Cpu cpu = {0};
  cpu_power_on(&cpu);
  cpu.PC = options.address;

  uint16_t prg_offset = 0;
  size_t prg_size = 0;

  /* Load the cartridge into memory. */
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
    rom_prg_data(&header, binary_data, &prg_data, &prg_size);

    printf("PRG ROM size: %lu bytes\n", prg_size);
    printf("PRG offset in ROM data: %lu\n", (prg_data - binary_data));
    printf("\n");

    prg_offset = options.address;
    memcpy(cpu.ram + prg_offset, prg_data, prg_size);
  }

  /* Initialize the debugger state. */
  struct Debugger debugger = {0};
  dbg_init(&debugger, &cpu, prg_offset, prg_size);

  notcurses_options opts = {0};
  opts.flags = NCOPTION_SUPPRESS_BANNERS;

  struct notcurses *nc;
  if ((nc = notcurses_core_init(&opts, NULL)) == NULL)
  {
    return EXIT_FAILURE;
  }

  /* Create log file if specified. */
  FILE *log_file = NULL;
  if (options.log_file_name)
  {
    if ((log_file = fopen(options.log_file_name, "w")) == NULL)
    {
      quit_strerror("Could not create log file '%s'", options.log_file_name);
    }
  }

  /* Create curses windows. */
  int term_lines;
  int term_cols;
  notcurses_term_dim_yx(nc, &term_lines, &term_cols);

  /* Absolute top-left coordinates of the assembly plane. */
  const int assembly_x = 1;
  const int assembly_y = 0;

  struct ncplane *std_plane = notcurses_stdplane(nc);
  struct ncplane *assembly_plane =
      make_assembly_plane(term_lines - 1, assembly_y, assembly_x, std_plane);
  struct ncplane *cpu_state_plane = make_cpu_state_plane(term_cols, std_plane);
  struct ncplane *status_line_plane = make_status_line_plane(std_plane);
  struct ncplane *pc_plane = make_pc_plane(assembly_plane, std_plane);

  /* Status line is always on top. */
  ncplane_move_top(status_line_plane);

  /* Draw a vertical line separating the assembly window from the rest of the
   * UI. */
  draw_assembly_plane_border(assembly_plane, std_plane, term_lines);

  /* Event loop; wait for user input. */
  struct ncinput input = {0};
  bool quit = false;
  bool focus_pc = true;
  bool run_mode = false;
  while (!quit)
  {
    if (focus_pc)
    {
      dbg_scroll_assembly_to_pc(&debugger, &cpu);
    }

    print_assembly(&debugger, &cpu, assembly_plane);

    /* Scroll the PC plane. */
    ncplane_move_yx(
        pc_plane,
        -(debugger.line - dbg_address_to_line(&debugger, &cpu, cpu.PC)) +
            assembly_y,
        0);

    /* Print the CPU state in the plane reserved for that, and highlight the
     * line of assembly the PC is pointing to. */
    print_cpu_state(&cpu, cpu_state_plane);

    /* Print the status line. */
    /* TODO(ton): not needed to be printed every time. */
    print_status_line(status_line_plane);

    /* Flip! */
    notcurses_render(nc);

    /* Read user input, and act accordingly. */
    if (run_mode)
    {
      if (log_file)
      {
        log_current_cpu_instruction(log_file, &cpu);
      }
      cpu_execute_next_instruction(&cpu);
    }
    else
    {
      notcurses_getc_blocking(nc, &input);
      switch (input.id)
      {
        case NCKEY_RESIZE:
          notcurses_render(nc);
          notcurses_term_dim_yx(nc, &term_lines, &term_cols);
          status_line_plane_resize(status_line_plane, term_lines, term_cols);
          draw_assembly_plane_border(assembly_plane, std_plane, term_lines);
          break;
        case 'n':
          if (log_file)
          {
            log_current_cpu_instruction(log_file, &cpu);
          }
          cpu_execute_next_instruction(&cpu);
          focus_pc = true;
          break;
        case 'r':
          run_mode = true;
          focus_pc = true;
          break;
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
            focus_pc = false;
          }
          break;
        case 'f':
          // Focus program counter.
          dbg_scroll_assembly_to_pc(&debugger, &cpu);
          break;
        case 'F':
          if (input.ctrl)
          {
            dbg_scroll_assembly(&debugger, term_lines);
            focus_pc = false;
          }
          break;
        case 'G':
          dbg_scroll_assembly_to_address(&debugger, &cpu, CPU_MAX_ADDRESS);
          focus_pc = false;
          break;
        case 'g':
        {
          struct timespec ts = {0};
          ts.tv_sec = 1;
          if (notcurses_getc(nc, &ts, NULL, NULL) == 'g')
          {
            dbg_scroll_assembly_to_address(&debugger, &cpu, 0x0);
            focus_pc = false;
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
            focus_pc = false;
          }
        }
        break;
        case 'j':
          dbg_scroll_assembly(&debugger, 1);
          focus_pc = false;
          break;
        case 'k':
          dbg_scroll_assembly(&debugger, -1);
          focus_pc = false;
          break;
        case 'q':
          quit = true;
          break;
        case '?':
          print_help(status_line_plane);
          notcurses_render(nc);
          notcurses_getc_blocking(nc, &input);
          ncplane_erase(status_line_plane);
          break;
      }
    }
  }

  notcurses_stop(nc);
}
