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
  cpu->regions = NULL;
}

/* Add a memory region */
void rv32i_add_region(rv32i_t *cpu, memory_region_t *region) {
  ASSERT(cpu, "NULL passed as cpu to rv32i_add_region");
  ASSERT(region, "NULL passed as region to rv32i_add_region");
  if (!(cpu->regions)) {
    cpu->regions = region;
    return;
  }
  memory_region_t *r = cpu->regions;
  while (r->next) r = r->next;
  r->next = region;
  region->next = NULL;
}
/* Remove a memory region */
void rv32i_remove_region(rv32i_t *cpu, memory_region_t *region) {
  ASSERT(cpu, "NULL passed as cpu to rv32i_remove_region");
  ASSERT(region, "NULL passed as region to rv32i_remove_region");
  if (!(cpu->regions)) {
    printf("rv32i_remove_region called on cpu with no regions");
    return;
  }
  memory_region_t *r = cpu->regions;
  while (r->next && r->next != region) r = r->next;
  if (!(r->next)) {
    printf("rv32i_remove_region tried to remove a region that doesn't exist");
    return;
  }
  r->next = region->next;
}

/* Dump the processor state to the console */
void rv32i_dump_state(rv32i_t *cpu) {
  ASSERT(cpu, "NULL passed as cpu to rv32i_dump_state");
  printf("processor state:\n\tpc = 0x%08x\n\n\tx0 = 0x00000000\n", cpu->pc);
  for (uint8_t i = 1; i < 32; i++)
    printf("\tx%d = 0x%08x\n", i, rv32i_get_reg(cpu, i));
  if (cpu->regions) {
    memory_region_t *r = cpu->regions;
    while (r) {
      printf(
          "memory region:\n"
          "\taddress = 0x%08x\n"
          "\tsize = 0x%08x\n"
          "\tallocation = %p\n",
          r->addr, r->size, r->data
      );
      r = r->next;
    }
  }
}
/* Dump the memory at a location to the console */
void rv32i_dump_mem(rv32i_t *cpu, uint32_t addr, uint32_t size);

/* Get the value of a register */
uint32_t rv32i_get_reg(rv32i_t *cpu, uint8_t reg) {
  ASSERT(cpu, "NULL passed as cpu to rv32i_get_reg");
  ASSERT(reg < 32, "value over 31 passed as reg to rv32i_get_reg");
  if (reg == 0) return 0;
  return cpu->x[reg-1];
}
/* Set the value of a register */
void rv32i_set_reg(rv32i_t *cpu, uint8_t reg, uint32_t val) {
  ASSERT(cpu, "NULL passed as cpu to rv32i_set_reg");
  ASSERT(reg < 32, "value over 31 passed as reg to rv32i_set_reg");
  if (reg == 0) return;
  cpu->x[reg-1] = val;
}

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
