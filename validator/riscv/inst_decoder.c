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

#include <stdint.h>
#include "riscv_isa.h"

#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#include "inst_decoder.h"

static uint32_t reorder_bits(uint16_t bits, char *format) {

  int i, npi, range;
  char *bp, *fp;
  uint32_t ret = 0;
  
  /* new positions, start @ -1 */
  int new_pos[sizeof(bits)*8];
  memset(new_pos, 0xff, sizeof(new_pos)); 

  npi = range = 0;

  /* start the format pointer at the end of the string */
  fp = format + strlen(format) - 1;

  while ( fp >= format ) {
    
    /* find a delim or beginning of format string */
    while ( (fp >= format) && (*fp != '|') && (*fp != ':') )
      fp--;
    
    /* store next offset(s) */
    if ( range )
      for ( i = new_pos[npi-1]+1; i <= atoi(fp+1); i++ )
	new_pos[npi++] = i;
    else
      new_pos[npi++] = atoi(fp+1);
    
    /* are we processing a range? */
    range = ( *fp == ':' );

    /* start on next item */
    fp--;
  }
  
  /* do bit shifting */
  for ( i = 0; i < npi; i++ )
    ret |= (bits & (0x1 << i)) ? (0x1 << new_pos[i]) : 0;

  return ret;
}

/* decode compressed instructions that end in 0b00 */
/* see table in riscv ISA (table 12.4 in spec v. 2.2) */
static int32_t rvc_q0_decode(cinsn ci, uint32_t *rs1, uint32_t *rs2, uint32_t *rs3,
			     uint32_t *rd, int32_t *imm, const char** name, uint32_t *op) {
  
  uint32_t flags = 0;
  
  switch ( ci.func ) {
    
  case 0b000: // C.ADDI4SPN -> addi rd' x2 nzuimm[9:2]
    *name = "addi";
    *op = RISCV_ADDI;
    *rd = EXPAND_CREG(ci.CIW.rdc);
    *rs1 = REG_X2; // (stack pointer)
    *imm = reorder_bits(ci.CIW.imm, "5:4|9:6|2|3");
    flags = RVC | HAS_RD | HAS_RS1 | HAS_IMM;    
    
    break;
    
  case 0b001: // C.FLD -> fld rd' offset[7:3](rs1')
    *name = "fld";
    *op = RISCV_FLD;
    *imm = reorder_bits(ci.CL.immh, "5:3") | reorder_bits(ci.CL.imml, "7:6");
    flags = RVC | HAS_RD | HAS_RS1 | HAS_IMM | HAS_LOAD;
    
    break;
    
  case 0b010: // C.LW -> lw rd' offset[6:2](rs1')
    *name = "lw";
    *op = RISCV_LW;
    *imm = reorder_bits(ci.CL.immh, "5:3") | reorder_bits(ci.CL.imml, "2|6");
    flags = RVC | HAS_RD | HAS_RS1 | HAS_LOAD | HAS_IMM;
    
    break;
    
  case 0b011: // C.FLW -> flw rd' offset[6:2](rs1')
    /* 
       TODO: ifdef 64 bit then this is C.LD
    */
    *name = "flw";
    *op = RISCV_FLW;
    *imm = reorder_bits(ci.CL.immh, "5:3") | reorder_bits(ci.CL.imml, "2|6");
    flags = RVC | HAS_RD | HAS_RS1 | HAS_IMM | HAS_LOAD;
    
    break;
    
  case 0b100: // RESERVED
    
    printf("reserved compressed instruction parsed\n");
    
    return -1;
    
  case 0b101: // C.FSD -> fsd rs2' offset[7:3](rs1')
    *name = "fsd";
    *op = RISCV_FSD;
    *imm = reorder_bits(ci.CS.immh, "5:3") | reorder_bits(ci.CS.imml, "7:6");
    flags = RVC | HAS_RS1 | HAS_RS2 | HAS_STORE | HAS_IMM;;
    
    break;
    
  case 0b110: // C.SW -> sw rs2' offset[6:2](rs1')
    *name = "sw";
    *op = RISCV_SW;
    *imm = reorder_bits(ci.CS.immh, "5:3") | reorder_bits(ci.CS.imml, "2|6");
    flags = RVC | HAS_RS1 | HAS_RS2 | HAS_STORE | HAS_IMM ;
    
    break;
    
  case 0b111: // C.FSW -> fsw rs2' offset[6:2](rs1')
    /* 
       TODO: ifdef 64 bit then this is C.SD
    */
    *name = "fsw";
    *op = RISCV_FSW;
    *imm = reorder_bits(ci.CS.immh, "5:3") | reorder_bits(ci.CS.imml, "2|6");
    flags = RVC | HAS_STORE | HAS_RS1 | HAS_RS2 | HAS_IMM;
    
    break;

  } // end switch

  /* pull register args for store or load instructions */
  if ( flags & HAS_STORE ) {
    *rs1 = EXPAND_CREG(ci.CS.rs1c);
    *rs2 = EXPAND_CREG(ci.CS.rs2c);
  }
  else if ( flags & HAS_LOAD ) {
    *rd  = EXPAND_CREG(ci.CL.rdc);
    *rs1 = EXPAND_CREG(ci.CL.rs1c);
  }
  
  return flags;
}

/* decode compressed instructions that end in 0b01 */
/* see table in riscv ISA (table 12.5 in spec v. 2.2) */
static int32_t rvc_q1_decode(cinsn ci, uint32_t *rs1, uint32_t *rs2, uint32_t *rs3,
			     uint32_t *rd, int32_t *imm, const char** name, uint32_t *op) {
  
  uint32_t flags = 0;
  uint16_t h, l;

  
  switch ( ci.func ) {
    
  case 0b000: // C.ADDI -> addi rd rd nzimm[5:0]
              // note: all zero args encodes to a NOP
    *name = "addi";
    *op = RISCV_ADDI;
    *rd = ci.CI.rds1;
    *rs1 = ci.CI.rds1;
    *imm = reorder_bits(ci.CI.immh, "5") | reorder_bits(ci.CI.imml, "4:0");
    flags = RVC | HAS_RD | HAS_RS1 | HAS_IMM;    

    /* note: some restrictions on arg and imm in ISA... 
       should check here? */
    
    break;
    
  case 0b001: // C.JAL -> jal x1 offset[11:1]
    /* 
       TODO: ifdef 64 bit then this is C.ADDIW
    */
    *name = "jal";
    *op = RISCV_JAL;
    *rd = REG_X1;
    *imm = reorder_bits(ci.CJ.target, "11|4|9:8|10|6|7|3:1|5");
    flags = RVC | HAS_RD;

    break;
    
  case 0b010: // C.LI -> addi rd, x0, imm[5:0]
    *name = "addi";
    *op = RISCV_ADDI;
    *rd  = ci.CI.rds1;
    *rs1 = REG_X0;
    *imm = reorder_bits(ci.CI.immh, "5") | reorder_bits(ci.CI.imml, "4:0");
    flags = RVC | HAS_RD | HAS_RS1 | HAS_IMM;
    
    break;
    
  case 0b011: // iff rd == 2; C.ADDI16SP -> addi x2, x2, nzimm[9:4]
              // else; C.LUI -> lui rd, nzimm[17:12]
    if ( ci.CI.rds1 == 2 ) {
      *name = "addi";
      *op = RISCV_ADDI;
      *rd = REG_X2; // (stack pointer) 
      *rs1 = REG_X2; // (stack pointer) 
      *imm = reorder_bits(ci.CI.immh, "9") | reorder_bits(ci.CI.imml, "4|6|8:7|5");
      flags = RVC | HAS_RD | HAS_RS1 | HAS_IMM;    
    }
    else {
      *name = "lui";
      *op = RISCV_LUI;
      *rd  = ci.CI.rds1;
      *imm = reorder_bits(ci.CI.immh, "17") | reorder_bits(ci.CI.imml, "16:12");
      flags = RVC | HAS_RD;
    }
    
    break;
    
  case 0b100: 

    /* note: for these instructions, [10:11] (op3) 
       & [5:6] (op2) further specify the instruction */

    // C.SRLI -> srli rd', rd', shamt[5:0] if [11:10] == 00
    // C.SRAI -> srai rd', rd', shamt[5:0] if [11:10] == 01
    // C.ANDI -> andi rd', rd', imm[5:0]   if [11:10] == 10
    if ( (ci.CBP.op3 & 0b11) != 0b11 ) {

      if ( ci.CBP.op3 & 0b01 ) {
	*name = "srai";
	*op = RISCV_SRAI;
      }
      else if ( ci.CBP.op3 & 0b10 ) {
	*name = "andi";
	*op = RISCV_ANDI;	
      }
      else { // ci.CBP.op3 == 0bx00 )
	*name = "srli";
	*op = RISCV_SRLI;
      }

      *rd  = EXPAND_CREG(ci.CBP.rs1c);
      *rs1 = EXPAND_CREG(ci.CBP.rs1c);
      *imm = ((ci.CBP.op3 & 0b100) ? (0x1 << 5) : 0) |
	(ci.CBP.op2 << 3) | ci.CBP.rs2c;
      flags = RVC | HAS_RD | HAS_RS1;
      break;
    }

    if ( ci.CBP.op3 == 0b111 ) {
      /* TODO: */
      // op2 == 0b00 -> C.SUBW (64-bit only)
      // op2 == 0b01 -> C.ADDW (64-bit only)
      // op2 == 0b10 -> reserved
      // op2 == 0b11 -> reserved

      break;
    }

    /* now we know that [12:10] == 0b011 */

    // C.xxx -> xxx rd', rd', rs2'
    *rd  = EXPAND_CREG(ci.CBP.rs1c);
    *rs1 = EXPAND_CREG(ci.CBP.rs1c);
    *rs2 = EXPAND_CREG(ci.CBP.rs2c);
    flags = RVC | HAS_RD | HAS_RS1 | HAS_RS2;

    if ( ci.CBP.op2 == 0b00 ) { 
      *name = "sub";
      *op = RISCV_SUB;
    }
    else if ( ci.CBP.op2 == 0b01 ) { 
      *name = "xor";
      *op = RISCV_XOR;
    }
    else if ( ci.CBP.op2 == 0b10 ) {     
      *name = "or";
      *op = RISCV_OR;
    }
    else { // ci.CBP.op2 == 0b11
      *name = "and";
      *op = RISCV_AND;
    }
    
    break;
    
  case 0b101: // C.J -> jal x0, offset[11:1]
    *name = "jal";
    *op = RISCV_JAL;
    *rd = REG_X0;
    *imm = reorder_bits(ci.CJ.target, "11|4|9:8|10|6|7|3:1|5");
    flags = RVC | HAS_RD;

    break;
    
  case 0b110: // C.BEQZ -> beq rs1', x0, offset[8:1]
    *name = "beq";
    *op = RISCV_BEQ;
    *rs1 = EXPAND_CREG(ci.CB.rs1c);
    *rs2 = REG_X0; 
    *imm = reorder_bits(ci.CB.offh, "8|4:3") | reorder_bits(ci.CB.offl, "7:6|2:1|5");
    flags = RVC | HAS_RS1 | HAS_RS2;
    break;
    
  case 0b111: // C.BNEZ -> bne rs1', x0, offset[8:1]
    *name = "bne";
    *op = RISCV_BNE;
    *rs1 = EXPAND_CREG(ci.CB.rs1c);
    *rs2 = REG_X0; 
    *imm = reorder_bits(ci.CB.offh, "8|4:3") | reorder_bits(ci.CB.offl, "7:6|2:1|5");
    flags = RVC | HAS_RS1 | HAS_RS2;
    break;

  } // end switch

  return flags;  
}

