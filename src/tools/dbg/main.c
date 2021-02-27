#include "assembly_pane.h"
#include "breakpoints_pane.h"
#include "cpu_pane.h"
#include "dbg.h"
#include "flat_set.h"
#include "options.h"
#include "status_pane.h"

#include <assert.h>
#include <cpu.h>
#include <instruction.h>
#include <io.h>
#include <locale.h>
#include <notcurses/notcurses.h>
#include <rom.h>
#include <stdlib.h>
#include <string.h>
#include <util.h>

/*
 * Logs the current CPU instruction to the given file, in Nintendulator format
 * so that it can be easily diffed with some verified output.
 */
static void log_current_cpu_instruction(FILE *log_file, struct Cpu *cpu)
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
      sprintf(encoding_buf, "%02X %02X   ", (uint8_t)((encoding & 0x00ff00) >> 8),
              (uint8_t)(encoding & 0x0000ff));
      break;
    case 3:
      sprintf(encoding_buf, "%02X %02X %02X", (uint8_t)((encoding & 0xff0000) >> 16),
              (uint8_t)((encoding & 0x00ff00) >> 8), (uint8_t)(encoding & 0x0000ff));
      break;
  }

  fprintf(log_file, "%X  %s  %-31s A:%02X X:%02X Y:%02X P:%02X SP:%02X CYC:%d\n", cpu->PC,
          encoding_buf, instruction_print_layout(&ins, encoding, IL_NINTENDULATOR, cpu), cpu->A,
          cpu->X, cpu->Y, cpu->P, cpu->S, cpu->cycle);
  fflush(log_file);
}

/*
 * Toggles focus between the given panes. Only one pane can have focus at the
 * same time. The breakpoints pane can not receive focus in case no breakpoints
 * are set.
 */
static void toggle_focus(struct assembly_pane *assembly_pane,
                         struct breakpoints_pane *breakpoints_pane, struct Debugger *debugger)
{
  if (assembly_pane->has_focus && debugger->breakpoints.size > 0)
  {
    assembly_pane_set_focus(assembly_pane, false);
    breakpoints_pane_set_focus(breakpoints_pane, true);
  }
  else
  {
    assembly_pane_set_focus(assembly_pane, true);
    breakpoints_pane_set_focus(breakpoints_pane, false);
  }
}

/*
 * Function called after a resize event occurs to properly position the status
 * line within the new dimensions of the terminal window.
 */
static void resize_panes(struct status_pane *status_pane, int term_rows, int term_cols)
{
  status_pane_resize(status_pane, term_rows, term_cols);
}

/*
 * Asks the user to input an address.
 */
