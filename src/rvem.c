/* Implements rvem.h */
#include <rvem.h>

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
static int64_t signedd(uint64_t val) { return *((int64_t *)(&val)); }
/* Unsigned value from signed (reinterpret) */
static uint32_t unsignedw(int32_t val) { return *((uint32_t *)(&val)); }
static uint64_t unsignedd(int64_t val) { return *((uint64_t *)(&val)); }

/* Initialise the processor */
void rv64i_init(rv64i_t *cpu) {
  ASSERT(cpu, "NULL passed as cpu to rv64i_init");
  cpu->regions = NULL;
}

/* Add a memory region */
void rv64i_add_region(rv64i_t *cpu, memory_region_t *region) {
  ASSERT(cpu, "NULL passed as cpu to rv64i_add_region");
  ASSERT(region, "NULL passed as region to rv64i_add_region");
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
void rv64i_remove_region(rv64i_t *cpu, memory_region_t *region) {
  ASSERT(cpu, "NULL passed as cpu to rv64i_remove_region");
  ASSERT(region, "NULL passed as region to rv64i_remove_region");
  if (!(cpu->regions)) {
    printf("rv64i_remove_region called on cpu with no regions");
    return;
  }
  memory_region_t *r = cpu->regions;
  while (r->next && r->next != region) r = r->next;
  if (!(r->next)) {
    printf("rv64i_remove_region tried to remove a region that doesn't exist");
    return;
  }
  r->next = region->next;
}

/* Dump the processor state to the console */
void rv64i_dump_state(rv64i_t *cpu) {
  ASSERT(cpu, "NULL passed as cpu to rv64i_dump_state");
  printf(
      "processor state:\n"
      "\tpc = 0x%016lx\n\n"
      "\tx0 = 0x0000000000000000\n", cpu->pc
  );
  for (uint8_t i = 1; i < 32; i++)
    printf("\tx%d = 0x%016lx\n", i, rv64i_get_reg(cpu, i));
  if (cpu->regions) {
    memory_region_t *r = cpu->regions;
    while (r) {
      printf(
          "memory region:\n"
          "\taddress = 0x%016lx\n"
          "\tsize = 0x%016lx\n"
          "\tallocation = %p\n",
          r->addr, r->size, r->data
      );
      r = r->next;
    }
  }
}
/* Dump the memory at a location to the console */
void rv64i_dump_mem(rv64i_t *cpu, uint64_t addr, uint64_t size) {
  ASSERT(cpu, "NULL passed as cpu to rv64i_dump_mem");
  printf("memory (%lu bytes, starting at 0x%016lx):\n", size, addr);
  for (uint64_t i = 0; i < size; i++)
    printf(
      "0x%02x%c", rv64i_getb(cpu, addr+i), i % 4 == 3 ? '\n' : ' '
    );
  if (size % 4 != 0) printf("\n");
}

/* Get the value of a register */
uint64_t rv64i_get_reg(rv64i_t *cpu, uint8_t reg) {
  ASSERT(cpu, "NULL passed as cpu to rv64i_get_reg");
  ASSERT(reg < 32, "value over 31 passed as reg to rv64i_get_reg");
  if (reg == 0) return 0;
  return cpu->x[reg-1];
}
/* Set the value of a register */
void rv64i_set_reg(rv64i_t *cpu, uint8_t reg, uint64_t val) {
  ASSERT(cpu, "NULL passed as cpu to rv64i_set_reg");
  ASSERT(reg < 32, "value over 31 passed as reg to rv64i_set_reg");
  if (reg == 0) return;
  cpu->x[reg-1] = val;
}

/* Get a byte from memory */
uint8_t rv64i_getb(rv64i_t *cpu, uint64_t addr) {
  ASSERT(cpu, "NULL passed as cpu to rv64i_getb");
  ASSERT(cpu->regions, "rv64i_getb called on cpu with no regions");
  memory_region_t *r = cpu->regions;
  while (r) {
    if (r->addr <= addr && r->addr + r->size > addr)
      return r->data[addr - r->addr];
    r = r->next;
  }
  printf("rv64i_getb called on addr 0x%016lx, which isn't mapped\n", addr);
  return 0;
}
/* Get a half word from memory */
uint16_t rv64i_geth(rv64i_t *cpu, uint64_t addr) {
  return (uint16_t)rv64i_getb(cpu, addr)
      | (((uint16_t)rv64i_getb(cpu, addr+1)) << 8);
}
/* Get a word from memory */
uint32_t rv64i_getw(rv64i_t *cpu, uint64_t addr) {
  return (uint32_t)rv64i_getb(cpu, addr)
      | (((uint32_t)rv64i_getb(cpu, addr+1)) << 8)
      | (((uint32_t)rv64i_getb(cpu, addr+2)) << 16)
      | (((uint32_t)rv64i_getb(cpu, addr+3)) << 24);
}
/* Get a double word from memory */
uint64_t rv64i_getd(rv64i_t *cpu, uint64_t addr) {
  return (uint64_t)rv64i_getb(cpu, addr)
      | (((uint64_t)rv64i_getb(cpu, addr+1)) << 8)
      | (((uint64_t)rv64i_getb(cpu, addr+2)) << 16)
      | (((uint64_t)rv64i_getb(cpu, addr+3)) << 24)
      | (((uint64_t)rv64i_getb(cpu, addr+4)) << 32)
      | (((uint64_t)rv64i_getb(cpu, addr+5)) << 40)
      | (((uint64_t)rv64i_getb(cpu, addr+6)) << 48)
      | (((uint64_t)rv64i_getb(cpu, addr+7)) << 56);
}
/* Set a byte to memory */
void rv64i_setb(rv64i_t *cpu, uint64_t addr, uint8_t val) {
  ASSERT(cpu, "NULL passed as cpu to rv64i_setb");
  ASSERT(cpu->regions, "rv64i_setb called on cpu with no regions");
  memory_region_t *r = cpu->regions;
  while (r) {
    if (r->addr <= addr && r->addr + r->size > addr) {
      r->data[addr - r->addr] = val;
      return;
    }
    r = r->next;
  }
  printf("rv64i_setb called on addr 0x%016lx, which isn't mapped", addr);
}
/* Set a half word to memory */
void rv64i_seth(rv64i_t *cpu, uint64_t addr, uint16_t val) {
  rv64i_setb(cpu, addr, (uint8_t)(val & 0xff));
  rv64i_setb(cpu, addr+1, (uint8_t)((val >> 8) & 0xff));
}
/* Set a word to memory */
void rv64i_setw(rv64i_t *cpu, uint64_t addr, uint32_t val) {
  rv64i_setb(cpu, addr, (uint8_t)(val & 0xff));
  rv64i_setb(cpu, addr+1, (uint8_t)((val >> 8) & 0xff));
  rv64i_setb(cpu, addr+2, (uint8_t)((val >> 16) & 0xff));
  rv64i_setb(cpu, addr+3, (uint8_t)((val >> 24) & 0xff));
}
/* Set a double word to memory */
void rv64i_setd(rv64i_t *cpu, uint64_t addr, uint64_t val) {
  rv64i_setb(cpu, addr, (uint8_t)(val & 0xff));
  rv64i_setb(cpu, addr+1, (uint8_t)((val >> 8) & 0xff));
  rv64i_setb(cpu, addr+2, (uint8_t)((val >> 16) & 0xff));
  rv64i_setb(cpu, addr+3, (uint8_t)((val >> 24) & 0xff));
  rv64i_setb(cpu, addr+4, (uint8_t)((val >> 32) & 0xff));
  rv64i_setb(cpu, addr+5, (uint8_t)((val >> 40) & 0xff));
  rv64i_setb(cpu, addr+6, (uint8_t)((val >> 48) & 0xff));
  rv64i_setb(cpu, addr+7, (uint8_t)((val >> 56) & 0xff));
}

/* Reset the processor */
void rv64i_reset(rv64i_t *cpu) {
  ASSERT(cpu, "NULL passed as cpu to rv64i_reset");
  for (uint8_t i = 0; i < 32; i++) rv64i_set_reg(cpu, i, 0);
  cpu->pc = 0;
  printf("reset ocurred\n");
}
/* Step the processor */
void rv64i_step(rv64i_t *cpu, bool verbose) {
  ASSERT(cpu, "NULL passed as cpu to rv64i_step");

  /* Fetch */
  uint32_t instr = rv64i_getw(cpu, cpu->pc);

  /* Decode */
  uint8_t opcode = instr & 0x7f;
  uint8_t rd = (instr >> 7) & 0x1f;
  uint8_t rs1 = (instr >> 15) & 0x1f;
  uint8_t rs2 = (instr >> 20) & 0x1f;
  uint8_t funct3 = (instr >> 12) & 0x7;
  uint8_t funct7 = (instr >> 25) & 0x7f;
  uint64_t i_imm = ((instr >> 20) & 0x7ff)
      | ((instr >> 31)*0xfffffffffffff800);
  uint64_t s_imm = ((instr >> 7) & 0x1f)
      | ((instr >> 20) & 0x7e0)
      | ((instr >> 31)*0xfffffffffffff800);
  uint64_t b_imm = ((instr >> 7) & 0x1e)
      | ((instr >> 20) & 0x7e0)
      | ((instr << 4) & 0x800)
      | ((instr >> 31)*0xfffffffffffff000);
  uint64_t u_imm = (instr & 0xfffff000)
      | ((instr >> 31)*0xffffffff00000000);
  uint64_t j_imm = (instr & 0xff000)
      | ((instr >> 9) & 0x800)
      | ((instr >> 20) & 0x7fe)
      | ((instr >> 31)*0xfffffffffff00000);

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
      rv64i_set_reg(cpu, rd, u_imm);
      if (verbose) printf("lui x%d, 0x%016lx\n", rd, u_imm);
      break;
    case 0x17: /* AUIPC */
      rv64i_set_reg(cpu, rd, u_imm + cpu->pc);
      if (verbose) printf("auipc x%d, 0x%016lx\n", rd, u_imm);
      break;
    case 0x6f: /* JAL */
      rv64i_set_reg(cpu, rd, cpu->pc + 4);
      cpu->pc += j_imm;
      incpc = false;
      if (verbose) printf("jal x%d, 0x%016lx\n", rd, j_imm);
      break;
    case 0x67: /* JALR */
      rv64i_set_reg(cpu, rd, cpu->pc + 4);
      cpu->pc = (rv64i_get_reg(cpu, rs1) + i_imm) & 0xfffffffe;
      incpc = false;
      if (verbose) printf("jalr x%d, 0x%016lx(x%d)\n", rd, i_imm, rs1);
      break;
    case 0x63: { /* Conditional branch */
      const char *mneumonic = NULL;
      bool branch = false;
      uint64_t rs1_val = rv64i_get_reg(cpu, rs1);
      uint64_t rs2_val = rv64i_get_reg(cpu, rs2);
      switch (funct3) {
        case 0:
          mneumonic = "beq";
          branch = rs1_val == rs2_val;
          break;
        case 1:
          mneumonic = "bne";
          branch = rs1_val != rs2_val;
          break;
        case 4:
          mneumonic = "blt";
          branch = signedd(rs1_val) < signedd(rs2_val);
          break;
        case 5:
          mneumonic = "bge";
          branch = signedd(rs1_val) >= signedd(rs2_val);
          break;
        case 6:
          mneumonic = "bltu";
          branch = rs1_val < rs2_val;
          break;
        case 7:
          mneumonic = "bgeu";
          branch = rs1_val >= rs2_val;
          break;
        default: UNRECOGNISED; break;
      };
      if (branch) {
        cpu->pc += b_imm;
        incpc = false;
      }
      if (verbose) printf("%s x%d, x%d, 0x%016lx\n", mneumonic, rs1, rs2, b_imm);
    } break;
    case 0x03: { /* Load */
      const char* mneumonic = NULL;
      uint64_t addr = i_imm + rv64i_get_reg(cpu, rs1);
      uint64_t res = 0;
      switch (funct3) {
        case 0:
          mneumonic = "lb";
          res = rv64i_getb(cpu, addr);
          res |= (res >> 7)*0xffffffffffffff00;
          break;
        case 1:
          mneumonic = "lh";
          res = rv64i_geth(cpu, addr);
          res |= (res >> 15)*0xffffffffffff0000;
          break;
        case 2:
          mneumonic = "lw";
          res = rv64i_getw(cpu, addr);
          res |= (res >> 31)*0xffffffff00000000;
          break;
        case 3:
          mneumonic = "ld";
          res = rv64i_getd(cpu, addr);
          break;
        case 4:
          mneumonic = "lbu";
          res = rv64i_getb(cpu, addr);
          break;
        case 5:
          mneumonic = "lhu";
          res = rv64i_geth(cpu, addr);
          break;
        case 6:
          mneumonic = "lwu";
          res = rv64i_getw(cpu, addr);
          break;
        default: UNRECOGNISED; break;
      };
      rv64i_set_reg(cpu, rd, res);
      if (verbose) printf("%s x%d, 0x%016lx(x%d)\n", mneumonic, rd, i_imm, rs1);
    } break;
    case 0x23: { /* Store */
      const char *mneumonic = NULL;
      uint32_t addr = s_imm + rv64i_get_reg(cpu, rs1);
      uint32_t rs2_val = rv64i_get_reg(cpu, rs2);
      switch (funct3) {
        case 0:
          mneumonic = "sb";
          rv64i_setb(cpu, addr, (uint8_t)(rs2_val & 0xff));
          break;
        case 1:
          mneumonic = "sh";
          rv64i_seth(cpu, addr, (uint16_t)(rs2_val & 0xffff));
          break;
        case 2:
          mneumonic = "sw";
          rv64i_setw(cpu, addr, (uint32_t)(rs2_val & 0xffffffff));
          break;
        case 3:
          mneumonic = "sd";
          rv64i_setd(cpu, addr, rs2_val);
          break;
        default: UNRECOGNISED; break;
      };
      if (verbose) printf("%s x%d, 0x%016lx(x%d)\n", mneumonic, rs2, s_imm, rs1);
    } break;
    case 0x13: { /* Arithmetic with immediate */
      const char *mneumonic = NULL;
      bool shift = false;
      uint64_t rs1_val = rv64i_get_reg(cpu, rs1);
      uint8_t shamt = i_imm & 0x3f;
      uint64_t res = 0;
      switch (funct3) {
        case 0:
          mneumonic = "addi";
          res = rs1_val + i_imm;
          break;
        case 1:
          mneumonic = "slli";
          res = rs1_val << shamt;
          shift = true;
          break;
        case 2:
          mneumonic = "slti";
          res = signedd(rs1_val) < signedd(i_imm);
          break;
        case 3:
          mneumonic = "sltiu";
          res = rs1_val < i_imm;
          break;
        case 4:
          mneumonic = "xori";
          res = rs1_val ^ i_imm;
          break;
        case 5:
          shift = true;
          switch (funct7 & 0x7e) {
            case 0:
              mneumonic = "srli";
              res = rs1_val >> shamt;
              break;
            case 0x20:
              mneumonic = "srai";
              res = unsignedd(signedd(rs1_val) >> shamt);
              break;
            default: UNRECOGNISED; break;
          }; break;
        case 6:
          mneumonic = "ori";
          res = rs1_val | i_imm;
          break;
        case 7:
          mneumonic = "andi";
          res = rs1_val & i_imm;
          break;
        default: UNRECOGNISED; break;
      };
      rv64i_set_reg(cpu, rd, res);
      if (verbose) printf(
            shift ? "%s x%d, x%d, 0x%02lx\n" : "%s x%d, x%d, 0x%016lx\n",
            mneumonic, rd, rs1, shift ? shamt : i_imm
        );
    } break;
    case 0x1b: { /* Arithmetic with immediate (word) */
      const char *mneumonic = NULL;
      bool shift = false;
      uint32_t rs1_val = (uint32_t)(rv64i_get_reg(cpu, rs1)&0xffffffff);
      uint32_t i_imm_word = (uint32_t)(i_imm&0xffffffff);
      uint8_t shamt = i_imm & 0x1f;
      uint32_t res = 0;
      switch (funct3) {
        case 0:
          mneumonic = "addiw";
          res = rs1_val + i_imm_word;
          break;
        case 1:
          mneumonic = "slliw";
          res = rs1_val << shamt;
          break;
        case 5:
          shift = true;
          switch (funct7) {
            case 0:
              mneumonic = "srliw";
              res = rs1_val >> shamt;
              break;
            case 0x20:
              mneumonic = "sraiw";
              res = unsignedw(signedw(rs1_val) >> shamt);
              break;
            default: UNRECOGNISED; break;
          }; break;
        default: UNRECOGNISED; break;
      };
      rv64i_set_reg(
          cpu, rd,
          ((uint64_t)res)|((((uint64_t)res)>>31)*0xffffffff00000000)
      );
      if (verbose) printf(
            shift ? "%s x%d, x%d, 0x%02x\n" : "%s x%d, x%d, 0x%08x\n",
            mneumonic, rd, rs1, shift ? shamt : i_imm_word
        );
    } break;
    case 0x33: { /* Arithmetic with registers */
      /* TODO: 64 everyting, add 64-specific */
      const char *mneumonic = NULL;
      uint32_t rs1_val = rv64i_get_reg(cpu, rs1);
      uint32_t rs2_val = rv64i_get_reg(cpu, rs2);
      uint8_t shamt = rs2_val & 0x1f;
      uint32_t res = 0;
      switch (funct3) {
        case 0:
          switch (funct7) {
            case 0:
              mneumonic = "add";
              res = rs1_val + rs2_val;
              break;
            case 0x20:
              mneumonic = "sub";
              res = rs1_val + ~rs2_val + 1;
              break;
            default: UNRECOGNISED; break;
          }; break;
        case 1:
          switch (funct7) {
            case 0:
              mneumonic = "sll";
              res = rs1_val << shamt;
              break;
            default: UNRECOGNISED; break;
          }; break;
        case 2:
          switch (funct7) {
            case 0:
              mneumonic = "slt";
              res = signedw(rs1_val) < signedw(rs2_val);
              break;
            default: UNRECOGNISED; break;
          }; break;
        case 3:
          switch (funct7) {
            case 0:
              mneumonic = "sltu";
              res = rs1_val < rs2_val;
              break;
            default: UNRECOGNISED; break;
          }; break;
        case 4:
          switch (funct7) {
            case 0:
              mneumonic = "xor";
              res = rs1_val ^ rs2_val;
              break;
            default: UNRECOGNISED; break;
          }; break;
        case 5:
          switch (funct7) {
            case 0:
              mneumonic = "srl";
              res = rs1_val >> shamt;
              break;
            case 0x20:
              mneumonic = "sra";
              res = unsignedw(signedw(rs1_val) >> shamt);
              break;
            default: UNRECOGNISED; break;
          }; break;
        case 6:
          switch (funct7) {
            case 0:
              mneumonic = "or";
              res = rs1_val | rs2_val;
              break;
            default: UNRECOGNISED; break;
          }; break;
        case 7:
          switch (funct7) {
            case 0:
              mneumonic = "and";
              res = rs1_val & rs2_val;
              break;
            default: UNRECOGNISED; break;
          }; break;
        default: UNRECOGNISED; break;
      };
      rv64i_set_reg(cpu, rd, res);
      if (verbose) printf("%s x%d, x%d, x%d\n", mneumonic, rd, rs1, rs2);
    } break;
    case 0x0f: /* Fence */
      switch (funct3) {
        case 0:
          switch (instr) {
            case 0x8330000f: /* fence.tso */
              if (verbose) printf("fence.tso\n");
              break;
            case 0x0100000f: /* pause */
              if (verbose) printf("pause\n");
              break;
            default: /* fence */
              if (verbose) printf(
                    "fence %s%s%s%s, %s%s%s%s\n",
                    instr & 0x08000000 ? "i" : "",
                    instr & 0x04000000 ? "o" : "",
                    instr & 0x02000000 ? "r" : "",
                    instr & 0x01000000 ? "w" : "",
                    instr & 0x00800000 ? "i" : "",
                    instr & 0x00400000 ? "o" : "",
                    instr & 0x00200000 ? "r" : "",
                    instr & 0x00100000 ? "w" : ""
                );
              break;
          };
          break;
        case 1:
          if (verbose) printf("fence.i\n");
          break;
        default: UNRECOGNISED; break;
      };
      break;
    case 0x73: /* System instructions */
      switch (instr) {
        case 0x00000073:
          if (verbose) printf("ecall\n");
          break;
        case 0x00100073:
          if (verbose) printf("ebreak\n");
          break;
        default: UNRECOGNISED; break;
      };
      break;
    default: UNRECOGNISED; break;
  };

  /* Increment program counter */
  if (incpc) cpu->pc += 4;
}
