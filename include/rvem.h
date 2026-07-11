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

/* Signed value from unsigned */
static int32_t signed32(uint32_t val) { return *((int32_t *)(&val)); }

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

  /* Execute */
  bool incpc = true;
  switch (opcode) {
    case 0x37: /* LUI */
      set_reg(cpu, rd, u_imm);
      break;
    case 0x17: /* AUIPC */
      set_reg(cpu, rd, u_imm + cpu->pc);
      break;
    case 0x6f: /* JAL */
      incpc = false;
      set_reg(cpu, rd, cpu->pc + 4);
      cpu->pc += j_imm;
      break;
    case 0x67:
      if (funct3 == 0) { /* JALR */
        incpc = false;
        set_reg(cpu, rd, cpu->pc + 4);
        cpu->pc = (i_imm + get_reg(cpu, rs1)) & 0xfffffffe;
      } else printf("Invalid instruction!");
      break;
    case 0x63: {/* Conditional branch */
      bool branch;
      switch (funct3) {
        case 0: /* BEQ */
          branch = get_reg(cpu, rs1) == get_reg(cpu, rs2);
          break;
        case 1: /* BNE */
          branch = get_reg(cpu, rs1) != get_reg(cpu, rs2);
          break;
        case 4: /* BLT */
          branch = signed32(get_reg(cpu, rs1)) < signed32(get_reg(cpu, rs2));
          break;
        case 5: /* BGE */
          branch = signed32(get_reg(cpu, rs1)) >= signed32(get_reg(cpu, rs2));
          break;
        case 6: /* BLTU */
          branch = get_reg(cpu, rs1) < get_reg(cpu, rs2);
          break;
        case 7: /* BGEU */
          branch = get_reg(cpu, rs1) >= get_reg(cpu, rs2);
          break;
        default:
          printf("Invalid instruction!");
          break;
      };
      if (branch) {
        cpu->pc += b_imm;
        incpc = false;
      }
      } break;
    case 0x03: { /* Load */
      uint32_t addr = get_reg(cpu, rs1) + i_imm;
      switch (funct3) {
        case 0: { /* LB */
          uint8_t val = get_mem_8(cpu, addr);
          set_reg(cpu, rd, val | (0xffffff00*(val>>7)));
          } break;
        case 1: { /* LH */
          uint16_t val = get_mem_16(cpu, addr);
          set_reg(cpu, rd, val | (0xffff0000*(val>>15)));
          } break;
        case 2: /* LW */
          set_reg(cpu, rd, get_mem_32(cpu, addr));
          break;
        case 4: /* LBU */
          set_reg(cpu, rd, get_mem_8(cpu, addr));
          break;
        case 5: /* LHU */
          set_reg(cpu, rd, get_mem_16(cpu, addr));
          break;
        default:
          printf("Invalid instruction!");
          break;
      };
      } break;
    case 0x23: {/* Store */
      uint32_t addr = get_reg(cpu, rs1) + i_imm;
      switch (funct3) {
        case 0: /* SB */
          set_mem_8(cpu, addr, (uint8_t)(get_reg(cpu, rs2)&0xff));
          break;
        case 1: /* SH */
          set_mem_16(cpu, addr, (uint16_t)(get_reg(cpu, rs2)&0xffff));
          break;
        case 2: /* SW */
          set_mem_32(cpu, addr, get_reg(cpu, rs2));
          break;
        default:
          printf("Invalid instruction!");
          break;
      };
      } break;
    case 0x13: /* Arithmetic rs1, imm -> rd */
      switch (funct3) {
        case 0: /* ADDI */
          set_reg(cpu, rd, get_reg(cpu, rs1) + i_imm);
          break;
        case 2: /* SLTI */
          set_reg(cpu, rd, signed32(get_reg(cpu, rs1)) < signed32(i_imm));
          break;
        case 3: /* SLTIU */
          set_reg(cpu, rd, get_reg(cpu, rs1) < i_imm);
          break;
        case 4: /* XORI */
          set_reg(cpu, rd, get_reg(cpu, rs1) ^ i_imm);
          break;
        case 6: /* ORI */
          set_reg(cpu, rd, get_reg(cpu, rs1) | i_imm);
          break;
        case 7: /* ANDI */
          set_reg(cpu, rd, get_reg(cpu, rs1) & i_imm);
          break;
        case 1: /* SLLI*/
          set_reg(cpu, rd, get_reg(cpu, rs1) << rs2);
          break;
        case 5: /* Right shift */
          switch (funct7) {
            case 0: /* SRLI */
              set_reg(cpu, rd, get_reg(cpu, rs1) >> rs2);
              break;
            case 0x20: /* SRAI */
              set_reg(
                  cpu, rd,
                  (get_reg(cpu, rs1) >> rs2) | (get_reg(cpu, rs1) & 0x80000000)
              );
              break;
            default:
              printf("Invalid instruction!");
              break;
          } break;
        default:
          printf("Invalid instruction!");
          break;
      };
      break;
    case 0x33: /* Arithmetic rs1, rs2 -> rd */
      switch (funct3) {
        case 0: /* Add/subtract */
          switch (funct7) {
            case 0: /* ADD */
              set_reg(cpu, rd, get_reg(cpu, rs1) + get_reg(cpu, rs2));
              break;
            case 0x20: /* SUB */
              set_reg(cpu, rd, get_reg(cpu, rs1) + ~(get_reg(cpu, rs2)) + 1);
              break;
            default:
              printf("Invalid instruction!");
              break;
          };
          break;
        case 1: /* SLL */
          set_reg(cpu, rd, get_reg(cpu, rs1) << (get_reg(cpu, rs2) & 0x1f));
          break;
        case 2: /* SLT */
          set_reg(
              cpu, rd,
              signed32(get_reg(cpu, rs1)) < signed32(get_reg(cpu, rs2))
          );
          break;
        case 3: /* SLTU */
          set_reg(cpu, rd, get_reg(cpu, rs1) < get_reg(cpu, rs2));
          break;
        case 4: /* XOR */
          set_reg(cpu, rd, get_reg(cpu, rs1) ^ get_reg(cpu, rs2));
          break;
        case 5: /* Right shift */
          switch (funct7) {
            case 0: /* SRL */
              set_reg(cpu, rd, get_reg(cpu, rs1) << (get_reg(cpu, rs2) & 0x1f));
              break;
            case 0x20: /* SRA */
              set_reg(
                  cpu, rd,
                  (get_reg(cpu, rs1) << (get_reg(cpu, rs2) & 0x1f))
                    | (get_reg(cpu, rs1) & 0x80000000)
              );
              break;
            default:
              printf("Invalid instruction!");
              break;
          };
          break;
        case 6: /* OR */
          set_reg(cpu, rd, get_reg(cpu, rs1) | get_reg(cpu, rs2));
          break;
        case 7: /* AND */
          set_reg(cpu, rd, get_reg(cpu, rs1) & get_reg(cpu, rs2));
          break;
        default:
          printf("Invalid instruction!");
          break;
      };
      break;
    case 0x0f: {/* Fence */
      uint32_t fm = (instr >> 28) & 0xf;
      uint32_t pred = (instr >> 24) & 0xf;
      uint32_t succ = (instr >> 20) & 0xf;
      /* NOTE: FENCE here, but acts as NOP (ordering already guaranteed) */
      /* NOTE: FENCE.TSO here, but acts as NOP (ordering already guaranteed) */
      /* NOTE: PAUSE here, but acts as NOP (HINT) */
      } break;
    case 0x73: /* Environment */
      /* TODO: ECALL and EBREAK */
      break;
    default:
      printf("Invalid instruction!");
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
