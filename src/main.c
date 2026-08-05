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
0xb7, 0x02, 0x00, 0x10,
0x93, 0x82, 0x02, 0x00,
0x37, 0x03, 0x00, 0x10,
0x13, 0x03, 0xc3, 0x01,
0x03, 0x23, 0x03, 0x00,
0x93, 0x03, 0x00, 0x00,
0x63, 0x86, 0x63, 0x04,
0x13, 0x0e, 0x00, 0x00,
0x93, 0x0e, 0x00, 0x00,
0x33, 0x0f, 0x73, 0x40,
0x13, 0x0f, 0xff, 0xff,
0x63, 0x06, 0xee, 0x03,
0x93, 0x1f, 0x2e, 0x00,
0xb3, 0x84, 0xf2, 0x01,
0x03, 0xa9, 0x04, 0x00,
0x83, 0xa9, 0x44, 0x00,
0x63, 0xd8, 0x29, 0x01,
0x23, 0xa0, 0x34, 0x01,
0x23, 0xa2, 0x24, 0x01,
0x93, 0x0e, 0x10, 0x00,
0x13, 0x0e, 0x1e, 0x00,
0x6f, 0xf0, 0x9f, 0xfd,
0x63, 0x86, 0x0e, 0x00,
0x93, 0x83, 0x13, 0x00,
0x6f, 0xf0, 0x9f, 0xfb,
};

/* TODO: remove */
const uint32_t arr[] = {
  64, 34, 25, 12, 22, 11, 90, 7
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
  
  /* TODO: remove */
  uint8_t *arr_mem = malloc(sizeof(arr));
  ASSERT(arr_mem, "malloc() failed");
  memcpy(arr_mem, arr, sizeof(arr));
  memory_region_t arr_region = {
    .addr = 0x10000000,
    .size = sizeof(arr),
    .data = arr_mem,
    .next = NULL
  };
  rv32i_add_region(&cpu, &arr_region);

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

  /* Cleanup */
  free(mem);
  free(arr_mem); /* TODO: remove */
  return 0;
}
