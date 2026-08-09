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
  0xb7, 0x01, 0x00, 0x10,

  0xb7, 0x22, 0x09, 0x00,
  0x9b, 0x82, 0xb2, 0xa2,
  0x93, 0x92, 0xc2, 0x00,
  0x93, 0x82, 0x52, 0x3c,
  0x93, 0x92, 0xd2, 0x00,
  0x93, 0x82, 0xd2, 0xab,
  0x93, 0x92, 0xc2, 0x00,
  0x93, 0x82, 0xf2, 0xde,

};

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
    .next = NULL
  };
  rv64i_add_region(&cpu, &prog_region);

  uint8_t *data_mem = malloc(0x1000);
  ASSERT(data_mem, "malloc() failed");
  memory_region_t data_region = {
    .addr = 0x10000000,
    .size = 0x1000,
    .data = data_mem,
    .next = NULL
  };
  rv64i_add_region(&cpu, &data_region);
  
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
          while (1) rv64i_step(&cpu, verbose != 'n' && verbose != 'N');
        } else {
          for (int i = 0; i < num_cycles; i++)
            rv64i_step(&cpu, verbose != 'n' && verbose != 'N');
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
  return 0;
}
