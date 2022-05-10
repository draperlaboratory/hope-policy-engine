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

static const decoded_instruction_t invalid_inst{.name=""};

static decoded_instruction_t r_type_inst(const std::string& name, uint32_t op, int rd, int rs1, int rs2, flags_t flags=0) { return decoded_instruction_t{
  .name=name,
  .op=op,
  .rd=rd,
  .rs1=rs1,
  .rs2=rs2,
  .rs3=-1,
  .imm=0,
  .flags=HAS_RD | HAS_RS1 | HAS_RS2 | flags
}; }

static decoded_instruction_t r4_type_inst(const std::string& name, uint32_t op, int rd, int rs1, int rs2, int rs3, flags_t flags=0) { return decoded_instruction_t {
  .name=name,
  .op=op,
  .rd=rd,
  .rs1=rs1,
  .rs2=rs2,
  .rs3=rs3,
  .imm=0,
  .flags=HAS_RD | HAS_RS1 | HAS_RS2 | HAS_RS3 | flags
}; }

static decoded_instruction_t fp_conv_inst(const std::string& name, uint32_t op, int rd, int rs1, flags_t flags=0) { return decoded_instruction_t{
  .name=name,
  .op=op,
  .rd=rd,
  .rs1=rs1,
  .rs2=-1,
  .rs3=-1,
  .imm=0,
  .flags=HAS_RD | HAS_RS1 | flags
}; }

static decoded_instruction_t i_type_inst(const std::string& name, uint32_t op, int rd, int rs1, int imm, flags_t flags=0) { return decoded_instruction_t{
  .name=name,
  .op=op,
  .rd=rd,
  .rs1=rs1,
  .rs2=-1,
  .rs3=-1,
  .imm=imm,
  .flags=HAS_RD | HAS_RS1 | HAS_IMM | flags
}; }

static decoded_instruction_t csr_inst(const std::string& name, uint32_t op, int rd, int rs1, uint16_t csr) { return decoded_instruction_t{
  .name=name,
  .op=op,
  .rd=rd,
  .rs1=rs1,
  .rs2=-1,
  .rs3=-1,
  .imm=csr,
  .flags=(rd ? HAS_RD : 0) | (rs1 >= 0 ? HAS_RS1 : 0) | HAS_IMM | (rd ? (HAS_CSR_LOAD | HAS_CSR_STORE) : HAS_CSR_STORE)
}; }

static decoded_instruction_t s_type_inst(const std::string& name, uint32_t op, int rs1, int rs2, int imm, flags_t flags=0) { return decoded_instruction_t{
  .name=name,
  .op=op,
  .rd=-1,
  .rs1=rs1,
  .rs2=rs2,
  .rs3=-1,
  .imm=imm,
  .flags=HAS_RS1 | HAS_RS2 | HAS_IMM | flags
}; }

static decoded_instruction_t u_type_inst(const std::string& name, uint32_t op, int rd, int imm, flags_t flags=0) { return decoded_instruction_t {
  .name=name,
  .op=op,
  .rd=rd,
  .rs1=-1,
  .rs2=-1,
  .rs3=-1,
  .imm=imm,
  .flags=HAS_RD | HAS_IMM | flags
}; }