/* decode compressed instructions that end in 0b10 */
/* see table in riscv ISA (table 12.6 in spec v. 2.2) */
static int32_t rvc_q2_decode(cinsn ci, uint32_t *rs1, uint32_t *rs2, uint32_t *rs3,
			     uint32_t *rd, int32_t *imm, const char** name, uint32_t *op) {
  
  uint32_t flags = 0;
  
  switch ( ci.func ) {
    
  case 0b000: // C.SLLI -> slli rd, rd, shamt[5:0]
    *name = "slli";
    *op = RISCV_SLLI;
    *rd  = ci.CI.rds1;
    *rs1 = ci.CI.rds1;
    *imm = reorder_bits(ci.CI.immh, "5") |
      reorder_bits(ci.CI.imml, "4:0");
    flags = RVC | HAS_RD | HAS_RS1;

    break;
    
  case 0b001: // C.FLDSP -> fld rd, offset[8:3](x2)
    *name = "fld";
    *op = RISCV_FLD;
    *rd  = ci.CI.rds1;
    *rs1 = REG_X2; // (stack pointer)
    *imm = reorder_bits(ci.CI.immh, "5") |
      reorder_bits(ci.CI.imml, "4:3|8:6");
    flags = RVC | HAS_LOAD | HAS_RD | HAS_RS1 | HAS_IMM;
    
    break;
    
  case 0b010: // C.LWSP -> lw rd, offset[7:2](x2)
    *name = "lw";
    *op = RISCV_LW;
    *rd  = ci.CI.rds1;
    *rs1 = REG_X2; // (stack pointer)
    *imm = reorder_bits(ci.CI.immh, "5") |
      reorder_bits(ci.CI.imml, "4:2|7:6");
    flags = RVC | HAS_RD | HAS_RS1 | HAS_LOAD | HAS_IMM;
        
    break;
    
  case 0b011: // C.FLWSP -> flw rd, offset[7:2](x2)
    /* 
       TODO: if 64-bit this is C.LDSP 
    */
    *name = "flw";
    *op = RISCV_FLW;
    *rd  = ci.CI.rds1;
    *rs1 = REG_X2; // (stack pointer)
    *imm = reorder_bits(ci.CI.immh, "5") |
      reorder_bits(ci.CI.imml, "4:2|7:6");
    flags = RVC | HAS_RD | HAS_RS1 | HAS_IMM;
    
    break;
    
  case 0b100:

    if ( !(ci.CR.funct4 & 0x1) && !ci.CR.rs2 ) {
      // C.JR -> jalr x0, rs1, 0

      *name = "jalr";
      *op = RISCV_JALR;
      *rd  = REG_X0;
      *rs1 = ci.CR.rds1;
      *imm = 0;
      flags = RVC | HAS_RD | HAS_RS1 | HAS_IMM;
    }
    else if ( !(ci.CR.funct4 & 0x1) && ci.CR.rs2 ) {
      // C.MV -> add rd, x0, rs2

      *name = "add";
      *op = RISCV_ADD;
      *rd  = ci.CR.rds1;
      *rs1 = REG_X0;
      *rs2 = ci.CR.rs2;
      flags = RVC | HAS_RD | HAS_RS1 | HAS_RS2;      
    }
    else if ( !ci.CR.rds1 && !ci.CR.rs2 ) {
      // C.EBREAK -> ebreak

      *name = "ebreak";
      *op = RISCV_EBREAK;
      flags = RVC;
    }
    else if ( !ci.CR.rs2 ) {
      // C.JALR -> jalr x1, rs1, 0

      *name = "jalr";
      *op = RISCV_JALR;
      *rd  = REG_X1;
      *rs1 = ci.CR.rds1;
      *imm = 0;
      flags = RVC | HAS_RD | HAS_RS1 | HAS_IMM;      
    }
    else {
      // C.ADD -> add rd, rd, rs2

      *name = "add";
      *op = RISCV_ADD;
      *rd  = ci.CR.rds1;
      *rs1 = ci.CR.rds1;
      *rs2 = ci.CR.rs2;
      flags = RVC | HAS_RD | HAS_RS1 | HAS_RS2;            
    }
    
    break;
    
  case 0b101: // C.FSDSP -> fsd rs2, offset[8:3](x2)
    *name = "fsd";
    *op = RISCV_FSD;
    *rs1 = REG_X2;
    *rs2 = ci.CSS.rs2;
    *imm = reorder_bits(ci.CSS.imm, "5:3|8:6");
    flags = RVC | HAS_STORE | HAS_RS1 | HAS_RS2 | HAS_IMM;
    
    break;
    
  case 0b110: // C.SWSP -> sw rs2, offset[7:2](x2)
    *name = "sw";
    *op = RISCV_SW;
    *rs1 = REG_X2;
    *rs2 = ci.CSS.rs2;
    *imm = reorder_bits(ci.CSS.imm, "5:2|7:6");
    flags = RVC | HAS_RS1 | HAS_RS2 | HAS_STORE | HAS_IMM;
    
    break;
    
  case 0b111: // C.FSWSP -> fsw rs2, offset[7:2](x2)
    /*
      TODO: if 64-bit, this is C.SDSP
     */
    *name = "fsw";
    *op = RISCV_FSW;
    *rs1 = REG_X2;
    *rs2 = ci.CSS.rs2;
    *imm = reorder_bits(ci.CSS.imm, "5:2|7:6");
    flags = RVC | HAS_RS1 | HAS_RS2 | HAS_STORE | HAS_IMM;
    
    break;

  } // end switch

  return flags;  
}

