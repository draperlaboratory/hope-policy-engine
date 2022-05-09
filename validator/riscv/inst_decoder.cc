/*
 * Copyright © 2017-2018 Dover Microsystems, Inc.
 * All rights reserved. 
 *
 * Use and disclosure subject to the following license. 
 *
 * Permission is hereby granted, free of charge, to any person obtaining
 * a copy of this software and associated documentation files (the
 * "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, sublicense, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject to
 * the following conditions:
 * 
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE
 * LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION
 * OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
 * WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

#include <cstdint>
#include <string>
#include <unordered_map>
#include "inst_decoder.h"
#include "platform_types.h"
#include "riscv_isa.h"

namespace policy_engine {

static decoded_instruction_t inst(const std::string& name, uint32_t op, uint32_t flags) { return decoded_instruction_t{
  .name=name,
  .op=op,
  .rd=-1,
  .rs1=-1,
  .rs2=-1,
  .rs3=-1,
  .imm=0,
  .flags=flags
}; }

static decoded_instruction_t inst_rd(const std::string& name, uint32_t op, int rd, uint32_t flags) { return decoded_instruction_t{
  .name=name,
  .op=op,
  .rd=rd,
  .rs1=-1,
  .rs2=-1,
  .rs3=-1,
  .imm=0,
  .flags=flags
}; }

static decoded_instruction_t inst_rd_rs1(const std::string& name, uint32_t op, int rd, int rs1, uint32_t flags) { return decoded_instruction_t{
  .name=name,
  .op=op,
  .rd=rd,
  .rs1=rs1,
  .rs2=-1,
  .rs3=-1,
  .imm=0,
  .flags=flags
}; }

static decoded_instruction_t inst_rs1_rs2(const std::string& name, uint32_t op, int rs1, int rs2, uint32_t flags) { return decoded_instruction_t{
  .name=name,
  .op=op,
  .rd=-1,
  .rs1=rs1,
  .rs2=rs2,
  .rs3=-1,
  .imm=0,
  .flags=flags
}; }

static decoded_instruction_t inst_rd_rs1_imm(const std::string& name, uint32_t op, int rd, int rs1, int imm, uint32_t flags) { return decoded_instruction_t{
  .name=name,
  .op=op,
  .rd=rd,
  .rs1=rs1,
  .rs2=-1,
  .rs3=-1,
  .imm=imm,
  .flags=flags
}; }

static decoded_instruction_t inst_rs1_rs2_imm(const std::string& name, uint32_t op, int rs1, int rs2, int imm, uint32_t flags) { return decoded_instruction_t{
  .name=name,
  .op=op,
  .rd=-1,
  .rs1=rs1,
  .rs2=rs2,
  .rs3=-1,
  .imm=imm,
  .flags=flags
}; }

static decoded_instruction_t inst_all_regs(const std::string& name, uint32_t op, int rd, int rs1, int rs2, int rs3, uint32_t flags) { return decoded_instruction_t{
  .name=name,
  .op=op,
  .rd=rd,
  .rs1=rs1,
  .rs2=rs2,
  .rs3=rs3,
  .imm=0,
  .flags=flags
}; }

static const decoded_instruction_t invalid_inst{.name=""};

static decoded_instruction_t r_type_inst(const std::string& name, uint32_t op, int rd, int rs1, int rs2, uint32_t flags=0) { return decoded_instruction_t{
  .name=name,
  .op=op,
  .rd=rd,
  .rs1=rs1,
  .rs2=rs2,
  .rs3=-1,
  .imm=0,
  .flags=HAS_RD | HAS_RS1 | HAS_RS2 | flags
}; }

static decoded_instruction_t decode_r_type(uint8_t code, uint8_t f3, uint8_t f7, int rd, int rs1, int rs2) {
  uint8_t f5 = f7 >> 2;
  switch (code) {
    case 0x33: switch (f3) {
      case 0x0: switch (f7) {
        case 0x00: return r_type_inst("add", RISCV_ADD, rd, rs1, rs2);
        case 0x01: return r_type_inst("mul", RISCV_MUL, rd, rs1, rs2);
        case 0x20: return r_type_inst("sub", RISCV_SUB, rd, rs1, rs2);
        default: return invalid_inst;
      }
      case 0x1: switch (f7) {
        case 0x00: return r_type_inst("sll", RISCV_SLL, rd, rs1, rs2);
        case 0x01: return r_type_inst("mulh", RISCV_MULH, rd, rs1, rs2);
        default: return invalid_inst;
      }
      case 0x2: switch (f7) {
        case 0x00: return r_type_inst("slt", RISCV_SLT, rd, rs1, rs2);
        case 0x01: return r_type_inst("mulhsu", RISCV_MULHSU, rd, rs1, rs2);
        default: return invalid_inst;
      }
      case 0x3: switch (f7) {
        case 0x00: return r_type_inst("sltu", RISCV_SLTU, rd, rs1, rs2);
        case 0x01: return r_type_inst("mulhu", RISCV_MULHU, rd, rs1, rs2);
        default: return invalid_inst;
      }
      case 0x4: switch (f7) {
        case 0x00: return r_type_inst("xor", RISCV_XOR, rd, rs1, rs2);
        case 0x01: return r_type_inst("div", RISCV_DIV, rd, rs1, rs2);
        default: return invalid_inst;
      }
      case 0x5: switch (f7) {
        case 0x00: return r_type_inst("srl", RISCV_SRL, rd, rs1, rs2);
        case 0x01: return r_type_inst("divu", RISCV_DIVU, rd, rs1, rs2);
        case 0x20: return r_type_inst("sra", RISCV_SRA, rd, rs1, rs2);
        default: return invalid_inst;
      }
      case 0x6: switch (f7) {
        case 0x00: return r_type_inst("or", RISCV_OR, rd, rs1, rs2);
        case 0x01: return r_type_inst("rem", RISCV_REM, rd, rs1, rs2);
        default: return invalid_inst;
      }
      case 0x7: switch (f7) {
        case 0x00: return r_type_inst("and", RISCV_AND, rd, rs1, rs2);
        case 0x01: return r_type_inst("remu", RISCV_REMU, rd, rs1, rs2);
        default: return invalid_inst;
      }
      default: return invalid_inst;
    }
    case 0x3b: switch (f3) {
      case 0x0: switch (f7) {
        case 0x00: return r_type_inst("addw", RISCV_ADDW, rd, rs1, rs2);
        case 0x01: return r_type_inst("mulw", RISCV_MULW, rd, rs1, rs2);
        case 0x20: return r_type_inst("subw", RISCV_SUBW, rd, rs1, rs2);
        default: return invalid_inst;
      }
      case 0x1: return r_type_inst("sllw", RISCV_SLLW, rd, rs1, rs2);
      case 0x4: switch (f7) {
        case 0x01: return r_type_inst("divw", RISCV_DIVW, rd, rs1, rs2);
        default: return invalid_inst;
      }
      case 0x5: switch (f7) {
        case 0x00: return r_type_inst("srlw", RISCV_ADDW, rd, rs1, rs2);
        case 0x01: return r_type_inst("divuw", RISCV_DIVUW, rd, rs1, rs2);
        case 0x20: return r_type_inst("sraw", RISCV_SRAW, rd, rs1, rs2);
        default: return invalid_inst;
      }
      case 0x6: switch (f7) {
        case 0x01: return r_type_inst("remw", RISCV_REMW, rd, rs1, rs2);
        default: return invalid_inst;
      }
      default: return invalid_inst;
    }
    case 0x53: switch (f7) {
      case 0x00: return r_type_inst("fadd.s", RISCV_FADD_S, rd, rs1, rs2);
      case 0x01: return r_type_inst("fadd.d", RISCV_FADD_D, rd, rs1, rs2);
      case 0x03: return r_type_inst("fadd.q", RISCV_FADD_Q, rd, rs1, rs2);
      case 0x04: return r_type_inst("fsub.s", RISCV_FSUB_S, rd, rs1, rs2);
      case 0x05: return r_type_inst("fsub.d", RISCV_FSUB_D, rd, rs1, rs2);
      case 0x07: return r_type_inst("fsub.q", RISCV_FSUB_Q, rd, rs1, rs2);
      case 0x08: return r_type_inst("fmul.s", RISCV_FMUL_S, rd, rs1, rs2);
      case 0x09: return r_type_inst("fmul.d", RISCV_FMUL_D, rd, rs1, rs2);
      case 0x0b: return r_type_inst("fmul.q", RISCV_FMUL_Q, rd, rs1, rs2);
      case 0x0c: return r_type_inst("fdiv.s", RISCV_FDIV_S, rd, rs1, rs2);
      case 0x0d: return r_type_inst("fdiv.d", RISCV_FDIV_D, rd, rs1, rs2);
      case 0x0f: return r_type_inst("fdiv.q", RISCV_FDIV_Q, rd, rs1, rs2);
      case 0x10: switch (f3) {
        case 0x0: return r_type_inst("fsgnj.s", RISCV_FSGNJ_S, rd, rs1, rs2);
        case 0x1: return r_type_inst("fsgnjn.s", RISCV_FSGNJN_S, rd, rs1, rs2);
        case 0x2: return r_type_inst("fsgnjx.s", RISCV_FSGNJX_S, rd, rs1, rs2);
        default: return invalid_inst;
      }
      case 0x11: switch (f3) {
        case 0x0: return r_type_inst("fsgnj.d", RISCV_FSGNJ_D, rd, rs1, rs2);
        case 0x1: return r_type_inst("fsgnjn.d", RISCV_FSGNJN_D, rd, rs1, rs2);
        case 0x2: return r_type_inst("fsgnjx.d", RISCV_FSGNJX_D, rd, rs1, rs2);
        default: return invalid_inst;
      }
      case 0x13: switch (f3) {
        case 0x0: return r_type_inst("fsgnj.q", RISCV_FSGNJ_Q, rd, rs1, rs2);
        case 0x1: return r_type_inst("fsgnjn.q", RISCV_FSGNJN_Q, rd, rs1, rs2);
        case 0x2: return r_type_inst("fsgnjx.q", RISCV_FSGNJX_Q, rd, rs1, rs2);
        default: return invalid_inst;
      }
      case 0x14: switch (f3) {
        case 0x0: return r_type_inst("fmin.s", RISCV_FMIN_S, rd, rs1, rs2);
        case 0x1: return r_type_inst("fmax.s", RISCV_FMAX_S, rd, rs1, rs2);
        default: return invalid_inst;
      }
      case 0x15: switch (f3) {
        case 0x0: return r_type_inst("fmin.d", RISCV_FMIN_D, rd, rs1, rs2);
        case 0x1: return r_type_inst("fmax.d", RISCV_FMAX_D, rd, rs1, rs2);
        default: return invalid_inst;
      }
      case 0x17: switch (f3) {
        case 0x0: return r_type_inst("fmin.q", RISCV_FMIN_Q, rd, rs1, rs2);
        case 0x1: return r_type_inst("fmax.q", RISCV_FMAX_Q, rd, rs1, rs2);
        default: return invalid_inst;
      }
      case 0x50: switch (f3) {
        case 0x0: return r_type_inst("fle.s", RISCV_FLE_S, rd, rs1, rs2);
        case 0x1: return r_type_inst("flt.s", RISCV_FLT_S, rd, rs1, rs2);
        case 0x2: return r_type_inst("feq.s", RISCV_FEQ_S, rd, rs1, rs2);
        default: return invalid_inst;
      }
      case 0x51: switch (f3) {
        case 0x0: return r_type_inst("fle.d", RISCV_FLE_D, rd, rs1, rs2);
        case 0x1: return r_type_inst("flt.d", RISCV_FLT_D, rd, rs1, rs2);
        case 0x2: return r_type_inst("feq.d", RISCV_FEQ_D, rd, rs1, rs2);
        default: return invalid_inst;
      }
      case 0x53: switch (f3) {
        case 0x0: return r_type_inst("fle.q", RISCV_FLE_Q, rd, rs1, rs2);
        case 0x1: return r_type_inst("flt.q", RISCV_FLT_Q, rd, rs1, rs2);
        case 0x2: return r_type_inst("feq.q", RISCV_FEQ_Q, rd, rs1, rs2);
        default: return invalid_inst;
      }
      default: return invalid_inst;
    }
    case 0x2f: switch (f5) {
      case 0x00: switch (f3) {
        case 0x2: return r_type_inst("amoadd.w", RISCV_AMOADD_W, rd, rs1, rs2);
        case 0x3: // amoadd.d
        default: return invalid_inst;
      }
      case 0x01: switch (f3) {
        case 0x2: return r_type_inst("amoswap.w", RISCV_AMOSWAP_W, rd, rs1, rs2);
        case 0x3: // amoswap.d
        default: return invalid_inst;
      }
      case 0x03: switch (f3) {
        case 0x2: return r_type_inst("sc.w", RISCV_SC_W, rd, rs1, rs2);
        case 0x3: // sc.d
        default: return invalid_inst;
      }
      case 0x04: switch (f3) {
        case 0x2: return r_type_inst("amoxor.w", RISCV_AMOXOR_W, rd, rs1, rs2);
        case 0x3: // amoxor.d
        default: return invalid_inst;
      }
      case 0x08: switch (f3) {
        case 0x2: return r_type_inst("amoor.w", RISCV_AMOOR_W, rd, rs1, rs2);
        case 0x3: // amoor.d
        default: return invalid_inst;
      }
      case 0x0c: switch (f3) {
        case 0x2: return r_type_inst("amoand.w", RISCV_AMOAND_W, rd, rs1, rs2);
        case 0x3: // amoand.d
        default: return invalid_inst;
      }
      case 0x10: switch (f3) {
        case 0x2: return r_type_inst("amomin.w", RISCV_AMOMIN_W, rd, rs1, rs2);
        case 0x3: // amomin.d
        default: return invalid_inst;
      }
      case 0x14: switch (f3) {
        case 0x2: return r_type_inst("amomax.w", RISCV_AMOMAX_W, rd, rs1, rs2);
        case 0x3: // amomax.d
        default: return invalid_inst;
      }
      case 0x18: switch (f3) {
        case 0x2: return r_type_inst("amominu.w", RISCV_AMOMINU_W, rd, rs1, rs2);
        case 0x3: // amominu.d
        default: return invalid_inst;
      }
      case 0x1c: switch (f3) {
        case 0x2: return r_type_inst("amomaxu.w", RISCV_AMOMAXU_W, rd, rs1, rs2);
        case 0x3: // amomaxu.d
        default: return invalid_inst;
      }
      default: return invalid_inst;
    }
    default: return invalid_inst;
  }
}

decoded_instruction_t decode(insn_bits_t bits) {
  uint8_t opcode = bits & 0x7f;
  uint8_t f3 = (bits & 0x7000) >> 12;
  uint8_t f7 = (bits & 0xfe000000) >> 25;
  int rd = (bits & 0xf80) >> 7;
  int rs1 = (bits & 0xf8000) >> 15;
  int rs2 = (bits & 0x1f00000) >> 20;
  int rs3 = (bits & 0xf8000000) >> 27;

  decoded_instruction_t r = decode_r_type(opcode, f3, f7, rd, rs1, rs2);
  if (r)
    return r;

  switch (bits & 0x7f) {
    case 0x6f: return inst_rd("jal", RISCV_JAL, rd, HAS_RD);
    case 0x37: return inst_rd("lui", RISCV_LUI, rd, HAS_RD);
    case 0x17: return inst_rd("auipc", RISCV_AUIPC, rd, HAS_RD);
  }
  switch (bits & 0x707f) {
    case 0x0063: return inst_rs1_rs2("beq", RISCV_BEQ, rs1, rs2, HAS_RS1 | HAS_RS2);
    case 0x1063: return inst_rs1_rs2("bne", RISCV_BNE, rs1, rs2, HAS_RS1 | HAS_RS2);
    case 0x4063: return inst_rs1_rs2("blt", RISCV_BLT, rs1, rs2, HAS_RS1 | HAS_RS2);
    case 0x5063: return inst_rs1_rs2("bge", RISCV_BGE, rs1, rs2, HAS_RS1 | HAS_RS2);
    case 0x6063: return inst_rs1_rs2("bltu", RISCV_BLTU, rs1, rs2, HAS_RS1 | HAS_RS2);
    case 0x7063: return inst_rs1_rs2("bgeu", RISCV_BGEU, rs1, rs2, HAS_RS1 | HAS_RS2);
    case 0x0067: return inst_rd_rs1_imm(
      "jalr", RISCV_JALR, rd, rs1,
      (bits & 0x80000000) ? ((bits & 0xFFF00000) >> 20) | 0xFFFFF000 : (bits & 0xFFF00000) >> 20,
      HAS_RD | HAS_RS1 | HAS_IMM
    );
    case 0x0013: return inst_rd_rs1_imm(
      "addi", RISCV_ADDI, rd, rs1,
      (bits & 0x80000000) ? ((bits & 0xFFF00000) >> 20) | 0xFFFFF000 : (bits & 0xFFF00000) >> 20,
      HAS_RD | HAS_RS1 | HAS_IMM
    );
    case 0x2013: return inst_rd_rs1_imm(
      "slti", RISCV_SLTI, rd, rs1,
      (bits & 0x80000000) ? ((bits & 0xFFF00000) >> 20) | 0xFFFFF000 : (bits & 0xFFF00000) >> 20,
      HAS_RD | HAS_RS1 | HAS_IMM
    );
    case 0x3013: return inst_rd_rs1_imm(
      "sltiu", RISCV_SLTIU, rd, rs1,
      (bits & 0x80000000) ? ((bits & 0xFFF00000) >> 20) | 0xFFFFF000 : (bits & 0xFFF00000) >> 20,
      HAS_RD | HAS_RS1 | HAS_IMM
    );
    case 0x4013: return inst_rd_rs1_imm(
      "xori", RISCV_XORI, rd, rs1,
      (bits & 0x80000000) ? ((bits & 0xFFF00000) >> 20) | 0xFFFFF000 : (bits & 0xFFF00000) >> 20,
      HAS_RD | HAS_RS1 | HAS_IMM
    );
    case 0x6013: return inst_rd_rs1_imm(
      "ori", RISCV_ORI, rd, rs1,
      (bits & 0x80000000) ? ((bits & 0xFFF00000) >> 20) | 0xFFFFF000 : (bits & 0xFFF00000) >> 20,
      HAS_RD | HAS_RS1 | HAS_IMM
    );
    case 0x7013: return inst_rd_rs1_imm(
      "andi", RISCV_ANDI, rd, rs1,
      (bits & 0x80000000) ? ((bits & 0xFFF00000) >> 20) | 0xFFFFF000 : (bits & 0xFFF00000) >> 20,
      HAS_RD | HAS_RS1 | HAS_IMM
    );
    case 0x001b: return inst_rd_rs1_imm(
      "addiw", RISCV_ADDIW, rd, rs1,
      (bits & 0x80000000) ? ((bits & 0xFFF00000) >> 20) | 0xFFFFF000 : (bits & 0xFFF00000) >> 20,
      HAS_RD | HAS_RS1 | HAS_IMM
    );
    case 0x0003: return inst_rd_rs1_imm(
      "lb", RISCV_LB, rd, rs1,
      (bits & 0x80000000) ? ((bits & 0xFFF00000) >> 20) | 0xFFFFF000 : (bits & 0xFFF00000) >> 20,
      HAS_RD | HAS_RS1 | HAS_IMM | HAS_LOAD
    );
    case 0x1003: return inst_rd_rs1_imm(
      "lh", RISCV_LH, rd, rs1,
      (bits & 0x80000000) ? ((bits & 0xFFF00000) >> 20) | 0xFFFFF000 : (bits & 0xFFF00000) >> 20,
      HAS_RD | HAS_RS1 | HAS_IMM | HAS_LOAD
    );
    case 0x2003: return inst_rd_rs1_imm(
      "lw", RISCV_LW, rd, rs1,
      (bits & 0x80000000) ? ((bits & 0xFFF00000) >> 20) | 0xFFFFF000 : (bits & 0xFFF00000) >> 20,
      HAS_RD | HAS_RS1 | HAS_IMM | HAS_LOAD
    );
    case 0x3003: return inst_rd_rs1_imm(
      "ld", RISCV_LD, rd, rs1,
      (bits & 0x80000000) ? ((bits & 0xFFF00000) >> 20) | 0xFFFFF000 : (bits & 0xFFF00000) >> 20,
      HAS_RD | HAS_RS1 | HAS_IMM | HAS_LOAD
    );
    case 0x4003: return inst_rd_rs1_imm(
      "lbu", RISCV_LBU, rd, rs1,
      (bits & 0x80000000) ? ((bits & 0xFFF00000) >> 20) | 0xFFFFF000 : (bits & 0xFFF00000) >> 20,
      HAS_RD | HAS_RS1 | HAS_IMM | HAS_LOAD
    );
    case 0x5003: return inst_rd_rs1_imm(
      "lhu", RISCV_LHU, rd, rs1,
      (bits & 0x80000000) ? ((bits & 0xFFF00000) >> 20) | 0xFFFFF000 : (bits & 0xFFF00000) >> 20,
      HAS_RD | HAS_RS1 | HAS_IMM | HAS_LOAD
    );
    case 0x6003: return inst_rd_rs1_imm(
      "lwu", RISCV_LWU, rd, rs1,
      (bits & 0x80000000) ? ((bits & 0xFFF00000) >> 20) | 0xFFFFF000 : (bits & 0xFFF00000) >> 20,
      HAS_RD | HAS_RS1 | HAS_IMM | HAS_LOAD
    );
    case 0x0023: return inst_rs1_rs2_imm(
      "sb", RISCV_SB, rs1, rs2,
      ((bits & 0x80000000) ? ((bits & 0xFE000000) >> 20) | 0xFFFFF000 : (bits & 0xFE000000) >> 20) | ((bits & 0x00000F80) >> 7),
      HAS_RS1 | HAS_RS2 | HAS_IMM | HAS_STORE
    );
    case 0x1023: return inst_rs1_rs2_imm(
      "sh", RISCV_SH, rs1, rs2,
      ((bits & 0x80000000) ? ((bits & 0xFE000000) >> 20) | 0xFFFFF000 : (bits & 0xFE000000) >> 20) | ((bits & 0x00000F80) >> 7),
      HAS_RS1 | HAS_RS2 | HAS_IMM | HAS_STORE
    );
    case 0x2023: return inst_rs1_rs2_imm(
      "sw", RISCV_SW, rs1, rs2,
      ((bits & 0x80000000) ? ((bits & 0xFE000000) >> 20) | 0xFFFFF000 : (bits & 0xFE000000) >> 20) | ((bits & 0x00000F80) >> 7),
      HAS_RS1 | HAS_RS2 | HAS_IMM | HAS_STORE
    );
    case 0x3023: return inst_rs1_rs2_imm(
      "sd", RISCV_SD, rs1, rs2,
      ((bits & 0x80000000) ? ((bits & 0xFE000000) >> 20) | 0xFFFFF000 : (bits & 0xFE000000) >> 20) | ((bits & 0x00000F80) >> 7),
      HAS_RS1 | HAS_RS2 | HAS_IMM | HAS_STORE
    );
    case 0x000f: return inst("fence", RISCV_FENCE, 0);
    case 0x100f: return inst("fence.i", RISCV_FENCE_I, 0);
    case 0x1073: return inst_rd_rs1_imm(
      "csrrw", RISCV_CSRRW, rd, rs1,
      (bits & 0xfff00000) >> 20,
      HAS_RS1 | HAS_IMM | (rd ? HAS_RD | HAS_CSR_LOAD | HAS_CSR_STORE : HAS_CSR_STORE)
    );
    case 0x2073: return inst_rd_rs1_imm(
      "csrrs", RISCV_CSRRS, rd, rs1,
      (bits & 0xfff00000) >> 20,
      HAS_RD | HAS_RS1 | HAS_IMM | (rs1 ? HAS_CSR_LOAD | HAS_CSR_STORE : HAS_CSR_LOAD)
    );
    case 0x3073: return inst_rd_rs1_imm(
      "csrrc", RISCV_CSRRC, rd, rs1,
      (bits & 0xfff00000) >> 20,
      HAS_RD | HAS_RS1 | HAS_RS2 | (rs1 ? HAS_CSR_LOAD | HAS_CSR_STORE : HAS_CSR_LOAD)
    );
    case 0x5073: return inst_rd_rs1_imm(
      "csrrwi", RISCV_CSRRWI, rd, rs1,
      (bits & 0xfff00000) >> 20,
      HAS_RS1 | HAS_IMM | (rd ? HAS_RD | HAS_CSR_LOAD | HAS_CSR_STORE : HAS_CSR_STORE)
    );
    case 0x6073: return inst_rd_rs1_imm(
      "csrrsi", RISCV_CSRRSI, rd, rs1,
      (bits & 0xfff00000) >> 20,
      HAS_RD | HAS_RS1 | HAS_IMM | (rs1 ? HAS_CSR_LOAD | HAS_CSR_STORE : HAS_CSR_LOAD)
    );
    case 0x7073: return inst_rd_rs1_imm(
      "csrrci", RISCV_CSRRCI, rd, rs1,
      (bits & 0xfff00000) >> 20,
      HAS_RD | HAS_RS1 | HAS_IMM | (rs1 ? HAS_CSR_LOAD | HAS_CSR_STORE : HAS_CSR_LOAD)
    );
    case 0x2007: return inst_rd_rs1_imm(
      "flw", RISCV_FLW, rd, rs1,
      (bits & 0x80000000) ? ((bits & 0xFFF00000) >> 20) | 0xFFFFF000 : (bits & 0xFFF00000) >> 20,
      HAS_RD | HAS_RS1 | HAS_IMM | HAS_LOAD
    );
    case 0x3007: return inst_rd_rs1_imm(
      "fld", RISCV_FLD, rd, rs1,
      (bits & 0x80000000) ? ((bits & 0xFFF00000) >> 20) | 0xFFFFF000 : (bits & 0xFFF00000) >> 20,
      HAS_RD | HAS_RS1 | HAS_IMM | HAS_LOAD
    );
    case 0x4007: return inst_rd_rs1_imm(
      "flq", RISCV_FLQ, rd, rs1,
      (bits & 0x80000000) ? ((bits & 0xFFF00000) >> 20) | 0xFFFFF000 : (bits & 0xFFF00000) >> 20,
      HAS_RD | HAS_RS1 | HAS_IMM | HAS_LOAD
    );
    case 0x2027: return inst_rs1_rs2_imm(
      "fsw", RISCV_FSW, rs1, rs2,
      ((bits & 0x80000000) ? ((bits & 0xFE000000) >> 20) | 0xFFFFF000 : (bits & 0xFE000000) >> 20) | ((bits & 0x00000F80) >> 7),
      HAS_RS1 | HAS_RS2 | HAS_IMM | HAS_STORE
    );
    case 0x3027: return inst_rs1_rs2_imm(
      "fsd", RISCV_FSD, rs1, rs2,
      ((bits & 0x80000000) ? ((bits & 0xFE000000) >> 20) | 0xFFFFF000 : (bits & 0xFE000000) >> 20) | ((bits & 0x00000F80) >> 7),
      HAS_RS1 | HAS_RS2 | HAS_IMM | HAS_STORE
    );
    case 0x4027: return inst_rs1_rs2_imm(
      "fsq", RISCV_FSQ, rs1, rs2,
      ((bits & 0x80000000) ? ((bits & 0xFE000000) >> 20) | 0xFFFFF000 : (bits & 0xFE000000) >> 20) | ((bits & 0x00000F80) >> 7),
      HAS_RS1 | HAS_RS2 | HAS_IMM | HAS_STORE
    );
  }
  switch (bits & 0x600007f) {
    case 0x0000043: return inst_all_regs("fmadd.s", RISCV_FMADD_S, rd, rs1, rs2, rs3, HAS_RD | HAS_RS1 | HAS_RS2 | HAS_RS3);
    case 0x0000047: return inst_all_regs("fmsub.s", RISCV_FMSUB_S, rd, rs1, rs2, rs3, HAS_RD | HAS_RS1 | HAS_RS2 | HAS_RS3);
    case 0x000004b: return inst_all_regs("fnmsub.s", RISCV_FNMSUB_S, rd, rs1, rs2, rs3, HAS_RD | HAS_RS1 | HAS_RS2 | HAS_RS3);
    case 0x000004f: return inst_all_regs("fnmadd.s", RISCV_FNMADD_S, rd, rs1, rs2, rs3, HAS_RD | HAS_RS1 | HAS_RS2 | HAS_RS3);
    case 0x2000043: return inst_all_regs("fmadd.d", RISCV_FMADD_D, rd, rs1, rs2, rs3, HAS_RD | HAS_RS1 | HAS_RS2 | HAS_RS3);
    case 0x2000047: return inst_all_regs("fmsub.d", RISCV_FMSUB_D, rd, rs1, rs2, rs3, HAS_RD | HAS_RS1 | HAS_RS2 | HAS_RS3);
    case 0x200004b: return inst_all_regs("fnmsub.d", RISCV_FNMSUB_D, rd, rs1, rs2, rs3, HAS_RD | HAS_RS1 | HAS_RS2 | HAS_RS3);
    case 0x200004f: return inst_all_regs("fnmadd.d", RISCV_FNMADD_D, rd, rs1, rs2, rs3, HAS_RD | HAS_RS1 | HAS_RS2 | HAS_RS3);
    case 0x6000043: return inst_all_regs("fmadd.q", RISCV_FMADD_Q, rd, rs1, rs2, rs3, HAS_RD | HAS_RS1 | HAS_RS2 | HAS_RS3);
    case 0x6000047: return inst_all_regs("fmsub.q", RISCV_FMSUB_Q, rd, rs1, rs2, rs3, HAS_RD | HAS_RS1 | HAS_RS2 | HAS_RS3);
    case 0x600004b: return inst_all_regs("fnmsub.q", RISCV_FNMSUB_Q, rd, rs1, rs2, rs3, HAS_RD | HAS_RS1 | HAS_RS2 | HAS_RS3);
    case 0x600004f: return inst_all_regs("fnmadd.q", RISCV_FNMADD_Q, rd, rs1, rs2, rs3, HAS_RD | HAS_RS1 | HAS_RS2 | HAS_RS3);
  }
  switch (bits & 0xf800707f) {
    case 0x1000202f: return inst_rd_rs1("lr.w", RISCV_LR_W, rd, rs1, HAS_RD | HAS_RS1 | HAS_LOAD);
  }
  switch (bits & 0xfc00707f) {
    case 0x00001013: return inst_rd_rs1("slli", RISCV_SLLI, rd, rs1, HAS_RD | HAS_RS1);
    case 0x00005013: return inst_rd_rs1("srli", RISCV_SRLI, rd, rs1, HAS_RD | HAS_RS1);
    case 0x40005013: return inst_rd_rs1("srai", RISCV_SRAI, rd, rs1, HAS_RD | HAS_RS1);
  }
  switch (bits & 0xfe00007f) {
    
  }
  switch (bits & 0xfe00707f) {
    case 0x0000101b: return inst_rd_rs1("slliw", RISCV_SLLIW, rd, rs1, HAS_RD | HAS_RS1);
    case 0x0000501b: return inst_rd_rs1("srliw", RISCV_SRLIW, rd, rs1, HAS_RD | HAS_RS1);
    case 0x4000501b: return inst_rd_rs1("sraiw", RISCV_SRAIW, rd, rs1, HAS_RD | HAS_RS1);
  }
  switch (bits & 0xfe007fff) {
    case 0x12000073: return inst_rs1_rs2("sfence.vma", RISCV_SFENCE_VMA, rs1, rs2, HAS_RS1 | HAS_RS2);
  }
  switch (bits & 0xfff0007f) {
    case 0x58000053: return inst_rd_rs1("fsqrt.s", RISCV_FSQRT_S, rd, rs1, HAS_RD | HAS_RS1);
    case 0x40100053: return inst_rd_rs1("fcvt.s.d", RISCV_FCVT_S_D, rd, rs1, HAS_RD | HAS_RS1);
    case 0x40200053: return inst_rd_rs1("fcvt.d.s", RISCV_FCVT_D_S, rd, rs1, HAS_RD | HAS_RS1);
    case 0x5a000053: return inst_rd_rs1("fsqrt.d", RISCV_FSQRT_D, rd, rs1, HAS_RD | HAS_RS1);
    case 0x40300053: return inst_rd_rs1("fcvt.s.q", RISCV_FCVT_S_Q, rd, rs1, HAS_RD | HAS_RS1);
    case 0x46000053: return inst_rd_rs1("fcvt.q.s", RISCV_FCVT_Q_S, rd, rs1, HAS_RD | HAS_RS1);
    case 0x42300053: return inst_rd_rs1("fcvt.d.q", RISCV_FCVT_D_Q, rd, rs1, HAS_RD | HAS_RS1);
    case 0x46100053: return inst_rd_rs1("fcvt.q.d", RISCV_FCVT_Q_D, rd, rs1, HAS_RD | HAS_RS1);
    case 0x5e000053: return inst_rd_rs1("fsqrt.q", RISCV_FSQRT_Q, rd, rs1, HAS_RD | HAS_RS1);
    case 0xc0000053: return inst_rd_rs1("fcvt.w.s", RISCV_FCVT_W_S, rd, rs1, HAS_RD | HAS_RS1);
    case 0xc0100053: return inst_rd_rs1("fcvt.wu.s", RISCV_FCVT_WU_S, rd, rs1, HAS_RD | HAS_RS1);
    case 0xc0200053: return inst_rd_rs1("fcvt.l.s", RISCV_FCVT_L_S, rd, rs1, HAS_RD | HAS_RS1);
    case 0xc0300053: return inst_rd_rs1("fcvt.lu.s", RISCV_FCVT_LU_S, rd, rs1, HAS_RD | HAS_RS1);
    case 0xc2000053: return inst_rd_rs1("fcvt.w.d", RISCV_FCVT_W_D, rd, rs1, HAS_RD | HAS_RS1);
    case 0xc2100053: return inst_rd_rs1("fcvt.wu.d", RISCV_FCVT_WU_D, rd, rs1, HAS_RD | HAS_RS1);
    case 0xc2200053: return inst_rd_rs1("fcvt.l.d", RISCV_FCVT_L_D, rd, rs1, HAS_RD | HAS_RS1);
    case 0xc2300053: return inst_rd_rs1("fcvt.lu.d", RISCV_FCVT_LU_D, rd, rs1, HAS_RD | HAS_RS1);
    case 0xc6000053: return inst_rd_rs1("fcvt.w.q", RISCV_FCVT_W_Q, rd, rs1, HAS_RD | HAS_RS1);
    case 0xc6100053: return inst_rd_rs1("fcvt.wu.q", RISCV_FCVT_WU_Q, rd, rs1, HAS_RD | HAS_RS1);
    case 0xc6200053: return inst_rd_rs1("fcvt.l.q", RISCV_FCVT_L_Q, rd, rs1, HAS_RD | HAS_RS1);
    case 0xc6300053: return inst_rd_rs1("fcvt.lu.q", RISCV_FCVT_LU_Q, rd, rs1, HAS_RD | HAS_RS1);
    case 0xd0000053: return inst_rd_rs1("fcvt.s.w", RISCV_FCVT_S_W, rd, rs1, HAS_RD | HAS_RS1);
    case 0xd0100053: return inst_rd_rs1("fcvt.s.wu", RISCV_FCVT_S_WU, rd, rs1, HAS_RD | HAS_RS1);
    case 0xd0200053: return inst_rd_rs1("fcvt.s.l", RISCV_FCVT_S_L, rd, rs1, HAS_RD | HAS_RS1);
    case 0xd0300053: return inst_rd_rs1("fcvt.s.lu", RISCV_FCVT_S_LU, rd, rs1, HAS_RD | HAS_RS1);
    case 0xd2000053: return inst_rd_rs1("fcvt.d.w", RISCV_FCVT_D_W, rd, rs1, HAS_RD | HAS_RS1);
    case 0xd2100053: return inst_rd_rs1("fcvt.d.wu", RISCV_FCVT_D_WU, rd, rs1, HAS_RD | HAS_RS1);
    case 0xd2200053: return inst_rd_rs1("fcvt.d.l", RISCV_FCVT_D_L, rd, rs1, HAS_RD | HAS_RS1);
    case 0xd2300053: return inst_rd_rs1("fcvt.d.lu", RISCV_FCVT_D_LU, rd, rs1, HAS_RD | HAS_RS1);
    case 0xd6000053: return inst_rd_rs1("fcvt.q.w", RISCV_FCVT_Q_W, rd, rs1, HAS_RD | HAS_RS1);
    case 0xd6100053: return inst_rd_rs1("fcvt.q.wu", RISCV_FCVT_Q_WU, rd, rs1, HAS_RD | HAS_RS1);
    case 0xd6200053: return inst_rd_rs1("fcvt.q.l", RISCV_FCVT_Q_L, rd, rs1, HAS_RD | HAS_RS1);
    case 0xd6300053: return inst_rd_rs1("fcvt.q.lu", RISCV_FCVT_Q_LU, rd, rs1, HAS_RD | HAS_RS1);
  }
  switch (bits & 0xfff0707f) {
    case 0xe0000053: return inst_rd_rs1("fmv.x.w", RISCV_FMV_X_W, rd, rs1, HAS_RD | HAS_RS1);
    case 0xe0001053: return inst_rd_rs1("fclass.s", RISCV_FCLASS_S, rd, rs1, HAS_RD | HAS_RS1);
    case 0xe2000053: return inst_rd_rs1("fmv.x.d", RISCV_FMV_X_D, rd, rs1, HAS_RD | HAS_RS1);
    case 0xe2001053: return inst_rd_rs1("fclass.d", RISCV_FCLASS_D, rd, rs1, HAS_RD | HAS_RS1);
    case 0xe6000053: return inst_rd_rs1("fmv.x.q", RISCV_FMV_X_Q, rd, rs1, HAS_RD | HAS_RS1);
    case 0xe6001053: return inst_rd_rs1("fclass.q", RISCV_FCLASS_Q, rd, rs1, HAS_RD | HAS_RS1);
    case 0xf0000053: return inst_rd_rs1("fmv.w.x", RISCV_FMV_W_X, rd, rs1, HAS_RD | HAS_RS1);
    case 0xf2000053: return inst_rd_rs1("fmv.d.x", RISCV_FMV_D_X, rd, rs1, HAS_RD | HAS_RS1);
    case 0xf6000053: return inst_rd_rs1("fmv.q.x", RISCV_FMV_Q_X, rd, rs1, HAS_RD | HAS_RS1);
  }
  switch (bits) {
    case 0x73: return inst("ecall", RISCV_ECALL, 0);
    case 0x100073: return inst("ebreak", RISCV_EBREAK, 0);
    case 0x200073: return inst("uret", RISCV_URET, 0);
    case 0x10200073: return inst("sret", RISCV_SRET, 0);
    case 0x30200073: return inst("mret", RISCV_MRET, 0);
    case 0x7b200073: return inst("dret", RISCV_DRET, 0);
    case 0x10500073: return inst("wfi", RISCV_WFI, 0);
  }
  return invalid_inst;
}

} // namespace policy_engine