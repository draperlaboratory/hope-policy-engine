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
#include <stdexcept>
#include <string>
#include <unordered_map>
#include "inst_decoder.h"
#include "option.h"
#include "platform_types.h"
#include "riscv_isa.h"

namespace policy_engine {

static constexpr int x0 = 0;
static constexpr int x1 = 1;
static constexpr int x2 = 2;
static const decoded_instruction_t invalid_inst{.name="", .op=RISCV_INVALID};

static decoded_instruction_t r_type_inst(const std::string& name, op_t op, int rd, int rs1, int rs2, flags_t flags=flags_t{}) { return decoded_instruction_t{
  .name=name,
  .op=op,
  .rd=rd,
  .rs1=rs1,
  .rs2=rs2,
  .rs3=none<int>(),
  .imm=none<int>(),
  .flags=flags
}; }

static decoded_instruction_t r4_type_inst(const std::string& name, op_t op, int rd, int rs1, int rs2, int rs3, flags_t flags=flags_t{}) { return decoded_instruction_t {
  .name=name,
  .op=op,
  .rd=rd,
  .rs1=rs1,
  .rs2=rs2,
  .rs3=rs3,
  .imm=none<int>(),
  .flags=flags
}; }

static decoded_instruction_t fp_conv_inst(const std::string& name, op_t op, int rd, int rs1, flags_t flags=flags_t{}) { return decoded_instruction_t{
  .name=name,
  .op=op,
  .rd=rd,
  .rs1=rs1,
  .rs2=none<int>(),
  .rs3=none<int>(),
  .imm=none<int>(),
  .flags=flags
}; }

static decoded_instruction_t i_type_inst(const std::string& name, op_t op, int rd, int rs1, int imm, flags_t flags=flags_t{}) { return decoded_instruction_t{
  .name=name,
  .op=op,
  .rd=rd,
  .rs1=rs1,
  .rs2=none<int>(),
  .rs3=none<int>(),
  .imm=imm,
  .flags=flags
}; }

static decoded_instruction_t csr_inst(const std::string& name, op_t op, int rd, int rs1, uint16_t csr) { return decoded_instruction_t{
  .name=name,
  .op=op,
  .rd=when(rd != 0, rd),
  .rs1=when(rs1 >= 0, rs1),
  .rs2=none<int>(),
  .rs3=none<int>(),
  .imm=csr,
  .flags=(rd != 0 ? (has_csr_load | has_csr_store) : has_csr_store)
}; }

static decoded_instruction_t s_type_inst(const std::string& name, op_t op, int rs1, int rs2, int imm, flags_t flags=flags_t{}) { return decoded_instruction_t{
  .name=name,
  .op=op,
  .rd=none<int>(),
  .rs1=rs1,
  .rs2=rs2,
  .rs3=none<int>(),
  .imm=imm,
  .flags=flags
}; }

static decoded_instruction_t u_type_inst(const std::string& name, op_t op, int rd, int imm, flags_t flags=flags_t{}) { return decoded_instruction_t {
  .name=name,
  .op=op,
  .rd=rd,
  .rs1=none<int>(),
  .rs2=none<int>(),
  .rs3=none<int>(),
  .imm=imm,
  .flags=flags
}; }

static decoded_instruction_t system_inst(const std::string& name, op_t op, flags_t flags=flags_t{}) { return decoded_instruction_t {
  .name=name,
  .op=op,
  .rd=none<int>(),
  .rs1=none<int>(),
  .rs2=none<int>(),
  .rs3=none<int>(),
  .imm=none<int>(),
  .flags=flags
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
        case 0x3: return r_type_inst("amoadd.d", RISCV_AMOADD_D, rd, rs1, rs2);
        default: return invalid_inst;
      }
      case 0x01: switch (f3) {
        case 0x2: return r_type_inst("amoswap.w", RISCV_AMOSWAP_W, rd, rs1, rs2);
        case 0x3: return r_type_inst("amoswap.d", RISCV_AMOSWAP_D, rd, rs1, rs2);
        default: return invalid_inst;
      }
      case 0x02: switch (f3) {
        case 0x2: return decoded_instruction_t{
          .name="lr.w",
          .op=RISCV_LR_W,
          .rd=rd,
          .rs1=rs1,
          .rs2=none<int>(),
          .rs3=none<int>(),
          .imm=none<int>(),
          .flags=has_load
        }; // not quite R-type, but grouped with other AMO instructions
        case 0x3: return decoded_instruction_t{
          .name="lr.d",
          .op=RISCV_LR_D,
          .rd=rd,
          .rs1=rs1,
          .rs2=none<int>(),
          .rs3=none<int>(),
          .imm=none<int>(),
          .flags=has_load
        }; // not quite R-type, but grouped with other AMO instructions
        default: return invalid_inst;
      }
      case 0x03: switch (f3) {
        case 0x2: return r_type_inst("sc.w", RISCV_SC_W, rd, rs1, rs2);
        case 0x3: return r_type_inst("sc.d", RISCV_SC_D, rd, rs1, rs2);
        default: return invalid_inst;
      }
      case 0x04: switch (f3) {
        case 0x2: return r_type_inst("amoxor.w", RISCV_AMOXOR_W, rd, rs1, rs2);
        case 0x3: return r_type_inst("amoxor.d", RISCV_AMOXOR_D, rd, rs1, rs2);
        default: return invalid_inst;
      }
      case 0x08: switch (f3) {
        case 0x2: return r_type_inst("amoor.w", RISCV_AMOOR_W, rd, rs1, rs2);
        case 0x3: return r_type_inst("ammor.d", RISCV_AMOOR_D, rd, rs1, rs2);
        default: return invalid_inst;
      }
      case 0x0c: switch (f3) {
        case 0x2: return r_type_inst("amoand.w", RISCV_AMOAND_W, rd, rs1, rs2);
        case 0x3: return r_type_inst("amoand.d", RISCV_AMOAND_D, rd, rs1, rs2);
        default: return invalid_inst;
      }
      case 0x10: switch (f3) {
        case 0x2: return r_type_inst("amomin.w", RISCV_AMOMIN_W, rd, rs1, rs2);
        case 0x3: return r_type_inst("amomin.d", RISCV_AMOMIN_D, rd, rs1, rs2);
        default: return invalid_inst;
      }
      case 0x14: switch (f3) {
        case 0x2: return r_type_inst("amomax.w", RISCV_AMOMAX_W, rd, rs1, rs2);
        case 0x3: return r_type_inst("amomax.d", RISCV_AMOMAX_D, rd, rs1, rs2);
        default: return invalid_inst;
      }
      case 0x18: switch (f3) {
        case 0x2: return r_type_inst("amominu.w", RISCV_AMOMINU_W, rd, rs1, rs2);
        case 0x3: return r_type_inst("amominu.d", RISCV_AMOMINU_D, rd, rs1, rs2);
        default: return invalid_inst;
      }
      case 0x1c: switch (f3) {
        case 0x2: return r_type_inst("amomaxu.w", RISCV_AMOMAXU_W, rd, rs1, rs2);
        case 0x3: return r_type_inst("amomaxu.d", RISCV_AMOMAXU_D, rd, rs1, rs2);
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
      case 0x0: return i_type_inst("lb", RISCV_LB, rd, rs1, imm, has_load);
      case 0x1: return i_type_inst("lh", RISCV_LH, rd, rs1, imm, has_load);
      case 0x2: return i_type_inst("lw", RISCV_LW, rd, rs1, imm, has_load);
      case 0x3: return i_type_inst("ld", RISCV_LD, rd, rs1, imm, has_load);
      case 0x4: return i_type_inst("lbu", RISCV_LBU, rd, rs1, imm, has_load);
      case 0x5: return i_type_inst("lhu", RISCV_LHU, rd, rs1, imm, has_load);
      case 0x6: return i_type_inst("lwu", RISCV_LWU, rd, rs1, imm, has_load);
      default: return invalid_inst;
    }
    case 0x07: switch (f3) {
      case 0x2: return i_type_inst("flw", RISCV_FLW, rd, rs1, imm, has_load);
      case 0x3: return i_type_inst("fld", RISCV_FLD, rd, rs1, imm, has_load);
      case 0x4: return i_type_inst("flq", RISCV_FLQ, rd, rs1, imm, has_load);
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
      case 0x0: return s_type_inst("sb", RISCV_SB, rs1, rs2, s_imm, has_store);
      case 0x1: return s_type_inst("sh", RISCV_SH, rs1, rs2, s_imm, has_store);
      case 0x2: return s_type_inst("sw", RISCV_SW, rs1, rs2, s_imm, has_store);
      case 0x3: return s_type_inst("sd", RISCV_SD, rs1, rs2, s_imm, has_store);
      default: return invalid_inst;
    }
    case 0x27: switch (f3) {
      case 0x2: return s_type_inst("fsw", RISCV_FSW, rs1, rs2, s_imm, has_store);
      case 0x3: return s_type_inst("fsd", RISCV_FSD, rs1, rs2, s_imm, has_store);
      case 0x4: return s_type_inst("fsq", RISCV_FSQ, rs1, rs2, s_imm, has_store);
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
            case 0x03: return fp_conv_inst("fcvt.s.q", RISCV_FCVT_S_Q, rd, rs1);
            default: return invalid_inst;
          }
          case 0x1: switch (rs2) {
            case 0x00: return fp_conv_inst("fcvt.d.s", RISCV_FCVT_D_S, rd, rs1);
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

static decoded_instruction_t decode_system(uint8_t code, uint8_t f7, uint8_t f3, int rs1, int rs2) {
  uint16_t f12 = f7 << 5 | (rs2 & 0x1f);
  switch (code) {
    case 0x0f: switch (f3) {
      case 0x0: return system_inst("fence", RISCV_FENCE);
      case 0x1: return system_inst("fence.i", RISCV_FENCE_I);
      default: return invalid_inst;
    }
    case 0x73: switch (f12) {
      case 0x000: return system_inst("ecall", RISCV_ECALL);
      case 0x001: return system_inst("ebreak", RISCV_EBREAK);
      case 0x002: return system_inst("uret", RISCV_URET);
      case 0x102: return system_inst("sret", RISCV_SRET);
      case 0x105: return system_inst("wfi", RISCV_WFI);
      case 0x302: return system_inst("mret", RISCV_MRET);
      case 0x7b2: return system_inst("dret", RISCV_DRET);
      default: switch (f7) {
        case 0x09: return decoded_instruction_t{
          .name="sfence.vma",
          .op=RISCV_SFENCE_VMA,
          .rd=none<int>(),
          .rs1=rs1,
          .rs2=rs2,
          .rs3=none<int>(),
          .imm=none<int>()
        };
        default: return invalid_inst;
      }
    }
    default: return invalid_inst;
  }
}

static decoded_instruction_t decode_ci_type(int xlen, uint8_t quad, uint8_t f3, int rds1, int imm) {
  unsigned int uimm = imm & 0x3f;
  unsigned int wuimm = (uimm & 0x3c) | ((uimm & 0x3) << 6);
  unsigned int duimm = (uimm & 0x38) | ((uimm & 0x7) << 6);

  switch (quad) {
    case 0x1: switch (f3) {
      case 0x0: switch (rds1) {
        case 0: return imm != 0 ? i_type_inst("c.nop", RISCV_ADDI, x0, x0, imm, is_compressed) : invalid_inst;
        default: return imm != 0 ? i_type_inst("c.addi", RISCV_ADDI, rds1, rds1, imm, is_compressed) : invalid_inst;
      }
      case 0x1: switch (xlen) {
        case 32: return invalid_inst;
        case 64: return rds1 != 0 ? i_type_inst("c.addiw", RISCV_ADDIW, rds1, rds1, imm, is_compressed) : invalid_inst;
      }
      case 0x2: return rds1 != 0 ? i_type_inst("c.li", RISCV_ADDI, rds1, x0, imm, is_compressed) : invalid_inst;
      case 0x3: switch (rds1) {
        case 0: return invalid_inst;
        case 2: return imm != 0 ? i_type_inst("c.addi16sp", RISCV_ADDI, x2, x2, imm, is_compressed) : invalid_inst;
        default: return imm != 0 ? u_type_inst("c.lui", RISCV_LUI, rds1, imm << 12, is_compressed) : invalid_inst;
      }
      default: return invalid_inst;
    }
    case 0x2: switch (f3) {
      case 0x0: return imm != 0 && (xlen > 32 || (uimm >> 5) == 0) ? i_type_inst("c.slli", RISCV_SLLI, rds1, rds1, uimm, is_compressed) : invalid_inst;
      case 0x1: switch (xlen) {
        case 32: case 64: return i_type_inst("c.fldsp", RISCV_FLD, rds1, x2, duimm, has_load | is_compressed);
      }
      case 0x2: return rds1 != 0 ? i_type_inst("c.lwsp", RISCV_LW, rds1, x2, wuimm, has_load | is_compressed) : invalid_inst;
      case 0x3: switch (xlen) {
        case 32: return i_type_inst("c.flwsp", RISCV_FLW, rds1, x2, wuimm, has_load | is_compressed);
        case 64: return rds1 != 0 ? i_type_inst("c.ldsp", RISCV_LD, rds1, x2, duimm, has_load | is_compressed) : invalid_inst;
      }
      default: return invalid_inst;
    }
    default: return invalid_inst;
  }
}

static decoded_instruction_t decode_css_type(int xlen, uint8_t quad, uint8_t f3, int rs2, unsigned int uimm) {
  unsigned int wuimm = (uimm & 0x3c) | ((uimm & 0x3) << 6);
  unsigned int duimm = (uimm & 0x38) | ((uimm & 0x7) << 6);

  switch (quad) {
    case 0x2: switch (f3) {
      case 0x5: switch (xlen) {
        case 32: case 64: return s_type_inst("c.fsdsp", RISCV_FSD, x2, rs2, duimm, has_store | is_compressed);
      }
      case 0x6: return s_type_inst("c.swsp", RISCV_SW, x2, rs2, wuimm, has_store | is_compressed);
      case 0x7: switch (xlen) {
        case 32: return s_type_inst("c.fswsp", RISCV_FSW, x2, rs2, wuimm, has_store | is_compressed);
        case 64: return s_type_inst("c.sdsp", RISCV_SD, x2, rs2, duimm, has_store | is_compressed);
      }
      default: return invalid_inst;
    }
    default: return invalid_inst;
  }
}

static decoded_instruction_t decode_cl_type(int xlen, uint8_t quad, uint8_t f3, int rdp, int rs1p, unsigned int uimm) {
  unsigned int wuimm = ((uimm & 0x1e) << 1) | ((uimm & 0x1) << 6);
  unsigned int duimm = ((uimm & 0x1c) << 1) | ((uimm & 0x3) << 6);

  switch (quad) {
    case 0x0: switch (f3) {
      case 0x1: switch (xlen) {
        case 32: case 64: return i_type_inst("c.fld", RISCV_FLD, rdp, rs1p, duimm << 3, has_load | is_compressed);
      }
      case 0x2: return i_type_inst("c.lw", RISCV_LW, rdp, rs1p, wuimm << 2, has_load | is_compressed);
      case 0x3: switch (xlen) {
        case 32: return i_type_inst("c.flw", RISCV_FLW, rdp, rs1p, wuimm << 2, has_load | is_compressed);
        case 64: return i_type_inst("c.ld", RISCV_LD, rdp, rs1p, duimm << 3, has_load | is_compressed);
      }
      default: return invalid_inst;
    }
    default: return invalid_inst;
  }
}

static decoded_instruction_t decode_cs_type(int xlen, uint8_t quad, uint8_t f3, int rs1p, int rs2p, unsigned int uimm) {
  unsigned int wuimm = ((uimm & 0x1e) << 1) | ((uimm & 0x1) << 6);
  unsigned int duimm = ((uimm & 0x1c) << 1) | ((uimm & 0x3) << 6);

  switch (quad) {
    case 0x0: switch (f3) {
      case 0x5: switch (xlen) {
        case 32: case 64: return s_type_inst("c.fsd", RISCV_FSD, rs1p, rs2p, duimm, has_store | is_compressed);
      }
      case 0x6: return s_type_inst("c.sw", RISCV_SW, rs1p, rs2p, wuimm, has_store | is_compressed);
      case 0x7: switch (xlen) {
        case 32: return s_type_inst("c.fsw", RISCV_FSW, rs1p, rs2p, wuimm, has_store | is_compressed);
        case 64: return s_type_inst("c.sd", RISCV_SD, rs1p, rs2p, duimm, has_store | is_compressed);
      }
      default: return invalid_inst;
    }
    default: return invalid_inst;
  }
}

static decoded_instruction_t decode_cj_type(int xlen, uint8_t quad, uint8_t f3, int imm) {
  switch (quad) {
    case 0x1: switch (f3) {
      case 0x1: switch (xlen) {
        case 32: return u_type_inst("c.jal", RISCV_JAL, x1, imm, is_compressed);
        default: return invalid_inst;
      }
      case 0x5: return u_type_inst("c.j", RISCV_JAL, x0, imm, is_compressed);
      default: return invalid_inst;
    }
    default: return invalid_inst;
  }
}

static decoded_instruction_t decode_cr_type(int xlen, uint8_t quad, uint8_t f4, int rs1, int rs2) {
  switch (quad) {
    case 0x2: switch (f4) {
      case 0x8: switch (rs2) {
        case 0: return rs1 != 0 ? i_type_inst("c.jr", RISCV_JALR, x0, rs1, 0, is_compressed) : invalid_inst;
        default: return rs1 != 0 ? r_type_inst("c.mv", RISCV_ADD, rs1, x0, rs2, is_compressed) : invalid_inst;
      }
      case 0x9: switch (rs2) {
        case 0: return rs1 != 0 ? i_type_inst("c.jalr", RISCV_JALR, x1, rs1, 0, is_compressed) : invalid_inst;
        default: return rs1 != 0 ? r_type_inst("c.add", RISCV_ADD, rs1, rs1, rs2, is_compressed) : invalid_inst;
      }
      default: return invalid_inst;
    }
    default: return invalid_inst;
  }
}

static decoded_instruction_t decode_cb_type(int xlen, uint8_t quad, uint8_t f3, int rs1p, int imm) {
  switch (quad) {
    case 0x1: switch (f3) {
      case 0x6: return s_type_inst("c.beqz", RISCV_BEQ, rs1p, x0, imm, is_compressed);
      case 0x7: return s_type_inst("c.bnez", RISCV_BNE, rs1p, x0, imm, is_compressed);
      default: return invalid_inst;
    }
    default: return invalid_inst;
  }
}

static decoded_instruction_t decode_ciw_type(int xlen, uint8_t quad, uint8_t f3, int rdp, int imm) {
  switch (quad) {
    case 0x0: switch (f3) {
      case 0x0: rdp != 0 && imm != 0 ? i_type_inst("c.addi4spn", RISCV_ADDI, rdp, x2, imm << 2, is_compressed) : invalid_inst;
      default: return invalid_inst;
    }
    default: return invalid_inst;
  }
}

static decoded_instruction_t decode_ca_type(int xlen, uint8_t quad, uint8_t f6, uint8_t f2, int rds1p, int rs2p) {
  unsigned int shamt = (f6 & 0x4 << 2) | (f2 << 3) | (rs2p & 0x7);

  switch (quad) {
    case 0x1: switch (f6) {
      case 0x23: switch (f2) {
        case 0x0: return r_type_inst("c.sub", RISCV_SUB, rds1p, rds1p, rs2p, is_compressed);
        case 0x1: return r_type_inst("c.xor", RISCV_XOR, rds1p, rds1p, rs2p, is_compressed);
        case 0x2: return r_type_inst("c.or", RISCV_OR, rds1p, rds1p, rs2p, is_compressed);
        case 0x3: return r_type_inst("c.and", RISCV_AND, rds1p, rds1p, rs2p, is_compressed);
        default: return invalid_inst;
      }
      case 0x27: switch (f2) {
        case 0x0: return r_type_inst("c.subw", RISCV_SUBW, rds1p, rds1p, rs2p, is_compressed);
        case 0x1: return r_type_inst("c.addw", RISCV_ADDW, rds1p, rds1p, rs2p, is_compressed);
        default: return invalid_inst;
      }
      default: return invalid_inst;
    }
    case 0x20: case 0x24: return shamt != 0 ? i_type_inst("c.srli", RISCV_SRLI, rds1p, rds1p, shamt, is_compressed) : invalid_inst; // actually cb-type, but easier to decode here
    case 0x21: case 0x25: return shamt != 0 ? i_type_inst("c.srai", RISCV_SRAI, rds1p, rds1p, shamt, is_compressed) : invalid_inst; // actually cb-type, but easier to decode here
    case 0x22: case 0x26: return i_type_inst("c.andi", RISCV_ANDI, rds1p, rds1p, (shamt >> 5) ? (shamt | ~0x3f) : shamt, is_compressed);
    default: return invalid_inst;
  }
}

decoded_instruction_t decode(insn_bits_t bits, int xlen) {
  if (xlen != 32 && xlen != 64) {
    throw std::invalid_argument("only RV32 and RV64 are supported");
  }

  // full instructions
  uint8_t opcode = bits & 0x7f;
  uint8_t f3 = (bits & 0x7000) >> 12;
  uint8_t f7 = (bits & 0xfe000000) >> 25;
  int rd = (bits & 0xf80) >> 7;
  int rs1 = (bits & 0xf8000) >> 15;
  int rs2 = (bits & 0x1f00000) >> 20;
  int i_imm = static_cast<int>(bits) >> 20;
  int s_imm = (static_cast<int>(bits & 0xfe000000) >> 20) | rd;
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
  if (decoded_instruction_t sys = decode_system(opcode, f7, f3, rs1, rs2))
    return sys;

  // compressed instructions
  uint8_t quad = bits & 0x3;
  uint8_t c_f2 = (bits & 0x60) >> 5;
  uint8_t c_f3 = (bits & 0xe000) >> 13;
  uint8_t c_f4 = (bits & 0xf000) >> 12;
  uint8_t c_f6 = (bits & 0xfc00) >> 10;
  int c_rs2 = (bits & 0x7c) >> 2;
  int rdp = ((bits & 0x1c) >> 2) | 0x8;
  int rs1p = ((bits & 0x380) >> 7) | 0x8;
  int ci_imm = c_rs2 | ((bits & 0x1000) ? ~0x1f : 0);
  unsigned int cls_imm = ((bits & 0x1c00) >> 8) | ((bits & 0x60) >> 5);
  int cj_imm = ((bits & 0x4) << 3) | ((bits & 0x38) >> 2) | ((bits & 0x40) << 1) | ((bits & 0x80) >> 1) | ((bits & 0x100) << 2) | ((bits & 0x600) >> 1) | ((bits & 0x800) >> 7) | ((bits & 0x1000) ? ~0x7ff : 0);
  int cb_imm = ((bits & 0x4) << 3) | ((bits & 0x18) >> 2) | ((bits & 0x60) << 1) | ((bits & 0xc00) >> 7) | ((bits & 0x1000) ? ~0x7ff : 0);
  unsigned int ciw_imm = ((bits & 0x20) >> 2) | ((bits & 0x40) >> 4) | ((bits & 0x78) >> 1) | ((bits & 0x1800) >> 7);

  if (decoded_instruction_t ci = decode_ci_type(xlen, quad, c_f3, rd, ci_imm))
    return ci;
  if (decoded_instruction_t css = decode_css_type(xlen, quad, c_f3, c_rs2, rd))
    return css;
  if (decoded_instruction_t cl = decode_cl_type(xlen, quad, c_f3, rdp, rs1p, cls_imm))
    return cl;
  if (decoded_instruction_t cs = decode_cs_type(xlen, quad, c_f3, rs1p, rdp, cls_imm))
    return cs;
  if (decoded_instruction_t cj = decode_cj_type(xlen, quad, c_f3, cj_imm))
    return cj;
  if (decoded_instruction_t cr = decode_cr_type(xlen, quad, c_f4, rd, c_rs2))
    return cr;
  if (decoded_instruction_t cb = decode_cb_type(xlen, quad, c_f3, rs1p, cb_imm))
    return cb;
  if (decoded_instruction_t ciw = decode_ciw_type(xlen, quad, c_f3, rdp, ciw_imm))
    return ciw;
  if (decoded_instruction_t ca = decode_ca_type(xlen, quad, c_f6, c_f2, rs1p, rdp))
    return ca;

  return invalid_inst;
}

} // namespace policy_engine