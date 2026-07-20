/* Includes */
#define _POSIX_C_SOURCE 200809L
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
  0x93, 0x00, 0x80, 0xff,
  0x13, 0x91, 0x20, 0x00,
  0x93, 0xd1, 0x20, 0x00,
  0x13, 0xd2, 0x20, 0x40
};

/* Entry point */
int main(int argc, char *argv[]) {
  rv32i_t cpu;
  rv32i_init(&cpu);
  
  uint8_t *mem = malloc(0x1000);
  ASSERT(mem, "malloc() failed");
  memcpy(mem, program, sizeof(program));
  memory_region_t mem_region = {
    .addr = 0x00000000,
    .size = 0x00001000,
    .data = mem,
    .next = NULL
  };
  rv32i_add_region(&cpu, &mem_region);

  rv32i_reset(&cpu);
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
      case 'r': rv32i_reset(&cpu); break;
      case 's': rv32i_dump_state(&cpu); break;
      case 'm': {
        uint32_t addr = 0;
        uint32_t size = 0;

        printf("addr: 0x");
        scanf("%8x", &addr);
        printf("size: 0x");
        scanf("%8x", &size);

        rv32i_dump_mem(&cpu, addr, size);
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
          while (1) rv32i_step(&cpu, verbose != 'n' && verbose != 'N');
        } else {
          for (int i = 0; i < num_cycles; i++)
            rv32i_step(&cpu, verbose != 'n' && verbose != 'N');
        }
      } break;
      case 'v': rv32i_step(&cpu, true); break;
      default: rv32i_step(&cpu, false); break;
    };
    c = getchar();
  }

  return 0;
}
