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
#include "inst_decoder.h"
#include "platform_types.h"
#include "riscv_isa.h"

namespace policy_engine {

static decoded_instruction_t inst(insn_bits_t bits, const std::string& name, uint32_t op, uint32_t flags) { return decoded_instruction_t{
  .bits=bits,
  .name=name,
  .op=op,
  .rd=-1,
  .rs1=-1,
  .rs2=-1,
  .rs3=-1,
  .imm=0,
  .flags=flags
}; }

static decoded_instruction_t inst_rd(insn_bits_t bits, const std::string& name, uint32_t op, int rd, uint32_t flags) { return decoded_instruction_t{
  .bits=bits,
  .name=name,
  .op=op,
  .rd=rd,
  .rs1=-1,
  .rs2=-1,
  .rs3=-1,
  .imm=0,
  .flags=flags
}; }

static decoded_instruction_t inst_rd_rs1(insn_bits_t bits, const std::string& name, uint32_t op, int rd, int rs1, uint32_t flags) { return decoded_instruction_t{
  .bits=bits,
  .name=name,
  .op=op,
  .rd=rd,
  .rs1=rs1,
  .rs2=-1,
  .rs3=-1,
  .imm=0,
  .flags=flags
}; }

static decoded_instruction_t inst_rs1_rs2(insn_bits_t bits, const std::string& name, uint32_t op, int rs1, int rs2, uint32_t flags) { return decoded_instruction_t{
  .bits=bits,
  .name=name,
  .op=op,
  .rd=-1,
  .rs1=rs1,
  .rs2=rs2,
  .rs3=-1,
  .imm=0,
  .flags=flags
}; }

static decoded_instruction_t inst_rd_rs1_imm(insn_bits_t bits, const std::string& name, uint32_t op, int rd, int rs1, int imm, uint32_t flags) { return decoded_instruction_t{
  .bits=bits,
  .name=name,
  .op=op,
  .rd=rd,
  .rs1=rs1,
  .rs2=-1,
  .rs3=-1,
  .imm=imm,
  .flags=flags
}; }

static decoded_instruction_t inst_rs1_rs2_imm(insn_bits_t bits, const std::string& name, uint32_t op, int rs1, int rs2, int imm, uint32_t flags) { return decoded_instruction_t{
  .bits=bits,
  .name=name,
  .op=op,
  .rd=-1,
  .rs1=rs1,
  .rs2=rs2,
  .rs3=-1,
  .imm=imm,
  .flags=flags
}; }

static decoded_instruction_t inst_rd_rs1_rs2(insn_bits_t bits, const std::string& name, uint32_t op, int rd, int rs1, int rs2, uint32_t flags) { return decoded_instruction_t{
  .bits=bits,
  .name=name,
  .op=op,
  .rd=rd,
  .rs1=rs1,
  .rs2=rs2,
  .rs3=-1,
  .imm=0,
  .flags=flags
}; }

static decoded_instruction_t inst_all_regs(insn_bits_t bits, const std::string& name, uint32_t op, int rd, int rs1, int rs2, int rs3, uint32_t flags) { return decoded_instruction_t{
  .bits=bits,
  .name=name,
  .op=op,
  .rd=rd,
  .rs1=rs1,
  .rs2=rs2,
  .rs3=rs3,
  .imm=0,
  .flags=flags
}; }

decoded_instruction_t decode(insn_bits_t bits) {
  int rd = (bits & 0xf80) >> 7;
  int rs1 = (bits & 0xf8000) >> 15;
  int rs2 = (bits & 0x1f00000) >> 20;
  int rs3 = (bits & 0xf8000000) >> 27;
  switch (bits & 0x7f) {
    case 0x6f: return inst_rd(bits, "jal", RISCV_JAL, rd, HAS_RD);
    case 0x37: return inst_rd(bits, "lui", RISCV_LUI, rd, HAS_RD);
    case 0x17: return inst_rd(bits, "auipc", RISCV_AUIPC, rd, HAS_RD);
  }
  switch (bits & 0x707f) {
    case 0x0063: return inst_rs1_rs2(bits, "beq", RISCV_BEQ, rs1, rs2, HAS_RS1 | HAS_RS2);
    case 0x1063: return inst_rs1_rs2(bits, "bne", RISCV_BNE, rs1, rs2, HAS_RS1 | HAS_RS2);
    case 0x4063: return inst_rs1_rs2(bits, "blt", RISCV_BLT, rs1, rs2, HAS_RS1 | HAS_RS2);
    case 0x5063: return inst_rs1_rs2(bits, "bge", RISCV_BGE, rs1, rs2, HAS_RS1 | HAS_RS2);
    case 0x6063: return inst_rs1_rs2(bits, "bltu", RISCV_BLTU, rs1, rs2, HAS_RS1 | HAS_RS2);
    case 0x7063: return inst_rs1_rs2(bits, "bgeu", RISCV_BGEU, rs1, rs2, HAS_RS1 | HAS_RS2);
    case 0x0067: return inst_rd_rs1_imm(
      bits, "jalr", RISCV_JALR, rd, rs1,
      (bits & 0x80000000) ? ((bits & 0xFFF00000) >> 20) | 0xFFFFF000 : (bits & 0xFFF00000) >> 20,
      HAS_RD | HAS_RS1 | HAS_IMM
    );
    case 0x0013: return inst_rd_rs1_imm(
      bits, "addi", RISCV_ADDI, rd, rs1,
      (bits & 0x80000000) ? ((bits & 0xFFF00000) >> 20) | 0xFFFFF000 : (bits & 0xFFF00000) >> 20,
      HAS_RD | HAS_RS1 | HAS_IMM
    );
    case 0x2013: return inst_rd_rs1_imm(
      bits, "slti", RISCV_SLTI, rd, rs1,
      (bits & 0x80000000) ? ((bits & 0xFFF00000) >> 20) | 0xFFFFF000 : (bits & 0xFFF00000) >> 20,
      HAS_RD | HAS_RS1 | HAS_IMM
    );
    case 0x3013: return inst_rd_rs1_imm(
      bits, "sltiu", RISCV_SLTIU, rd, rs1,
      (bits & 0x80000000) ? ((bits & 0xFFF00000) >> 20) | 0xFFFFF000 : (bits & 0xFFF00000) >> 20,
      HAS_RD | HAS_RS1 | HAS_IMM
    );
    case 0x4013: return inst_rd_rs1_imm(
      bits, "xori", RISCV_XORI, rd, rs1,
      (bits & 0x80000000) ? ((bits & 0xFFF00000) >> 20) | 0xFFFFF000 : (bits & 0xFFF00000) >> 20,
      HAS_RD | HAS_RS1 | HAS_IMM
    );
    case 0x6013: return inst_rd_rs1_imm(
      bits, "ori", RISCV_ORI, rd, rs1,
      (bits & 0x80000000) ? ((bits & 0xFFF00000) >> 20) | 0xFFFFF000 : (bits & 0xFFF00000) >> 20,
      HAS_RD | HAS_RS1 | HAS_IMM
    );
    case 0x7013: return inst_rd_rs1_imm(
      bits, "andi", RISCV_ANDI, rd, rs1,
      (bits & 0x80000000) ? ((bits & 0xFFF00000) >> 20) | 0xFFFFF000 : (bits & 0xFFF00000) >> 20,
      HAS_RD | HAS_RS1 | HAS_IMM
    );
    case 0x001b: return inst_rd_rs1_imm(
      bits, "addiw", RISCV_ADDIW, rd, rs1,
      (bits & 0x80000000) ? ((bits & 0xFFF00000) >> 20) | 0xFFFFF000 : (bits & 0xFFF00000) >> 20,
      HAS_RD | HAS_RS1 | HAS_IMM
    );
    case 0x0003: return inst_rd_rs1_imm(
      bits, "lb", RISCV_LB, rd, rs1,
      (bits & 0x80000000) ? ((bits & 0xFFF00000) >> 20) | 0xFFFFF000 : (bits & 0xFFF00000) >> 20,
      HAS_RD | HAS_RS1 | HAS_IMM | HAS_LOAD
    );
    case 0x1003: return inst_rd_rs1_imm(
      bits, "lh", RISCV_LH, rd, rs1,
      (bits & 0x80000000) ? ((bits & 0xFFF00000) >> 20) | 0xFFFFF000 : (bits & 0xFFF00000) >> 20,
      HAS_RD | HAS_RS1 | HAS_IMM | HAS_LOAD
    );
    case 0x2003: return inst_rd_rs1_imm(
      bits, "lw", RISCV_LW, rd, rs1,
      (bits & 0x80000000) ? ((bits & 0xFFF00000) >> 20) | 0xFFFFF000 : (bits & 0xFFF00000) >> 20,
      HAS_RD | HAS_RS1 | HAS_IMM | HAS_LOAD
    );
    case 0x3003: return inst_rd_rs1_imm(
      bits, "ld", RISCV_LD, rd, rs1,
      (bits & 0x80000000) ? ((bits & 0xFFF00000) >> 20) | 0xFFFFF000 : (bits & 0xFFF00000) >> 20,
      HAS_RD | HAS_RS1 | HAS_IMM | HAS_LOAD
    );
    case 0x4003: return inst_rd_rs1_imm(
      bits, "lbu", RISCV_LBU, rd, rs1,
      (bits & 0x80000000) ? ((bits & 0xFFF00000) >> 20) | 0xFFFFF000 : (bits & 0xFFF00000) >> 20,
      HAS_RD | HAS_RS1 | HAS_IMM | HAS_LOAD
    );
    case 0x5003: return inst_rd_rs1_imm(
      bits, "lhu", RISCV_LHU, rd, rs1,
      (bits & 0x80000000) ? ((bits & 0xFFF00000) >> 20) | 0xFFFFF000 : (bits & 0xFFF00000) >> 20,
      HAS_RD | HAS_RS1 | HAS_IMM | HAS_LOAD
    );
    case 0x6003: return inst_rd_rs1_imm(
      bits, "lwu", RISCV_LWU, rd, rs1,
      (bits & 0x80000000) ? ((bits & 0xFFF00000) >> 20) | 0xFFFFF000 : (bits & 0xFFF00000) >> 20,
      HAS_RD | HAS_RS1 | HAS_IMM | HAS_LOAD
    );
    case 0x0023: return inst_rs1_rs2_imm(
      bits, "sb", RISCV_SB, rs1, rs2,
      ((bits & 0x80000000) ? ((bits & 0xFE000000) >> 20) | 0xFFFFF000 : (bits & 0xFE000000) >> 20) | ((bits & 0x00000F80) >> 7),
      HAS_RS1 | HAS_RS2 | HAS_IMM | HAS_STORE
    );
    case 0x1023: return inst_rs1_rs2_imm(
      bits, "sh", RISCV_SH, rs1, rs2,
      ((bits & 0x80000000) ? ((bits & 0xFE000000) >> 20) | 0xFFFFF000 : (bits & 0xFE000000) >> 20) | ((bits & 0x00000F80) >> 7),
      HAS_RS1 | HAS_RS2 | HAS_IMM | HAS_STORE
    );
    case 0x2023: return inst_rs1_rs2_imm(
      bits, "sw", RISCV_SW, rs1, rs2,
      ((bits & 0x80000000) ? ((bits & 0xFE000000) >> 20) | 0xFFFFF000 : (bits & 0xFE000000) >> 20) | ((bits & 0x00000F80) >> 7),
      HAS_RS1 | HAS_RS2 | HAS_IMM | HAS_STORE
    );
    case 0x3023: return inst_rs1_rs2_imm(
      bits, "sd", RISCV_SD, rs1, rs2,
      ((bits & 0x80000000) ? ((bits & 0xFE000000) >> 20) | 0xFFFFF000 : (bits & 0xFE000000) >> 20) | ((bits & 0x00000F80) >> 7),
      HAS_RS1 | HAS_RS2 | HAS_IMM | HAS_STORE
    );
    case 0x000f: return inst(bits, "fence", RISCV_FENCE, 0);
    case 0x100f: return inst(bits, "fence.i", RISCV_FENCE_I, 0);
    case 0x1073: return inst_rd_rs1_imm(
      bits, "csrrw", RISCV_CSRRW, rd, rs1,
      (bits & 0xfff00000) >> 20,
      HAS_RS1 | HAS_IMM | (rd ? HAS_RD | HAS_CSR_LOAD | HAS_CSR_STORE : HAS_CSR_STORE)
    );
    case 0x2073: return inst_rd_rs1_imm(
      bits, "csrrs", RISCV_CSRRS, rd, rs1,
      (bits & 0xfff00000) >> 20,
      HAS_RD | HAS_RS1 | HAS_IMM | (rs1 ? HAS_CSR_LOAD | HAS_CSR_STORE : HAS_CSR_LOAD)
    );
    case 0x3073: return inst_rd_rs1_imm(
      bits, "csrrc", RISCV_CSRRC, rd, rs1,
      (bits & 0xfff00000) >> 20,
      HAS_RD | HAS_RS1 | HAS_RS2 | (rs1 ? HAS_CSR_LOAD | HAS_CSR_STORE : HAS_CSR_LOAD)
    );
    case 0x5073: return inst_rd_rs1_imm(
      bits, "csrrwi", RISCV_CSRRWI, rd, rs1,
      (bits & 0xfff00000) >> 20,
      HAS_RS1 | HAS_IMM | (rd ? HAS_RD | HAS_CSR_LOAD | HAS_CSR_STORE : HAS_CSR_STORE)
    );
    case 0x6073: return inst_rd_rs1_imm(
      bits, "csrrsi", RISCV_CSRRSI, rd, rs1,
      (bits & 0xfff00000) >> 20,
      HAS_RD | HAS_RS1 | HAS_IMM | (rs1 ? HAS_CSR_LOAD | HAS_CSR_STORE : HAS_CSR_LOAD)
    );
    case 0x7073: return inst_rd_rs1_imm(
      bits, "csrrci", RISCV_CSRRCI, rd, rs1,
      (bits & 0xfff00000) >> 20,
      HAS_RD | HAS_RS1 | HAS_IMM | (rs1 ? HAS_CSR_LOAD | HAS_CSR_STORE : HAS_CSR_LOAD)
    );
    case 0x2007: return inst_rd_rs1_imm(
      bits, "flw", RISCV_FLW, rd, rs1,
      (bits & 0x80000000) ? ((bits & 0xFFF00000) >> 20) | 0xFFFFF000 : (bits & 0xFFF00000) >> 20,
      HAS_RD | HAS_RS1 | HAS_IMM | HAS_LOAD
    );
    case 0x3007: return inst_rd_rs1_imm(
      bits, "fld", RISCV_FLD, rd, rs1,
      (bits & 0x80000000) ? ((bits & 0xFFF00000) >> 20) | 0xFFFFF000 : (bits & 0xFFF00000) >> 20,
      HAS_RD | HAS_RS1 | HAS_IMM | HAS_LOAD
    );
    case 0x4007: return inst_rd_rs1_imm(
      bits, "flq", RISCV_FLQ, rd, rs1,
      (bits & 0x80000000) ? ((bits & 0xFFF00000) >> 20) | 0xFFFFF000 : (bits & 0xFFF00000) >> 20,
      HAS_RD | HAS_RS1 | HAS_IMM | HAS_LOAD
    );
    case 0x2027: return inst_rs1_rs2_imm(
      bits, "fsw", RISCV_FSW, rs1, rs2,
      ((bits & 0x80000000) ? ((bits & 0xFE000000) >> 20) | 0xFFFFF000 : (bits & 0xFE000000) >> 20) | ((bits & 0x00000F80) >> 7),
      HAS_RS1 | HAS_RS2 | HAS_IMM | HAS_STORE
    );
    case 0x3027: return inst_rs1_rs2_imm(
      bits, "fsd", RISCV_FSD, rs1, rs2,
      ((bits & 0x80000000) ? ((bits & 0xFE000000) >> 20) | 0xFFFFF000 : (bits & 0xFE000000) >> 20) | ((bits & 0x00000F80) >> 7),
      HAS_RS1 | HAS_RS2 | HAS_IMM | HAS_STORE
    );
    case 0x4027: return inst_rs1_rs2_imm(
      bits, "fsq", RISCV_FSQ, rs1, rs2,
      ((bits & 0x80000000) ? ((bits & 0xFE000000) >> 20) | 0xFFFFF000 : (bits & 0xFE000000) >> 20) | ((bits & 0x00000F80) >> 7),
      HAS_RS1 | HAS_RS2 | HAS_IMM | HAS_STORE
    );
  }
  switch (bits & 0x600007f) {
    case 0x0000043: return inst_all_regs(bits, "fmadd.s", RISCV_FMADD_S, rd, rs1, rs2, rs3, HAS_RD | HAS_RS1 | HAS_RS2 | HAS_RS3);
    case 0x0000047: return inst_all_regs(bits, "fmsub.s", RISCV_FMSUB_S, rd, rs1, rs2, rs3, HAS_RD | HAS_RS1 | HAS_RS2 | HAS_RS3);
    case 0x000004b: return inst_all_regs(bits, "fnmsub.s", RISCV_FNMSUB_S, rd, rs1, rs2, rs3, HAS_RD | HAS_RS1 | HAS_RS2 | HAS_RS3);
    case 0x000004f: return inst_all_regs(bits, "fnmadd.s", RISCV_FNMADD_S, rd, rs1, rs2, rs3, HAS_RD | HAS_RS1 | HAS_RS2 | HAS_RS3);
    case 0x2000043: return inst_all_regs(bits, "fmadd.d", RISCV_FMADD_D, rd, rs1, rs2, rs3, HAS_RD | HAS_RS1 | HAS_RS2 | HAS_RS3);
    case 0x2000047: return inst_all_regs(bits, "fmsub.d", RISCV_FMSUB_D, rd, rs1, rs2, rs3, HAS_RD | HAS_RS1 | HAS_RS2 | HAS_RS3);
    case 0x200004b: return inst_all_regs(bits, "fnmsub.d", RISCV_FNMSUB_D, rd, rs1, rs2, rs3, HAS_RD | HAS_RS1 | HAS_RS2 | HAS_RS3);
    case 0x200004f: return inst_all_regs(bits, "fnmadd.d", RISCV_FNMADD_D, rd, rs1, rs2, rs3, HAS_RD | HAS_RS1 | HAS_RS2 | HAS_RS3);
    case 0x6000043: return inst_all_regs(bits, "fmadd.q", RISCV_FMADD_Q, rd, rs1, rs2, rs3, HAS_RD | HAS_RS1 | HAS_RS2 | HAS_RS3);
    case 0x6000047: return inst_all_regs(bits, "fmsub.q", RISCV_FMSUB_Q, rd, rs1, rs2, rs3, HAS_RD | HAS_RS1 | HAS_RS2 | HAS_RS3);
    case 0x600004b: return inst_all_regs(bits, "fnmsub.q", RISCV_FNMSUB_Q, rd, rs1, rs2, rs3, HAS_RD | HAS_RS1 | HAS_RS2 | HAS_RS3);
    case 0x600004f: return inst_all_regs(bits, "fnmadd.q", RISCV_FNMADD_Q, rd, rs1, rs2, rs3, HAS_RD | HAS_RS1 | HAS_RS2 | HAS_RS3);
  }
  switch (bits & 0xf800707f) {
    case 0x0000202f: return inst_rd_rs1_rs2(bits, "amoadd.w", RISCV_AMOADD_W, rd, rs1, rs2, HAS_RD | HAS_RS1 | HAS_RS2);
    case 0x2000202f: return inst_rd_rs1_rs2(bits, "amoxor.w", RISCV_AMOXOR_W, rd, rs1, rs2, HAS_RD | HAS_RS1 | HAS_RS2);
    case 0x4000202f: return inst_rd_rs1_rs2(bits, "amoor.w", RISCV_AMOOR_W, rd, rs1, rs2, HAS_RD | HAS_RS1 | HAS_RS2);
    case 0x6000202f: return inst_rd_rs1_rs2(bits, "amoand.w", RISCV_AMOAND_W, rd, rs1, rs2, HAS_RD | HAS_RS1 | HAS_RS2);
    case 0x8000202f: return inst_rd_rs1_rs2(bits, "amomin.w", RISCV_AMOMIN_W, rd, rs1, rs2, HAS_RD | HAS_RS1 | HAS_RS2);
    case 0xa000202f: return inst_rd_rs1_rs2(bits, "amomax.w", RISCV_AMOMAX_W, rd, rs1, rs2, HAS_RD | HAS_RS1 | HAS_RS2);
    case 0xc000202f: return inst_rd_rs1_rs2(bits, "amominu.w", RISCV_AMOMINU_W, rd, rs1, rs2, HAS_RD | HAS_RS1 | HAS_RS2);
    case 0xe000202f: return inst_rd_rs1_rs2(bits, "amomaxu.w", RISCV_AMOMAXU_W, rd, rs1, rs2, HAS_RD | HAS_RS1 | HAS_RS2);
    case 0x0800202f: return inst_rd_rs1_rs2(bits, "amoswap.w", RISCV_AMOSWAP_W, rd, rs1, rs2, HAS_RD | HAS_RS1 | HAS_RS2);
    case 0x1800202f: return inst_rd_rs1_rs2(bits, "sc.w", RISCV_SC_W, rd, rs1, rs2, HAS_RD | HAS_RS1 | HAS_RS2 | HAS_STORE);
    case 0x1000202f: return inst_rd_rs1(bits, "lr.w", RISCV_LR_W, rd, rs1, HAS_RD | HAS_RS1 | HAS_LOAD);
  }
  switch (bits & 0xfc00707f) {
    case 0x00001013: return inst_rd_rs1(bits, "slli", RISCV_SLLI, rd, rs1, HAS_RD | HAS_RS1);
    case 0x00005013: return inst_rd_rs1(bits, "srli", RISCV_SRLI, rd, rs1, HAS_RD | HAS_RS1);
    case 0x40005013: return inst_rd_rs1(bits, "srai", RISCV_SRAI, rd, rs1, HAS_RD | HAS_RS1);
  }
  switch (bits & 0xfe00007f) {
    case 0x00000053: return inst_rd_rs1_rs2(bits, "fadd.s", RISCV_FADD_S, rd, rs1, rs2, HAS_RD | HAS_RS1 | HAS_RS2);
    case 0x08000053: return inst_rd_rs1_rs2(bits, "fsub.s", RISCV_FSUB_D, rd, rs1, rs2, HAS_RD | HAS_RS1 | HAS_RS2);
    case 0x10000053: return inst_rd_rs1_rs2(bits, "fmul.s", RISCV_FMUL_S, rd, rs1, rs2, HAS_RD | HAS_RS1 | HAS_RS2);
    case 0x18000053: return inst_rd_rs1_rs2(bits, "fdiv.s", RISCV_FDIV_S, rd, rs1, rs2, HAS_RD | HAS_RS1 | HAS_RS2);
    case 0x02000053: return inst_rd_rs1_rs2(bits, "fadd.d", RISCV_FADD_D, rd, rs1, rs2, HAS_RD | HAS_RS1 | HAS_RS2);
    case 0x0a000053: return inst_rd_rs1_rs2(bits, "fsub.d", RISCV_FSUB_D, rd, rs1, rs2, HAS_RD | HAS_RS1 | HAS_RS2);
    case 0x12000053: return inst_rd_rs1_rs2(bits, "fmul.d", RISCV_FMUL_D, rd, rs1, rs2, HAS_RD | HAS_RS1 | HAS_RS2);
    case 0x1a000053: return inst_rd_rs1_rs2(bits, "fdiv.d", RISCV_FDIV_D, rd, rs1, rs2, HAS_RD | HAS_RS1 | HAS_RS2);
    case 0x06000053: return inst_rd_rs1_rs2(bits, "fadd.q", RISCV_FADD_Q, rd, rs1, rs2, HAS_RD | HAS_RS1 | HAS_RS2);
    case 0x0e000053: return inst_rd_rs1_rs2(bits, "fsub.q", RISCV_FSUB_Q, rd, rs1, rs2, HAS_RD | HAS_RS1 | HAS_RS2);
    case 0x16000053: return inst_rd_rs1_rs2(bits, "fmul.q", RISCV_FMUL_Q, rd, rs1, rs2, HAS_RD | HAS_RS1 | HAS_RS2);
    case 0x1e000053: return inst_rd_rs1_rs2(bits, "fdiv.q", RISCV_FDIV_Q, rd, rs1, rs2, HAS_RD | HAS_RS1 | HAS_RS2);
  }
  switch (bits & 0xfe00707f) {
    case 0x00000033: return inst_rd_rs1_rs2(bits, "add", RISCV_ADD, rd, rs1, rs2, HAS_RD | HAS_RS1 | HAS_RS2);
    case 0x40000033: return inst_rd_rs1_rs2(bits, "sub", RISCV_SUB, rd, rs1, rs2, HAS_RD | HAS_RS1 | HAS_RS2);
    case 0x00001033: return inst_rd_rs1_rs2(bits, "sll", RISCV_SLL, rd, rs1, rs2, HAS_RD | HAS_RS1 | HAS_RS2);
    case 0x00002033: return inst_rd_rs1_rs2(bits, "slt", RISCV_SLT, rd, rs1, rs2, HAS_RD | HAS_RS1 | HAS_RS2);
    case 0x00003033: return inst_rd_rs1_rs2(bits, "sltu", RISCV_SLTU, rd, rs1, rs2, HAS_RD | HAS_RS1 | HAS_RS2);
    case 0x00004033: return inst_rd_rs1_rs2(bits, "xor", RISCV_XOR, rd, rs1, rs2, HAS_RD | HAS_RS1 | HAS_RS2);
    case 0x00005033: return inst_rd_rs1_rs2(bits, "srl", RISCV_SRL, rd, rs1, rs2, HAS_RD | HAS_RS1 | HAS_RS2);
    case 0x40005033: return inst_rd_rs1_rs2(bits, "sra", RISCV_SRA, rd, rs1, rs2, HAS_RD | HAS_RS1 | HAS_RS2);
    case 0x00006033: return inst_rd_rs1_rs2(bits, "or", RISCV_OR, rd, rs1, rs2, HAS_RD | HAS_RS1 | HAS_RS2);
    case 0x00007033: return inst_rd_rs1_rs2(bits, "and", RISCV_AND, rd, rs1, rs2, HAS_RD | HAS_RS1 | HAS_RS2);
    case 0x0000101b: return inst_rd_rs1(bits, "slliw", RISCV_SLLIW, rd, rs1, HAS_RD | HAS_RS1);
    case 0x0000501b: return inst_rd_rs1(bits, "srliw", RISCV_SRLIW, rd, rs1, HAS_RD | HAS_RS1);
    case 0x4000501b: return inst_rd_rs1(bits, "sraiw", RISCV_SRAIW, rd, rs1, HAS_RD | HAS_RS1);
    case 0x0000003b: return inst_rd_rs1_rs2(bits, "addw", RISCV_ADDW, rd, rs1, rs2, HAS_RD | HAS_RS1 | HAS_RS2);
    case 0x4000003b: return inst_rd_rs1_rs2(bits, "subw", RISCV_SUBW, rd, rs1, rs2, HAS_RD | HAS_RS1 | HAS_RS2);
    case 0x0000103b: return inst_rd_rs1_rs2(bits, "sllw", RISCV_SLLW, rd, rs1, rs2, HAS_RD | HAS_RS1 | HAS_RS2);
    case 0x0000503b: return inst_rd_rs1_rs2(bits, "srlw", RISCV_ADDW, rd, rs1, rs2, HAS_RD | HAS_RS1 | HAS_RS2);
    case 0x4000503b: return inst_rd_rs1_rs2(bits, "sraw", RISCV_SRAW, rd, rs1, rs2, HAS_RD | HAS_RS1 | HAS_RS2);
    case 0x02000033: return inst_rd_rs1_rs2(bits, "mul", RISCV_MUL, rd, rs1, rs2, HAS_RD | HAS_RS1 | HAS_RS2);
    case 0x0200003b: return inst_rd_rs1_rs2(bits, "mulw", RISCV_MULW, rd, rs1, rs2, HAS_RD | HAS_RS1 | HAS_RS2);
    case 0x02001033: return inst_rd_rs1_rs2(bits, "mulh", RISCV_MULH, rd, rs1, rs2, HAS_RD | HAS_RS1 | HAS_RS2);
    case 0x02002033: return inst_rd_rs1_rs2(bits, "mulhsu", RISCV_MULHSU, rd, rs1, rs2, HAS_RD | HAS_RS1 | HAS_RS2);
    case 0x02003033: return inst_rd_rs1_rs2(bits, "mulhu", RISCV_MULHU, rd, rs1, rs2, HAS_RD | HAS_RS1 | HAS_RS2);
    case 0x02004033: return inst_rd_rs1_rs2(bits, "div", RISCV_DIV, rd, rs1, rs2, HAS_RD | HAS_RS1 | HAS_RS2);
    case 0x0200403b: return inst_rd_rs1_rs2(bits, "divw", RISCV_DIVW, rd, rs1, rs2, HAS_RD | HAS_RS1 | HAS_RS2);
    case 0x02005033: return inst_rd_rs1_rs2(bits, "divu", RISCV_DIVU, rd, rs1, rs2, HAS_RD | HAS_RS1 | HAS_RS2);
    case 0x0200503b: return inst_rd_rs1_rs2(bits, "divuw", RISCV_ADDW, rd, rs1, rs2, HAS_RD | HAS_RS1 | HAS_RS2);
    case 0x02006033: return inst_rd_rs1_rs2(bits, "rem", RISCV_REM, rd, rs1, rs2, HAS_RD | HAS_RS1 | HAS_RS2);
    case 0x0200603b: return inst_rd_rs1_rs2(bits, "remw", RISCV_REMW, rd, rs1, rs2, HAS_RD | HAS_RS1 | HAS_RS2);
    case 0x02007033: return inst_rd_rs1_rs2(bits, "remu", RISCV_REMU, rd, rs1, rs2, HAS_RD | HAS_RS1 | HAS_RS2);
    case 0x20000053: return inst_rd_rs1_rs2(bits, "fsgnj.s", RISCV_FSGNJ_S, rd, rs1, rs2, HAS_RD | HAS_RS1 | HAS_RS2);
    case 0x20001053: return inst_rd_rs1_rs2(bits, "fsgnjn.s", RISCV_FSGNJN_S, rd, rs1, rs2, HAS_RD | HAS_RS1 | HAS_RS2);
    case 0x20002053: return inst_rd_rs1_rs2(bits, "fsgnjx.s", RISCV_FSGNJX_S, rd, rs1, rs2, HAS_RD | HAS_RS1 | HAS_RS2);
    case 0x28000053: return inst_rd_rs1_rs2(bits, "fmin.s", RISCV_FMIN_S, rd, rs1, rs2, HAS_RD | HAS_RS1 | HAS_RS2);
    case 0x28001053: return inst_rd_rs1_rs2(bits, "fmax.s", RISCV_FMAX_S, rd, rs1, rs2, HAS_RD | HAS_RS1 | HAS_RS2);
    case 0x22000053: return inst_rd_rs1_rs2(bits, "fsgnj.d", RISCV_FSGNJ_D, rd, rs1, rs2, HAS_RD | HAS_RS1 | HAS_RS2);
    case 0x22001053: return inst_rd_rs1_rs2(bits, "fsgnjn.d", RISCV_FSGNJN_D, rd, rs1, rs2, HAS_RD | HAS_RS1 | HAS_RS2);
    case 0x22002053: return inst_rd_rs1_rs2(bits, "fsgnjx.d", RISCV_FSGNJX_D, rd, rs1, rs2, HAS_RD | HAS_RS1 | HAS_RS2);
    case 0x2a000053: return inst_rd_rs1_rs2(bits, "fmin.d", RISCV_FMIN_D, rd, rs1, rs2, HAS_RD | HAS_RS1 | HAS_RS2);
    case 0x2a001053: return inst_rd_rs1_rs2(bits, "fmax.d", RISCV_FMAX_D, rd, rs1, rs2, HAS_RD | HAS_RS1 | HAS_RS2);
    case 0x26000053: return inst_rd_rs1_rs2(bits, "fsgnj.q", RISCV_FSGNJ_Q, rd, rs1, rs2, HAS_RD | HAS_RS1 | HAS_RS2);
    case 0x26001053: return inst_rd_rs1_rs2(bits, "fsgnjn.q", RISCV_FSGNJN_Q, rd, rs1, rs2, HAS_RD | HAS_RS1 | HAS_RS2);
    case 0x26002053: return inst_rd_rs1_rs2(bits, "fsgnjx.q", RISCV_FSGNJX_Q, rd, rs1, rs2, HAS_RD | HAS_RS1 | HAS_RS2);
    case 0x2e000053: return inst_rd_rs1_rs2(bits, "fmin.q", RISCV_FMIN_Q, rd, rs1, rs2, HAS_RD | HAS_RS1 | HAS_RS2);
    case 0x2e001053: return inst_rd_rs1_rs2(bits, "fmax.q", RISCV_FMAX_Q, rd, rs1, rs2, HAS_RD | HAS_RS1 | HAS_RS2);
    case 0xa0000053: return inst_rd_rs1_rs2(bits, "fle.s", RISCV_FLE_S, rd, rs1, rs2, HAS_RD | HAS_RS1 | HAS_RS2);
    case 0xa0001053: return inst_rd_rs1_rs2(bits, "flt.s", RISCV_FLT_S, rd, rs1, rs2, HAS_RD | HAS_RS1 | HAS_RS2);
    case 0xa0002053: return inst_rd_rs1_rs2(bits, "feq.s", RISCV_FEQ_S, rd, rs1, rs2, HAS_RD | HAS_RS1 | HAS_RS2);
    case 0xa2000053: return inst_rd_rs1_rs2(bits, "fle.d", RISCV_FLE_D, rd, rs1, rs2, HAS_RD | HAS_RS1 | HAS_RS2);
    case 0xa2001053: return inst_rd_rs1_rs2(bits, "flt.d", RISCV_FLT_D, rd, rs1, rs2, HAS_RD | HAS_RS1 | HAS_RS2);
    case 0xa2002053: return inst_rd_rs1_rs2(bits, "feq.d", RISCV_FEQ_D, rd, rs1, rs2, HAS_RD | HAS_RS1 | HAS_RS2);
    case 0xa6000053: return inst_rd_rs1_rs2(bits, "fle.q", RISCV_FLE_Q, rd, rs1, rs2, HAS_RD | HAS_RS1 | HAS_RS2);
    case 0xa6001053: return inst_rd_rs1_rs2(bits, "flt.q", RISCV_FLT_Q, rd, rs1, rs2, HAS_RD | HAS_RS1 | HAS_RS2);
    case 0xa6002053: return inst_rd_rs1_rs2(bits, "feq.q", RISCV_FEQ_Q, rd, rs1, rs2, HAS_RD | HAS_RS1 | HAS_RS2);
  }
  switch (bits & 0xfe007fff) {
    case 0x12000073: return inst_rs1_rs2(bits, "sfence.vma", RISCV_SFENCE_VMA, rs1, rs2, HAS_RS1 | HAS_RS2);
  }
  switch (bits & 0xfff0007f) {
    case 0x58000053: return inst_rd_rs1(bits, "fsqrt.s", RISCV_FSQRT_S, rd, rs1, HAS_RD | HAS_RS1);
    case 0x40100053: return inst_rd_rs1(bits, "fcvt.s.d", RISCV_FCVT_S_D, rd, rs1, HAS_RD | HAS_RS1);
    case 0x40200053: return inst_rd_rs1(bits, "fcvt.d.s", RISCV_FCVT_D_S, rd, rs1, HAS_RD | HAS_RS1);
    case 0x5a000053: return inst_rd_rs1(bits, "fsqrt.d", RISCV_FSQRT_D, rd, rs1, HAS_RD | HAS_RS1);
    case 0x40300053: return inst_rd_rs1(bits, "fcvt.s.q", RISCV_FCVT_S_Q, rd, rs1, HAS_RD | HAS_RS1);
    case 0x46000053: return inst_rd_rs1(bits, "fcvt.q.s", RISCV_FCVT_Q_S, rd, rs1, HAS_RD | HAS_RS1);
    case 0x42300053: return inst_rd_rs1(bits, "fcvt.d.q", RISCV_FCVT_D_Q, rd, rs1, HAS_RD | HAS_RS1);
    case 0x46100053: return inst_rd_rs1(bits, "fcvt.q.d", RISCV_FCVT_Q_D, rd, rs1, HAS_RD | HAS_RS1);
    case 0x5e000053: return inst_rd_rs1(bits, "fsqrt.q", RISCV_FSQRT_Q, rd, rs1, HAS_RD | HAS_RS1);
    case 0xc0000053: return inst_rd_rs1(bits, "fcvt.w.s", RISCV_FCVT_W_S, rd, rs1, HAS_RD | HAS_RS1);
    case 0xc0100053: return inst_rd_rs1(bits, "fcvt.wu.s", RISCV_FCVT_WU_S, rd, rs1, HAS_RD | HAS_RS1);
    case 0xc0200053: return inst_rd_rs1(bits, "fcvt.l.s", RISCV_FCVT_L_S, rd, rs1, HAS_RD | HAS_RS1);
    case 0xc0300053: return inst_rd_rs1(bits, "fcvt.lu.s", RISCV_FCVT_LU_S, rd, rs1, HAS_RD | HAS_RS1);
    case 0xc2000053: return inst_rd_rs1(bits, "fcvt.w.d", RISCV_FCVT_W_D, rd, rs1, HAS_RD | HAS_RS1);
    case 0xc2100053: return inst_rd_rs1(bits, "fcvt.wu.d", RISCV_FCVT_WU_D, rd, rs1, HAS_RD | HAS_RS1);
    case 0xc2200053: return inst_rd_rs1(bits, "fcvt.l.d", RISCV_FCVT_L_D, rd, rs1, HAS_RD | HAS_RS1);
    case 0xc2300053: return inst_rd_rs1(bits, "fcvt.lu.d", RISCV_FCVT_LU_D, rd, rs1, HAS_RD | HAS_RS1);
    case 0xc6000053: return inst_rd_rs1(bits, "fcvt.w.q", RISCV_FCVT_W_Q, rd, rs1, HAS_RD | HAS_RS1);
    case 0xc6100053: return inst_rd_rs1(bits, "fcvt.wu.q", RISCV_FCVT_WU_Q, rd, rs1, HAS_RD | HAS_RS1);
    case 0xc6200053: return inst_rd_rs1(bits, "fcvt.l.q", RISCV_FCVT_L_Q, rd, rs1, HAS_RD | HAS_RS1);
    case 0xc6300053: return inst_rd_rs1(bits, "fcvt.lu.q", RISCV_FCVT_LU_Q, rd, rs1, HAS_RD | HAS_RS1);
    case 0xd0000053: return inst_rd_rs1(bits, "fcvt.s.w", RISCV_FCVT_S_W, rd, rs1, HAS_RD | HAS_RS1);
    case 0xd0100053: return inst_rd_rs1(bits, "fcvt.s.wu", RISCV_FCVT_S_WU, rd, rs1, HAS_RD | HAS_RS1);
    case 0xd0200053: return inst_rd_rs1(bits, "fcvt.s.l", RISCV_FCVT_S_L, rd, rs1, HAS_RD | HAS_RS1);
    case 0xd0300053: return inst_rd_rs1(bits, "fcvt.s.lu", RISCV_FCVT_S_LU, rd, rs1, HAS_RD | HAS_RS1);
    case 0xd2000053: return inst_rd_rs1(bits, "fcvt.d.w", RISCV_FCVT_D_W, rd, rs1, HAS_RD | HAS_RS1);
    case 0xd2100053: return inst_rd_rs1(bits, "fcvt.d.wu", RISCV_FCVT_D_WU, rd, rs1, HAS_RD | HAS_RS1);
    case 0xd2200053: return inst_rd_rs1(bits, "fcvt.d.l", RISCV_FCVT_D_L, rd, rs1, HAS_RD | HAS_RS1);
    case 0xd2300053: return inst_rd_rs1(bits, "fcvt.d.lu", RISCV_FCVT_D_LU, rd, rs1, HAS_RD | HAS_RS1);
    case 0xd6000053: return inst_rd_rs1(bits, "fcvt.q.w", RISCV_FCVT_Q_W, rd, rs1, HAS_RD | HAS_RS1);
    case 0xd6100053: return inst_rd_rs1(bits, "fcvt.q.wu", RISCV_FCVT_Q_WU, rd, rs1, HAS_RD | HAS_RS1);
    case 0xd6200053: return inst_rd_rs1(bits, "fcvt.q.l", RISCV_FCVT_Q_L, rd, rs1, HAS_RD | HAS_RS1);
    case 0xd6300053: return inst_rd_rs1(bits, "fcvt.q.lu", RISCV_FCVT_Q_LU, rd, rs1, HAS_RD | HAS_RS1);
  }
  switch (bits & 0xfff0707f) {
    case 0xe0000053: return inst_rd_rs1(bits, "fmv.x.w", RISCV_FMV_X_W, rd, rs1, HAS_RD | HAS_RS1);
    case 0xe0001053: return inst_rd_rs1(bits, "fclass.s", RISCV_FCLASS_S, rd, rs1, HAS_RD | HAS_RS1);
    case 0xe2000053: return inst_rd_rs1(bits, "fmv.x.d", RISCV_FMV_X_D, rd, rs1, HAS_RD | HAS_RS1);
    case 0xe2001053: return inst_rd_rs1(bits, "fclass.d", RISCV_FCLASS_D, rd, rs1, HAS_RD | HAS_RS1);
    case 0xe6000053: return inst_rd_rs1(bits, "fmv.x.q", RISCV_FMV_X_Q, rd, rs1, HAS_RD | HAS_RS1);
    case 0xe6001053: return inst_rd_rs1(bits, "fclass.q", RISCV_FCLASS_Q, rd, rs1, HAS_RD | HAS_RS1);
    case 0xf0000053: return inst_rd_rs1(bits, "fmv.w.x", RISCV_FMV_W_X, rd, rs1, HAS_RD | HAS_RS1);
    case 0xf2000053: return inst_rd_rs1(bits, "fmv.d.x", RISCV_FMV_D_X, rd, rs1, HAS_RD | HAS_RS1);
    case 0xf6000053: return inst_rd_rs1(bits, "fmv.q.x", RISCV_FMV_Q_X, rd, rs1, HAS_RD | HAS_RS1);
  }
  switch (bits) {
    case 0x73: return inst(bits, "ecall", RISCV_ECALL, 0);
    case 0x100073: return inst(bits, "ebreak", RISCV_EBREAK, 0);
    case 0x200073: return inst(bits, "uret", RISCV_URET, 0);
    case 0x10200073: return inst(bits, "sret", RISCV_SRET, 0);
    case 0x30200073: return inst(bits, "mret", RISCV_MRET, 0);
    case 0x7b200073: return inst(bits, "dret", RISCV_DRET, 0);
    case 0x10500073: return inst(bits, "wfi", RISCV_WFI, 0);
  }
  return decoded_instruction_t{.bits=bits, .name=""};
}

} // namespace policy_engine