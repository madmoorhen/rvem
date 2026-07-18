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
  uint32_t x[31];
  uint32_t pc;
  memory_region_t *regions;
} rv32i_t;

/* Initialise the processor */
extern void rv32i_init(rv32i_t *cpu);

/* Add a memory region */
extern void rv32i_add_region(rv32i_t *cpu, memory_region_t *region);
/* Remove a memory region */
extern void rv32i_remove_region(rv32i_t *cpu, memory_region_t *region);

/* Dump the processor state to the console */
extern void rv32i_dump_state(rv32i_t *cpu);
/* Dump the memory at a location to the console */
extern void rv32i_dump_mem(rv32i_t *cpu, uint32_t addr, uint32_t size);

/* Get the value of a register */
extern uint32_t rv32i_get_reg(rv32i_t *cpu, uint8_t reg);
/* Set the value of a register */
extern void rv32i_set_reg(rv32i_t *cpu, uint8_t reg, uint32_t val);

/* Get a byte from memory */
extern uint8_t rv32i_getb(rv32i_t *cpu, uint32_t addr);
/* Get a half word from memory */
extern uint16_t rv32i_geth(rv32i_t *cpu, uint32_t addr);
/* Get a word from memory */
extern uint32_t rv32i_getw(rv32i_t *cpu, uint32_t addr);
/* Set a byte to memory */
extern void rv32i_setb(rv32i_t *cpu, uint32_t addr, uint8_t val);
/* Set a half word to memory */
extern void rv32i_seth(rv32i_t *cpu, uint32_t addr, uint16_t val);
/* Set a word to memory */
extern void rv32i_setw(rv32i_t *cpu, uint32_t addr, uint32_t val);

/* Reset the processor */
extern void rv32i_reset(rv32i_t *cpu);
/* Step the processor */
extern void rv32i_step(rv32i_t *cpu);

#endif /* RVEM_H */