static int user_query_address(struct notcurses *nc, struct ncplane *plane, const char *question,
                              Address *address)
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
  while ((c = notcurses_getc_blocking(nc, &input)) != NCKEY_ENTER && c != NCKEY_ESC)
  {
    if (('0' <= c && c <= '9') || ('a' <= c && c <= 'f') || ('A' <= c && c <= 'F') ||
        c == NCKEY_BACKSPACE)
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
  const int is_success = (c != NCKEY_ESC && sscanf(contents, "%4hx", address) == 1) ? 0 : -1;

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
    quit_strerror("Could not open the given ROM file '%s' for reading", options.binary_file_name);
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
  struct Debugger debugger = make_debugger(prg_offset, prg_size);

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

  int term_rows;
  int term_cols;
  notcurses_term_dim_yx(nc, &term_rows, &term_cols);

  /* Create notcurses planes. */
  struct ncplane *std_plane = notcurses_stdplane(nc);

  /* Absolute top-left coordinates of the assembly plane. */
  const int assembly_x = 0;
  const int assembly_y = 0;
  const int assembly_cols = 80;
  struct assembly_pane assembly_pane = make_assembly_pane(std_plane, &debugger, &cpu, term_rows - 1,
                                                          assembly_cols, assembly_y, assembly_x);
  assembly_pane.has_focus = true;
  assembly_pane_scroll_to_pc(&assembly_pane, &debugger, &cpu);

  struct breakpoints_pane breakpoints_pane = make_breakpoints_pane(std_plane, &debugger, 9, 30);

  struct cpu_pane cpu_pane = make_cpu_pane(9, 20, std_plane);
  struct status_pane status_pane = make_status_pane(std_plane);

  /* Move the CPU state plane to the upper right corner. */
  const int cpu_state_plane_x = term_cols - ncplane_dim_x(cpu_pane.decoration_plane);
  ncplane_move_yx(cpu_pane.decoration_plane, 0, cpu_state_plane_x);

  /* Move the breakpoints plane to the left of the CPU state plane. */
  const int breakpoints_plane_x =
      cpu_state_plane_x - ncplane_dim_x(breakpoints_pane.decoration_plane);
  ncplane_move_yx(breakpoints_pane.decoration_plane, 0, breakpoints_plane_x);

  /* Status line is always on top. */
  ncplane_move_top(status_pane.plane);

  /* Event loop; wait for user input. */
  struct ncinput input = {0};
  bool quit = false;
  bool interactive_mode = true;

  while (!quit)
  {
    /* TODO(ton): the following planes do not need to be printed every frame?
     * Check out how notcurses handles this in their demos. */

    assembly_pane_update(&assembly_pane);
    breakpoints_pane_update(&breakpoints_pane, &debugger, &cpu);
    cpu_pane_update(&cpu_pane, &cpu);
    status_pane_update(&status_pane);

    /* Flip! */
    notcurses_render(nc);

    if (interactive_mode)
    {
      /* Read user input, and act accordingly. */
      notcurses_getc_blocking(nc, &input);
      switch (input.id)
      {
        case NCKEY_RESIZE:
          notcurses_term_dim_yx(nc, &term_rows, &term_cols);
          notcurses_render(nc);
          resize_panes(&status_pane, term_rows, term_cols);
          /* status_line_plane_resize(status_line_plane, term_rows, term_cols); */
          /* assembly_pane_resize(&assembly_pane, std_plane, term_rows); */
          break;
        case '\t':
          toggle_focus(&assembly_pane, &breakpoints_pane, &debugger);
          break;
        case NCKEY_SPACE: /* toggle breakpoint at cursor */
          debugger_toggle_breakpoint_at(
              &debugger, assembly_pane_cursor_address(&assembly_pane, &debugger, &cpu));
          break;
        case 'c': /* remove all breakpoints */
          flat_set_clear(&debugger.breakpoints);

          /* In case the breakpoints pane was in focus, focus the assembly pane
           * instead. */
          if (breakpoints_pane.has_focus)
          {
            breakpoints_pane_set_focus(&breakpoints_pane, false);
            assembly_pane_set_focus(&assembly_pane, true);
          }
          break;
        case 'd': /* remove selected breakpoint */
        {
          /* Remove the selected breakpoint. */
          int idx;
          if ((idx = breakpoints_pane_selected_breakpoint(&breakpoints_pane)) >= 0)
          {
            if (idx > 0 && idx == (int)debugger.breakpoints.size - 1)
            {
              breakpoints_pane_move_cursor(&breakpoints_pane, -1);
            }

            debugger_toggle_breakpoint_at(&debugger, debugger.breakpoints.data[idx]);

            /* In case no more breakpoints remain, automatically shift focus to
             * the assembly pane. */
            if (debugger.breakpoints.size == 0)
            {
              breakpoints_pane_set_focus(&breakpoints_pane, false);
              assembly_pane_set_focus(&assembly_pane, true);
            }
          }
        }
        break;
        case 'n': /* next instruction (step) */
          if (log_file)
          {
            log_current_cpu_instruction(log_file, &cpu);
          }
          cpu_execute_next_instruction(&cpu);
          assembly_pane_scroll_to_pc(&assembly_pane, &debugger, &cpu);
          break;
        case 'r': /* run */
          interactive_mode = false;
          break;
        case 'L': /* refresh screen */
          if (input.ctrl)
          {
            notcurses_refresh(nc, NULL, NULL);
          }
          break;
        case 'B': /* page up */
          if (input.ctrl && assembly_pane.has_focus)
          {
            assembly_pane_move_cursor(&assembly_pane, -term_rows);
          }
          else if (!input.ctrl) /* set breakpoint */
          {
            uint16_t address;
            if (user_query_address(nc, status_pane.plane, "Break at address: ", &address) == 0)
            {
              debugger_toggle_breakpoint_at(&debugger, address);
            }
          }
          break;
        case 'F': /* page down */
          if (assembly_pane.has_focus)
          {
            if (input.ctrl)
            {
              assembly_pane_move_cursor(&assembly_pane, term_rows);
            }
            else /* focus program counter */
            {
              assembly_pane_scroll_to_address(&assembly_pane, &debugger, &cpu, cpu.PC);
            }
          }
          break;
        case 'G': /* scroll to bottom */
          if (assembly_pane.has_focus)
          {
            assembly_pane_scroll_to_address(&assembly_pane, &debugger, &cpu, CPU_MAX_ADDRESS);
          }
          break;
        case 'g': /* scroll to top */
        {
          if (assembly_pane.has_focus)
          {
            struct timespec ts = {0};
            ts.tv_sec = 1;
            if (notcurses_getc(nc, &ts, NULL, NULL) == 'g')
            {
              assembly_pane_scroll_to_address(&assembly_pane, &debugger, &cpu, 0x0);
            }
          }
          break;
        }
        case ':': /* jump to address */
        {
          Address address;
          if (user_query_address(nc, status_pane.plane, "Jump to address: $", &address) == 0)
          {
            assembly_pane_scroll_to_address(&assembly_pane, &debugger, &cpu, address);
          }
        }
        break;
        case 'j': /* scroll one line down */
          if (assembly_pane.has_focus)
          {
            assembly_pane_move_cursor(&assembly_pane, 1);
          }
          else if (breakpoints_pane.has_focus)
          {
            breakpoints_pane_move_cursor(&breakpoints_pane, 1);
          }
          break;
        case 'k': /* scroll one line up */
          if (assembly_pane.has_focus)
          {
            assembly_pane_move_cursor(&assembly_pane, -1);
          }
          else if (breakpoints_pane.has_focus)
          {
            breakpoints_pane_move_cursor(&breakpoints_pane, -1);
          }
          break;
        case 'q': /* quit */
          quit = true;
          break;
        case '?': /* show help */
          status_pane_print_help(&status_pane);
          notcurses_render(nc);
          notcurses_getc_blocking(nc, &input);
          ncplane_erase(status_pane.plane);
          break;
      }
    }
    else /* !interactive_mode */
    {
      if (log_file)
      {
        log_current_cpu_instruction(log_file, &cpu);
      }

      cpu_execute_next_instruction(&cpu);
      assembly_pane_scroll_to_pc(&assembly_pane, &debugger, &cpu);
      interactive_mode = debugger_has_breakpoint_at(&debugger, cpu.PC);
    }
  }

  notcurses_stop(nc);
  destroy_debugger(&debugger);
}
