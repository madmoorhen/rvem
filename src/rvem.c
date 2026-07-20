/* Implements rvem.h */
#include <rvem.h>

/*
 * TESTED:
 * - addi
 * - slti
 * - sltiu 
 * - xori 
 * - ori 
 * - andi 
 * - slli 
 * - srli 
 * - srai 
 * - lui
 * - auipc
 * IMPLEMENTED:
 */

/* Includes */
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
void rv32i_dump_mem(rv32i_t *cpu, uint32_t addr, uint32_t size) {
  ASSERT(cpu, "NULL passed as cpu to rv32i_dump_mem");
  printf("memory (%d bytes, starting at 0x%08x):\n", size, addr);
  for (uint32_t i = 0; i < size; i++)
    printf(
      "0x%02x%c", rv32i_getb(cpu, addr+i), i % 4 == 3 ? '\n' : ' '
    );
  if (size % 4 != 0) printf("\n");
}

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
uint8_t rv32i_getb(rv32i_t *cpu, uint32_t addr) {
  ASSERT(cpu, "NULL passed as cpu to rv32i_getb");
  ASSERT(cpu->regions, "rv32i_getb called on cpu with no regions");
  memory_region_t *r = cpu->regions;
  while (r) {
    if (r->addr <= addr && r->addr + r->size > addr)
      return r->data[addr - r->addr];
    r = r->next;
  }
  printf("rv32i_getb called on addr 0x%08x, which isn't mapped", addr);
  return 0;
}
/* Get a half word from memory */
uint16_t rv32i_geth(rv32i_t *cpu, uint32_t addr) {
  return (uint16_t)rv32i_getb(cpu, addr)
      | (((uint16_t)rv32i_getb(cpu, addr+1)) << 8);
}
/* Get a word from memory */
uint32_t rv32i_getw(rv32i_t *cpu, uint32_t addr) {
  return (uint32_t)rv32i_getb(cpu, addr)
      | (((uint32_t)rv32i_getb(cpu, addr+1)) << 8)
      | (((uint32_t)rv32i_getb(cpu, addr+2)) << 16)
      | (((uint32_t)rv32i_getb(cpu, addr+3)) << 24);
}
/* Set a byte to memory */
void rv32i_setb(rv32i_t *cpu, uint32_t addr, uint8_t val) {
  ASSERT(cpu, "NULL passed as cpu to rv32i_setb");
  ASSERT(cpu->regions, "rv32i_setb called on cpu with no regions");
  memory_region_t *r = cpu->regions;
  while (r) {
    if (r->addr <= addr && r->addr + r->size > addr) {
      r->data[addr - r->addr] = val;
      return;
    }
    r = r->next;
  }
  printf("rv32i_setb called on addr 0x%08x, which isn't mapped", addr);
}
/* Set a half word to memory */
void rv32i_seth(rv32i_t *cpu, uint32_t addr, uint16_t val) {
  rv32i_setb(cpu, addr, (uint8_t)(val & 0xff));
  rv32i_setb(cpu, addr+1, (uint8_t)((val >> 8) & 0xff));
}
/* Set a word to memory */
void rv32i_setw(rv32i_t *cpu, uint32_t addr, uint32_t val) {
  rv32i_setb(cpu, addr, (uint8_t)(val & 0xff));
  rv32i_setb(cpu, addr+1, (uint8_t)((val >> 8) & 0xff));
  rv32i_setb(cpu, addr+2, (uint8_t)((val >> 16) & 0xff));
  rv32i_setb(cpu, addr+3, (uint8_t)((val >> 24) & 0xff));
}

