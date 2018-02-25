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

int32_t  decode(uint32_t ibits, uint32_t *rs1, uint32_t *rs2, uint32_t *rs3, uint32_t *rd, int32_t *imm, const char** name){
    int32_t flags = 0;
    *imm = 0;
    switch(ibits & 0x7f){
        case 0x6f:
            *name = "jal";
            flags = 0 | HAS_RD;
            *rd  = (ibits & 0x00000F80) >> 7;
            return flags;
        case 0x37:
            *name = "lui";
            flags = 0 | HAS_RD;
            *rd  = (ibits & 0x00000F80) >> 7;
            return flags;
        case 0x17:
            *name = "auipc";
            flags = 0 | HAS_RD;
            *rd  = (ibits & 0x00000F80) >> 7;
            return flags;
    }
    switch(ibits & 0x707f){
        case 0x63:
            *name = "beq";
            flags = 0 | HAS_RS1 | HAS_RS2;
            *rs1 = (ibits & 0x000F8000) >> 15;
            *rs2 = (ibits & 0x01F00000) >> 20;
            return flags;
        case 0x1063:
            *name = "bne";
            flags = 0 | HAS_RS1 | HAS_RS2;
            *rs1 = (ibits & 0x000F8000) >> 15;
            *rs2 = (ibits & 0x01F00000) >> 20;
            return flags;
        case 0x4063:
            *name = "blt";
            flags = 0 | HAS_RS1 | HAS_RS2;
            *rs1 = (ibits & 0x000F8000) >> 15;
            *rs2 = (ibits & 0x01F00000) >> 20;
            return flags;
        case 0x5063:
            *name = "bge";
            flags = 0 | HAS_RS1 | HAS_RS2;
            *rs1 = (ibits & 0x000F8000) >> 15;
            *rs2 = (ibits & 0x01F00000) >> 20;
            return flags;
        case 0x6063:
            *name = "bltu";
            flags = 0 | HAS_RS1 | HAS_RS2;
            *rs1 = (ibits & 0x000F8000) >> 15;
            *rs2 = (ibits & 0x01F00000) >> 20;
            return flags;
        case 0x7063:
            *name = "bgeu";
            flags = 0 | HAS_RS1 | HAS_RS2;
            *rs1 = (ibits & 0x000F8000) >> 15;
            *rs2 = (ibits & 0x01F00000) >> 20;
            return flags;
        case 0x67:
            *name = "jalr";
            flags = 0 | HAS_RD | HAS_RS1 | HAS_IMM;
            *rd  = (ibits & 0x00000F80) >> 7;
            *rs1 = (ibits & 0x000F8000) >> 15;
            *imm |= ((ibits & 0x80000000) ? ((ibits & 0xFFF00000) >> 20) | 0xFFFFF000 : (ibits & 0xFFF00000) >> 20);
            return flags;
        case 0x13:
            *name = "addi";
            flags = 0 | HAS_RD | HAS_RS1 | HAS_IMM;
            *rd  = (ibits & 0x00000F80) >> 7;
            *rs1 = (ibits & 0x000F8000) >> 15;
            *imm |= ((ibits & 0x80000000) ? ((ibits & 0xFFF00000) >> 20) | 0xFFFFF000 : (ibits & 0xFFF00000) >> 20);
            return flags;
        case 0x2013:
            *name = "slti";
            flags = 0 | HAS_RD | HAS_RS1 | HAS_IMM;
            *rd  = (ibits & 0x00000F80) >> 7;
            *rs1 = (ibits & 0x000F8000) >> 15;
            *imm |= ((ibits & 0x80000000) ? ((ibits & 0xFFF00000) >> 20) | 0xFFFFF000 : (ibits & 0xFFF00000) >> 20);
            return flags;
        case 0x3013:
            *name = "sltiu";
            flags = 0 | HAS_RD | HAS_RS1 | HAS_IMM;
            *rd  = (ibits & 0x00000F80) >> 7;
            *rs1 = (ibits & 0x000F8000) >> 15;
            *imm |= ((ibits & 0x80000000) ? ((ibits & 0xFFF00000) >> 20) | 0xFFFFF000 : (ibits & 0xFFF00000) >> 20);
            return flags;
        case 0x4013:
            *name = "xori";
            flags = 0 | HAS_RD | HAS_RS1 | HAS_IMM;
            *rd  = (ibits & 0x00000F80) >> 7;
            *rs1 = (ibits & 0x000F8000) >> 15;
            *imm |= ((ibits & 0x80000000) ? ((ibits & 0xFFF00000) >> 20) | 0xFFFFF000 : (ibits & 0xFFF00000) >> 20);
            return flags;
        case 0x6013:
            *name = "ori";
            flags = 0 | HAS_RD | HAS_RS1 | HAS_IMM;
            *rd  = (ibits & 0x00000F80) >> 7;
            *rs1 = (ibits & 0x000F8000) >> 15;
            *imm |= ((ibits & 0x80000000) ? ((ibits & 0xFFF00000) >> 20) | 0xFFFFF000 : (ibits & 0xFFF00000) >> 20);
            return flags;
        case 0x7013:
            *name = "andi";
            flags = 0 | HAS_RD | HAS_RS1 | HAS_IMM;
            *rd  = (ibits & 0x00000F80) >> 7;
            *rs1 = (ibits & 0x000F8000) >> 15;
            *imm |= ((ibits & 0x80000000) ? ((ibits & 0xFFF00000) >> 20) | 0xFFFFF000 : (ibits & 0xFFF00000) >> 20);
            return flags;
        case 0x1b:
            *name = "addiw";
            flags = 0 | HAS_RD | HAS_RS1 | HAS_IMM;
            *rd  = (ibits & 0x00000F80) >> 7;
            *rs1 = (ibits & 0x000F8000) >> 15;
            *imm |= ((ibits & 0x80000000) ? ((ibits & 0xFFF00000) >> 20) | 0xFFFFF000 : (ibits & 0xFFF00000) >> 20);
            return flags;
        case 0x3:
            *name = "lb";
            flags = 0 | HAS_RD | HAS_RS1 | HAS_LOAD | HAS_IMM;
            *rd  = (ibits & 0x00000F80) >> 7;
            *rs1 = (ibits & 0x000F8000) >> 15;
            *imm |= ((ibits & 0x80000000) ? ((ibits & 0xFFF00000) >> 20) | 0xFFFFF000 : (ibits & 0xFFF00000) >> 20);
            return flags;
        case 0x1003:
            *name = "lh";
            flags = 0 | HAS_RD | HAS_RS1 | HAS_LOAD | HAS_IMM;
            *rd  = (ibits & 0x00000F80) >> 7;
            *rs1 = (ibits & 0x000F8000) >> 15;
            *imm |= ((ibits & 0x80000000) ? ((ibits & 0xFFF00000) >> 20) | 0xFFFFF000 : (ibits & 0xFFF00000) >> 20);
            return flags;
        case 0x2003:
            *name = "lw";
            flags = 0 | HAS_RD | HAS_RS1 | HAS_LOAD | HAS_IMM;
            *rd  = (ibits & 0x00000F80) >> 7;
            *rs1 = (ibits & 0x000F8000) >> 15;
            *imm |= ((ibits & 0x80000000) ? ((ibits & 0xFFF00000) >> 20) | 0xFFFFF000 : (ibits & 0xFFF00000) >> 20);
            return flags;
        case 0x3003:
            *name = "ld";
            flags = 0 | HAS_RD | HAS_RS1 | HAS_LOAD | HAS_IMM;
            *rd  = (ibits & 0x00000F80) >> 7;
            *rs1 = (ibits & 0x000F8000) >> 15;
            *imm |= ((ibits & 0x80000000) ? ((ibits & 0xFFF00000) >> 20) | 0xFFFFF000 : (ibits & 0xFFF00000) >> 20);
            return flags;
        case 0x4003:
            *name = "lbu";
            flags = 0 | HAS_RD | HAS_RS1 | HAS_LOAD | HAS_IMM;
            *rd  = (ibits & 0x00000F80) >> 7;
            *rs1 = (ibits & 0x000F8000) >> 15;
            *imm |= ((ibits & 0x80000000) ? ((ibits & 0xFFF00000) >> 20) | 0xFFFFF000 : (ibits & 0xFFF00000) >> 20);
            return flags;
        case 0x5003:
            *name = "lhu";
            flags = 0 | HAS_RD | HAS_RS1 | HAS_LOAD | HAS_IMM;
            *rd  = (ibits & 0x00000F80) >> 7;
            *rs1 = (ibits & 0x000F8000) >> 15;
            *imm |= ((ibits & 0x80000000) ? ((ibits & 0xFFF00000) >> 20) | 0xFFFFF000 : (ibits & 0xFFF00000) >> 20);
            return flags;
        case 0x6003:
            *name = "lwu";
            flags = 0 | HAS_RD | HAS_RS1 | HAS_LOAD | HAS_IMM;
            *rd  = (ibits & 0x00000F80) >> 7;
            *rs1 = (ibits & 0x000F8000) >> 15;
            *imm |= ((ibits & 0x80000000) ? ((ibits & 0xFFF00000) >> 20) | 0xFFFFF000 : (ibits & 0xFFF00000) >> 20);
            return flags;
        case 0x23:
            *name = "sb";
            flags = 0 | HAS_RS1 | HAS_RS2 | HAS_STORE | HAS_IMM | HAS_IMM;
            *rs1 = (ibits & 0x000F8000) >> 15;
            *rs2 = (ibits & 0x01F00000) >> 20;
            *imm |= ((ibits & 0x80000000) ? ((ibits & 0xFE000000) >> 25) | 0xFFFFFF80 : (ibits & 0xFE000000) >> 25);
            *imm |= ((ibits & 0x00000F80) >> 7);
            return flags;
        case 0x1023:
            *name = "sh";
            flags = 0 | HAS_RS1 | HAS_RS2 | HAS_STORE | HAS_IMM | HAS_IMM;
            *rs1 = (ibits & 0x000F8000) >> 15;
            *rs2 = (ibits & 0x01F00000) >> 20;
            *imm |= ((ibits & 0x80000000) ? ((ibits & 0xFE000000) >> 25) | 0xFFFFFF80 : (ibits & 0xFE000000) >> 25);
            *imm |= ((ibits & 0x00000F80) >> 7);
            return flags;
        case 0x2023:
            *name = "sw";
            flags = 0 | HAS_RS1 | HAS_RS2 | HAS_STORE | HAS_IMM | HAS_IMM;
            *rs1 = (ibits & 0x000F8000) >> 15;
            *rs2 = (ibits & 0x01F00000) >> 20;
            *imm |= ((ibits & 0x80000000) ? ((ibits & 0xFE000000) >> 25) | 0xFFFFFF80 : (ibits & 0xFE000000) >> 25);
            *imm |= ((ibits & 0x00000F80) >> 7);
            return flags;
        case 0x3023:
            *name = "sd";
            flags = 0 | HAS_RS1 | HAS_RS2 | HAS_STORE | HAS_IMM | HAS_IMM;
            *rs1 = (ibits & 0x000F8000) >> 15;
            *rs2 = (ibits & 0x01F00000) >> 20;
            *imm |= ((ibits & 0x80000000) ? ((ibits & 0xFE000000) >> 25) | 0xFFFFFF80 : (ibits & 0xFE000000) >> 25);
            *imm |= ((ibits & 0x00000F80) >> 7);
            return flags;
        case 0xf:
            *name = "fence";
            flags = 0;
            return flags;
        case 0x100f:
            *name = "fence.i";
            flags = 0;
            return flags;
        case 0x1073:
            *name = "csrrw";
            flags = 0 | HAS_RD | HAS_RS1 | HAS_IMM;
            *rd  = (ibits & 0x00000F80) >> 7;
            *rs1 = (ibits & 0x000F8000) >> 15;
            *imm |= ((ibits & 0x80000000) ? ((ibits & 0xFFF00000) >> 20) | 0xFFFFF000 : (ibits & 0xFFF00000) >> 20);
            if(*rd == 0){
                flags |= HAS_CSR_STORE;
                flags &= ~HAS_RD;
            }
            else
                flags |= HAS_CSR_LOAD | HAS_CSR_STORE;
            return flags;
        case 0x2073:
            *name = "csrrs";
            flags = 0 | HAS_RD | HAS_RS1 | HAS_IMM;
            *rd  = (ibits & 0x00000F80) >> 7;
            *rs1 = (ibits & 0x000F8000) >> 15;
            *imm |= ((ibits & 0x80000000) ? ((ibits & 0xFFF00000) >> 20) | 0xFFFFF000 : (ibits & 0xFFF00000) >> 20);
            if(*rs1 == 0){
                flags |= HAS_CSR_LOAD;
                flags &= ~HAS_RS1;
            }
            else
                flags |= HAS_CSR_LOAD | HAS_CSR_STORE;
            return flags;
        case 0x3073:
            *name = "csrrc";
            flags = 0 | HAS_RD | HAS_RS1 | HAS_IMM;
            *rd  = (ibits & 0x00000F80) >> 7;
            *rs1 = (ibits & 0x000F8000) >> 15;
            *imm |= ((ibits & 0x80000000) ? ((ibits & 0xFFF00000) >> 20) | 0xFFFFF000 : (ibits & 0xFFF00000) >> 20);
            if(*rs1 == 0){
                flags |= HAS_CSR_LOAD;
                flags &= ~HAS_RS1;
            }
            else
                flags |= HAS_CSR_LOAD | HAS_CSR_STORE;
            return flags;
        case 0x5073:
            *name = "csrrwi";
            flags = 0 | HAS_RD | HAS_RS1 | HAS_IMM;
            *rd  = (ibits & 0x00000F80) >> 7;
            *rs1 = (ibits & 0x000F8000) >> 15;
            *imm |= ((ibits & 0x80000000) ? ((ibits & 0xFFF00000) >> 20) | 0xFFFFF000 : (ibits & 0xFFF00000) >> 20);
            if(*rd == 0){
                flags |= HAS_CSR_STORE;
                flags &= ~HAS_RD;
            }
            else
                flags |= HAS_CSR_LOAD | HAS_CSR_STORE;
            return flags;
        case 0x6073:
            *name = "csrrsi";
            flags = 0 | HAS_RD | HAS_RS1 | HAS_IMM;
            *rd  = (ibits & 0x00000F80) >> 7;
            *rs1 = (ibits & 0x000F8000) >> 15;
            *imm |= ((ibits & 0x80000000) ? ((ibits & 0xFFF00000) >> 20) | 0xFFFFF000 : (ibits & 0xFFF00000) >> 20);
            if(*rs1 == 0){
                flags |= HAS_CSR_LOAD;
                flags &= ~HAS_RS1;
            }
            else
                flags |= HAS_CSR_LOAD | HAS_CSR_STORE;
            return flags;
        case 0x7073:
            *name = "csrrci";
            flags = 0 | HAS_RD | HAS_RS1 | HAS_IMM;
            *rd  = (ibits & 0x00000F80) >> 7;
            *rs1 = (ibits & 0x000F8000) >> 15;
            *imm |= ((ibits & 0x80000000) ? ((ibits & 0xFFF00000) >> 20) | 0xFFFFF000 : (ibits & 0xFFF00000) >> 20);
            if(*rs1 == 0){
                flags |= HAS_CSR_LOAD;
                flags &= ~HAS_RS1;
            }
            else
                flags |= HAS_CSR_LOAD | HAS_CSR_STORE;
            return flags;
        case 0x2007:
            *name = "flw";
            flags = 0 | HAS_RD | HAS_RS1 | HAS_IMM;
            *rd  = (ibits & 0x00000F80) >> 7;
            *rs1 = (ibits & 0x000F8000) >> 15;
            *imm |= ((ibits & 0x80000000) ? ((ibits & 0xFFF00000) >> 20) | 0xFFFFF000 : (ibits & 0xFFF00000) >> 20);
            return flags;
        case 0x3007:
            *name = "fld";
            flags = 0 | HAS_RD | HAS_RS1 | HAS_IMM;
            *rd  = (ibits & 0x00000F80) >> 7;
            *rs1 = (ibits & 0x000F8000) >> 15;
            *imm |= ((ibits & 0x80000000) ? ((ibits & 0xFFF00000) >> 20) | 0xFFFFF000 : (ibits & 0xFFF00000) >> 20);
            return flags;
        case 0x4007:
            *name = "flq";
            flags = 0 | HAS_RD | HAS_RS1 | HAS_IMM;
            *rd  = (ibits & 0x00000F80) >> 7;
            *rs1 = (ibits & 0x000F8000) >> 15;
            *imm |= ((ibits & 0x80000000) ? ((ibits & 0xFFF00000) >> 20) | 0xFFFFF000 : (ibits & 0xFFF00000) >> 20);
            return flags;
        case 0x2027:
            *name = "fsw";
            flags = 0 | HAS_RS1 | HAS_RS2 | HAS_IMM | HAS_IMM;
            *rs1 = (ibits & 0x000F8000) >> 15;
            *rs2 = (ibits & 0x01F00000) >> 20;
            *imm |= ((ibits & 0x80000000) ? ((ibits & 0xFE000000) >> 25) | 0xFFFFFF80 : (ibits & 0xFE000000) >> 25);
            *imm |= ((ibits & 0x00000F80) >> 7);
            return flags;
        case 0x3027:
            *name = "fsd";
            flags = 0 | HAS_RS1 | HAS_RS2 | HAS_IMM | HAS_IMM;
            *rs1 = (ibits & 0x000F8000) >> 15;
            *rs2 = (ibits & 0x01F00000) >> 20;
            *imm |= ((ibits & 0x80000000) ? ((ibits & 0xFE000000) >> 25) | 0xFFFFFF80 : (ibits & 0xFE000000) >> 25);
            *imm |= ((ibits & 0x00000F80) >> 7);
            return flags;
        case 0x4027:
            *name = "fsq";
            flags = 0 | HAS_RS1 | HAS_RS2 | HAS_IMM | HAS_IMM;
            *rs1 = (ibits & 0x000F8000) >> 15;
            *rs2 = (ibits & 0x01F00000) >> 20;
            *imm |= ((ibits & 0x80000000) ? ((ibits & 0xFE000000) >> 25) | 0xFFFFFF80 : (ibits & 0xFE000000) >> 25);
            *imm |= ((ibits & 0x00000F80) >> 7);
            return flags;
    }
    switch(ibits & 0x600007f){
        case 0x43:
            *name = "fmadd.s";
            flags = 0 | HAS_RD | HAS_RS1 | HAS_RS2 | HAS_RS3;
            *rd  = (ibits & 0x00000F80) >> 7;
            *rs1 = (ibits & 0x000F8000) >> 15;
            *rs2 = (ibits & 0x01F00000) >> 20;
            *rs3 = (ibits & 0xF8000000) >> 27;
            return flags;
        case 0x47:
            *name = "fmsub.s";
            flags = 0 | HAS_RD | HAS_RS1 | HAS_RS2 | HAS_RS3;
            *rd  = (ibits & 0x00000F80) >> 7;
            *rs1 = (ibits & 0x000F8000) >> 15;
            *rs2 = (ibits & 0x01F00000) >> 20;
            *rs3 = (ibits & 0xF8000000) >> 27;
            return flags;
        case 0x4b:
            *name = "fnmsub.s";
            flags = 0 | HAS_RD | HAS_RS1 | HAS_RS2 | HAS_RS3;
            *rd  = (ibits & 0x00000F80) >> 7;
            *rs1 = (ibits & 0x000F8000) >> 15;
            *rs2 = (ibits & 0x01F00000) >> 20;
            *rs3 = (ibits & 0xF8000000) >> 27;
            return flags;
        case 0x4f:
            *name = "fnmadd.s";
            flags = 0 | HAS_RD | HAS_RS1 | HAS_RS2 | HAS_RS3;
            *rd  = (ibits & 0x00000F80) >> 7;
            *rs1 = (ibits & 0x000F8000) >> 15;
            *rs2 = (ibits & 0x01F00000) >> 20;
            *rs3 = (ibits & 0xF8000000) >> 27;
            return flags;
        case 0x2000043:
            *name = "fmadd.d";
            flags = 0 | HAS_RD | HAS_RS1 | HAS_RS2 | HAS_RS3;
            *rd  = (ibits & 0x00000F80) >> 7;
            *rs1 = (ibits & 0x000F8000) >> 15;
            *rs2 = (ibits & 0x01F00000) >> 20;
            *rs3 = (ibits & 0xF8000000) >> 27;
            return flags;
        case 0x2000047:
            *name = "fmsub.d";
            flags = 0 | HAS_RD | HAS_RS1 | HAS_RS2 | HAS_RS3;
            *rd  = (ibits & 0x00000F80) >> 7;
            *rs1 = (ibits & 0x000F8000) >> 15;
            *rs2 = (ibits & 0x01F00000) >> 20;
            *rs3 = (ibits & 0xF8000000) >> 27;
            return flags;
        case 0x200004b:
            *name = "fnmsub.d";
            flags = 0 | HAS_RD | HAS_RS1 | HAS_RS2 | HAS_RS3;
            *rd  = (ibits & 0x00000F80) >> 7;
            *rs1 = (ibits & 0x000F8000) >> 15;
            *rs2 = (ibits & 0x01F00000) >> 20;
            *rs3 = (ibits & 0xF8000000) >> 27;
            return flags;
        case 0x200004f:
            *name = "fnmadd.d";
            flags = 0 | HAS_RD | HAS_RS1 | HAS_RS2 | HAS_RS3;
            *rd  = (ibits & 0x00000F80) >> 7;
            *rs1 = (ibits & 0x000F8000) >> 15;
            *rs2 = (ibits & 0x01F00000) >> 20;
            *rs3 = (ibits & 0xF8000000) >> 27;
            return flags;
        case 0x6000043:
            *name = "fmadd.q";
            flags = 0 | HAS_RD | HAS_RS1 | HAS_RS2 | HAS_RS3;
            *rd  = (ibits & 0x00000F80) >> 7;
            *rs1 = (ibits & 0x000F8000) >> 15;
            *rs2 = (ibits & 0x01F00000) >> 20;
            *rs3 = (ibits & 0xF8000000) >> 27;
            return flags;
        case 0x6000047:
            *name = "fmsub.q";
            flags = 0 | HAS_RD | HAS_RS1 | HAS_RS2 | HAS_RS3;
            *rd  = (ibits & 0x00000F80) >> 7;
            *rs1 = (ibits & 0x000F8000) >> 15;
            *rs2 = (ibits & 0x01F00000) >> 20;
            *rs3 = (ibits & 0xF8000000) >> 27;
            return flags;
        case 0x600004b:
            *name = "fnmsub.q";
            flags = 0 | HAS_RD | HAS_RS1 | HAS_RS2 | HAS_RS3;
            *rd  = (ibits & 0x00000F80) >> 7;
            *rs1 = (ibits & 0x000F8000) >> 15;
            *rs2 = (ibits & 0x01F00000) >> 20;
            *rs3 = (ibits & 0xF8000000) >> 27;
            return flags;
        case 0x600004f:
            *name = "fnmadd.q";
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
            flags = 0 | HAS_RD | HAS_RS1 | HAS_RS2;
            *rd  = (ibits & 0x00000F80) >> 7;
            *rs1 = (ibits & 0x000F8000) >> 15;
            *rs2 = (ibits & 0x01F00000) >> 20;
            return flags;
        case 0x2000202f:
            *name = "amoxor.w";
            flags = 0 | HAS_RD | HAS_RS1 | HAS_RS2;
            *rd  = (ibits & 0x00000F80) >> 7;
            *rs1 = (ibits & 0x000F8000) >> 15;
            *rs2 = (ibits & 0x01F00000) >> 20;
            return flags;
        case 0x4000202f:
            *name = "amoor.w";
            flags = 0 | HAS_RD | HAS_RS1 | HAS_RS2;
            *rd  = (ibits & 0x00000F80) >> 7;
            *rs1 = (ibits & 0x000F8000) >> 15;
            *rs2 = (ibits & 0x01F00000) >> 20;
            return flags;
        case 0x6000202f:
            *name = "amoand.w";
            flags = 0 | HAS_RD | HAS_RS1 | HAS_RS2;
            *rd  = (ibits & 0x00000F80) >> 7;
            *rs1 = (ibits & 0x000F8000) >> 15;
            *rs2 = (ibits & 0x01F00000) >> 20;
            return flags;
        case 0x8000202f:
            *name = "amomin.w";
            flags = 0 | HAS_RD | HAS_RS1 | HAS_RS2;
            *rd  = (ibits & 0x00000F80) >> 7;
            *rs1 = (ibits & 0x000F8000) >> 15;
            *rs2 = (ibits & 0x01F00000) >> 20;
            return flags;
        case 0xa000202f:
            *name = "amomax.w";
            flags = 0 | HAS_RD | HAS_RS1 | HAS_RS2;
            *rd  = (ibits & 0x00000F80) >> 7;
            *rs1 = (ibits & 0x000F8000) >> 15;
            *rs2 = (ibits & 0x01F00000) >> 20;
            return flags;
        case 0xc000202f:
            *name = "amominu.w";
            flags = 0 | HAS_RD | HAS_RS1 | HAS_RS2;
            *rd  = (ibits & 0x00000F80) >> 7;
            *rs1 = (ibits & 0x000F8000) >> 15;
            *rs2 = (ibits & 0x01F00000) >> 20;
            return flags;
        case 0xe000202f:
            *name = "amomaxu.w";
            flags = 0 | HAS_RD | HAS_RS1 | HAS_RS2;
            *rd  = (ibits & 0x00000F80) >> 7;
            *rs1 = (ibits & 0x000F8000) >> 15;
            *rs2 = (ibits & 0x01F00000) >> 20;
            return flags;
        case 0x800202f:
            *name = "amoswap.w";
            flags = 0 | HAS_RD | HAS_RS1 | HAS_RS2;
            *rd  = (ibits & 0x00000F80) >> 7;
            *rs1 = (ibits & 0x000F8000) >> 15;
            *rs2 = (ibits & 0x01F00000) >> 20;
            return flags;
        case 0x1800202f:
            *name = "sc.w";
            flags = 0 | HAS_RD | HAS_RS1 | HAS_RS2;
            *rd  = (ibits & 0x00000F80) >> 7;
            *rs1 = (ibits & 0x000F8000) >> 15;
            *rs2 = (ibits & 0x01F00000) >> 20;
            return flags;
    }
    switch(ibits & 0xf9f0707f){
        case 0x1000202f:
            *name = "lr.w";
            flags = 0 | HAS_RD | HAS_RS1;
            *rd  = (ibits & 0x00000F80) >> 7;
            *rs1 = (ibits & 0x000F8000) >> 15;
            return flags;
    }
    switch(ibits & 0xfc00707f){
        case 0x1013:
            *name = "slli";
            flags = 0 | HAS_RD | HAS_RS1;
            *rd  = (ibits & 0x00000F80) >> 7;
            *rs1 = (ibits & 0x000F8000) >> 15;
            return flags;
        case 0x5013:
            *name = "srli";
            flags = 0 | HAS_RD | HAS_RS1;
            *rd  = (ibits & 0x00000F80) >> 7;
            *rs1 = (ibits & 0x000F8000) >> 15;
            return flags;
        case 0x40005013:
            *name = "srai";
            flags = 0 | HAS_RD | HAS_RS1;
            *rd  = (ibits & 0x00000F80) >> 7;
            *rs1 = (ibits & 0x000F8000) >> 15;
            return flags;
    }
    switch(ibits & 0xfe00007f){
        case 0x53:
            *name = "fadd.s";
            flags = 0 | HAS_RD | HAS_RS1 | HAS_RS2;
            *rd  = (ibits & 0x00000F80) >> 7;
            *rs1 = (ibits & 0x000F8000) >> 15;
            *rs2 = (ibits & 0x01F00000) >> 20;
            return flags;
        case 0x8000053:
            *name = "fsub.s";
            flags = 0 | HAS_RD | HAS_RS1 | HAS_RS2;
            *rd  = (ibits & 0x00000F80) >> 7;
            *rs1 = (ibits & 0x000F8000) >> 15;
            *rs2 = (ibits & 0x01F00000) >> 20;
            return flags;
        case 0x10000053:
            *name = "fmul.s";
            flags = 0 | HAS_RD | HAS_RS1 | HAS_RS2;
            *rd  = (ibits & 0x00000F80) >> 7;
            *rs1 = (ibits & 0x000F8000) >> 15;
            *rs2 = (ibits & 0x01F00000) >> 20;
            return flags;
        case 0x18000053:
            *name = "fdiv.s";
            flags = 0 | HAS_RD | HAS_RS1 | HAS_RS2;
            *rd  = (ibits & 0x00000F80) >> 7;
            *rs1 = (ibits & 0x000F8000) >> 15;
            *rs2 = (ibits & 0x01F00000) >> 20;
            return flags;
        case 0x2000053:
            *name = "fadd.d";
            flags = 0 | HAS_RD | HAS_RS1 | HAS_RS2;
            *rd  = (ibits & 0x00000F80) >> 7;
            *rs1 = (ibits & 0x000F8000) >> 15;
            *rs2 = (ibits & 0x01F00000) >> 20;
            return flags;
        case 0xa000053:
            *name = "fsub.d";
            flags = 0 | HAS_RD | HAS_RS1 | HAS_RS2;
            *rd  = (ibits & 0x00000F80) >> 7;
            *rs1 = (ibits & 0x000F8000) >> 15;
            *rs2 = (ibits & 0x01F00000) >> 20;
            return flags;
        case 0x12000053:
            *name = "fmul.d";
            flags = 0 | HAS_RD | HAS_RS1 | HAS_RS2;
            *rd  = (ibits & 0x00000F80) >> 7;
            *rs1 = (ibits & 0x000F8000) >> 15;
            *rs2 = (ibits & 0x01F00000) >> 20;
            return flags;
        case 0x1a000053:
            *name = "fdiv.d";
            flags = 0 | HAS_RD | HAS_RS1 | HAS_RS2;
            *rd  = (ibits & 0x00000F80) >> 7;
            *rs1 = (ibits & 0x000F8000) >> 15;
            *rs2 = (ibits & 0x01F00000) >> 20;
            return flags;
        case 0x6000053:
            *name = "fadd.q";
            flags = 0 | HAS_RD | HAS_RS1 | HAS_RS2;
            *rd  = (ibits & 0x00000F80) >> 7;
            *rs1 = (ibits & 0x000F8000) >> 15;
            *rs2 = (ibits & 0x01F00000) >> 20;
            return flags;
        case 0xe000053:
            *name = "fsub.q";
            flags = 0 | HAS_RD | HAS_RS1 | HAS_RS2;
            *rd  = (ibits & 0x00000F80) >> 7;
            *rs1 = (ibits & 0x000F8000) >> 15;
            *rs2 = (ibits & 0x01F00000) >> 20;
            return flags;
        case 0x16000053:
            *name = "fmul.q";
            flags = 0 | HAS_RD | HAS_RS1 | HAS_RS2;
            *rd  = (ibits & 0x00000F80) >> 7;
            *rs1 = (ibits & 0x000F8000) >> 15;
            *rs2 = (ibits & 0x01F00000) >> 20;
            return flags;
        case 0x1e000053:
            *name = "fdiv.q";
            flags = 0 | HAS_RD | HAS_RS1 | HAS_RS2;
            *rd  = (ibits & 0x00000F80) >> 7;
            *rs1 = (ibits & 0x000F8000) >> 15;
            *rs2 = (ibits & 0x01F00000) >> 20;
            return flags;
    }
    switch(ibits & 0xfe00707f){
        case 0x33:
            *name = "add";
            flags = 0 | HAS_RD | HAS_RS1 | HAS_RS2;
            *rd  = (ibits & 0x00000F80) >> 7;
            *rs1 = (ibits & 0x000F8000) >> 15;
            *rs2 = (ibits & 0x01F00000) >> 20;
            return flags;
        case 0x40000033:
            *name = "sub";
            flags = 0 | HAS_RD | HAS_RS1 | HAS_RS2;
            *rd  = (ibits & 0x00000F80) >> 7;
            *rs1 = (ibits & 0x000F8000) >> 15;
            *rs2 = (ibits & 0x01F00000) >> 20;
            return flags;
        case 0x1033:
            *name = "sll";
            flags = 0 | HAS_RD | HAS_RS1 | HAS_RS2;
            *rd  = (ibits & 0x00000F80) >> 7;
            *rs1 = (ibits & 0x000F8000) >> 15;
            *rs2 = (ibits & 0x01F00000) >> 20;
            return flags;
        case 0x2033:
            *name = "slt";
            flags = 0 | HAS_RD | HAS_RS1 | HAS_RS2;
            *rd  = (ibits & 0x00000F80) >> 7;
            *rs1 = (ibits & 0x000F8000) >> 15;
            *rs2 = (ibits & 0x01F00000) >> 20;
            return flags;
        case 0x3033:
            *name = "sltu";
            flags = 0 | HAS_RD | HAS_RS1 | HAS_RS2;
            *rd  = (ibits & 0x00000F80) >> 7;
            *rs1 = (ibits & 0x000F8000) >> 15;
            *rs2 = (ibits & 0x01F00000) >> 20;
            return flags;
        case 0x4033:
            *name = "xor";
            flags = 0 | HAS_RD | HAS_RS1 | HAS_RS2;
            *rd  = (ibits & 0x00000F80) >> 7;
            *rs1 = (ibits & 0x000F8000) >> 15;
            *rs2 = (ibits & 0x01F00000) >> 20;
            return flags;
        case 0x5033:
            *name = "srl";
            flags = 0 | HAS_RD | HAS_RS1 | HAS_RS2;
            *rd  = (ibits & 0x00000F80) >> 7;
            *rs1 = (ibits & 0x000F8000) >> 15;
            *rs2 = (ibits & 0x01F00000) >> 20;
            return flags;
        case 0x40005033:
            *name = "sra";
            flags = 0 | HAS_RD | HAS_RS1 | HAS_RS2;
            *rd  = (ibits & 0x00000F80) >> 7;
            *rs1 = (ibits & 0x000F8000) >> 15;
            *rs2 = (ibits & 0x01F00000) >> 20;
            return flags;
        case 0x6033:
            *name = "or";
            flags = 0 | HAS_RD | HAS_RS1 | HAS_RS2;
            *rd  = (ibits & 0x00000F80) >> 7;
            *rs1 = (ibits & 0x000F8000) >> 15;
            *rs2 = (ibits & 0x01F00000) >> 20;
            return flags;
        case 0x7033:
            *name = "and";
            flags = 0 | HAS_RD | HAS_RS1 | HAS_RS2;
            *rd  = (ibits & 0x00000F80) >> 7;
            *rs1 = (ibits & 0x000F8000) >> 15;
            *rs2 = (ibits & 0x01F00000) >> 20;
            return flags;
        case 0x101b:
            *name = "slliw";
            flags = 0 | HAS_RD | HAS_RS1;
            *rd  = (ibits & 0x00000F80) >> 7;
            *rs1 = (ibits & 0x000F8000) >> 15;
            return flags;
        case 0x501b:
            *name = "srliw";
            flags = 0 | HAS_RD | HAS_RS1;
            *rd  = (ibits & 0x00000F80) >> 7;
            *rs1 = (ibits & 0x000F8000) >> 15;
            return flags;
        case 0x4000501b:
            *name = "sraiw";
            flags = 0 | HAS_RD | HAS_RS1;
            *rd  = (ibits & 0x00000F80) >> 7;
            *rs1 = (ibits & 0x000F8000) >> 15;
            return flags;
        case 0x3b:
            *name = "addw";
            flags = 0 | HAS_RD | HAS_RS1 | HAS_RS2;
            *rd  = (ibits & 0x00000F80) >> 7;
            *rs1 = (ibits & 0x000F8000) >> 15;
            *rs2 = (ibits & 0x01F00000) >> 20;
            return flags;
        case 0x4000003b:
            *name = "subw";
            flags = 0 | HAS_RD | HAS_RS1 | HAS_RS2;
            *rd  = (ibits & 0x00000F80) >> 7;
            *rs1 = (ibits & 0x000F8000) >> 15;
            *rs2 = (ibits & 0x01F00000) >> 20;
            return flags;
        case 0x103b:
            *name = "sllw";
            flags = 0 | HAS_RD | HAS_RS1 | HAS_RS2;
            *rd  = (ibits & 0x00000F80) >> 7;
            *rs1 = (ibits & 0x000F8000) >> 15;
            *rs2 = (ibits & 0x01F00000) >> 20;
            return flags;
        case 0x503b:
            *name = "srlw";
            flags = 0 | HAS_RD | HAS_RS1 | HAS_RS2;
            *rd  = (ibits & 0x00000F80) >> 7;
            *rs1 = (ibits & 0x000F8000) >> 15;
            *rs2 = (ibits & 0x01F00000) >> 20;
            return flags;
        case 0x4000503b:
            *name = "sraw";
            flags = 0 | HAS_RD | HAS_RS1 | HAS_RS2;
            *rd  = (ibits & 0x00000F80) >> 7;
            *rs1 = (ibits & 0x000F8000) >> 15;
            *rs2 = (ibits & 0x01F00000) >> 20;
            return flags;
        case 0x2000033:
            *name = "mul";
            flags = 0 | HAS_RD | HAS_RS1 | HAS_RS2;
            *rd  = (ibits & 0x00000F80) >> 7;
            *rs1 = (ibits & 0x000F8000) >> 15;
            *rs2 = (ibits & 0x01F00000) >> 20;
            return flags;
        case 0x2001033:
            *name = "mulh";
            flags = 0 | HAS_RD | HAS_RS1 | HAS_RS2;
            *rd  = (ibits & 0x00000F80) >> 7;
            *rs1 = (ibits & 0x000F8000) >> 15;
            *rs2 = (ibits & 0x01F00000) >> 20;
            return flags;
        case 0x2002033:
            *name = "mulhsu";
            flags = 0 | HAS_RD | HAS_RS1 | HAS_RS2;
            *rd  = (ibits & 0x00000F80) >> 7;
            *rs1 = (ibits & 0x000F8000) >> 15;
            *rs2 = (ibits & 0x01F00000) >> 20;
            return flags;
        case 0x2003033:
            *name = "mulhu";
            flags = 0 | HAS_RD | HAS_RS1 | HAS_RS2;
            *rd  = (ibits & 0x00000F80) >> 7;
            *rs1 = (ibits & 0x000F8000) >> 15;
            *rs2 = (ibits & 0x01F00000) >> 20;
            return flags;
        case 0x2004033:
            *name = "div";
            flags = 0 | HAS_RD | HAS_RS1 | HAS_RS2;
            *rd  = (ibits & 0x00000F80) >> 7;
            *rs1 = (ibits & 0x000F8000) >> 15;
            *rs2 = (ibits & 0x01F00000) >> 20;
            return flags;
        case 0x2005033:
            *name = "divu";
            flags = 0 | HAS_RD | HAS_RS1 | HAS_RS2;
            *rd  = (ibits & 0x00000F80) >> 7;
            *rs1 = (ibits & 0x000F8000) >> 15;
            *rs2 = (ibits & 0x01F00000) >> 20;
            return flags;
        case 0x2006033:
            *name = "rem";
            flags = 0 | HAS_RD | HAS_RS1 | HAS_RS2;
            *rd  = (ibits & 0x00000F80) >> 7;
            *rs1 = (ibits & 0x000F8000) >> 15;
            *rs2 = (ibits & 0x01F00000) >> 20;
            return flags;
        case 0x2007033:
            *name = "remu";
            flags = 0 | HAS_RD | HAS_RS1 | HAS_RS2;
            *rd  = (ibits & 0x00000F80) >> 7;
            *rs1 = (ibits & 0x000F8000) >> 15;
            *rs2 = (ibits & 0x01F00000) >> 20;
            return flags;
        case 0x20000053:
            *name = "fsgnj.s";
            flags = 0 | HAS_RD | HAS_RS1 | HAS_RS2;
            *rd  = (ibits & 0x00000F80) >> 7;
            *rs1 = (ibits & 0x000F8000) >> 15;
            *rs2 = (ibits & 0x01F00000) >> 20;
            return flags;
        case 0x20001053:
            *name = "fsgnjn.s";
            flags = 0 | HAS_RD | HAS_RS1 | HAS_RS2;
            *rd  = (ibits & 0x00000F80) >> 7;
            *rs1 = (ibits & 0x000F8000) >> 15;
            *rs2 = (ibits & 0x01F00000) >> 20;
            return flags;
        case 0x20002053:
            *name = "fsgnjx.s";
            flags = 0 | HAS_RD | HAS_RS1 | HAS_RS2;
            *rd  = (ibits & 0x00000F80) >> 7;
            *rs1 = (ibits & 0x000F8000) >> 15;
            *rs2 = (ibits & 0x01F00000) >> 20;
            return flags;
        case 0x28000053:
            *name = "fmin.s";
            flags = 0 | HAS_RD | HAS_RS1 | HAS_RS2;
            *rd  = (ibits & 0x00000F80) >> 7;
            *rs1 = (ibits & 0x000F8000) >> 15;
            *rs2 = (ibits & 0x01F00000) >> 20;
            return flags;
        case 0x28001053:
            *name = "fmax.s";
            flags = 0 | HAS_RD | HAS_RS1 | HAS_RS2;
            *rd  = (ibits & 0x00000F80) >> 7;
            *rs1 = (ibits & 0x000F8000) >> 15;
            *rs2 = (ibits & 0x01F00000) >> 20;
            return flags;
        case 0x22000053:
            *name = "fsgnj.d";
            flags = 0 | HAS_RD | HAS_RS1 | HAS_RS2;
            *rd  = (ibits & 0x00000F80) >> 7;
            *rs1 = (ibits & 0x000F8000) >> 15;
            *rs2 = (ibits & 0x01F00000) >> 20;
            return flags;
        case 0x22001053:
            *name = "fsgnjn.d";
            flags = 0 | HAS_RD | HAS_RS1 | HAS_RS2;
            *rd  = (ibits & 0x00000F80) >> 7;
            *rs1 = (ibits & 0x000F8000) >> 15;
            *rs2 = (ibits & 0x01F00000) >> 20;
            return flags;
        case 0x22002053:
            *name = "fsgnjx.d";
            flags = 0 | HAS_RD | HAS_RS1 | HAS_RS2;
            *rd  = (ibits & 0x00000F80) >> 7;
            *rs1 = (ibits & 0x000F8000) >> 15;
            *rs2 = (ibits & 0x01F00000) >> 20;
            return flags;
        case 0x2a000053:
            *name = "fmin.d";
            flags = 0 | HAS_RD | HAS_RS1 | HAS_RS2;
            *rd  = (ibits & 0x00000F80) >> 7;
            *rs1 = (ibits & 0x000F8000) >> 15;
            *rs2 = (ibits & 0x01F00000) >> 20;
            return flags;
        case 0x2a001053:
            *name = "fmax.d";
            flags = 0 | HAS_RD | HAS_RS1 | HAS_RS2;
            *rd  = (ibits & 0x00000F80) >> 7;
            *rs1 = (ibits & 0x000F8000) >> 15;
            *rs2 = (ibits & 0x01F00000) >> 20;
            return flags;
        case 0x26000053:
            *name = "fsgnj.q";
            flags = 0 | HAS_RD | HAS_RS1 | HAS_RS2;
            *rd  = (ibits & 0x00000F80) >> 7;
            *rs1 = (ibits & 0x000F8000) >> 15;
            *rs2 = (ibits & 0x01F00000) >> 20;
            return flags;
        case 0x26001053:
            *name = "fsgnjn.q";
            flags = 0 | HAS_RD | HAS_RS1 | HAS_RS2;
            *rd  = (ibits & 0x00000F80) >> 7;
            *rs1 = (ibits & 0x000F8000) >> 15;
            *rs2 = (ibits & 0x01F00000) >> 20;
            return flags;
        case 0x26002053:
            *name = "fsgnjx.q";
            flags = 0 | HAS_RD | HAS_RS1 | HAS_RS2;
            *rd  = (ibits & 0x00000F80) >> 7;
            *rs1 = (ibits & 0x000F8000) >> 15;
            *rs2 = (ibits & 0x01F00000) >> 20;
            return flags;
        case 0x2e000053:
            *name = "fmin.q";
            flags = 0 | HAS_RD | HAS_RS1 | HAS_RS2;
            *rd  = (ibits & 0x00000F80) >> 7;
            *rs1 = (ibits & 0x000F8000) >> 15;
            *rs2 = (ibits & 0x01F00000) >> 20;
            return flags;
        case 0x2e001053:
            *name = "fmax.q";
            flags = 0 | HAS_RD | HAS_RS1 | HAS_RS2;
            *rd  = (ibits & 0x00000F80) >> 7;
            *rs1 = (ibits & 0x000F8000) >> 15;
            *rs2 = (ibits & 0x01F00000) >> 20;
            return flags;
        case 0xa0000053:
            *name = "fle.s";
            flags = 0 | HAS_RD | HAS_RS1 | HAS_RS2;
            *rd  = (ibits & 0x00000F80) >> 7;
            *rs1 = (ibits & 0x000F8000) >> 15;
            *rs2 = (ibits & 0x01F00000) >> 20;
            return flags;
        case 0xa0001053:
            *name = "flt.s";
            flags = 0 | HAS_RD | HAS_RS1 | HAS_RS2;
            *rd  = (ibits & 0x00000F80) >> 7;
            *rs1 = (ibits & 0x000F8000) >> 15;
            *rs2 = (ibits & 0x01F00000) >> 20;
            return flags;
        case 0xa0002053:
            *name = "feq.s";
            flags = 0 | HAS_RD | HAS_RS1 | HAS_RS2;
            *rd  = (ibits & 0x00000F80) >> 7;
            *rs1 = (ibits & 0x000F8000) >> 15;
            *rs2 = (ibits & 0x01F00000) >> 20;
            return flags;
        case 0xa2000053:
            *name = "fle.d";
            flags = 0 | HAS_RD | HAS_RS1 | HAS_RS2;
            *rd  = (ibits & 0x00000F80) >> 7;
            *rs1 = (ibits & 0x000F8000) >> 15;
            *rs2 = (ibits & 0x01F00000) >> 20;
            return flags;
        case 0xa2001053:
            *name = "flt.d";
            flags = 0 | HAS_RD | HAS_RS1 | HAS_RS2;
            *rd  = (ibits & 0x00000F80) >> 7;
            *rs1 = (ibits & 0x000F8000) >> 15;
            *rs2 = (ibits & 0x01F00000) >> 20;
            return flags;
        case 0xa2002053:
            *name = "feq.d";
            flags = 0 | HAS_RD | HAS_RS1 | HAS_RS2;
            *rd  = (ibits & 0x00000F80) >> 7;
            *rs1 = (ibits & 0x000F8000) >> 15;
            *rs2 = (ibits & 0x01F00000) >> 20;
            return flags;
        case 0xa6000053:
            *name = "fle.q";
            flags = 0 | HAS_RD | HAS_RS1 | HAS_RS2;
            *rd  = (ibits & 0x00000F80) >> 7;
            *rs1 = (ibits & 0x000F8000) >> 15;
            *rs2 = (ibits & 0x01F00000) >> 20;
            return flags;
        case 0xa6001053:
            *name = "flt.q";
            flags = 0 | HAS_RD | HAS_RS1 | HAS_RS2;
            *rd  = (ibits & 0x00000F80) >> 7;
            *rs1 = (ibits & 0x000F8000) >> 15;
            *rs2 = (ibits & 0x01F00000) >> 20;
            return flags;
        case 0xa6002053:
            *name = "feq.q";
            flags = 0 | HAS_RD | HAS_RS1 | HAS_RS2;
            *rd  = (ibits & 0x00000F80) >> 7;
            *rs1 = (ibits & 0x000F8000) >> 15;
            *rs2 = (ibits & 0x01F00000) >> 20;
            return flags;
    }
    switch(ibits & 0xfe007fff){
        case 0x12000073:
            *name = "sfence.vma";
            flags = 0 | HAS_RS1 | HAS_RS2;
            *rs1 = (ibits & 0x000F8000) >> 15;
            *rs2 = (ibits & 0x01F00000) >> 20;
            return flags;
    }
    switch(ibits & 0xfff0007f){
        case 0x58000053:
            *name = "fsqrt.s";
            flags = 0 | HAS_RD | HAS_RS1;
            *rd  = (ibits & 0x00000F80) >> 7;
            *rs1 = (ibits & 0x000F8000) >> 15;
            return flags;
        case 0x40100053:
            *name = "fcvt.s.d";
            flags = 0 | HAS_RD | HAS_RS1;
            *rd  = (ibits & 0x00000F80) >> 7;
            *rs1 = (ibits & 0x000F8000) >> 15;
            return flags;
        case 0x42000053:
            *name = "fcvt.d.s";
            flags = 0 | HAS_RD | HAS_RS1;
            *rd  = (ibits & 0x00000F80) >> 7;
            *rs1 = (ibits & 0x000F8000) >> 15;
            return flags;
        case 0x5a000053:
            *name = "fsqrt.d";
            flags = 0 | HAS_RD | HAS_RS1;
            *rd  = (ibits & 0x00000F80) >> 7;
            *rs1 = (ibits & 0x000F8000) >> 15;
            return flags;
        case 0x40300053:
            *name = "fcvt.s.q";
            flags = 0 | HAS_RD | HAS_RS1;
            *rd  = (ibits & 0x00000F80) >> 7;
            *rs1 = (ibits & 0x000F8000) >> 15;
            return flags;
        case 0x46000053:
            *name = "fcvt.q.s";
            flags = 0 | HAS_RD | HAS_RS1;
            *rd  = (ibits & 0x00000F80) >> 7;
            *rs1 = (ibits & 0x000F8000) >> 15;
            return flags;
        case 0x42300053:
            *name = "fcvt.d.q";
            flags = 0 | HAS_RD | HAS_RS1;
            *rd  = (ibits & 0x00000F80) >> 7;
            *rs1 = (ibits & 0x000F8000) >> 15;
            return flags;
        case 0x46100053:
            *name = "fcvt.q.d";
            flags = 0 | HAS_RD | HAS_RS1;
            *rd  = (ibits & 0x00000F80) >> 7;
            *rs1 = (ibits & 0x000F8000) >> 15;
            return flags;
        case 0x5e000053:
            *name = "fsqrt.q";
            flags = 0 | HAS_RD | HAS_RS1;
            *rd  = (ibits & 0x00000F80) >> 7;
            *rs1 = (ibits & 0x000F8000) >> 15;
            return flags;
        case 0xc0000053:
            *name = "fcvt.w.s";
            flags = 0 | HAS_RD | HAS_RS1;
            *rd  = (ibits & 0x00000F80) >> 7;
            *rs1 = (ibits & 0x000F8000) >> 15;
            return flags;
        case 0xc0100053:
            *name = "fcvt.wu.s";
            flags = 0 | HAS_RD | HAS_RS1;
            *rd  = (ibits & 0x00000F80) >> 7;
            *rs1 = (ibits & 0x000F8000) >> 15;
            return flags;
        case 0xc0200053:
            *name = "fcvt.l.s";
            flags = 0 | HAS_RD | HAS_RS1;
            *rd  = (ibits & 0x00000F80) >> 7;
            *rs1 = (ibits & 0x000F8000) >> 15;
            return flags;
        case 0xc0300053:
            *name = "fcvt.lu.s";
            flags = 0 | HAS_RD | HAS_RS1;
            *rd  = (ibits & 0x00000F80) >> 7;
            *rs1 = (ibits & 0x000F8000) >> 15;
            return flags;
        case 0xc2000053:
            *name = "fcvt.w.d";
            flags = 0 | HAS_RD | HAS_RS1;
            *rd  = (ibits & 0x00000F80) >> 7;
            *rs1 = (ibits & 0x000F8000) >> 15;
            return flags;
        case 0xc2100053:
            *name = "fcvt.wu.d";
            flags = 0 | HAS_RD | HAS_RS1;
            *rd  = (ibits & 0x00000F80) >> 7;
            *rs1 = (ibits & 0x000F8000) >> 15;
            return flags;
        case 0xc2200053:
            *name = "fcvt.l.d";
            flags = 0 | HAS_RD | HAS_RS1;
            *rd  = (ibits & 0x00000F80) >> 7;
            *rs1 = (ibits & 0x000F8000) >> 15;
            return flags;
        case 0xc2300053:
            *name = "fcvt.lu.d";
            flags = 0 | HAS_RD | HAS_RS1;
            *rd  = (ibits & 0x00000F80) >> 7;
            *rs1 = (ibits & 0x000F8000) >> 15;
            return flags;
        case 0xc6000053:
            *name = "fcvt.w.q";
            flags = 0 | HAS_RD | HAS_RS1;
            *rd  = (ibits & 0x00000F80) >> 7;
            *rs1 = (ibits & 0x000F8000) >> 15;
            return flags;
        case 0xc6100053:
            *name = "fcvt.wu.q";
            flags = 0 | HAS_RD | HAS_RS1;
            *rd  = (ibits & 0x00000F80) >> 7;
            *rs1 = (ibits & 0x000F8000) >> 15;
            return flags;
        case 0xc6200053:
            *name = "fcvt.l.q";
            flags = 0 | HAS_RD | HAS_RS1;
            *rd  = (ibits & 0x00000F80) >> 7;
            *rs1 = (ibits & 0x000F8000) >> 15;
            return flags;
        case 0xc6300053:
            *name = "fcvt.lu.q";
            flags = 0 | HAS_RD | HAS_RS1;
            *rd  = (ibits & 0x00000F80) >> 7;
            *rs1 = (ibits & 0x000F8000) >> 15;
            return flags;
        case 0xd0000053:
            *name = "fcvt.s.w";
            flags = 0 | HAS_RD | HAS_RS1;
            *rd  = (ibits & 0x00000F80) >> 7;
            *rs1 = (ibits & 0x000F8000) >> 15;
            return flags;
        case 0xd0100053:
            *name = "fcvt.s.wu";
            flags = 0 | HAS_RD | HAS_RS1;
            *rd  = (ibits & 0x00000F80) >> 7;
            *rs1 = (ibits & 0x000F8000) >> 15;
            return flags;
        case 0xd0200053:
            *name = "fcvt.s.l";
            flags = 0 | HAS_RD | HAS_RS1;
            *rd  = (ibits & 0x00000F80) >> 7;
            *rs1 = (ibits & 0x000F8000) >> 15;
            return flags;
        case 0xd0300053:
            *name = "fcvt.s.lu";
            flags = 0 | HAS_RD | HAS_RS1;
            *rd  = (ibits & 0x00000F80) >> 7;
            *rs1 = (ibits & 0x000F8000) >> 15;
            return flags;
        case 0xd2000053:
            *name = "fcvt.d.w";
            flags = 0 | HAS_RD | HAS_RS1;
            *rd  = (ibits & 0x00000F80) >> 7;
            *rs1 = (ibits & 0x000F8000) >> 15;
            return flags;
        case 0xd2100053:
            *name = "fcvt.d.wu";
            flags = 0 | HAS_RD | HAS_RS1;
            *rd  = (ibits & 0x00000F80) >> 7;
            *rs1 = (ibits & 0x000F8000) >> 15;
            return flags;
        case 0xd2200053:
            *name = "fcvt.d.l";
            flags = 0 | HAS_RD | HAS_RS1;
            *rd  = (ibits & 0x00000F80) >> 7;
            *rs1 = (ibits & 0x000F8000) >> 15;
            return flags;
        case 0xd2300053:
            *name = "fcvt.d.lu";
            flags = 0 | HAS_RD | HAS_RS1;
            *rd  = (ibits & 0x00000F80) >> 7;
            *rs1 = (ibits & 0x000F8000) >> 15;
            return flags;
        case 0xd6000053:
            *name = "fcvt.q.w";
            flags = 0 | HAS_RD | HAS_RS1;
            *rd  = (ibits & 0x00000F80) >> 7;
            *rs1 = (ibits & 0x000F8000) >> 15;
            return flags;
        case 0xd6100053:
            *name = "fcvt.q.wu";
            flags = 0 | HAS_RD | HAS_RS1;
            *rd  = (ibits & 0x00000F80) >> 7;
            *rs1 = (ibits & 0x000F8000) >> 15;
            return flags;
        case 0xd6200053:
            *name = "fcvt.q.l";
            flags = 0 | HAS_RD | HAS_RS1;
            *rd  = (ibits & 0x00000F80) >> 7;
            *rs1 = (ibits & 0x000F8000) >> 15;
            return flags;
        case 0xd6300053:
            *name = "fcvt.q.lu";
            flags = 0 | HAS_RD | HAS_RS1;
            *rd  = (ibits & 0x00000F80) >> 7;
            *rs1 = (ibits & 0x000F8000) >> 15;
            return flags;
    }
    switch(ibits & 0xfff0707f){
        case 0xe0000053:
            *name = "fmv.x.w";
            flags = 0 | HAS_RD | HAS_RS1;
            *rd  = (ibits & 0x00000F80) >> 7;
            *rs1 = (ibits & 0x000F8000) >> 15;
            return flags;
        case 0xe0001053:
            *name = "fclass.s";
            flags = 0 | HAS_RD | HAS_RS1;
            *rd  = (ibits & 0x00000F80) >> 7;
            *rs1 = (ibits & 0x000F8000) >> 15;
            return flags;
        case 0xe2000053:
            *name = "fmv.x.d";
            flags = 0 | HAS_RD | HAS_RS1;
            *rd  = (ibits & 0x00000F80) >> 7;
            *rs1 = (ibits & 0x000F8000) >> 15;
            return flags;
        case 0xe2001053:
            *name = "fclass.d";
            flags = 0 | HAS_RD | HAS_RS1;
            *rd  = (ibits & 0x00000F80) >> 7;
            *rs1 = (ibits & 0x000F8000) >> 15;
            return flags;
        case 0xe6000053:
            *name = "fmv.x.q";
            flags = 0 | HAS_RD | HAS_RS1;
            *rd  = (ibits & 0x00000F80) >> 7;
            *rs1 = (ibits & 0x000F8000) >> 15;
            return flags;
        case 0xe6001053:
            *name = "fclass.q";
            flags = 0 | HAS_RD | HAS_RS1;
            *rd  = (ibits & 0x00000F80) >> 7;
            *rs1 = (ibits & 0x000F8000) >> 15;
            return flags;
        case 0xf0000053:
            *name = "fmv.w.x";
            flags = 0 | HAS_RD | HAS_RS1;
            *rd  = (ibits & 0x00000F80) >> 7;
            *rs1 = (ibits & 0x000F8000) >> 15;
            return flags;
        case 0xf2000053:
            *name = "fmv.d.x";
            flags = 0 | HAS_RD | HAS_RS1;
            *rd  = (ibits & 0x00000F80) >> 7;
            *rs1 = (ibits & 0x000F8000) >> 15;
            return flags;
        case 0xf6000053:
            *name = "fmv.q.x";
            flags = 0 | HAS_RD | HAS_RS1;
            *rd  = (ibits & 0x00000F80) >> 7;
            *rs1 = (ibits & 0x000F8000) >> 15;
            return flags;
    }
    switch(ibits & 0xffffffff){
        case 0x73:
            *name = "ecall";
            flags = 0;
            return flags;
        case 0x100073:
            *name = "ebreak";
            flags = 0;
            return flags;
        case 0x200073:
            *name = "uret";
            flags = 0;
            return flags;
        case 0x10200073:
            *name = "sret";
            flags = 0;
            return flags;
        case 0x30200073:
            *name = "mret";
            flags = 0;
            return flags;
        case 0x7b200073:
            *name = "dret";
            flags = 0;
            return flags;
        case 0x10500073:
            *name = "wfi";
            flags = 0;
            return flags;
    }
    return -1;
}
