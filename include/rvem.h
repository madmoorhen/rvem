/* Include guard */
#ifndef RVEM_H
#define RVEM_H

/*
 * Unprivileged ISA:
 * - https://riscv.github.io/riscv-isa-manual/snapshot/spec/#vol:unpriv
 * Privileged ISA:
 * - https://riscv.github.io/riscv-isa-manual/snapshot/spec/#vol:priv
 * Profiles:
 * - https://riscv.github.io/riscv-isa-manual/snapshot/spec/#vol:profiles
 */

/* Includes */
#include <stdint.h>

/* Memory region */
typedef struct {
  uint32_t addr;
  uint32_t size;
  uint8_t *data;
  void *next;
} memory_region_t;

/* Processor state */
typedef struct {
  uint32_t x[32];
  uint32_t pc;
  memory_region_t *regions;
} rv32i_t;

/* Initialise the processor */
void rv32i_init(rv32i_t *cpu, uint32_t program_size, uint8_t *program);
/* Cleanup the resources used by the processor */
void rv32i_destroy(rv32i_t *cpu);

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

#endif /* RVEM_H */