static decoded_instruction_t decode_r_type(uint8_t code, uint8_t f3, uint8_t f7, int rd, int rs1, int rs2) {
  uint8_t f5 = f7 >> 2;
  uint8_t fmt = f7 & 0x3;
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

static decoded_instruction_t decode_i_type(uint8_t code, uint8_t f3, int rd, int rs1, int imm) {
  // decode as if for RV64; for RV32, MSB of shamt should always be 0
  uint8_t shamt = imm & 0x3f;
  uint8_t f6 = static_cast<unsigned int>(imm) >> 6;
  uint16_t csr = static_cast<uint16_t>(imm) & 0xfff;
  switch (code) {
    case 0x03: switch (f3) {
      case 0x0: return i_type_inst("lb", RISCV_LB, rd, rs1, imm, HAS_LOAD);
      case 0x1: return i_type_inst("lh", RISCV_LH, rd, rs1, imm, HAS_LOAD);
      case 0x2: return i_type_inst("lw", RISCV_LW, rd, rs1, imm, HAS_LOAD);
      case 0x3: return i_type_inst("ld", RISCV_LD, rd, rs1, imm, HAS_LOAD);
      case 0x4: return i_type_inst("lbu", RISCV_LBU, rd, rs1, imm, HAS_LOAD);
      case 0x5: return i_type_inst("lhu", RISCV_LHU, rd, rs1, imm, HAS_LOAD);
      case 0x6: return i_type_inst("lwu", RISCV_LWU, rd, rs1, imm, HAS_LOAD);
      default: return invalid_inst;
    }
    case 0x07: switch (f3) {
      case 0x2: return i_type_inst("flw", RISCV_FLW, rd, rs1, imm, HAS_LOAD);
      case 0x3: return i_type_inst("fld", RISCV_FLD, rd, rs1, imm, HAS_LOAD);
      case 0x4: return i_type_inst("flq", RISCV_FLQ, rd, rs1, imm, HAS_LOAD);
      default: return invalid_inst;
    }
    case 0x13: switch (f3) {
      case 0x0: return i_type_inst("addi", RISCV_ADDI, rd, rs1, imm);
      case 0x1: return i_type_inst("slli", RISCV_SLLI, rd, rs1, shamt);
      case 0x2: return i_type_inst("slti", RISCV_SLTI, rd, rs1, imm);
      case 0x3: return i_type_inst("sltiu", RISCV_SLTIU, rd, rs1, imm);
      case 0x4: return i_type_inst("xori", RISCV_XORI, rd, rs1, imm);
      case 0x5: switch (f6) {
        case 0x00: return i_type_inst("srli", RISCV_SRLI, rd, rs1, shamt);
        case 0x10: return i_type_inst("srai", RISCV_SRAI, rd, rs1, shamt);
        default: return invalid_inst;
      }
      case 0x6: return i_type_inst("ori", RISCV_ORI, rd, rs1, imm);
      case 0x7: return i_type_inst("andi", RISCV_ANDI, rd, rs1, imm);
      default: return invalid_inst;
    }
    case 0x1b: switch (f3) {
      case 0x0: return i_type_inst("addiw", RISCV_ADDIW, rd, rs1, imm);
      case 0x1: return i_type_inst("slliw", RISCV_SLLIW, rd, rs1, shamt);
      case 0x5: switch (f6) {
        case 0x00: return i_type_inst("srliw", RISCV_SRLIW, rd, rs1, shamt);
        case 0x10: return i_type_inst("sraiw", RISCV_SRAIW, rd, rs1, shamt);
        default: return invalid_inst;
      }
      default: return invalid_inst;
    }
    case 0x67: return i_type_inst("jalr", RISCV_JALR, rd, rs1, imm);
    case 0x73: switch (f3) {
      case 0x1: return csr_inst("csrrw", RISCV_CSRRW, rd, rs1, csr);
      case 0x2: return csr_inst("csrrs", RISCV_CSRRS, rd, rs1, csr);
      case 0x3: return csr_inst("csrrc", RISCV_CSRRC, rd, rs1, csr);
      case 0x5: return csr_inst("csrrwi", RISCV_CSRRWI, rd, -1, csr);
      case 0x6: return csr_inst("csrrsi", RISCV_CSRRSI, rd, -1, csr);
      case 0x7: return csr_inst("csrrci", RISCV_CSRRCI, rd, -1, csr);
      default: return invalid_inst;
    }
    default: return invalid_inst;
  }
}

static decoded_instruction_t decode_s_type(uint8_t code, uint8_t f3, int rs1, int rs2, int s_imm) {
  int b_imm = ((s_imm & 0x1) << 11) | (s_imm & 0x7fe) | ((s_imm & 0x800) ? ~0x7ff : 0);
  switch (code) {
    case 0x23: switch (f3) {
      case 0x0: return s_type_inst("sb", RISCV_SB, rs1, rs2, s_imm, HAS_STORE);
      case 0x1: return s_type_inst("sh", RISCV_SH, rs1, rs2, s_imm, HAS_STORE);
      case 0x2: return s_type_inst("sw", RISCV_SW, rs1, rs2, s_imm, HAS_STORE);
      case 0x3: return s_type_inst("sd", RISCV_SD, rs1, rs2, s_imm, HAS_STORE);
      default: return invalid_inst;
    }
    case 0x27: switch (f3) {
      case 0x2: return s_type_inst("fsw", RISCV_FSW, rs1, rs2, s_imm, HAS_STORE);
      case 0x3: return s_type_inst("fsd", RISCV_FSD, rs1, rs2, s_imm, HAS_STORE);
      case 0x4: return s_type_inst("fsq", RISCV_FSQ, rs1, rs2, s_imm, HAS_STORE);
      default: return invalid_inst;
    }
    case 0x63: switch (f3) {
      case 0x0: return s_type_inst("beq", RISCV_BEQ, rs1, rs2, b_imm);
      case 0x1: return s_type_inst("bne", RISCV_BNE, rs1, rs2, b_imm);
      case 0x4: return s_type_inst("blt", RISCV_BLT, rs1, rs2, b_imm);
      case 0x5: return s_type_inst("bge", RISCV_BGE, rs1, rs2, b_imm);
      case 0x6: return s_type_inst("bltu", RISCV_BLTU, rs1, rs2, b_imm);
      case 0x7: return s_type_inst("bgeu", RISCV_BGEU, rs1, rs2, b_imm);
      default: return invalid_inst;
    }
    default: return invalid_inst;
  }
}

static decoded_instruction_t decode_u_type(uint8_t code, int rd, int u_imm) {
  int j_imm = (u_imm & 0xff000) | ((u_imm & 0x100000) >> 9) | ((u_imm & 0x7fe00000) >> 20) | (u_imm >> 30 ? (-1 & ~0xfffff) : 0);
  switch (code) {
    case 0x17: return u_type_inst("auipc", RISCV_AUIPC, rd, u_imm);
    case 0x37: return u_type_inst("lui", RISCV_LUI, rd, u_imm);
    case 0x6f: return u_type_inst("jal", RISCV_JAL, rd, j_imm);
    default: return invalid_inst;
  }
}

static decoded_instruction_t decode_fp(uint8_t code, int f7, int f3, int rd, int rs1, int rs2) {
  int f5 = f7 >> 2;
  const int& rs3 = f5;
  int fmt = f7 & 0x3;

  switch (code) {
    case 0x43: switch (fmt) {
      case 0x0: return r4_type_inst("fmadd.s", RISCV_FMADD_S, rd, rs1, rs2, rs3);
      case 0x1: return r4_type_inst("fmadd.d", RISCV_FMADD_D, rd, rs1, rs2, rs3);
      case 0x3: return r4_type_inst("fmadd.q", RISCV_FMADD_Q, rd, rs1, rs2, rs3);
      default: return invalid_inst;
    }
    case 0x47: switch (fmt) {
      case 0x0: return r4_type_inst("fmsub.s", RISCV_FMSUB_S, rd, rs1, rs2, rs3);
      case 0x1: return r4_type_inst("fmsub.d", RISCV_FMSUB_D, rd, rs1, rs2, rs3);
      case 0x3: return r4_type_inst("fmsub.q", RISCV_FMSUB_Q, rd, rs1, rs2, rs3);
      default: return invalid_inst;
    }
    case 0x4b: switch (fmt) {
      case 0x0: return r4_type_inst("fnmsub.s", RISCV_FNMSUB_S, rd, rs1, rs2, rs3);
      case 0x1: return r4_type_inst("fnmsub.d", RISCV_FNMSUB_D, rd, rs1, rs2, rs3);
      case 0x3: return r4_type_inst("fnmsub.q", RISCV_FNMSUB_Q, rd, rs1, rs2, rs3);
      default: return invalid_inst;
    }
    case 0x4f: switch (fmt) {
      case 0x0: return r4_type_inst("fnmadd.s", RISCV_FNMADD_S, rd, rs1, rs2, rs3);
      case 0x1: return r4_type_inst("fnmadd.d", RISCV_FNMADD_D, rd, rs1, rs2, rs3);
      case 0x3: return r4_type_inst("fnmadd.q", RISCV_FNMADD_Q, rd, rs1, rs2, rs3);
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
      default: switch (f5) {
        case 0x08: switch (fmt) {
          case 0x0: switch (rs2) {
            case 0x01: return fp_conv_inst("fcvt.s.d", RISCV_FCVT_S_D, rd, rs1);
            case 0x02: return fp_conv_inst("fcvt.d.s", RISCV_FCVT_D_S, rd, rs1);
            case 0x03: return fp_conv_inst("fcvt.s.q", RISCV_FCVT_S_Q, rd, rs1);
            default: return invalid_inst;
          }
          case 0x1: switch (rs2) {
            case 0x03: return fp_conv_inst("fcvt.d.q", RISCV_FCVT_D_Q, rd, rs1);
            default: return invalid_inst;
          }
          case 0x3: switch (rs2) {
            case 0x00: return fp_conv_inst("fcvt.q.s", RISCV_FCVT_Q_S, rd, rs1);
            case 0x01: return fp_conv_inst("fcvt.q.d", RISCV_FCVT_Q_D, rd, rs1);
            default: return invalid_inst;
          }
          default: return invalid_inst;
        }
        case 0x0b: switch (fmt) {
          case 0x0: return fp_conv_inst("fsqrt.s", RISCV_FSQRT_S, rd, rs1);
          case 0x1: return fp_conv_inst("fsqrt.d", RISCV_FSQRT_D, rd, rs1);
          case 0x3: return fp_conv_inst("fsqrt.q", RISCV_FSQRT_Q, rd, rs1);
          default: return invalid_inst;
        }
        case 0x18: switch (fmt) {
          case 0x0: switch (rs2) {
            case 0x00: return fp_conv_inst("fcvt.w.s", RISCV_FCVT_W_S, rd, rs1);
            case 0x01: return fp_conv_inst("fcvt.wu.s", RISCV_FCVT_WU_S, rd, rs1);
            case 0x02: return fp_conv_inst("fcvt.l.s", RISCV_FCVT_L_S, rd, rs1);
            case 0x03: return fp_conv_inst("fcvt.lu.s", RISCV_FCVT_LU_S, rd, rs1);
            default: return invalid_inst;
          }
          case 0x1: switch (rs2) {
            case 0x00: return fp_conv_inst("fcvt.w.d", RISCV_FCVT_W_D, rd, rs1);
            case 0x01: return fp_conv_inst("fcvt.wu.d", RISCV_FCVT_WU_D, rd, rs1);
            case 0x02: return fp_conv_inst("fcvt.l.d", RISCV_FCVT_L_D, rd, rs1);
            case 0x03: return fp_conv_inst("fcvt.lu.d", RISCV_FCVT_LU_D, rd, rs1);
            default: return invalid_inst;
          }
          case 0x3: switch (rs2) {
            case 0x00: return fp_conv_inst("fcvt.w.q", RISCV_FCVT_W_Q, rd, rs1);
            case 0x01: return fp_conv_inst("fcvt.wu.q", RISCV_FCVT_WU_Q, rd, rs1);
            case 0x02: return fp_conv_inst("fcvt.l.q", RISCV_FCVT_L_Q, rd, rs1);
            case 0x03: return fp_conv_inst("fcvt.lu.q", RISCV_FCVT_LU_Q, rd, rs1);
            default: return invalid_inst;
          }
          default: return invalid_inst;
        }
        case 0x1a: switch (fmt) {
          case 0x0: switch (rs2) {
            case 0x00: return fp_conv_inst("fcvt.s.w", RISCV_FCVT_S_W, rd, rs1);
            case 0x01: return fp_conv_inst("fcvt.s.wu", RISCV_FCVT_S_WU, rd, rs1);
            case 0x02: return fp_conv_inst("fcvt.s.l", RISCV_FCVT_S_L, rd, rs1);
            case 0x03: return fp_conv_inst("fcvt.s.lu", RISCV_FCVT_S_LU, rd, rs1);
            default: return invalid_inst;
          }
          case 0x1: switch (rs2) {
            case 0x00: return fp_conv_inst("fcvt.d.w", RISCV_FCVT_D_W, rd, rs1);
            case 0x01: return fp_conv_inst("fcvt.d.wu", RISCV_FCVT_D_WU, rd, rs1);
            case 0x02: return fp_conv_inst("fcvt.d.l", RISCV_FCVT_D_L, rd, rs1);
            case 0x03: return fp_conv_inst("fcvt.d.lu", RISCV_FCVT_D_LU, rd, rs1);
            default: return invalid_inst;
          }
          case 0x3: switch (rs2) {
            case 0x00: return fp_conv_inst("fcvt.q.w", RISCV_FCVT_Q_W, rd, rs1);
            case 0x01: return fp_conv_inst("fcvt.q.wu", RISCV_FCVT_Q_WU, rd, rs1);
            case 0x02: return fp_conv_inst("fcvt.q.l", RISCV_FCVT_Q_L, rd, rs1);
            case 0x03: return fp_conv_inst("fcvt.q.lu", RISCV_FCVT_Q_LU, rd, rs1);
            default: return invalid_inst;
          }
          default: return invalid_inst;
        }
        case 0x1c: switch (fmt) {
          case 0x0: switch (f3) {
            case 0x0: return fp_conv_inst("fmv.x.w", RISCV_FMV_X_W, rd, rs1);
            case 0x1: return fp_conv_inst("fclass.s", RISCV_FCLASS_S, rd, rs1);
            default: return invalid_inst;
          }
          case 0x1: switch (f3) {
            case 0x0: return fp_conv_inst("fmv.x.d", RISCV_FMV_X_D, rd, rs1);
            case 0x1: return fp_conv_inst("fclass.d", RISCV_FCLASS_D, rd, rs1);
            default: return invalid_inst;
          }
          case 0x3: switch (f3) {
            case 0x0: return fp_conv_inst("fmv.x.q", RISCV_FMV_X_Q, rd, rs1);
            case 0x1: return fp_conv_inst("fclass.q", RISCV_FCLASS_Q, rd, rs1);
            default: return invalid_inst;
          }
          default: return invalid_inst;
        }
        case 0x1e: switch (fmt) {
          case 0x0: return fp_conv_inst("fmv.w.x", RISCV_FMV_W_X, rd, rs1);
          case 0x1: return fp_conv_inst("fmv.d.x", RISCV_FMV_D_X, rd, rs1);
          case 0x3: return fp_conv_inst("fmv.q.x", RISCV_FMV_Q_X, rd, rs1);
          default: return invalid_inst;
        }
        default: return invalid_inst;
      }
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
  int i_imm = static_cast<int>(bits) >> 20;
  int s_imm = (f7 << 25) | rd;
  int u_imm = static_cast<int>(bits) & ~0xfff;

  if (decoded_instruction_t r = decode_r_type(opcode, f3, f7, rd, rs1, rs2))
    return r;
  if (decoded_instruction_t i = decode_i_type(opcode, f3, rd, rs1, i_imm))
    return i;
  if (decoded_instruction_t s = decode_s_type(opcode, f3, rs1, rs2, s_imm))
    return s;
  if (decoded_instruction_t u = decode_u_type(opcode, rd, u_imm))
    return u;
  if (decoded_instruction_t fp = decode_fp(opcode, f7, f3, rd, rs1, rs2))
    return fp;

  switch (bits & 0x707f) {
    case 0x000f: return inst("fence", RISCV_FENCE, 0);
    case 0x100f: return inst("fence.i", RISCV_FENCE_I, 0);
  }
  switch (bits & 0xf800707f) {
    case 0x1000202f: return decoded_instruction_t{
      .name="lr.w",
      .op=RISCV_LR_W,
      .rd=rd,
      .rs1=rs1,
      .rs2=-1,
      .rs3=-1,
      .imm=0,
      .flags=HAS_RD | HAS_RS1 | HAS_LOAD
    };
    // lr.d
  }
  switch (bits & 0xfe007fff) {
    case 0x12000073: return decoded_instruction_t{
      .name="sfence.vma",
      .op=RISCV_SFENCE_VMA,
      .rd=-1,
      .rs1=rs1,
      .rs2=rs2,
      .rs3=-1,
      .imm=0,
      .flags=HAS_RS1 | HAS_RS2
    };
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