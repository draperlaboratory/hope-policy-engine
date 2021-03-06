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

#define RISCV_BEQ 1
#define RISCV_BNE 2
#define RISCV_BLT 3
#define RISCV_BGE 4
#define RISCV_BLTU 5
#define RISCV_BGEU 6
#define RISCV_JALR 7
#define RISCV_JAL 8
#define RISCV_LUI 9
#define RISCV_AUIPC 10
#define RISCV_ADDI 11
#define RISCV_SLLI 12
#define RISCV_SLTI 13
#define RISCV_SLTIU 14
#define RISCV_XORI 15
#define RISCV_SRLI 16
#define RISCV_SRAI 17
#define RISCV_ORI 18
#define RISCV_ANDI 19
#define RISCV_ADD 20
#define RISCV_SUB 21
#define RISCV_SLL 22
#define RISCV_SLT 23
#define RISCV_SLTU 24
#define RISCV_XOR 25
#define RISCV_SRL 26
#define RISCV_SRA 27
#define RISCV_OR 28
#define RISCV_AND 29
#define RISCV_ADDIW 30
#define RISCV_SLLIW 31
#define RISCV_SRLIW 32
#define RISCV_SRAIW 33
#define RISCV_ADDW 34
#define RISCV_SUBW 35
#define RISCV_SLLW 36
#define RISCV_SRLW 37
#define RISCV_SRAW 38
#define RISCV_LB 39
#define RISCV_LH 40
#define RISCV_LW 41
#define RISCV_LD 42
#define RISCV_LBU 43
#define RISCV_LHU 44
#define RISCV_LWU 45
#define RISCV_SB 46
#define RISCV_SH 47
#define RISCV_SW 48
#define RISCV_SD 49
#define RISCV_FENCE 50
#define RISCV_FENCE_I 51
#define RISCV_MUL 52
#define RISCV_MULH 53
#define RISCV_MULHSU 54
#define RISCV_MULHU 55
#define RISCV_DIV 56
#define RISCV_DIVU 57
#define RISCV_REM 58
#define RISCV_REMU 59
#define RISCV_AMOADD_W 60
#define RISCV_AMOXOR_W 61
#define RISCV_AMOOR_W 62
#define RISCV_AMOAND_W 63
#define RISCV_AMOMIN_W 64
#define RISCV_AMOMAX_W 65
#define RISCV_AMOMINU_W 66
#define RISCV_AMOMAXU_W 67
#define RISCV_AMOSWAP_W 68
#define RISCV_LR_W 69
#define RISCV_SC_W 70
#define RISCV_ECALL 71
#define RISCV_EBREAK 72
#define RISCV_URET 73
#define RISCV_SRET 74
#define RISCV_MRET 75
#define RISCV_DRET 76
#define RISCV_SFENCE_VMA 77
#define RISCV_WFI 78
#define RISCV_CSRRW 79
#define RISCV_CSRRS 80
#define RISCV_CSRRC 81
#define RISCV_CSRRWI 82
#define RISCV_CSRRSI 83
#define RISCV_CSRRCI 84
#define RISCV_FADD_S 85
#define RISCV_FSUB_S 86
#define RISCV_FMUL_S 87
#define RISCV_FDIV_S 88
#define RISCV_FSGNJ_S 89
#define RISCV_FSGNJN_S 90
#define RISCV_FSGNJX_S 91
#define RISCV_FMIN_S 92
#define RISCV_FMAX_S 93
#define RISCV_FSQRT_S 94
#define RISCV_FADD_D 95
#define RISCV_FSUB_D 96
#define RISCV_FMUL_D 97
#define RISCV_FDIV_D 98
#define RISCV_FSGNJ_D 99
#define RISCV_FSGNJN_D 100
#define RISCV_FSGNJX_D 101
#define RISCV_FMIN_D 102
#define RISCV_FMAX_D 103
#define RISCV_FCVT_S_D 104
#define RISCV_FCVT_D_S 105
#define RISCV_FSQRT_D 106
#define RISCV_FADD_Q 107
#define RISCV_FSUB_Q 108
#define RISCV_FMUL_Q 109
#define RISCV_FDIV_Q 110
#define RISCV_FSGNJ_Q 111
#define RISCV_FSGNJN_Q 112
#define RISCV_FSGNJX_Q 113
#define RISCV_FMIN_Q 114
#define RISCV_FMAX_Q 115
#define RISCV_FCVT_S_Q 116
#define RISCV_FCVT_Q_S 117
#define RISCV_FCVT_D_Q 118
#define RISCV_FCVT_Q_D 119
#define RISCV_FSQRT_Q 120
#define RISCV_FLE_S 121
#define RISCV_FLT_S 122
#define RISCV_FEQ_S 123
#define RISCV_FLE_D 124
#define RISCV_FLT_D 125
#define RISCV_FEQ_D 126
#define RISCV_FLE_Q 127
#define RISCV_FLT_Q 128
#define RISCV_FEQ_Q 129
#define RISCV_FCVT_W_S 130
#define RISCV_FCVT_WU_S 131
#define RISCV_FCVT_L_S 132
#define RISCV_FCVT_LU_S 133
#define RISCV_FMV_X_W 134
#define RISCV_FCLASS_S 135
#define RISCV_FCVT_W_D 136
#define RISCV_FCVT_WU_D 137
#define RISCV_FCVT_L_D 138
#define RISCV_FCVT_LU_D 139
#define RISCV_FMV_X_D 140
#define RISCV_FCLASS_D 141
#define RISCV_FCVT_W_Q 142
#define RISCV_FCVT_WU_Q 143
#define RISCV_FCVT_L_Q 144
#define RISCV_FCVT_LU_Q 145
#define RISCV_FMV_X_Q 146
#define RISCV_FCLASS_Q 147
#define RISCV_FCVT_S_W 148
#define RISCV_FCVT_S_WU 149
#define RISCV_FCVT_S_L 150
#define RISCV_FCVT_S_LU 151
#define RISCV_FMV_W_X 152
#define RISCV_FCVT_D_W 153
#define RISCV_FCVT_D_WU 154
#define RISCV_FCVT_D_L 155
#define RISCV_FCVT_D_LU 156
#define RISCV_FMV_D_X 157
#define RISCV_FCVT_Q_W 158
#define RISCV_FCVT_Q_WU 159
#define RISCV_FCVT_Q_L 160
#define RISCV_FCVT_Q_LU 161
#define RISCV_FMV_Q_X 162
#define RISCV_FLW 163
#define RISCV_FLD 164
#define RISCV_FLQ 165
#define RISCV_FSW 166
#define RISCV_FSD 167
#define RISCV_FSQ 168
#define RISCV_FMADD_S 169
#define RISCV_FMSUB_S 170
#define RISCV_FNMSUB_S 171
#define RISCV_FNMADD_S 172
#define RISCV_FMADD_D 173
#define RISCV_FMSUB_D 174
#define RISCV_FNMSUB_D 175
#define RISCV_FNMADD_D 176
#define RISCV_FMADD_Q 177
#define RISCV_FMSUB_Q 178
#define RISCV_FNMSUB_Q 179
#define RISCV_FNMADD_Q 180
#define RISCV_MULW 181
#define RISCV_REMW 182
#define RISCV_DIVW 183
#define RISCV_DIVUW 184
#define RISCV_REMUW 185

#endif

