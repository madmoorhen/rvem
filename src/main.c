/* Includes */
#include <rvem.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

/* Assertion */
#define ASSERT(expr, msg) do {\
  if (!(expr)) {\
    fprintf(stderr, msg);\
    exit(EXIT_FAILURE);\
  }\
} while (0)

/* Program */
const uint8_t program[] = {
  0x13, 0x07, 0xf0, 0x03,
  0x13, 0x05, 0x17, 0x00,
  0xb7, 0x06, 0x00, 0x09,
  0x93, 0x05, 0x00, 0x02,
  0x13, 0x08, 0xa0, 0x00,
  0x93, 0x08, 0xf0, 0xff,
  0x93, 0x07, 0x00, 0x00,
  0x6f, 0x00, 0xc0, 0x00,
  0x23, 0xa0, 0xb6, 0x00,
  0x93, 0x87, 0x17, 0x00,
  0xe3, 0x9c, 0xe7, 0xfe,
  0x13, 0x06, 0x00, 0x00,
  0x33, 0x03, 0xe5, 0x40,
  0x63, 0x1c, 0x66, 0x00,
  0x13, 0x07, 0xf7, 0xff,
  0x23, 0xa0, 0x06, 0x01,
  0xe3, 0x1c, 0x17, 0xfd,
  0x13, 0x05, 0x00, 0x00,
  0x6f, 0x00, 0x80, 0x02,
  0xb3, 0x77, 0xe6, 0x00,
  0x93, 0xb7, 0x17, 0x00,
  0xb3, 0x07, 0xf0, 0x40,
  0x93, 0xf7, 0xa7, 0x00,
  0x93, 0x87, 0x07, 0x02,
  0x23, 0xa0, 0xf6, 0x00,
  0x23, 0xa0, 0xb6, 0x00,
  0x13, 0x06, 0x16, 0x00,
  0x6f, 0xf0, 0x9f, 0xfc,
  0x73, 0x00, 0x10, 0x00
};

/* UART set callback */
void uart_set_callback(uint64_t addr, uint8_t val) {
  if (addr == 0x9000000) putchar(val);
}

/* Entry point */
int main(int argc, char *argv[]) {
  rv64i_t cpu;
  rv64i_init(&cpu);
  
  uint8_t *prog_mem = malloc(sizeof(program));
  ASSERT(prog_mem, "malloc() failed");
  memcpy(prog_mem, program, sizeof(program));
  memory_region_t prog_region = {
    .addr = 0,
    .size = sizeof(program),
    .data = prog_mem,
    .get_callback = NULL,
    .set_callback = NULL,
    .next = NULL
  };
  rv64i_add_region(&cpu, &prog_region);

  uint8_t *data_mem = malloc(0x1000);
  ASSERT(data_mem, "malloc() failed");
  memory_region_t data_region = {
    .addr = 0x10000000,
    .size = 0x1000,
    .data = data_mem,
    .get_callback = NULL,
    .set_callback = NULL,
    .next = NULL
  };
  rv64i_add_region(&cpu, &data_region);

  uint8_t *uart_mem = malloc(8);
  ASSERT(uart_mem, "malloc() failed");
  memory_region_t uart_region = {
    .addr = 0x9000000,
    .size = 8,
    .data = uart_mem,
    .get_callback = NULL,
    .set_callback = uart_set_callback,
    .next = NULL
  };
  rv64i_add_region(&cpu, &uart_region);
  
  rv64i_reset(&cpu);
  /* Main loop */
  char c = getchar();
  while (c != 'q') {
    if (c != '\n') getchar();
    switch (c) {
      case 'h':
        printf(
          "q:\t\tquit\n"
          "h:\t\thelp\n"
          "r:\t\treset\n"
          "s:\t\tdump processor state\n"
          "m:\t\tdump memory contents (specific region)\n"
          "n:\t\tstep n times (-1 for indefinitely)\n"
          "v:\t\tstep once (verbose)\n"
          "nothing:\tstep once (quiet)\n"
        );
        break;
      case 'r': rv64i_reset(&cpu); break;
      case 's': rv64i_dump_state(&cpu); break;
      case 'm': {
        uint64_t addr = 0;
        uint64_t size = 0;

        printf("addr: 0x");
        scanf("%16lx", &addr);
        printf("size: 0x");
        scanf("%16lx", &size);

        rv64i_dump_mem(&cpu, addr, size);
        getchar();
      } break;
      case 'n': {
        int num_cycles;
        char verbose;
        printf("num_cycles: ");
        scanf("%d", &num_cycles);
        printf("verbose(y/n): ");
        getchar();
        verbose = getchar();
        printf("%d cycles\n", num_cycles);
        if (num_cycles <= 0) {
          while (!rv64i_step(&cpu, verbose != 'n' && verbose != 'N'));
        } else {
          for (int i = 0; i < num_cycles; i++) {
            if (rv64i_step(&cpu, verbose != 'n' && verbose != 'N')) break;
          }
        }
      } break;
      case 'v': rv64i_step(&cpu, true); break;
      default: rv64i_step(&cpu, false); break;
    };
    c = getchar();
  }

  /* Cleanup */
  free(prog_mem);
  free(data_mem);
  free(uart_mem);
  return 0;
}