/* Reset the processor */
void rv32i_reset(rv32i_t *cpu) {
  ASSERT(cpu, "NULL passed as cpu to rv32i_reset");
  for (uint8_t i = 0; i < 32; i++) rv32i_set_reg(cpu, i, 0);
  cpu->pc = 0;
  printf("reset ocurred\n");
}
/* Step the processor */
void rv32i_step(rv32i_t *cpu, bool verbose) {
  ASSERT(cpu, "NULL passed as cpu to rv32i_step");

  /* Fetch */
  uint32_t instr = rv32i_getw(cpu, cpu->pc);

  /* Decode */
  uint8_t opcode = instr & 0x7f;
  uint8_t rd = (instr >> 7) & 0x1f;
  uint8_t rs1 = (instr >> 15) & 0x1f;
  uint8_t rs2 = (instr >> 20) & 0x1f;
  uint8_t funct3 = (instr >> 12) & 0x7;
  uint8_t funct7 = (instr >> 25) & 0x7f;
  uint32_t i_imm = ((instr >> 20) & 0x7ff) | ((instr >> 31)*0xfffff800);
  uint32_t s_imm = ((instr >> 7) & 0x1f)
      | ((instr >> 20) & 0x7e0)
      | ((instr >> 31)*0xfffff800);
  uint32_t b_imm = ((instr >> 7) & 0x3e)
      | ((instr >> 20) & 0x7e0)
      | ((instr << 4) & 0x800)
      | ((instr >> 31)*0xfffff000);
  uint32_t u_imm = (instr & 0xfffff000);
  uint32_t j_imm = (instr & 0xff000)
      | ((instr >> 9) & 0x800)
      | ((instr >> 20) & 0x7fe)
      | ((instr >> 31)*0xfff00000);

  /* TODO: set incpc false for successful branches and jumps */
  /* TODO: verbose output on successful branches and jumps */

  /* Unrecognized instructions */
#define UNRECOGNISED do {\
  printf("Unrecognised instruction!\n");\
  cpu->pc += 4;\
  return;\
} while (0)

  /* Execute */
  bool incpc = true;
  switch (opcode) {
    case 0x37: /* LUI */
      rv32i_set_reg(cpu, rd, u_imm);
      if (verbose) printf("lui x%d, 0x%08x\n", rd, u_imm);
      break;
    case 0x17: /* AUIPC */
      rv32i_set_reg(cpu, rd, u_imm + cpu->pc);
      if (verbose) printf("auipc x%d, 0x%08x\n", rd, u_imm);
      break;
    case 0x13: { /* Arithmetic with immediate */
      const char *mneumonic = NULL;
      bool shift = false;
      uint32_t rs1_val = rv32i_get_reg(cpu, rs1);
      uint8_t shamt = i_imm & 0x1f;
      uint32_t res = 0;
      switch (funct3) {
        case 0:
          mneumonic = "add";
          res = rs1_val + i_imm;
          break;
        case 2:
          mneumonic = "slti";
          res = signedw(rs1_val) < signedw(i_imm);
          break;
        case 3:
          mneumonic = "sltiu";
          res = rs1_val < i_imm;
          break;
        case 4:
          mneumonic = "xori";
          res = rs1_val ^ i_imm;
          break;
        case 6:
          mneumonic = "ori";
          res = rs1_val | i_imm;
          break;
        case 7:
          mneumonic = "andi";
          res = rs1_val & i_imm;
          break;
        case 1:
          mneumonic = "slli";
          res = rs1_val << shamt;
          shift = true;
          break;
        case 5:
          shift = true;
          switch (funct7) {
            case 0:
              mneumonic = "srli";
              res = rs1_val >> shamt;
              break;
            case 0x20:
              mneumonic = "srai";
              res = (uint32_t)(signedw(rs1_val) >> shamt);
              break;
            default: UNRECOGNISED; break;
          };
          break;
        default: UNRECOGNISED; break;
      };
      rv32i_set_reg(cpu, rd, res);
      if (verbose) printf(
            shift ? "%s x%d, x%d, 0x%05x\n" : "%s x%d, x%d, 0x%08x\n",
            mneumonic, rd, rs1, shift ? rs2 : i_imm
        );
    } break;
    case 0x33: {
      const char *mneumonic = NULL;
      uint32_t rs1_val = rv32i_get_reg(cpu, rs1);
      uint32_t rs2_val = rv32i_get_reg(cpu, rs2);
      uint8_t shamt = rs2_val & 0x1f;
      uint32_t res = 0;
      switch (funct3) {
        /* TODO */
        default: UNRECOGNISED; break;
      };
      rv32i_set_reg(cpu, rd, res);
      if (verbose) printf("%s x%d, x%d, x%d", mneumonic, rd, rs1, rs2);
    } break;
    default: UNRECOGNISED; break;
  };

  /* Increment program counter */
  if (incpc) cpu->pc += 4;
}
