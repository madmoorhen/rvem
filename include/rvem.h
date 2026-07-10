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
#include <stdio.h>
#include <stdlib.h>

/* Assertion */
#define ASSERT(expr, msg) do {\
  if (!(expr)) {\
    fprintf(stderr, msg);\
    exit(EXIT_FAILURE);\
  }\
} while (0)

/* Memory region */
typedef struct {
  uint32_t addr;
  uint32_t size;
  uint8_t *mem;
  void *next;
} region_t;

/* Processor state */
typedef struct {
  uint32_t x[31];
  uint32_t pc;
  region_t *regions;
} rv32i_t;

/* Get a register's value */
static uint32_t get_reg(rv32i_t *cpu, uint8_t reg) {
  ASSERT(cpu, "NULL passed as cpu to get_reg");
  ASSERT(reg < 32, "Value greater than 32 passed as reg to get_reg");
  if (reg == 0) return 0;
  return cpu->x[reg-1];
}
/* Set a register's value */
static void set_reg(rv32i_t *cpu, uint8_t reg, uint32_t val) {
  ASSERT(cpu, "NULL passed as cpu to set_reg");
  ASSERT(reg < 32, "Value greater than 32 passed as reg to set_reg");
  if (reg == 0) return;
  cpu->x[reg-1] = val;
}

/* Get an 8 bit value from memory */
static uint8_t get_mem_8(rv32i_t *cpu, uint32_t addr) {
  ASSERT(cpu, "NULL passed as cpu to get_mem_8");
  ASSERT(cpu->regions, "cpu with no memory regions passed to get_mem_8");
  region_t *r = cpu->regions;
  while (r) {
    if (r->addr <= addr && r->addr + r->size >= addr) {
      return r->mem[addr - r->addr];
    }
    r = (region_t *)(r->next);
  }
  return 0;
}
/* Get a 16 bit value from memory */
static uint16_t get_mem_16(rv32i_t *cpu, uint32_t addr) {
  return (uint16_t)get_mem_8(cpu, addr)
      | (((uint16_t)get_mem_8(cpu, addr+1)) << 8);
}
/* Get a 32 bit value from memory */
static uint32_t get_mem_32(rv32i_t *cpu, uint32_t addr) {
  return (uint32_t)get_mem_8(cpu, addr)
      | (((uint32_t)get_mem_8(cpu, addr+1)) << 8)
      | (((uint32_t)get_mem_8(cpu, addr+2)) << 16)
      | (((uint32_t)get_mem_8(cpu, addr+3)) << 24);
}

/* Set an 8 bit value to memory */
static void set_mem_8(rv32i_t *cpu, uint32_t addr, uint8_t val) {
  ASSERT(cpu, "NULL passed as cpu to set_mem_8");
  ASSERT(cpu->regions, "cpu with no memory regions passed to set_mem_8");
  region_t *r = cpu->regions;
  while (r) {
    if (r->addr <= addr && r->addr + r->size >= addr) {
      r->mem[addr - r->addr] = val;
    }
    r = (region_t *)(r->next);
  }
}
/* Set a 16 bit value to memory */
static void set_mem_16(rv32i_t *cpu, uint32_t addr, uint16_t val) {
  set_mem_8(cpu, addr, (uint8_t)(val & 0xff));
  set_mem_8(cpu, addr + 1, (uint8_t)((val >> 8) & 0xff));
}
/* Set a 32 bit value to memory */
static void set_mem_32(rv32i_t *cpu, uint32_t addr, uint32_t val) {
  set_mem_8(cpu, addr, (uint8_t)(val & 0xff));
  set_mem_8(cpu, addr + 1, (uint8_t)((val >> 8) & 0xff));
  set_mem_8(cpu, addr + 2, (uint8_t)((val >> 16) & 0xff));
  set_mem_8(cpu, addr + 3, (uint8_t)((val >> 24) & 0xff));
}

/* Dump the state to the console */
static void dump_state(rv32i_t *cpu) {
  ASSERT(cpu, "NULL passed as cpu to dump_state");
  printf("processor state:\n\tpc = 0x%08x\n\n\tx0 = 0x00000000\n", cpu->pc);
  for (uint8_t i = 1; i < 32; i++)
    printf("\tx%d = 0x%08x\n", i, get_reg(cpu, i));
  if (cpu->regions) {
    region_t *r = cpu->regions;
    while (r) {
      printf(
          "memory region:\n"
          "\taddress = 0x%08x\n"
          "\tsize = 0x%08x\n"
          "\tallocation = %p\n",
          r->addr, r->size, r->mem
      );
      r = (region_t *)(r->next);
    }
  }
}

/* Add a memory region */
static void add_region(
    rv32i_t *cpu, uint32_t addr, uint32_t size, uint8_t *mem
) {
  ASSERT(cpu, "NULL passed as cpu to add_region");
  region_t *region = (region_t *)malloc(sizeof(region_t));
  ASSERT(region, "Failed to allocate space for region structure in add_region");
  region->addr = addr;
  region->size = size;
  region->mem = mem;
  region->next = NULL;
  if (cpu->regions) {
    region_t *r = cpu->regions;
    while (r->next) r = (region_t *)(r->next);
    r->next = (void *)region;
    return;
  }
  cpu->regions = region;
}

