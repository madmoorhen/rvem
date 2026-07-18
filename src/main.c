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
  0x10, 0x20, 0x30, 0x40
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
  char c = 0;
  while (c != 'q') {
    switch (c) {
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
      case 'v': rv32i_step(&cpu, true); break;
      default: rv32i_step(&cpu, false); break;
    };
    c = getchar();
  }

  return 0;
}
