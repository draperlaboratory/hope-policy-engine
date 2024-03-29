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

#ifndef INSTRUCTION_DECODER
#define INSTRUCTION_DECODER

#ifdef __cplusplus
namespace policy_engine {
#endif

enum op_t {
  RISCV_INVALID,
  RISCV_BEQ,
  RISCV_BNE,
  RISCV_BLT,
  RISCV_BGE,
  RISCV_BLTU,
  RISCV_BGEU,
  RISCV_JALR,
  RISCV_JAL,
  RISCV_LUI,
  RISCV_AUIPC,
  RISCV_ADDI,
  RISCV_SLLI,
  RISCV_SLTI,
  RISCV_SLTIU,
  RISCV_XORI,
  RISCV_SRLI,
  RISCV_SRAI,
  RISCV_ORI,
  RISCV_ANDI,
  RISCV_ADD,
  RISCV_SUB,
  RISCV_SLL,
  RISCV_SLT,
  RISCV_SLTU,
  RISCV_XOR,
  RISCV_SRL,
  RISCV_SRA,
  RISCV_OR,
  RISCV_AND,
  RISCV_ADDIW,
  RISCV_SLLIW,
  RISCV_SRLIW,
  RISCV_SRAIW,
  RISCV_ADDW,
  RISCV_SUBW,
  RISCV_SLLW,
  RISCV_SRLW,
  RISCV_SRAW,
  RISCV_LB,
  RISCV_LH,
  RISCV_LW,
  RISCV_LD,
  RISCV_LBU,
  RISCV_LHU,
  RISCV_LWU,
  RISCV_SB,
  RISCV_SH,
  RISCV_SW,
  RISCV_SD,
  RISCV_FENCE,
  RISCV_FENCE_I,
  RISCV_MUL,
  RISCV_MULH,
  RISCV_MULHSU,
  RISCV_MULHU,
  RISCV_DIV,
  RISCV_DIVU,
  RISCV_REM,
  RISCV_REMU,
  RISCV_AMOADD_W,
  RISCV_AMOXOR_W,
  RISCV_AMOOR_W,
  RISCV_AMOAND_W,
  RISCV_AMOMIN_W,
  RISCV_AMOMAX_W,
  RISCV_AMOMINU_W,
  RISCV_AMOMAXU_W,
  RISCV_AMOSWAP_W,
  RISCV_LR_W,
  RISCV_SC_W,
  RISCV_AMOADD_D,
  RISCV_AMOXOR_D,
  RISCV_AMOOR_D,
  RISCV_AMOAND_D,
  RISCV_AMOMIN_D,
  RISCV_AMOMAX_D,
  RISCV_AMOMINU_D,
  RISCV_AMOMAXU_D,
  RISCV_AMOSWAP_D,
  RISCV_LR_D,
  RISCV_SC_D,
  RISCV_ECALL,
  RISCV_EBREAK,
  RISCV_URET,
  RISCV_SRET,
  RISCV_MRET,
  RISCV_DRET,
  RISCV_SFENCE_VMA,
  RISCV_WFI,
  RISCV_CSRRW,
  RISCV_CSRRS,
  RISCV_CSRRC,
  RISCV_CSRRWI,
  RISCV_CSRRSI,
  RISCV_CSRRCI,
  RISCV_FADD_S,
  RISCV_FSUB_S,
  RISCV_FMUL_S,
  RISCV_FDIV_S,
  RISCV_FSGNJ_S,
  RISCV_FSGNJN_S,
  RISCV_FSGNJX_S,
  RISCV_FMIN_S,
  RISCV_FMAX_S,
  RISCV_FSQRT_S,
  RISCV_FADD_D,
  RISCV_FSUB_D,
  RISCV_FMUL_D,
  RISCV_FDIV_D,
  RISCV_FSGNJ_D,
  RISCV_FSGNJN_D,
  RISCV_FSGNJX_D,
  RISCV_FMIN_D,
  RISCV_FMAX_D,
  RISCV_FCVT_S_D,
  RISCV_FCVT_D_S,
  RISCV_FSQRT_D,
  RISCV_FADD_Q,
  RISCV_FSUB_Q,
  RISCV_FMUL_Q,
  RISCV_FDIV_Q,
  RISCV_FSGNJ_Q,
  RISCV_FSGNJN_Q,
  RISCV_FSGNJX_Q,
  RISCV_FMIN_Q,
  RISCV_FMAX_Q,
  RISCV_FCVT_S_Q,
  RISCV_FCVT_Q_S,
  RISCV_FCVT_D_Q,
  RISCV_FCVT_Q_D,
  RISCV_FSQRT_Q,
  RISCV_FLE_S,
  RISCV_FLT_S,
  RISCV_FEQ_S,
  RISCV_FLE_D,
  RISCV_FLT_D,
  RISCV_FEQ_D,
  RISCV_FLE_Q,
  RISCV_FLT_Q,
  RISCV_FEQ_Q,
  RISCV_FCVT_W_S,
  RISCV_FCVT_WU_S,
  RISCV_FCVT_L_S,
  RISCV_FCVT_LU_S,
  RISCV_FMV_X_W,
  RISCV_FCLASS_S,
  RISCV_FCVT_W_D,
  RISCV_FCVT_WU_D,
  RISCV_FCVT_L_D,
  RISCV_FCVT_LU_D,
  RISCV_FMV_X_D,
  RISCV_FCLASS_D,
  RISCV_FCVT_W_Q,
  RISCV_FCVT_WU_Q,
  RISCV_FCVT_L_Q,
  RISCV_FCVT_LU_Q,
  RISCV_FMV_X_Q,
  RISCV_FCLASS_Q,
  RISCV_FCVT_S_W,
  RISCV_FCVT_S_WU,
  RISCV_FCVT_S_L,
  RISCV_FCVT_S_LU,
  RISCV_FMV_W_X,
  RISCV_FCVT_D_W,
  RISCV_FCVT_D_WU,
  RISCV_FCVT_D_L,
  RISCV_FCVT_D_LU,
  RISCV_FMV_D_X,
  RISCV_FCVT_Q_W,
  RISCV_FCVT_Q_WU,
  RISCV_FCVT_Q_L,
  RISCV_FCVT_Q_LU,
  RISCV_FMV_Q_X,
  RISCV_FLW,
  RISCV_FLD,
  RISCV_FLQ,
  RISCV_FSW,
  RISCV_FSD,
  RISCV_FSQ,
  RISCV_FMADD_S,
  RISCV_FMSUB_S,
  RISCV_FNMSUB_S,
  RISCV_FNMADD_S,
  RISCV_FMADD_D,
  RISCV_FMSUB_D,
  RISCV_FNMSUB_D,
  RISCV_FNMADD_D,
  RISCV_FMADD_Q,
  RISCV_FMSUB_Q,
  RISCV_FNMSUB_Q,
  RISCV_FNMADD_Q,
  RISCV_MULW,
  RISCV_REMW,
  RISCV_DIVW,
  RISCV_DIVUW,
  RISCV_REMUW
};

#ifdef __cplusplus
} // namespace policy_engine
#endif

#endif