/* Reset the processor */
static void reset(rv32i_t *cpu) {
  ASSERT(cpu, "NULL passed as cpu to reset");
  for (uint8_t i = 1; i < 32; i++) set_reg(cpu, i, 0);
  cpu->pc = 0;
  printf("reset\n");
  dump_state(cpu);
}

/* Step the processor */
static void step(rv32i_t *cpu) {
  ASSERT(cpu, "NULL passed as cpu to step");
  
  /* Fetch */
  uint32_t instr = get_mem_32(cpu, cpu->pc);

  /* Decode */
  uint8_t opcode = instr & 0x7f;
  uint8_t rd = (instr >> 7) & 0x1f;
  uint8_t rs1 = (instr >> 15) & 0x1f;
  uint8_t rs2 = (instr >> 20) & 0x1f;
  uint8_t funct3 = (instr >> 12) & 0x7;
  uint8_t funct7 = (instr >> 25) & 0x7f;
  uint32_t i_imm = ((instr >> 20) & 0x7ff) | ((instr >> 31)*0xfffff800);
  uint32_t s_imm = ((instr >> 7) & 0x1f)
      | ((instr >> 25) & 0x7f)
      | ((instr >> 31)*0xfffff000);
  uint32_t b_imm = ((instr >> 7) & 0x1e)
      | ((instr << 4) & 0x800)
      | ((instr >> 31)*0xffffe000);
  uint32_t u_imm = (instr & 0xfffff000);
  uint32_t j_imm = (instr & 0xff000)
      | ((instr >> 9) & 0x800)
      | ((instr >> 20) & 0x7fe)
      | ((instr >> 31)*0xfff00000);

  /* NOTE/TODO: For jumps and branches, incpc = false */
  /* NOTE/TODO: For jumps and branches, ialign should be checked */

  /* Execute */
  bool incpc = true;
  switch (opcode) {
    case 0x37: /* LUI */
      break;
    case 0x17: /* AUIPC */
      break;
    case 0x6f: /* JAL */
      break;
    case 0x67:
      if (funct3 == 0) { /* JALR */
      } break;
    case 0x63: /* Conditional branch */
      switch (funct3) {
        case 0: /* BEQ */
          break;
        case 1: /* BNE */
          break;
        case 4: /* BLT */
          break;
        case 5: /* BGE */
          break;
        case 6: /* BLTU */
          break;
        case 7: /* BGEU */
          break;
      };
      break;
    case 0x03: /* Load */
      switch (funct3) {
        case 0: /* LB */
          break;
        case 1: /* LH */
          break;
        case 2: /* LW */
          break;
        case 4: /* LBU */
          break;
        case 5: /* LHU */
          break;
      };
      break;
    case 0x23: /* Store */
      switch (funct3) {
        case 0: /* SB */
          break;
        case 1: /* SH */
          break;
        case 2: /* SW */
          break;
      };
      break;
    case 0x13: /* Arithmetic rs1, imm -> rd */
      switch (funct3) {
        case 0: /* ADDI */
          break;
        case 2: /* SLTI */
          break;
        case 3: /* SLTIU */
          break;
        case 4: /* XORI */
          break;
        case 6: /* ORI */
          break;
        case 7: /* ANDI */
          break;
        case 1: /* SLLI*/
          break;
        case 5: /* Right shift */
          switch (funct7) {
            case 0: /* SRLI */
              break;
            case 0x20: /* SRAI */
              break;
          } break;
      };
      break;
    case 0x33: /* Arithmetic rs1, rs2 -> rd */
      switch (funct3) {
        case 0: /* Add/subtract */
          switch (funct7) {
            case 0: /* ADD */
              break;
            case 0x20: /* SUB */
              break;
          };
          break;
        case 1: /* SLL */
          break;
        case 2: /* SLT */
          break;
        case 3: /* SLTU */
          break;
        case 4: /* XOR */
          break;
        case 5: /* Right shift */
          switch (funct7) {
            case 0: /* SRL */
              break;
            case 0x20: /* SRA */
              break;
          };
          break;
        case 6: /* OR */
          break;
        case 7: /* AND */
          break;
      };
      break;
    case 0x0f: {/* Fence */
      uint32_t fm = (instr >> 28) & 0xf;
      uint32_t pred = (instr >> 24) & 0xf;
      uint32_t succ = (instr >> 20) & 0xf;
      /* TODO: FENCE, FENCE.TSO, or PAUSE */
      } break;
    case 0x73: /* Environment */
      /* TODO: ECALL or EBREAK */
      break;
  };

  /* Increment PC */
  if (incpc) cpu->pc += 4;
}

/* Initialise the processor structure */
static void init(rv32i_t *cpu) {
  ASSERT(cpu, "NULL passed as cpu to init");
  cpu->regions = NULL;
}
/* Free all resources used by the processor structure */
static void destroy(rv32i_t *cpu) {
  ASSERT(cpu, "NULL passed as cpu to destroy");
  region_t *r = cpu->regions;
  if (!r) return;
  while (r->next) {
    region_t *next = (region_t *)(r->next);
    free(r);
    r = next;
  }
}

#endif /* RVEM_H */
