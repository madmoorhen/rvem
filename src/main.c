/* Includes */
#include <rvem.h>

/* Entry point */
int main(int argc, char *argv[]) {
  rv32i_t cpu;

  /* Initialise cpu */
  init(&cpu);

  /* Initialise memory */
  uint8_t *mem = malloc(0x1000);
  add_region(&cpu, 0x00000000, 0x1000, mem);

  /* Reset */
  reset(&cpu);

  /* Main loop */
  while (getchar() != 'q') {
    step(&cpu);
  }

  /* Cleanup */
  destroy(&cpu);
  free(mem);
  return 0;
}
