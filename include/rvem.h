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
#include <stdbool.h>

/* Memory region */
typedef struct {
  uint64_t addr;
  uint64_t size;
  uint8_t *data;
  void (*get_callback)(uint64_t addr);
  void (*set_callback)(uint64_t addr, uint8_t val);
  void *next;
} memory_region_t;

/* CSR */
typedef struct {
  uint16_t addr: 12;
  uint64_t value;
  const char *name;
  uint64_t (*get)(void);
  void (*set)(uint64_t val);
  void *next;
} rv64i_csr_t;

/* Processor state */
typedef struct {
  uint64_t x[31];
  uint64_t pc;
  memory_region_t *regions;
  rv64i_csr_t *csrs;
} rv64i_t;

/* Initialise the processor */
extern void rv64i_init(rv64i_t *cpu);

/* Add a memory region */
extern void rv64i_add_region(rv64i_t *cpu, memory_region_t *region);
/* Remove a memory region */
extern void rv64i_remove_region(rv64i_t *cpu, memory_region_t *region);

/* Add a CSR */
extern void rv64i_add_csr(rv64i_t *cpu, rv64i_csr_t *csr);
/* Remove a CSR */
extern void rv64i_remove_csr(rv64i_t *cpu, rv64i_csr_t *csr);

/* Dump the processor state to the console */
extern void rv64i_dump_state(rv64i_t *cpu);
/* Dump the memory at a location to the console */
extern void rv64i_dump_mem(rv64i_t *cpu, uint64_t addr, uint64_t size);

/* Get the value of a register */
extern uint64_t rv64i_get_reg(rv64i_t *cpu, uint8_t reg);
/* Set the value of a register */
extern void rv64i_set_reg(rv64i_t *cpu, uint8_t reg, uint64_t val);

/* Get a byte from memory */
extern uint8_t rv64i_getb(rv64i_t *cpu, uint64_t addr);
/* Get a half word from memory */
extern uint16_t rv64i_geth(rv64i_t *cpu, uint64_t addr);
/* Get a word from memory */
extern uint32_t rv64i_getw(rv64i_t *cpu, uint64_t addr);
/* Get a double word from memory */
extern uint64_t rv64i_getd(rv64i_t *cpu, uint64_t addr);
/* Set a byte to memory */
extern void rv64i_setb(rv64i_t *cpu, uint64_t addr, uint8_t val);
/* Set a half word to memory */
extern void rv64i_seth(rv64i_t *cpu, uint64_t addr, uint16_t val);
/* Set a word to memory */
extern void rv64i_setw(rv64i_t *cpu, uint64_t addr, uint32_t val);
/* Set a double word to memory */
extern void rv64i_setd(rv64i_t *cpu, uint64_t addr, uint64_t val);

/* Reset the processor */
extern void rv64i_reset(rv64i_t *cpu);
/* Step the processor - returns true on ebreak */
extern bool rv64i_step(rv64i_t *cpu, bool verbose);

#endif /* RVEM_H */
