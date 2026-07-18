/* Implements rvem.h */
#include <rvem.h>

/* Includes */
#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>

/* Assertion */
#define ASSERT(expr, msg) do {\
  if (!(expr)) {\
    fprintf(stderr, msg);\
    exit(EXIT_FAILURE);\
  }\
} while (0)

/* Signed value from unsigned (reinterpret) */
static int32_t signedw(uint32_t val) { return *((int32_t *)(&val)); }

/* Initialise the processor */
void rv32i_init(rv32i_t *cpu) {
  ASSERT(cpu, "NULL passed as cpu to rv32i_init");
  cpu->regions.addr = 0;
  cpu->regions.size = 0;
  cpu->regions.data = NULL;
  cpu->regions.next = NULL;
}

/* Add a memory region */
void rv32i_add_region(rv32i_t *cpu, memory_region_t *region);
/* Remove a memory region */
void rv32i_remove_region(rv32i_t *cpu, memory_region_t *region);

/* Dump the processor state to the console */
void rv32i_dumpstate(rv32i_t *cpu);
/* Dump the memory at a location to the console */
void rv32i_dumpmem(rv32i_t *cpu, uint32_t addr, uint32_t size);

/* Get the value of a register */
uint32_t rv32i_get_reg(rv32i_t *cpu, uint8_t reg);
/* Set the value of a register */
void rv32i_set_reg(rv32i_t *cpu, uint8_t reg, uint32_t val);

/* Get a byte from memory */
uint8_t rv32i_getb(rv32i_t *cpu, uint32_t addr);
/* Get a half word from memory */
uint16_t rv32i_geth(rv32i_t *cpu, uint32_t addr);
/* Get a word from memory */
uint32_t rv32i_getw(rv32i_t *cpu, uint32_t addr);
/* Set a byte to memory */
void rv32i_setb(rv32i_t *cpu, uint32_t addr, uint8_t val);
/* Set a half word to memory */
void rv32i_seth(rv32i_t *cpu, uint32_t addr, uint16_t val);
/* Set a word to memory */
void rv32i_setw(rv32i_t *cpu, uint32_t addr, uint32_t val);

/* Reset the processor */
void rv32i_reset(rv32i_t *cpu);
/* Step the processor */
void rv32i_step(rv32i_t *cpu);