int32_t  decode(uint32_t ibits, uint32_t *rs1, uint32_t *rs2, uint32_t *rs3, uint32_t *rd, int32_t *imm, const char** name, uint32_t *op){
  int32_t flags = 0;
  cinsn ci;
  *imm = 0;
  *op = 0;

  /* if 2 LSBs are not both 1, instr is 16 bits long */
  if ( (ibits & 0b11) != 0b11 ) {

    /* all zeros is an illegal instruction */
    if ( !(ibits & 0xffff) ) {
      printf("warning: illegal instruction (0x0) found\n");
      return -1;
    }
    
    /* access fields using cinsn union */
    ci.bits = ibits & 0xffff;
      
    /* top level switch is on quadrant to match with instruction 
       listing tables in RISC V documentation */
    if ( ci.op == RVC_Q0 ) 
      flags = rvc_q0_decode(ci, rs1, rs2, rs3, rd, imm, name, op);
    else if ( ci.op == RVC_Q1 ) 
      flags = rvc_q1_decode(ci, rs1, rs2, rs3, rd, imm, name, op);
    else if ( ci.op == RVC_Q2 ) 
      flags = rvc_q2_decode(ci, rs1, rs2, rs3, rd, imm, name, op);
    else
      return -1;

    return flags;
  }

  switch(ibits & 0x7f){
  case 0x6f:
    *name = "jal";
    *op = RISCV_JAL;
    flags = 0 | HAS_RD;
    *rd  = (ibits & 0x00000F80) >> 7;
    return flags;
  case 0x37:
    *name = "lui";
    *op = RISCV_LUI;
    flags = 0 | HAS_RD;
    *rd  = (ibits & 0x00000F80) >> 7;
    return flags;
  case 0x17:
    *name = "auipc";
    *op = RISCV_AUIPC;
    flags = 0 | HAS_RD;
    *rd  = (ibits & 0x00000F80) >> 7;
    return flags;
  }
  switch(ibits & 0x707f){
  case 0x63:
    *name = "beq";
    *op = RISCV_BEQ;
    flags = 0 | HAS_RS1 | HAS_RS2;
    *rs1 = (ibits & 0x000F8000) >> 15;
    *rs2 = (ibits & 0x01F00000) >> 20;
    return flags;
  case 0x1063:
    *name = "bne";
    *op = RISCV_BNE;
    flags = 0 | HAS_RS1 | HAS_RS2;
    *rs1 = (ibits & 0x000F8000) >> 15;
    *rs2 = (ibits & 0x01F00000) >> 20;
    return flags;
  case 0x4063:
    *name = "blt";
    *op = RISCV_BLT;
    flags = 0 | HAS_RS1 | HAS_RS2;
    *rs1 = (ibits & 0x000F8000) >> 15;
    *rs2 = (ibits & 0x01F00000) >> 20;
    return flags;
  case 0x5063:
    *name = "bge";
    *op = RISCV_BGE;
    flags = 0 | HAS_RS1 | HAS_RS2;
    *rs1 = (ibits & 0x000F8000) >> 15;
    *rs2 = (ibits & 0x01F00000) >> 20;
    return flags;
  case 0x6063:
    *name = "bltu";
    *op = RISCV_BLTU;
    flags = 0 | HAS_RS1 | HAS_RS2;
    *rs1 = (ibits & 0x000F8000) >> 15;
    *rs2 = (ibits & 0x01F00000) >> 20;
    return flags;
  case 0x7063:
    *name = "bgeu";
    *op = RISCV_BGEU;
    flags = 0 | HAS_RS1 | HAS_RS2;
    *rs1 = (ibits & 0x000F8000) >> 15;
    *rs2 = (ibits & 0x01F00000) >> 20;
    return flags;
  case 0x67:
    *name = "jalr";
    *op = RISCV_JALR;
    flags = 0 | HAS_RD | HAS_RS1 | HAS_IMM;
    *rd  = (ibits & 0x00000F80) >> 7;
    *rs1 = (ibits & 0x000F8000) >> 15;
    *imm |= (((ibits & 0x80000000) == 0x80000000) ? ((ibits & 0xFFF00000) >> 20) | 0xFFFFF000 : (ibits & 0xFFF00000) >> 20);
    return flags;
  case 0x13:
    *name = "addi";
    *op = RISCV_ADDI;
    flags = 0 | HAS_RD | HAS_RS1 | HAS_IMM;
    *rd  = (ibits & 0x00000F80) >> 7;
    *rs1 = (ibits & 0x000F8000) >> 15;
    *imm |= (((ibits & 0x80000000) == 0x80000000) ? ((ibits & 0xFFF00000) >> 20) | 0xFFFFF000 : (ibits & 0xFFF00000) >> 20);
    return flags;
  case 0x2013:
    *name = "slti";
    *op = RISCV_SLTI;
    flags = 0 | HAS_RD | HAS_RS1 | HAS_IMM;
    *rd  = (ibits & 0x00000F80) >> 7;
    *rs1 = (ibits & 0x000F8000) >> 15;
    *imm |= (((ibits & 0x80000000) == 0x80000000) ? ((ibits & 0xFFF00000) >> 20) | 0xFFFFF000 : (ibits & 0xFFF00000) >> 20);
    return flags;
  case 0x3013:
    *name = "sltiu";
    *op = RISCV_SLTIU;
    flags = 0 | HAS_RD | HAS_RS1 | HAS_IMM;
    *rd  = (ibits & 0x00000F80) >> 7;
    *rs1 = (ibits & 0x000F8000) >> 15;
    *imm |= (((ibits & 0x80000000) == 0x80000000) ? ((ibits & 0xFFF00000) >> 20) | 0xFFFFF000 : (ibits & 0xFFF00000) >> 20);
    return flags;
  case 0x4013:
    *name = "xori";
    *op = RISCV_XORI;
    flags = 0 | HAS_RD | HAS_RS1 | HAS_IMM;
    *rd  = (ibits & 0x00000F80) >> 7;
    *rs1 = (ibits & 0x000F8000) >> 15;
    *imm |= (((ibits & 0x80000000) == 0x80000000) ? ((ibits & 0xFFF00000) >> 20) | 0xFFFFF000 : (ibits & 0xFFF00000) >> 20);
    return flags;
  case 0x6013:
    *name = "ori";
    *op = RISCV_ORI;
    flags = 0 | HAS_RD | HAS_RS1 | HAS_IMM;
    *rd  = (ibits & 0x00000F80) >> 7;
    *rs1 = (ibits & 0x000F8000) >> 15;
    *imm |= (((ibits & 0x80000000) == 0x80000000) ? ((ibits & 0xFFF00000) >> 20) | 0xFFFFF000 : (ibits & 0xFFF00000) >> 20);
    return flags;
  case 0x7013:
    *name = "andi";
    *op = RISCV_ANDI;
    flags = 0 | HAS_RD | HAS_RS1 | HAS_IMM;
    *rd  = (ibits & 0x00000F80) >> 7;
    *rs1 = (ibits & 0x000F8000) >> 15;
    *imm |= (((ibits & 0x80000000) == 0x80000000) ? ((ibits & 0xFFF00000) >> 20) | 0xFFFFF000 : (ibits & 0xFFF00000) >> 20);
    return flags;
  case 0x1b:
    *name = "addiw";
    *op = RISCV_ADDIW;
    flags = 0 | HAS_RD | HAS_RS1 | HAS_IMM;
    *rd  = (ibits & 0x00000F80) >> 7;
    *rs1 = (ibits & 0x000F8000) >> 15;
    *imm |= (((ibits & 0x80000000) == 0x80000000) ? ((ibits & 0xFFF00000) >> 20) | 0xFFFFF000 : (ibits & 0xFFF00000) >> 20);
    return flags;
  case 0x3:
    *name = "lb";
    *op = RISCV_LB;
    flags = 0 | HAS_RD | HAS_RS1 | HAS_LOAD | HAS_IMM;
    *rd  = (ibits & 0x00000F80) >> 7;
    *rs1 = (ibits & 0x000F8000) >> 15;
    *imm |= (((ibits & 0x80000000) == 0x80000000) ? ((ibits & 0xFFF00000) >> 20) | 0xFFFFF000 : (ibits & 0xFFF00000) >> 20);
    return flags;
  case 0x1003:
    *name = "lh";
    *op = RISCV_LH;
    flags = 0 | HAS_RD | HAS_RS1 | HAS_LOAD | HAS_IMM;
    *rd  = (ibits & 0x00000F80) >> 7;
    *rs1 = (ibits & 0x000F8000) >> 15;
    *imm |= (((ibits & 0x80000000) == 0x80000000) ? ((ibits & 0xFFF00000) >> 20) | 0xFFFFF000 : (ibits & 0xFFF00000) >> 20);
    return flags;
  case 0x2003:
    *name = "lw";
    *op = RISCV_LW;
    flags = 0 | HAS_RD | HAS_RS1 | HAS_LOAD | HAS_IMM;
    *rd  = (ibits & 0x00000F80) >> 7;
    *rs1 = (ibits & 0x000F8000) >> 15;
    *imm |= (((ibits & 0x80000000) == 0x80000000) ? ((ibits & 0xFFF00000) >> 20) | 0xFFFFF000 : (ibits & 0xFFF00000) >> 20);
    return flags;
  case 0x3003:
    *name = "ld";
    *op = RISCV_LD;
    flags = 0 | HAS_RD | HAS_RS1 | HAS_LOAD | HAS_IMM;
    *rd  = (ibits & 0x00000F80) >> 7;
    *rs1 = (ibits & 0x000F8000) >> 15;
    *imm |= (((ibits & 0x80000000) == 0x80000000) ? ((ibits & 0xFFF00000) >> 20) | 0xFFFFF000 : (ibits & 0xFFF00000) >> 20);
    return flags;
  case 0x4003:
    *name = "lbu";
    *op = RISCV_LBU;
    flags = 0 | HAS_RD | HAS_RS1 | HAS_LOAD | HAS_IMM;
    *rd  = (ibits & 0x00000F80) >> 7;
    *rs1 = (ibits & 0x000F8000) >> 15;
    *imm |= (((ibits & 0x80000000) == 0x80000000) ? ((ibits & 0xFFF00000) >> 20) | 0xFFFFF000 : (ibits & 0xFFF00000) >> 20);
    return flags;
  case 0x5003:
    *name = "lhu";
    *op = RISCV_LHU;
    flags = 0 | HAS_RD | HAS_RS1 | HAS_LOAD | HAS_IMM;
    *rd  = (ibits & 0x00000F80) >> 7;
    *rs1 = (ibits & 0x000F8000) >> 15;
    *imm |= (((ibits & 0x80000000) == 0x80000000) ? ((ibits & 0xFFF00000) >> 20) | 0xFFFFF000 : (ibits & 0xFFF00000) >> 20);
    return flags;
  case 0x6003:
    *name = "lwu";
    *op = RISCV_LWU;
    flags = 0 | HAS_RD | HAS_RS1 | HAS_LOAD | HAS_IMM;
    *rd  = (ibits & 0x00000F80) >> 7;
    *rs1 = (ibits & 0x000F8000) >> 15;
    *imm |= (((ibits & 0x80000000) == 0x80000000) ? ((ibits & 0xFFF00000) >> 20) | 0xFFFFF000 : (ibits & 0xFFF00000) >> 20);
    return flags;
  case 0x23:
    *name = "sb";
    *op = RISCV_SB;
    flags = 0 | HAS_RS1 | HAS_RS2 | HAS_STORE | HAS_IMM | HAS_IMM;
    *rs1 = (ibits & 0x000F8000) >> 15;
    *rs2 = (ibits & 0x01F00000) >> 20;
    *imm |= (((ibits & 0x80000000) == 0x80000000) ? ((ibits & 0xFE000000) >> 20) | 0xFFFFF000 : (ibits & 0xFE000000) >> 20);
    *imm |= ((ibits & 0x00000F80) >> 7);
    return flags;
  case 0x1023:
    *name = "sh";
    *op = RISCV_SH;
    flags = 0 | HAS_RS1 | HAS_RS2 | HAS_STORE | HAS_IMM | HAS_IMM;
    *rs1 = (ibits & 0x000F8000) >> 15;
    *rs2 = (ibits & 0x01F00000) >> 20;
    *imm |= (((ibits & 0x80000000) == 0x80000000) ? ((ibits & 0xFE000000) >> 20) | 0xFFFFF000 : (ibits & 0xFE000000) >> 20);
    *imm |= ((ibits & 0x00000F80) >> 7);
    return flags;
  case 0x2023:
    *name = "sw";
    *op = RISCV_SW;
    flags = 0 | HAS_RS1 | HAS_RS2 | HAS_STORE | HAS_IMM | HAS_IMM;
    *rs1 = (ibits & 0x000F8000) >> 15;
    *rs2 = (ibits & 0x01F00000) >> 20;
    *imm |= (((ibits & 0x80000000) == 0x80000000) ? ((ibits & 0xFE000000) >> 20) | 0xFFFFF000 : (ibits & 0xFE000000) >> 20);
    *imm |= ((ibits & 0x00000F80) >> 7);
    return flags;
  case 0x3023:
    *name = "sd";
    *op = RISCV_SD;
    flags = 0 | HAS_RS1 | HAS_RS2 | HAS_STORE | HAS_IMM | HAS_IMM;
    *rs1 = (ibits & 0x000F8000) >> 15;
    *rs2 = (ibits & 0x01F00000) >> 20;
    *imm |= (((ibits & 0x80000000) == 0x80000000) ? ((ibits & 0xFE000000) >> 20) | 0xFFFFF000 : (ibits & 0xFE000000) >> 20);
    *imm |= ((ibits & 0x00000F80) >> 7);
    return flags;
  case 0xf:
    *name = "fence";
    *op = RISCV_FENCE;
    flags = 0;
    return flags;
  case 0x100f:
    *name = "fence.i";
    *op = RISCV_FENCE_I;
    flags = 0;
    return flags;
  case 0x1073:
    *name = "csrrw";
    *op = RISCV_CSRRW;
    flags = 0 | HAS_RD | HAS_RS1 | HAS_IMM;
    *rd  = (ibits & 0x00000F80) >> 7;
    *rs1 = (ibits & 0x000F8000) >> 15;
    *imm |= ((ibits & 0xFFF00000) >> 20);
    if(*rd == 0){
      flags |= HAS_CSR_STORE;
      flags &= ~HAS_RD;
    }
    else
      flags |= HAS_CSR_LOAD | HAS_CSR_STORE;
    return flags;
  case 0x2073:
    *name = "csrrs";
    *op = RISCV_CSRRS;
    flags = 0 | HAS_RD | HAS_RS1 | HAS_IMM;
    *rd  = (ibits & 0x00000F80) >> 7;
    *rs1 = (ibits & 0x000F8000) >> 15;
    *imm |= ((ibits & 0xFFF00000) >> 20);
    if(*rs1 == 0){
      flags |= HAS_CSR_LOAD;
      // flags &= ~HAS_RS1; TODO: Uncomment when DPL supports x0 reg
    }
    else
      flags |= HAS_CSR_LOAD | HAS_CSR_STORE;
    return flags;
  case 0x3073:
    *name = "csrrc";
    *op = RISCV_CSRRC;
    flags = 0 | HAS_RD | HAS_RS1 | HAS_IMM;
    *rd  = (ibits & 0x00000F80) >> 7;
    *rs1 = (ibits & 0x000F8000) >> 15;
    *imm |= ((ibits & 0xFFF00000) >> 20);
    if(*rs1 == 0){
      flags |= HAS_CSR_LOAD;
      // flags &= ~HAS_RS1; TODO: Uncomment when DPL supports x0 reg
    }
    else
      flags |= HAS_CSR_LOAD | HAS_CSR_STORE;
    return flags;
  case 0x5073:
    *name = "csrrwi";
    *op = RISCV_CSRRWI;
    flags = 0 | HAS_RD | HAS_RS1 | HAS_IMM;
    *rd  = (ibits & 0x00000F80) >> 7;
    *rs1 = (ibits & 0x000F8000) >> 15;
    *imm |= ((ibits & 0xFFF00000) >> 20);
    if(*rd == 0){
      flags |= HAS_CSR_STORE;
      flags &= ~HAS_RD;
    }
    else
      flags |= HAS_CSR_LOAD | HAS_CSR_STORE;
    return flags;
  case 0x6073:
    *name = "csrrsi";
    *op = RISCV_CSRRSI;
    flags = 0 | HAS_RD | HAS_RS1 | HAS_IMM;
    *rd  = (ibits & 0x00000F80) >> 7;
    *rs1 = (ibits & 0x000F8000) >> 15;
    *imm |= ((ibits & 0xFFF00000) >> 20);
    if(*rs1 == 0){
      flags |= HAS_CSR_LOAD;
      // flags &= ~HAS_RS1; TODO: Uncomment when DPL supports x0 reg
    }
    else
      flags |= HAS_CSR_LOAD | HAS_CSR_STORE;
    return flags;
  case 0x7073:
    *name = "csrrci";
    *op = RISCV_CSRRCI;
    flags = 0 | HAS_RD | HAS_RS1 | HAS_IMM;
    *rd  = (ibits & 0x00000F80) >> 7;
    *rs1 = (ibits & 0x000F8000) >> 15;
    *imm |= ((ibits & 0xFFF00000) >> 20);
    if(*rs1 == 0){
      flags |= HAS_CSR_LOAD;
      // flags &= ~HAS_RS1; TODO: Uncomment when DPL supports x0 reg
    }
    else
      flags |= HAS_CSR_LOAD | HAS_CSR_STORE;
    return flags;
  case 0x2007:
    *name = "flw";
    *op = RISCV_FLW;
    flags = 0 | HAS_RD | HAS_RS1 | HAS_IMM | HAS_LOAD;
    *rd  = (ibits & 0x00000F80) >> 7;
    *rs1 = (ibits & 0x000F8000) >> 15;
    *imm |= (((ibits & 0x80000000) == 0x80000000) ? ((ibits & 0xFFF00000) >> 20) | 0xFFFFF000 : (ibits & 0xFFF00000) >> 20);
    return flags;
  case 0x3007: 
    *name = "fld";
    *op = RISCV_FLD;
    flags = 0 | HAS_RD | HAS_RS1 | HAS_IMM | HAS_LOAD;
    *rd  = (ibits & 0x00000F80) >> 7;
    *rs1 = (ibits & 0x000F8000) >> 15;
    *imm |= (((ibits & 0x80000000) == 0x80000000) ? ((ibits & 0xFFF00000) >> 20) | 0xFFFFF000 : (ibits & 0xFFF00000) >> 20);
    return flags;
  case 0x4007:
    *name = "flq";
    *op = RISCV_FLQ;
    flags = 0 | HAS_RD | HAS_RS1 | HAS_IMM;
    *rd  = (ibits & 0x00000F80) >> 7;
    *rs1 = (ibits & 0x000F8000) >> 15;
    *imm |= (((ibits & 0x80000000) == 0x80000000) ? ((ibits & 0xFFF00000) >> 20) | 0xFFFFF000 : (ibits & 0xFFF00000) >> 20);
    return flags;
  case 0x2027:
    *name = "fsw";
    *op = RISCV_FSW;
    flags = 0 | HAS_RS1 | HAS_RS2 | HAS_IMM | HAS_STORE;
    *rs1 = (ibits & 0x000F8000) >> 15;
    *rs2 = (ibits & 0x01F00000) >> 20;
    *imm |= (((ibits & 0x80000000) == 0x80000000) ? ((ibits & 0xFE000000) >> 20) | 0xFFFFF000 : (ibits & 0xFE000000) >> 20);
    *imm |= ((ibits & 0x00000F80) >> 7);
    return flags;
  case 0x3027: 
    *name = "fsd";
    *op = RISCV_FSD;
    flags = 0 | HAS_RS1 | HAS_RS2 | HAS_IMM | HAS_STORE;
    *rs1 = (ibits & 0x000F8000) >> 15;
    *rs2 = (ibits & 0x01F00000) >> 20;
    *imm |= (((ibits & 0x80000000) == 0x80000000) ? ((ibits & 0xFE000000) >> 20) | 0xFFFFF000 : (ibits & 0xFE000000) >> 20);
    *imm |= ((ibits & 0x00000F80) >> 7);
    return flags;
  case 0x4027:
    *name = "fsq";
    *op = RISCV_FSQ;
    flags = 0 | HAS_RS1 | HAS_RS2 | HAS_IMM | HAS_STORE;
    *rs1 = (ibits & 0x000F8000) >> 15;
    *rs2 = (ibits & 0x01F00000) >> 20;
    *imm |= (((ibits & 0x80000000) == 0x80000000) ? ((ibits & 0xFE000000) >> 20) | 0xFFFFF000 : (ibits & 0xFE000000) >> 20);
    *imm |= ((ibits & 0x00000F80) >> 7);
    return flags;
  }
  switch(ibits & 0x600007f){
  case 0x43:
    *name = "fmadd.s";
    *op = RISCV_FMADD_S;
    flags = 0 | HAS_RD | HAS_RS1 | HAS_RS2 | HAS_RS3;
    *rd  = (ibits & 0x00000F80) >> 7;
    *rs1 = (ibits & 0x000F8000) >> 15;
    *rs2 = (ibits & 0x01F00000) >> 20;
    *rs3 = (ibits & 0xF8000000) >> 27;
    return flags;
  case 0x47:
    *name = "fmsub.s";
    *op = RISCV_FMSUB_S;
    flags = 0 | HAS_RD | HAS_RS1 | HAS_RS2 | HAS_RS3;
    *rd  = (ibits & 0x00000F80) >> 7;
    *rs1 = (ibits & 0x000F8000) >> 15;
    *rs2 = (ibits & 0x01F00000) >> 20;
    *rs3 = (ibits & 0xF8000000) >> 27;
    return flags;
  case 0x4b:
    *name = "fnmsub.s";
    *op = RISCV_FNMSUB_S;
    flags = 0 | HAS_RD | HAS_RS1 | HAS_RS2 | HAS_RS3;
    *rd  = (ibits & 0x00000F80) >> 7;
    *rs1 = (ibits & 0x000F8000) >> 15;
    *rs2 = (ibits & 0x01F00000) >> 20;
    *rs3 = (ibits & 0xF8000000) >> 27;
    return flags;
  case 0x4f:
    *name = "fnmadd.s";
    *op = RISCV_FNMADD_S;
    flags = 0 | HAS_RD | HAS_RS1 | HAS_RS2 | HAS_RS3;
    *rd  = (ibits & 0x00000F80) >> 7;
    *rs1 = (ibits & 0x000F8000) >> 15;
    *rs2 = (ibits & 0x01F00000) >> 20;
    *rs3 = (ibits & 0xF8000000) >> 27;
    return flags;
  case 0x2000043:
    *name = "fmadd.d";
    *op = RISCV_FMADD_D;
    flags = 0 | HAS_RD | HAS_RS1 | HAS_RS2 | HAS_RS3;
    *rd  = (ibits & 0x00000F80) >> 7;
    *rs1 = (ibits & 0x000F8000) >> 15;
    *rs2 = (ibits & 0x01F00000) >> 20;
    *rs3 = (ibits & 0xF8000000) >> 27;
    return flags;
  case 0x2000047:
    *name = "fmsub.d";
    *op = RISCV_FMSUB_D;
    flags = 0 | HAS_RD | HAS_RS1 | HAS_RS2 | HAS_RS3;
    *rd  = (ibits & 0x00000F80) >> 7;
    *rs1 = (ibits & 0x000F8000) >> 15;
    *rs2 = (ibits & 0x01F00000) >> 20;
    *rs3 = (ibits & 0xF8000000) >> 27;
    return flags;
  case 0x200004b:
    *name = "fnmsub.d";
    *op = RISCV_FNMSUB_D;
    flags = 0 | HAS_RD | HAS_RS1 | HAS_RS2 | HAS_RS3;
    *rd  = (ibits & 0x00000F80) >> 7;
    *rs1 = (ibits & 0x000F8000) >> 15;
    *rs2 = (ibits & 0x01F00000) >> 20;
    *rs3 = (ibits & 0xF8000000) >> 27;
    return flags;
  case 0x200004f:
    *name = "fnmadd.d";
    *op = RISCV_FNMADD_D;
    flags = 0 | HAS_RD | HAS_RS1 | HAS_RS2 | HAS_RS3;
    *rd  = (ibits & 0x00000F80) >> 7;
    *rs1 = (ibits & 0x000F8000) >> 15;
    *rs2 = (ibits & 0x01F00000) >> 20;
    *rs3 = (ibits & 0xF8000000) >> 27;
    return flags;
  case 0x6000043:
    *name = "fmadd.q";
    *op = RISCV_FMADD_Q;
    flags = 0 | HAS_RD | HAS_RS1 | HAS_RS2 | HAS_RS3;
    *rd  = (ibits & 0x00000F80) >> 7;
    *rs1 = (ibits & 0x000F8000) >> 15;
    *rs2 = (ibits & 0x01F00000) >> 20;
    *rs3 = (ibits & 0xF8000000) >> 27;
    return flags;
  case 0x6000047:
    *name = "fmsub.q";
    *op = RISCV_FMSUB_Q;
    flags = 0 | HAS_RD | HAS_RS1 | HAS_RS2 | HAS_RS3;
    *rd  = (ibits & 0x00000F80) >> 7;
    *rs1 = (ibits & 0x000F8000) >> 15;
    *rs2 = (ibits & 0x01F00000) >> 20;
    *rs3 = (ibits & 0xF8000000) >> 27;
    return flags;
  case 0x600004b:
    *name = "fnmsub.q";
    *op = RISCV_FNMSUB_Q;
    flags = 0 | HAS_RD | HAS_RS1 | HAS_RS2 | HAS_RS3;
    *rd  = (ibits & 0x00000F80) >> 7;
    *rs1 = (ibits & 0x000F8000) >> 15;
    *rs2 = (ibits & 0x01F00000) >> 20;
    *rs3 = (ibits & 0xF8000000) >> 27;
    return flags;
  case 0x600004f:
    *name = "fnmadd.q";
    *op = RISCV_FNMADD_Q;
    flags = 0 | HAS_RD | HAS_RS1 | HAS_RS2 | HAS_RS3;
    *rd  = (ibits & 0x00000F80) >> 7;
    *rs1 = (ibits & 0x000F8000) >> 15;
    *rs2 = (ibits & 0x01F00000) >> 20;
    *rs3 = (ibits & 0xF8000000) >> 27;
    return flags;
  }
  switch(ibits & 0xf800707f){
  case 0x202f:
    *name = "amoadd.w";
    *op = RISCV_AMOADD_W;
    flags = 0 | HAS_RD | HAS_RS1 | HAS_RS2;
    *rd  = (ibits & 0x00000F80) >> 7;
    *rs1 = (ibits & 0x000F8000) >> 15;
    *rs2 = (ibits & 0x01F00000) >> 20;
    return flags;
  case 0x2000202f:
    *name = "amoxor.w";
    *op = RISCV_AMOXOR_W;
    flags = 0 | HAS_RD | HAS_RS1 | HAS_RS2;
    *rd  = (ibits & 0x00000F80) >> 7;
    *rs1 = (ibits & 0x000F8000) >> 15;
    *rs2 = (ibits & 0x01F00000) >> 20;
    return flags;
  case 0x4000202f:
    *name = "amoor.w";
    *op = RISCV_AMOOR_W;
    flags = 0 | HAS_RD | HAS_RS1 | HAS_RS2;
    *rd  = (ibits & 0x00000F80) >> 7;
    *rs1 = (ibits & 0x000F8000) >> 15;
    *rs2 = (ibits & 0x01F00000) >> 20;
    return flags;
  case 0x6000202f:
    *name = "amoand.w";
    *op = RISCV_AMOAND_W;
    flags = 0 | HAS_RD | HAS_RS1 | HAS_RS2;
    *rd  = (ibits & 0x00000F80) >> 7;
    *rs1 = (ibits & 0x000F8000) >> 15;
    *rs2 = (ibits & 0x01F00000) >> 20;
    return flags;
  case 0x8000202f:
    *name = "amomin.w";
    *op = RISCV_AMOMIN_W;
    flags = 0 | HAS_RD | HAS_RS1 | HAS_RS2;
    *rd  = (ibits & 0x00000F80) >> 7;
    *rs1 = (ibits & 0x000F8000) >> 15;
    *rs2 = (ibits & 0x01F00000) >> 20;
    return flags;
  case 0xa000202f:
    *name = "amomax.w";
    *op = RISCV_AMOMAX_W;
    flags = 0 | HAS_RD | HAS_RS1 | HAS_RS2;
    *rd  = (ibits & 0x00000F80) >> 7;
    *rs1 = (ibits & 0x000F8000) >> 15;
    *rs2 = (ibits & 0x01F00000) >> 20;
    return flags;
  case 0xc000202f:
    *name = "amominu.w";
    *op = RISCV_AMOMINU_W;
    flags = 0 | HAS_RD | HAS_RS1 | HAS_RS2;
    *rd  = (ibits & 0x00000F80) >> 7;
    *rs1 = (ibits & 0x000F8000) >> 15;
    *rs2 = (ibits & 0x01F00000) >> 20;
    return flags;
  case 0xe000202f:
    *name = "amomaxu.w";
    *op = RISCV_AMOMAXU_W;
    flags = 0 | HAS_RD | HAS_RS1 | HAS_RS2;
    *rd  = (ibits & 0x00000F80) >> 7;
    *rs1 = (ibits & 0x000F8000) >> 15;
    *rs2 = (ibits & 0x01F00000) >> 20;
    return flags;
  case 0x800202f:
    *name = "amoswap.w";
    *op = RISCV_AMOSWAP_W;
    flags = 0 | HAS_RD | HAS_RS1 | HAS_RS2;
    *rd  = (ibits & 0x00000F80) >> 7;
    *rs1 = (ibits & 0x000F8000) >> 15;
    *rs2 = (ibits & 0x01F00000) >> 20;
    return flags;
  case 0x1800202f:
    *name = "sc.w";
    *op = RISCV_SC_W;
    flags = 0 | HAS_RD | HAS_RS1 | HAS_RS2;
    *rd  = (ibits & 0x00000F80) >> 7;
    *rs1 = (ibits & 0x000F8000) >> 15;
    *rs2 = (ibits & 0x01F00000) >> 20;
    return flags;
  }
  switch(ibits & 0xf9f0707f){
  case 0x1000202f:
    *name = "lr.w";
    *op = RISCV_LR_W;
    flags = 0 | HAS_RD | HAS_RS1;
    *rd  = (ibits & 0x00000F80) >> 7;
    *rs1 = (ibits & 0x000F8000) >> 15;
    return flags;
  }
  switch(ibits & 0xfc00707f){
  case 0x1013:
    *name = "slli";
    *op = RISCV_SLLI;
    flags = 0 | HAS_RD | HAS_RS1;
    *rd  = (ibits & 0x00000F80) >> 7;
    *rs1 = (ibits & 0x000F8000) >> 15;
    return flags;
  case 0x5013:
    *name = "srli";
    *op = RISCV_SRLI;
    flags = 0 | HAS_RD | HAS_RS1;
    *rd  = (ibits & 0x00000F80) >> 7;
    *rs1 = (ibits & 0x000F8000) >> 15;
    return flags;
  case 0x40005013:
    *name = "srai";
    *op = RISCV_SRAI;
    flags = 0 | HAS_RD | HAS_RS1;
    *rd  = (ibits & 0x00000F80) >> 7;
    *rs1 = (ibits & 0x000F8000) >> 15;
    return flags;
  }
  switch(ibits & 0xfe00007f){
  case 0x53:
    *name = "fadd.s";
    *op = RISCV_FADD_S;
    flags = 0 | HAS_RD | HAS_RS1 | HAS_RS2;
    *rd  = (ibits & 0x00000F80) >> 7;
    *rs1 = (ibits & 0x000F8000) >> 15;
    *rs2 = (ibits & 0x01F00000) >> 20;
    return flags;
  case 0x8000053:
    *name = "fsub.s";
    *op = RISCV_FSUB_S;
    flags = 0 | HAS_RD | HAS_RS1 | HAS_RS2;
    *rd  = (ibits & 0x00000F80) >> 7;
    *rs1 = (ibits & 0x000F8000) >> 15;
    *rs2 = (ibits & 0x01F00000) >> 20;
    return flags;
  case 0x10000053:
    *name = "fmul.s";
    *op = RISCV_FMUL_S;
    flags = 0 | HAS_RD | HAS_RS1 | HAS_RS2;
    *rd  = (ibits & 0x00000F80) >> 7;
    *rs1 = (ibits & 0x000F8000) >> 15;
    *rs2 = (ibits & 0x01F00000) >> 20;
    return flags;
  case 0x18000053:
    *name = "fdiv.s";
    *op = RISCV_FDIV_S;
    flags = 0 | HAS_RD | HAS_RS1 | HAS_RS2;
    *rd  = (ibits & 0x00000F80) >> 7;
    *rs1 = (ibits & 0x000F8000) >> 15;
    *rs2 = (ibits & 0x01F00000) >> 20;
    return flags;
  case 0x2000053:
    *name = "fadd.d";
    *op = RISCV_FADD_D;
    flags = 0 | HAS_RD | HAS_RS1 | HAS_RS2;
    *rd  = (ibits & 0x00000F80) >> 7;
    *rs1 = (ibits & 0x000F8000) >> 15;
    *rs2 = (ibits & 0x01F00000) >> 20;
    return flags;
  case 0xa000053:
    *name = "fsub.d";
    *op = RISCV_FSUB_D;
    flags = 0 | HAS_RD | HAS_RS1 | HAS_RS2;
    *rd  = (ibits & 0x00000F80) >> 7;
    *rs1 = (ibits & 0x000F8000) >> 15;
    *rs2 = (ibits & 0x01F00000) >> 20;
    return flags;
  case 0x12000053:
    *name = "fmul.d";
    *op = RISCV_FMUL_D;
    flags = 0 | HAS_RD | HAS_RS1 | HAS_RS2;
    *rd  = (ibits & 0x00000F80) >> 7;
    *rs1 = (ibits & 0x000F8000) >> 15;
    *rs2 = (ibits & 0x01F00000) >> 20;
    return flags;
  case 0x1a000053:
    *name = "fdiv.d";
    *op = RISCV_FDIV_D;
    flags = 0 | HAS_RD | HAS_RS1 | HAS_RS2;
    *rd  = (ibits & 0x00000F80) >> 7;
    *rs1 = (ibits & 0x000F8000) >> 15;
    *rs2 = (ibits & 0x01F00000) >> 20;
    return flags;
  case 0x6000053:
    *name = "fadd.q";
    *op = RISCV_FADD_Q;
    flags = 0 | HAS_RD | HAS_RS1 | HAS_RS2;
    *rd  = (ibits & 0x00000F80) >> 7;
    *rs1 = (ibits & 0x000F8000) >> 15;
    *rs2 = (ibits & 0x01F00000) >> 20;
    return flags;
  case 0xe000053:
    *name = "fsub.q";
    *op = RISCV_FSUB_Q;
    flags = 0 | HAS_RD | HAS_RS1 | HAS_RS2;
    *rd  = (ibits & 0x00000F80) >> 7;
    *rs1 = (ibits & 0x000F8000) >> 15;
    *rs2 = (ibits & 0x01F00000) >> 20;
    return flags;
  case 0x16000053:
    *name = "fmul.q";
    *op = RISCV_FMUL_Q;
    flags = 0 | HAS_RD | HAS_RS1 | HAS_RS2;
    *rd  = (ibits & 0x00000F80) >> 7;
    *rs1 = (ibits & 0x000F8000) >> 15;
    *rs2 = (ibits & 0x01F00000) >> 20;
    return flags;
  case 0x1e000053:
    *name = "fdiv.q";
    *op = RISCV_FDIV_Q;
    flags = 0 | HAS_RD | HAS_RS1 | HAS_RS2;
    *rd  = (ibits & 0x00000F80) >> 7;
    *rs1 = (ibits & 0x000F8000) >> 15;
    *rs2 = (ibits & 0x01F00000) >> 20;
    return flags;
  }
  switch(ibits & 0xfe00707f){
  case 0x33:
    *name = "add";
    *op = RISCV_ADD;
    flags = 0 | HAS_RD | HAS_RS1 | HAS_RS2;
    *rd  = (ibits & 0x00000F80) >> 7;
    *rs1 = (ibits & 0x000F8000) >> 15;
    *rs2 = (ibits & 0x01F00000) >> 20;
    return flags;
  case 0x40000033:
    *name = "sub";
    *op = RISCV_SUB;
    flags = 0 | HAS_RD | HAS_RS1 | HAS_RS2;
    *rd  = (ibits & 0x00000F80) >> 7;
    *rs1 = (ibits & 0x000F8000) >> 15;
    *rs2 = (ibits & 0x01F00000) >> 20;
    return flags;
  case 0x1033:
    *name = "sll";
    *op = RISCV_SLL;
    flags = 0 | HAS_RD | HAS_RS1 | HAS_RS2;
    *rd  = (ibits & 0x00000F80) >> 7;
    *rs1 = (ibits & 0x000F8000) >> 15;
    *rs2 = (ibits & 0x01F00000) >> 20;
    return flags;
  case 0x2033:
    *name = "slt";
    *op = RISCV_SLT;
    flags = 0 | HAS_RD | HAS_RS1 | HAS_RS2;
    *rd  = (ibits & 0x00000F80) >> 7;
    *rs1 = (ibits & 0x000F8000) >> 15;
    *rs2 = (ibits & 0x01F00000) >> 20;
    return flags;
  case 0x3033:
    *name = "sltu";
    *op = RISCV_SLTU;
    flags = 0 | HAS_RD | HAS_RS1 | HAS_RS2;
    *rd  = (ibits & 0x00000F80) >> 7;
    *rs1 = (ibits & 0x000F8000) >> 15;
    *rs2 = (ibits & 0x01F00000) >> 20;
    return flags;
  case 0x4033:
    *name = "xor";
    *op = RISCV_XOR;
    flags = 0 | HAS_RD | HAS_RS1 | HAS_RS2;
    *rd  = (ibits & 0x00000F80) >> 7;
    *rs1 = (ibits & 0x000F8000) >> 15;
    *rs2 = (ibits & 0x01F00000) >> 20;
    return flags;
  case 0x5033:
    *name = "srl";
    *op = RISCV_SRL;
    flags = 0 | HAS_RD | HAS_RS1 | HAS_RS2;
    *rd  = (ibits & 0x00000F80) >> 7;
    *rs1 = (ibits & 0x000F8000) >> 15;
    *rs2 = (ibits & 0x01F00000) >> 20;
    return flags;
  case 0x40005033:
    *name = "sra";
    *op = RISCV_SRA;
    flags = 0 | HAS_RD | HAS_RS1 | HAS_RS2;
    *rd  = (ibits & 0x00000F80) >> 7;
    *rs1 = (ibits & 0x000F8000) >> 15;
    *rs2 = (ibits & 0x01F00000) >> 20;
    return flags;
  case 0x6033:
    *name = "or";
    *op = RISCV_OR;
    flags = 0 | HAS_RD | HAS_RS1 | HAS_RS2;
    *rd  = (ibits & 0x00000F80) >> 7;
    *rs1 = (ibits & 0x000F8000) >> 15;
    *rs2 = (ibits & 0x01F00000) >> 20;
    return flags;
  case 0x7033:
    *name = "and";
    *op = RISCV_AND;
    flags = 0 | HAS_RD | HAS_RS1 | HAS_RS2;
    *rd  = (ibits & 0x00000F80) >> 7;
    *rs1 = (ibits & 0x000F8000) >> 15;
    *rs2 = (ibits & 0x01F00000) >> 20;
    return flags;
  case 0x101b:
    *name = "slliw";
    *op = RISCV_SLLIW;
    flags = 0 | HAS_RD | HAS_RS1;
    *rd  = (ibits & 0x00000F80) >> 7;
    *rs1 = (ibits & 0x000F8000) >> 15;
    return flags;
  case 0x501b:
    *name = "srliw";
    *op = RISCV_SRLIW;
    flags = 0 | HAS_RD | HAS_RS1;
    *rd  = (ibits & 0x00000F80) >> 7;
    *rs1 = (ibits & 0x000F8000) >> 15;
    return flags;
  case 0x4000501b:
    *name = "sraiw";
    *op = RISCV_SRAIW;
    flags = 0 | HAS_RD | HAS_RS1;
    *rd  = (ibits & 0x00000F80) >> 7;
    *rs1 = (ibits & 0x000F8000) >> 15;
    return flags;
  case 0x3b:
    *name = "addw";
    *op = RISCV_ADDW;
    flags = 0 | HAS_RD | HAS_RS1 | HAS_RS2;
    *rd  = (ibits & 0x00000F80) >> 7;
    *rs1 = (ibits & 0x000F8000) >> 15;
    *rs2 = (ibits & 0x01F00000) >> 20;
    return flags;
  case 0x4000003b:
    *name = "subw";
    *op = RISCV_SUBW;
    flags = 0 | HAS_RD | HAS_RS1 | HAS_RS2;
    *rd  = (ibits & 0x00000F80) >> 7;
    *rs1 = (ibits & 0x000F8000) >> 15;
    *rs2 = (ibits & 0x01F00000) >> 20;
    return flags;
  case 0x103b:
    *name = "sllw";
    *op = RISCV_SLLW;
    flags = 0 | HAS_RD | HAS_RS1 | HAS_RS2;
    *rd  = (ibits & 0x00000F80) >> 7;
    *rs1 = (ibits & 0x000F8000) >> 15;
    *rs2 = (ibits & 0x01F00000) >> 20;
    return flags;
  case 0x503b:
    *name = "srlw";
    *op = RISCV_SRLW;
    flags = 0 | HAS_RD | HAS_RS1 | HAS_RS2;
    *rd  = (ibits & 0x00000F80) >> 7;
    *rs1 = (ibits & 0x000F8000) >> 15;
    *rs2 = (ibits & 0x01F00000) >> 20;
    return flags;
  case 0x4000503b:
    *name = "sraw";
    *op = RISCV_SRAW;
    flags = 0 | HAS_RD | HAS_RS1 | HAS_RS2;
    *rd  = (ibits & 0x00000F80) >> 7;
    *rs1 = (ibits & 0x000F8000) >> 15;
    *rs2 = (ibits & 0x01F00000) >> 20;
    return flags;
  case 0x2000033:
    *name = "mul";
    *op = RISCV_MUL;
    flags = 0 | HAS_RD | HAS_RS1 | HAS_RS2;
    *rd  = (ibits & 0x00000F80) >> 7;
    *rs1 = (ibits & 0x000F8000) >> 15;
    *rs2 = (ibits & 0x01F00000) >> 20;
    return flags;
  case 0x2001033:
    *name = "mulh";
    *op = RISCV_MULH;
    flags = 0 | HAS_RD | HAS_RS1 | HAS_RS2;
    *rd  = (ibits & 0x00000F80) >> 7;
    *rs1 = (ibits & 0x000F8000) >> 15;
    *rs2 = (ibits & 0x01F00000) >> 20;
    return flags;
  case 0x2002033:
    *name = "mulhsu";
    *op = RISCV_MULHSU;
    flags = 0 | HAS_RD | HAS_RS1 | HAS_RS2;
    *rd  = (ibits & 0x00000F80) >> 7;
    *rs1 = (ibits & 0x000F8000) >> 15;
    *rs2 = (ibits & 0x01F00000) >> 20;
    return flags;
  case 0x2003033:
    *name = "mulhu";
    *op = RISCV_MULHU;
    flags = 0 | HAS_RD | HAS_RS1 | HAS_RS2;
    *rd  = (ibits & 0x00000F80) >> 7;
    *rs1 = (ibits & 0x000F8000) >> 15;
    *rs2 = (ibits & 0x01F00000) >> 20;
    return flags;
  case 0x2004033:
    *name = "div";
    *op = RISCV_DIV;
    flags = 0 | HAS_RD | HAS_RS1 | HAS_RS2;
    *rd  = (ibits & 0x00000F80) >> 7;
    *rs1 = (ibits & 0x000F8000) >> 15;
    *rs2 = (ibits & 0x01F00000) >> 20;
    return flags;
  case 0x2005033:
    *name = "divu";
    *op = RISCV_DIVU;
    flags = 0 | HAS_RD | HAS_RS1 | HAS_RS2;
    *rd  = (ibits & 0x00000F80) >> 7;
    *rs1 = (ibits & 0x000F8000) >> 15;
    *rs2 = (ibits & 0x01F00000) >> 20;
    return flags;
  case 0x2006033:
    *name = "rem";
    *op = RISCV_REM;
    flags = 0 | HAS_RD | HAS_RS1 | HAS_RS2;
    *rd  = (ibits & 0x00000F80) >> 7;
    *rs1 = (ibits & 0x000F8000) >> 15;
    *rs2 = (ibits & 0x01F00000) >> 20;
    return flags;
  case 0x2007033:
    *name = "remu";
    *op = RISCV_REMU;
    flags = 0 | HAS_RD | HAS_RS1 | HAS_RS2;
    *rd  = (ibits & 0x00000F80) >> 7;
    *rs1 = (ibits & 0x000F8000) >> 15;
    *rs2 = (ibits & 0x01F00000) >> 20;
    return flags;
  case 0x20000053:
    *name = "fsgnj.s";
    *op = RISCV_FSGNJ_S;
    flags = 0 | HAS_RD | HAS_RS1 | HAS_RS2;
    *rd  = (ibits & 0x00000F80) >> 7;
    *rs1 = (ibits & 0x000F8000) >> 15;
    *rs2 = (ibits & 0x01F00000) >> 20;
    return flags;
  case 0x20001053:
    *name = "fsgnjn.s";
    *op = RISCV_FSGNJN_S;
    flags = 0 | HAS_RD | HAS_RS1 | HAS_RS2;
    *rd  = (ibits & 0x00000F80) >> 7;
    *rs1 = (ibits & 0x000F8000) >> 15;
    *rs2 = (ibits & 0x01F00000) >> 20;
    return flags;
  case 0x20002053:
    *name = "fsgnjx.s";
    *op = RISCV_FSGNJX_S;
    flags = 0 | HAS_RD | HAS_RS1 | HAS_RS2;
    *rd  = (ibits & 0x00000F80) >> 7;
    *rs1 = (ibits & 0x000F8000) >> 15;
    *rs2 = (ibits & 0x01F00000) >> 20;
    return flags;
  case 0x28000053:
    *name = "fmin.s";
    *op = RISCV_FMIN_S;
    flags = 0 | HAS_RD | HAS_RS1 | HAS_RS2;
    *rd  = (ibits & 0x00000F80) >> 7;
    *rs1 = (ibits & 0x000F8000) >> 15;
    *rs2 = (ibits & 0x01F00000) >> 20;
    return flags;
  case 0x28001053:
    *name = "fmax.s";
    *op = RISCV_FMAX_S;
    flags = 0 | HAS_RD | HAS_RS1 | HAS_RS2;
    *rd  = (ibits & 0x00000F80) >> 7;
    *rs1 = (ibits & 0x000F8000) >> 15;
    *rs2 = (ibits & 0x01F00000) >> 20;
    return flags;
  case 0x22000053:
    *name = "fsgnj.d";
    *op = RISCV_FSGNJ_D;
    flags = 0 | HAS_RD | HAS_RS1 | HAS_RS2;
    *rd  = (ibits & 0x00000F80) >> 7;
    *rs1 = (ibits & 0x000F8000) >> 15;
    *rs2 = (ibits & 0x01F00000) >> 20;
    return flags;
  case 0x22001053:
    *name = "fsgnjn.d";
    *op = RISCV_FSGNJN_D;
    flags = 0 | HAS_RD | HAS_RS1 | HAS_RS2;
    *rd  = (ibits & 0x00000F80) >> 7;
    *rs1 = (ibits & 0x000F8000) >> 15;
    *rs2 = (ibits & 0x01F00000) >> 20;
    return flags;
  case 0x22002053:
    *name = "fsgnjx.d";
    *op = RISCV_FSGNJX_D;
    flags = 0 | HAS_RD | HAS_RS1 | HAS_RS2;
    *rd  = (ibits & 0x00000F80) >> 7;
    *rs1 = (ibits & 0x000F8000) >> 15;
    *rs2 = (ibits & 0x01F00000) >> 20;
    return flags;
  case 0x2a000053:
    *name = "fmin.d";
    *op = RISCV_FMIN_D;
    flags = 0 | HAS_RD | HAS_RS1 | HAS_RS2;
    *rd  = (ibits & 0x00000F80) >> 7;
    *rs1 = (ibits & 0x000F8000) >> 15;
    *rs2 = (ibits & 0x01F00000) >> 20;
    return flags;
  case 0x2a001053:
    *name = "fmax.d";
    *op = RISCV_FMAX_D;
    flags = 0 | HAS_RD | HAS_RS1 | HAS_RS2;
    *rd  = (ibits & 0x00000F80) >> 7;
    *rs1 = (ibits & 0x000F8000) >> 15;
    *rs2 = (ibits & 0x01F00000) >> 20;
    return flags;
  case 0x26000053:
    *name = "fsgnj.q";
    *op = RISCV_FSGNJ_Q;
    flags = 0 | HAS_RD | HAS_RS1 | HAS_RS2;
    *rd  = (ibits & 0x00000F80) >> 7;
    *rs1 = (ibits & 0x000F8000) >> 15;
    *rs2 = (ibits & 0x01F00000) >> 20;
    return flags;
  case 0x26001053:
    *name = "fsgnjn.q";
    *op = RISCV_FSGNJN_Q;
    flags = 0 | HAS_RD | HAS_RS1 | HAS_RS2;
    *rd  = (ibits & 0x00000F80) >> 7;
    *rs1 = (ibits & 0x000F8000) >> 15;
    *rs2 = (ibits & 0x01F00000) >> 20;
    return flags;
  case 0x26002053:
    *name = "fsgnjx.q";
    *op = RISCV_FSGNJX_Q;
    flags = 0 | HAS_RD | HAS_RS1 | HAS_RS2;
    *rd  = (ibits & 0x00000F80) >> 7;
    *rs1 = (ibits & 0x000F8000) >> 15;
    *rs2 = (ibits & 0x01F00000) >> 20;
    return flags;
  case 0x2e000053:
    *name = "fmin.q";
    *op = RISCV_FMIN_Q;
    flags = 0 | HAS_RD | HAS_RS1 | HAS_RS2;
    *rd  = (ibits & 0x00000F80) >> 7;
    *rs1 = (ibits & 0x000F8000) >> 15;
    *rs2 = (ibits & 0x01F00000) >> 20;
    return flags;
  case 0x2e001053:
    *name = "fmax.q";
    *op = RISCV_FMAX_Q;
    flags = 0 | HAS_RD | HAS_RS1 | HAS_RS2;
    *rd  = (ibits & 0x00000F80) >> 7;
    *rs1 = (ibits & 0x000F8000) >> 15;
    *rs2 = (ibits & 0x01F00000) >> 20;
    return flags;
  case 0xa0000053:
    *name = "fle.s";
    *op = RISCV_FLE_S;
    flags = 0 | HAS_RD | HAS_RS1 | HAS_RS2;
    *rd  = (ibits & 0x00000F80) >> 7;
    *rs1 = (ibits & 0x000F8000) >> 15;
    *rs2 = (ibits & 0x01F00000) >> 20;
    return flags;
  case 0xa0001053:
    *name = "flt.s";
    *op = RISCV_FLT_S;
    flags = 0 | HAS_RD | HAS_RS1 | HAS_RS2;
    *rd  = (ibits & 0x00000F80) >> 7;
    *rs1 = (ibits & 0x000F8000) >> 15;
    *rs2 = (ibits & 0x01F00000) >> 20;
    return flags;
  case 0xa0002053:
    *name = "feq.s";
    *op = RISCV_FEQ_S;
    flags = 0 | HAS_RD | HAS_RS1 | HAS_RS2;
    *rd  = (ibits & 0x00000F80) >> 7;
    *rs1 = (ibits & 0x000F8000) >> 15;
    *rs2 = (ibits & 0x01F00000) >> 20;
    return flags;
  case 0xa2000053:
    *name = "fle.d";
    *op = RISCV_FLE_D;
    flags = 0 | HAS_RD | HAS_RS1 | HAS_RS2;
    *rd  = (ibits & 0x00000F80) >> 7;
    *rs1 = (ibits & 0x000F8000) >> 15;
    *rs2 = (ibits & 0x01F00000) >> 20;
    return flags;
  case 0xa2001053:
    *name = "flt.d";
    *op = RISCV_FLT_D;
    flags = 0 | HAS_RD | HAS_RS1 | HAS_RS2;
    *rd  = (ibits & 0x00000F80) >> 7;
    *rs1 = (ibits & 0x000F8000) >> 15;
    *rs2 = (ibits & 0x01F00000) >> 20;
    return flags;
  case 0xa2002053:
    *name = "feq.d";
    *op = RISCV_FEQ_D;
    flags = 0 | HAS_RD | HAS_RS1 | HAS_RS2;
    *rd  = (ibits & 0x00000F80) >> 7;
    *rs1 = (ibits & 0x000F8000) >> 15;
    *rs2 = (ibits & 0x01F00000) >> 20;
    return flags;
  case 0xa6000053:
    *name = "fle.q";
    *op = RISCV_FLE_Q;
    flags = 0 | HAS_RD | HAS_RS1 | HAS_RS2;
    *rd  = (ibits & 0x00000F80) >> 7;
    *rs1 = (ibits & 0x000F8000) >> 15;
    *rs2 = (ibits & 0x01F00000) >> 20;
    return flags;
  case 0xa6001053:
    *name = "flt.q";
    *op = RISCV_FLT_Q;
    flags = 0 | HAS_RD | HAS_RS1 | HAS_RS2;
    *rd  = (ibits & 0x00000F80) >> 7;
    *rs1 = (ibits & 0x000F8000) >> 15;
    *rs2 = (ibits & 0x01F00000) >> 20;
    return flags;
  case 0xa6002053:
    *name = "feq.q";
    *op = RISCV_FEQ_Q;
    flags = 0 | HAS_RD | HAS_RS1 | HAS_RS2;
    *rd  = (ibits & 0x00000F80) >> 7;
    *rs1 = (ibits & 0x000F8000) >> 15;
    *rs2 = (ibits & 0x01F00000) >> 20;
    return flags;
  }
  switch(ibits & 0xfe007fff){
  case 0x12000073:
    *name = "sfence.vma";
    *op = RISCV_SFENCE_VMA;
    flags = 0 | HAS_RS1 | HAS_RS2;
    *rs1 = (ibits & 0x000F8000) >> 15;
    *rs2 = (ibits & 0x01F00000) >> 20;
    return flags;
  }
  switch(ibits & 0xfff0007f){
  case 0x58000053:
    *name = "fsqrt.s";
    *op = RISCV_FSQRT_S;
    flags = 0 | HAS_RD | HAS_RS1;
    *rd  = (ibits & 0x00000F80) >> 7;
    *rs1 = (ibits & 0x000F8000) >> 15;
    return flags;
  case 0x40100053:
    *name = "fcvt.s.d";
    *op = RISCV_FCVT_S_D;
    flags = 0 | HAS_RD | HAS_RS1;
    *rd  = (ibits & 0x00000F80) >> 7;
    *rs1 = (ibits & 0x000F8000) >> 15;
    return flags;
  case 0x42000053:
    *name = "fcvt.d.s";
    *op = RISCV_FCVT_D_S;
    flags = 0 | HAS_RD | HAS_RS1;
    *rd  = (ibits & 0x00000F80) >> 7;
    *rs1 = (ibits & 0x000F8000) >> 15;
    return flags;
  case 0x5a000053:
    *name = "fsqrt.d";
    *op = RISCV_FSQRT_D;
    flags = 0 | HAS_RD | HAS_RS1;
    *rd  = (ibits & 0x00000F80) >> 7;
    *rs1 = (ibits & 0x000F8000) >> 15;
    return flags;
  case 0x40300053:
    *name = "fcvt.s.q";
    *op = RISCV_FCVT_S_Q;
    flags = 0 | HAS_RD | HAS_RS1;
    *rd  = (ibits & 0x00000F80) >> 7;
    *rs1 = (ibits & 0x000F8000) >> 15;
    return flags;
  case 0x46000053:
    *name = "fcvt.q.s";
    *op = RISCV_FCVT_Q_S;
    flags = 0 | HAS_RD | HAS_RS1;
    *rd  = (ibits & 0x00000F80) >> 7;
    *rs1 = (ibits & 0x000F8000) >> 15;
    return flags;
  case 0x42300053:
    *name = "fcvt.d.q";
    *op = RISCV_FCVT_D_Q;
    flags = 0 | HAS_RD | HAS_RS1;
    *rd  = (ibits & 0x00000F80) >> 7;
    *rs1 = (ibits & 0x000F8000) >> 15;
    return flags;
  case 0x46100053:
    *name = "fcvt.q.d";
    *op = RISCV_FCVT_Q_D;
    flags = 0 | HAS_RD | HAS_RS1;
    *rd  = (ibits & 0x00000F80) >> 7;
    *rs1 = (ibits & 0x000F8000) >> 15;
    return flags;
  case 0x5e000053:
    *name = "fsqrt.q";
    *op = RISCV_FSQRT_Q;
    flags = 0 | HAS_RD | HAS_RS1;
    *rd  = (ibits & 0x00000F80) >> 7;
    *rs1 = (ibits & 0x000F8000) >> 15;
    return flags;
  case 0xc0000053:
    *name = "fcvt.w.s";
    *op = RISCV_FCVT_W_S;
    flags = 0 | HAS_RD | HAS_RS1;
    *rd  = (ibits & 0x00000F80) >> 7;
    *rs1 = (ibits & 0x000F8000) >> 15;
    return flags;
  case 0xc0100053:
    *name = "fcvt.wu.s";
    *op = RISCV_FCVT_WU_S;
    flags = 0 | HAS_RD | HAS_RS1;
    *rd  = (ibits & 0x00000F80) >> 7;
    *rs1 = (ibits & 0x000F8000) >> 15;
    return flags;
  case 0xc0200053:
    *name = "fcvt.l.s";
    *op = RISCV_FCVT_L_S;
    flags = 0 | HAS_RD | HAS_RS1;
    *rd  = (ibits & 0x00000F80) >> 7;
    *rs1 = (ibits & 0x000F8000) >> 15;
    return flags;
  case 0xc0300053:
    *name = "fcvt.lu.s";
    *op = RISCV_FCVT_LU_S;
    flags = 0 | HAS_RD | HAS_RS1;
    *rd  = (ibits & 0x00000F80) >> 7;
    *rs1 = (ibits & 0x000F8000) >> 15;
    return flags;
  case 0xc2000053:
    *name = "fcvt.w.d";
    *op = RISCV_FCVT_W_D;
    flags = 0 | HAS_RD | HAS_RS1;
    *rd  = (ibits & 0x00000F80) >> 7;
    *rs1 = (ibits & 0x000F8000) >> 15;
    return flags;
  case 0xc2100053:
    *name = "fcvt.wu.d";
    *op = RISCV_FCVT_WU_D;
    flags = 0 | HAS_RD | HAS_RS1;
    *rd  = (ibits & 0x00000F80) >> 7;
    *rs1 = (ibits & 0x000F8000) >> 15;
    return flags;
  case 0xc2200053:
    *name = "fcvt.l.d";
    *op = RISCV_FCVT_L_D;
    flags = 0 | HAS_RD | HAS_RS1;
    *rd  = (ibits & 0x00000F80) >> 7;
    *rs1 = (ibits & 0x000F8000) >> 15;
    return flags;
  case 0xc2300053:
    *name = "fcvt.lu.d";
    *op = RISCV_FCVT_LU_D;
    flags = 0 | HAS_RD | HAS_RS1;
    *rd  = (ibits & 0x00000F80) >> 7;
    *rs1 = (ibits & 0x000F8000) >> 15;
    return flags;
  case 0xc6000053:
    *name = "fcvt.w.q";
    *op = RISCV_FCVT_W_Q;
    flags = 0 | HAS_RD | HAS_RS1;
    *rd  = (ibits & 0x00000F80) >> 7;
    *rs1 = (ibits & 0x000F8000) >> 15;
    return flags;
  case 0xc6100053:
    *name = "fcvt.wu.q";
    *op = RISCV_FCVT_WU_Q;
    flags = 0 | HAS_RD | HAS_RS1;
    *rd  = (ibits & 0x00000F80) >> 7;
    *rs1 = (ibits & 0x000F8000) >> 15;
    return flags;
  case 0xc6200053:
    *name = "fcvt.l.q";
    *op = RISCV_FCVT_L_Q;
    flags = 0 | HAS_RD | HAS_RS1;
    *rd  = (ibits & 0x00000F80) >> 7;
    *rs1 = (ibits & 0x000F8000) >> 15;
    return flags;
  case 0xc6300053:
    *name = "fcvt.lu.q";
    *op = RISCV_FCVT_LU_Q;
    flags = 0 | HAS_RD | HAS_RS1;
    *rd  = (ibits & 0x00000F80) >> 7;
    *rs1 = (ibits & 0x000F8000) >> 15;
    return flags;
  case 0xd0000053:
    *name = "fcvt.s.w";
    *op = RISCV_FCVT_S_W;
    flags = 0 | HAS_RD | HAS_RS1;
    *rd  = (ibits & 0x00000F80) >> 7;
    *rs1 = (ibits & 0x000F8000) >> 15;
    return flags;
  case 0xd0100053:
    *name = "fcvt.s.wu";
    *op = RISCV_FCVT_S_WU;
    flags = 0 | HAS_RD | HAS_RS1;
    *rd  = (ibits & 0x00000F80) >> 7;
    *rs1 = (ibits & 0x000F8000) >> 15;
    return flags;
  case 0xd0200053:
    *name = "fcvt.s.l";
    *op = RISCV_FCVT_S_L;
    flags = 0 | HAS_RD | HAS_RS1;
    *rd  = (ibits & 0x00000F80) >> 7;
    *rs1 = (ibits & 0x000F8000) >> 15;
    return flags;
  case 0xd0300053:
    *name = "fcvt.s.lu";
    *op = RISCV_FCVT_S_LU;
    flags = 0 | HAS_RD | HAS_RS1;
    *rd  = (ibits & 0x00000F80) >> 7;
    *rs1 = (ibits & 0x000F8000) >> 15;
    return flags;
  case 0xd2000053:
    *name = "fcvt.d.w";
    *op = RISCV_FCVT_D_W;
    flags = 0 | HAS_RD | HAS_RS1;
    *rd  = (ibits & 0x00000F80) >> 7;
    *rs1 = (ibits & 0x000F8000) >> 15;
    return flags;
  case 0xd2100053:
    *name = "fcvt.d.wu";
    *op = RISCV_FCVT_D_WU;
    flags = 0 | HAS_RD | HAS_RS1;
    *rd  = (ibits & 0x00000F80) >> 7;
    *rs1 = (ibits & 0x000F8000) >> 15;
    return flags;
  case 0xd2200053:
    *name = "fcvt.d.l";
    *op = RISCV_FCVT_D_L;
    flags = 0 | HAS_RD | HAS_RS1;
    *rd  = (ibits & 0x00000F80) >> 7;
    *rs1 = (ibits & 0x000F8000) >> 15;
    return flags;
  case 0xd2300053:
    *name = "fcvt.d.lu";
    *op = RISCV_FCVT_D_LU;
    flags = 0 | HAS_RD | HAS_RS1;
    *rd  = (ibits & 0x00000F80) >> 7;
    *rs1 = (ibits & 0x000F8000) >> 15;
    return flags;
  case 0xd6000053:
    *name = "fcvt.q.w";
    *op = RISCV_FCVT_Q_W;
    flags = 0 | HAS_RD | HAS_RS1;
    *rd  = (ibits & 0x00000F80) >> 7;
    *rs1 = (ibits & 0x000F8000) >> 15;
    return flags;
  case 0xd6100053:
    *name = "fcvt.q.wu";
    *op = RISCV_FCVT_Q_WU;
    flags = 0 | HAS_RD | HAS_RS1;
    *rd  = (ibits & 0x00000F80) >> 7;
    *rs1 = (ibits & 0x000F8000) >> 15;
    return flags;
  case 0xd6200053:
    *name = "fcvt.q.l";
    *op = RISCV_FCVT_Q_L;
    flags = 0 | HAS_RD | HAS_RS1;
    *rd  = (ibits & 0x00000F80) >> 7;
    *rs1 = (ibits & 0x000F8000) >> 15;
    return flags;
  case 0xd6300053:
    *name = "fcvt.q.lu";
    *op = RISCV_FCVT_Q_LU;
    flags = 0 | HAS_RD | HAS_RS1;
    *rd  = (ibits & 0x00000F80) >> 7;
    *rs1 = (ibits & 0x000F8000) >> 15;
    return flags;
  }
  switch(ibits & 0xfff0707f){
  case 0xe0000053:
    *name = "fmv.x.w";
    *op = RISCV_FMV_X_W;
    flags = 0 | HAS_RD | HAS_RS1;
    *rd  = (ibits & 0x00000F80) >> 7;
    *rs1 = (ibits & 0x000F8000) >> 15;
    return flags;
  case 0xe0001053:
    *name = "fclass.s";
    *op = RISCV_FCLASS_S;
    flags = 0 | HAS_RD | HAS_RS1;
    *rd  = (ibits & 0x00000F80) >> 7;
    *rs1 = (ibits & 0x000F8000) >> 15;
    return flags;
  case 0xe2000053:
    *name = "fmv.x.d";
    *op = RISCV_FMV_X_D;
    flags = 0 | HAS_RD | HAS_RS1;
    *rd  = (ibits & 0x00000F80) >> 7;
    *rs1 = (ibits & 0x000F8000) >> 15;
    return flags;
  case 0xe2001053:
    *name = "fclass.d";
    *op = RISCV_FCLASS_D;
    flags = 0 | HAS_RD | HAS_RS1;
    *rd  = (ibits & 0x00000F80) >> 7;
    *rs1 = (ibits & 0x000F8000) >> 15;
    return flags;
  case 0xe6000053:
    *name = "fmv.x.q";
    *op = RISCV_FMV_X_Q;
    flags = 0 | HAS_RD | HAS_RS1;
    *rd  = (ibits & 0x00000F80) >> 7;
    *rs1 = (ibits & 0x000F8000) >> 15;
    return flags;
  case 0xe6001053:
    *name = "fclass.q";
    *op = RISCV_FCLASS_Q;
    flags = 0 | HAS_RD | HAS_RS1;
    *rd  = (ibits & 0x00000F80) >> 7;
    *rs1 = (ibits & 0x000F8000) >> 15;
    return flags;
  case 0xf0000053:
    *name = "fmv.w.x";
    *op = RISCV_FMV_W_X;
    flags = 0 | HAS_RD | HAS_RS1;
    *rd  = (ibits & 0x00000F80) >> 7;
    *rs1 = (ibits & 0x000F8000) >> 15;
    return flags;
  case 0xf2000053:
    *name = "fmv.d.x";
    *op = RISCV_FMV_D_X;
    flags = 0 | HAS_RD | HAS_RS1;
    *rd  = (ibits & 0x00000F80) >> 7;
    *rs1 = (ibits & 0x000F8000) >> 15;
    return flags;
  case 0xf6000053:
    *name = "fmv.q.x";
    *op = RISCV_FMV_Q_X;
    flags = 0 | HAS_RD | HAS_RS1;
    *rd  = (ibits & 0x00000F80) >> 7;
    *rs1 = (ibits & 0x000F8000) >> 15;
    return flags;
  }
  switch(ibits & 0xffffffff){
  case 0x73:
    *name = "ecall";
    *op = RISCV_ECALL;
    flags = 0;
    return flags;
  case 0x100073:
    *name = "ebreak";
    *op = RISCV_EBREAK;
    flags = 0;
    return flags;
  case 0x200073:
    *name = "uret";
    *op = RISCV_URET;
    flags = 0;
    return flags;
  case 0x10200073:
    *name = "sret";
    *op = RISCV_SRET;
    flags = 0;
    return flags;
  case 0x30200073:
    *name = "mret";
    *op = RISCV_MRET;
    flags = 0;
    return flags;
  case 0x7b200073:
    *name = "dret";
    *op = RISCV_DRET;
    flags = 0;
    return flags;
  case 0x10500073:
    *name = "wfi";
    *op = RISCV_WFI;
    flags = 0;
    return flags;
  }
  return -1;
}
