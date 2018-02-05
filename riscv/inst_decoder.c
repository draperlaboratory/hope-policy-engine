#include <stdint.h>

#define HAS_RS1   1
#define HAS_RS2   2
#define HAS_RS3   4
#define HAS_RD    8
#define HAS_IMM   16
#define HAS_LOAD  32
#define HAS_STORE 64

int32_t  decode(uint32_t ibits, uint32_t *rs1, uint32_t *rs2, uint32_t *rs3, uint32_t *rd, int32_t *imm, const char** name){
    *imm = 0;
    switch(ibits & 0x7f){
        case 0x6f:
            *name = "jal";
            *rd  = (ibits & 0x00000F80) >> 7;
            return 0 | HAS_RD;
        case 0x37:
            *name = "lui";
            *rd  = (ibits & 0x00000F80) >> 7;
            return 0 | HAS_RD;
        case 0x17:
            *name = "auipc";
            *rd  = (ibits & 0x00000F80) >> 7;
            return 0 | HAS_RD;
    }
    switch(ibits & 0x707f){
        case 0x63:
            *name = "beq";
            *rs1 = (ibits & 0x000F8000) >> 15;
            *rs2 = (ibits & 0x01F00000) >> 20;
            return 0 | HAS_RS1 | HAS_RS2;
        case 0x1063:
            *name = "bne";
            *rs1 = (ibits & 0x000F8000) >> 15;
            *rs2 = (ibits & 0x01F00000) >> 20;
            return 0 | HAS_RS1 | HAS_RS2;
        case 0x4063:
            *name = "blt";
            *rs1 = (ibits & 0x000F8000) >> 15;
            *rs2 = (ibits & 0x01F00000) >> 20;
            return 0 | HAS_RS1 | HAS_RS2;
        case 0x5063:
            *name = "bge";
            *rs1 = (ibits & 0x000F8000) >> 15;
            *rs2 = (ibits & 0x01F00000) >> 20;
            return 0 | HAS_RS1 | HAS_RS2;
        case 0x6063:
            *name = "bltu";
            *rs1 = (ibits & 0x000F8000) >> 15;
            *rs2 = (ibits & 0x01F00000) >> 20;
            return 0 | HAS_RS1 | HAS_RS2;
        case 0x7063:
            *name = "bgeu";
            *rs1 = (ibits & 0x000F8000) >> 15;
            *rs2 = (ibits & 0x01F00000) >> 20;
            return 0 | HAS_RS1 | HAS_RS2;
        case 0x67:
            *name = "jalr";
            *rd  = (ibits & 0x00000F80) >> 7;
            *rs1 = (ibits & 0x000F8000) >> 15;
            *imm |= ((ibits & 0x80000000) ? ((ibits & 0xFFF00000) >> 20) | 0xFFFFF000 : (ibits & 0xFFF00000) >> 20);
            return 0 | HAS_RD | HAS_RS1 | HAS_IMM;
        case 0x13:
            *name = "addi";
            *rd  = (ibits & 0x00000F80) >> 7;
            *rs1 = (ibits & 0x000F8000) >> 15;
            *imm |= ((ibits & 0x80000000) ? ((ibits & 0xFFF00000) >> 20) | 0xFFFFF000 : (ibits & 0xFFF00000) >> 20);
            return 0 | HAS_RD | HAS_RS1 | HAS_IMM;
        case 0x2013:
            *name = "slti";
            *rd  = (ibits & 0x00000F80) >> 7;
            *rs1 = (ibits & 0x000F8000) >> 15;
            *imm |= ((ibits & 0x80000000) ? ((ibits & 0xFFF00000) >> 20) | 0xFFFFF000 : (ibits & 0xFFF00000) >> 20);
            return 0 | HAS_RD | HAS_RS1 | HAS_IMM;
        case 0x3013:
            *name = "sltiu";
            *rd  = (ibits & 0x00000F80) >> 7;
            *rs1 = (ibits & 0x000F8000) >> 15;
            *imm |= ((ibits & 0x80000000) ? ((ibits & 0xFFF00000) >> 20) | 0xFFFFF000 : (ibits & 0xFFF00000) >> 20);
            return 0 | HAS_RD | HAS_RS1 | HAS_IMM;
        case 0x4013:
            *name = "xori";
            *rd  = (ibits & 0x00000F80) >> 7;
            *rs1 = (ibits & 0x000F8000) >> 15;
            *imm |= ((ibits & 0x80000000) ? ((ibits & 0xFFF00000) >> 20) | 0xFFFFF000 : (ibits & 0xFFF00000) >> 20);
            return 0 | HAS_RD | HAS_RS1 | HAS_IMM;
        case 0x6013:
            *name = "ori";
            *rd  = (ibits & 0x00000F80) >> 7;
            *rs1 = (ibits & 0x000F8000) >> 15;
            *imm |= ((ibits & 0x80000000) ? ((ibits & 0xFFF00000) >> 20) | 0xFFFFF000 : (ibits & 0xFFF00000) >> 20);
            return 0 | HAS_RD | HAS_RS1 | HAS_IMM;
        case 0x7013:
            *name = "andi";
            *rd  = (ibits & 0x00000F80) >> 7;
            *rs1 = (ibits & 0x000F8000) >> 15;
            *imm |= ((ibits & 0x80000000) ? ((ibits & 0xFFF00000) >> 20) | 0xFFFFF000 : (ibits & 0xFFF00000) >> 20);
            return 0 | HAS_RD | HAS_RS1 | HAS_IMM;
        case 0x1b:
            *name = "addiw";
            *rd  = (ibits & 0x00000F80) >> 7;
            *rs1 = (ibits & 0x000F8000) >> 15;
            *imm |= ((ibits & 0x80000000) ? ((ibits & 0xFFF00000) >> 20) | 0xFFFFF000 : (ibits & 0xFFF00000) >> 20);
            return 0 | HAS_RD | HAS_RS1 | HAS_IMM;
        case 0x3:
            *name = "lb";
            *rd  = (ibits & 0x00000F80) >> 7;
            *rs1 = (ibits & 0x000F8000) >> 15;
            *imm |= ((ibits & 0x80000000) ? ((ibits & 0xFFF00000) >> 20) | 0xFFFFF000 : (ibits & 0xFFF00000) >> 20);
            return 0 | HAS_RD | HAS_RS1 | HAS_LOAD | HAS_IMM;
        case 0x1003:
            *name = "lh";
            *rd  = (ibits & 0x00000F80) >> 7;
            *rs1 = (ibits & 0x000F8000) >> 15;
            *imm |= ((ibits & 0x80000000) ? ((ibits & 0xFFF00000) >> 20) | 0xFFFFF000 : (ibits & 0xFFF00000) >> 20);
            return 0 | HAS_RD | HAS_RS1 | HAS_LOAD | HAS_IMM;
        case 0x2003:
            *name = "lw";
            *rd  = (ibits & 0x00000F80) >> 7;
            *rs1 = (ibits & 0x000F8000) >> 15;
            *imm |= ((ibits & 0x80000000) ? ((ibits & 0xFFF00000) >> 20) | 0xFFFFF000 : (ibits & 0xFFF00000) >> 20);
            return 0 | HAS_RD | HAS_RS1 | HAS_LOAD | HAS_IMM;
        case 0x3003:
            *name = "ld";
            *rd  = (ibits & 0x00000F80) >> 7;
            *rs1 = (ibits & 0x000F8000) >> 15;
            *imm |= ((ibits & 0x80000000) ? ((ibits & 0xFFF00000) >> 20) | 0xFFFFF000 : (ibits & 0xFFF00000) >> 20);
            return 0 | HAS_RD | HAS_RS1 | HAS_LOAD | HAS_IMM;
        case 0x4003:
            *name = "lbu";
            *rd  = (ibits & 0x00000F80) >> 7;
            *rs1 = (ibits & 0x000F8000) >> 15;
            *imm |= ((ibits & 0x80000000) ? ((ibits & 0xFFF00000) >> 20) | 0xFFFFF000 : (ibits & 0xFFF00000) >> 20);
            return 0 | HAS_RD | HAS_RS1 | HAS_LOAD | HAS_IMM;
        case 0x5003:
            *name = "lhu";
            *rd  = (ibits & 0x00000F80) >> 7;
            *rs1 = (ibits & 0x000F8000) >> 15;
            *imm |= ((ibits & 0x80000000) ? ((ibits & 0xFFF00000) >> 20) | 0xFFFFF000 : (ibits & 0xFFF00000) >> 20);
            return 0 | HAS_RD | HAS_RS1 | HAS_LOAD | HAS_IMM;
        case 0x6003:
            *name = "lwu";
            *rd  = (ibits & 0x00000F80) >> 7;
            *rs1 = (ibits & 0x000F8000) >> 15;
            *imm |= ((ibits & 0x80000000) ? ((ibits & 0xFFF00000) >> 20) | 0xFFFFF000 : (ibits & 0xFFF00000) >> 20);
            return 0 | HAS_RD | HAS_RS1 | HAS_LOAD | HAS_IMM;
        case 0x23:
            *name = "sb";
            *rs1 = (ibits & 0x000F8000) >> 15;
            *rs2 = (ibits & 0x01F00000) >> 20;
            *imm |= ((ibits & 0x80000000) ? ((ibits & 0xFE000000) >> 25) | 0xFFFFFF80 : (ibits & 0xFE000000) >> 25);
            *imm |= ((ibits & 0x00000F80) >> 7);
            return 0 | HAS_RS1 | HAS_RS2 | HAS_STORE | HAS_IMM | HAS_IMM;
        case 0x1023:
            *name = "sh";
            *rs1 = (ibits & 0x000F8000) >> 15;
            *rs2 = (ibits & 0x01F00000) >> 20;
            *imm |= ((ibits & 0x80000000) ? ((ibits & 0xFE000000) >> 25) | 0xFFFFFF80 : (ibits & 0xFE000000) >> 25);
            *imm |= ((ibits & 0x00000F80) >> 7);
            return 0 | HAS_RS1 | HAS_RS2 | HAS_STORE | HAS_IMM | HAS_IMM;
        case 0x2023:
            *name = "sw";
            *rs1 = (ibits & 0x000F8000) >> 15;
            *rs2 = (ibits & 0x01F00000) >> 20;
            *imm |= ((ibits & 0x80000000) ? ((ibits & 0xFE000000) >> 25) | 0xFFFFFF80 : (ibits & 0xFE000000) >> 25);
            *imm |= ((ibits & 0x00000F80) >> 7);
            return 0 | HAS_RS1 | HAS_RS2 | HAS_STORE | HAS_IMM | HAS_IMM;
        case 0x3023:
            *name = "sd";
            *rs1 = (ibits & 0x000F8000) >> 15;
            *rs2 = (ibits & 0x01F00000) >> 20;
            *imm |= ((ibits & 0x80000000) ? ((ibits & 0xFE000000) >> 25) | 0xFFFFFF80 : (ibits & 0xFE000000) >> 25);
            *imm |= ((ibits & 0x00000F80) >> 7);
            return 0 | HAS_RS1 | HAS_RS2 | HAS_STORE | HAS_IMM | HAS_IMM;
        case 0xf:
            *name = "fence";
            return 0;
        case 0x100f:
            *name = "fence.i";
            return 0;
        case 0x1073:
            *name = "csrrw";
            *rd  = (ibits & 0x00000F80) >> 7;
            *rs1 = (ibits & 0x000F8000) >> 15;
            *imm |= ((ibits & 0x80000000) ? ((ibits & 0xFFF00000) >> 20) | 0xFFFFF000 : (ibits & 0xFFF00000) >> 20);
            return 0 | HAS_RD | HAS_RS1 | HAS_IMM;
        case 0x2073:
            *name = "csrrs";
            *rd  = (ibits & 0x00000F80) >> 7;
            *rs1 = (ibits & 0x000F8000) >> 15;
            *imm |= ((ibits & 0x80000000) ? ((ibits & 0xFFF00000) >> 20) | 0xFFFFF000 : (ibits & 0xFFF00000) >> 20);
            return 0 | HAS_RD | HAS_RS1 | HAS_IMM;
        case 0x3073:
            *name = "csrrc";
            *rd  = (ibits & 0x00000F80) >> 7;
            *rs1 = (ibits & 0x000F8000) >> 15;
            *imm |= ((ibits & 0x80000000) ? ((ibits & 0xFFF00000) >> 20) | 0xFFFFF000 : (ibits & 0xFFF00000) >> 20);
            return 0 | HAS_RD | HAS_RS1 | HAS_IMM;
        case 0x5073:
            *name = "csrrwi";
            *rd  = (ibits & 0x00000F80) >> 7;
            *rs1 = (ibits & 0x000F8000) >> 15;
            *imm |= ((ibits & 0x80000000) ? ((ibits & 0xFFF00000) >> 20) | 0xFFFFF000 : (ibits & 0xFFF00000) >> 20);
            return 0 | HAS_RD | HAS_RS1 | HAS_IMM;
        case 0x6073:
            *name = "csrrsi";
            *rd  = (ibits & 0x00000F80) >> 7;
            *rs1 = (ibits & 0x000F8000) >> 15;
            *imm |= ((ibits & 0x80000000) ? ((ibits & 0xFFF00000) >> 20) | 0xFFFFF000 : (ibits & 0xFFF00000) >> 20);
            return 0 | HAS_RD | HAS_RS1 | HAS_IMM;
        case 0x7073:
            *name = "csrrci";
            *rd  = (ibits & 0x00000F80) >> 7;
            *rs1 = (ibits & 0x000F8000) >> 15;
            *imm |= ((ibits & 0x80000000) ? ((ibits & 0xFFF00000) >> 20) | 0xFFFFF000 : (ibits & 0xFFF00000) >> 20);
            return 0 | HAS_RD | HAS_RS1 | HAS_IMM;
        case 0x2007:
            *name = "flw";
            *rd  = (ibits & 0x00000F80) >> 7;
            *rs1 = (ibits & 0x000F8000) >> 15;
            *imm |= ((ibits & 0x80000000) ? ((ibits & 0xFFF00000) >> 20) | 0xFFFFF000 : (ibits & 0xFFF00000) >> 20);
            return 0 | HAS_RD | HAS_RS1 | HAS_IMM;
        case 0x3007:
            *name = "fld";
            *rd  = (ibits & 0x00000F80) >> 7;
            *rs1 = (ibits & 0x000F8000) >> 15;
            *imm |= ((ibits & 0x80000000) ? ((ibits & 0xFFF00000) >> 20) | 0xFFFFF000 : (ibits & 0xFFF00000) >> 20);
            return 0 | HAS_RD | HAS_RS1 | HAS_IMM;
        case 0x2027:
            *name = "fsw";
            *rs1 = (ibits & 0x000F8000) >> 15;
            *rs2 = (ibits & 0x01F00000) >> 20;
            *imm |= ((ibits & 0x80000000) ? ((ibits & 0xFE000000) >> 25) | 0xFFFFFF80 : (ibits & 0xFE000000) >> 25);
            *imm |= ((ibits & 0x00000F80) >> 7);
            return 0 | HAS_RS1 | HAS_RS2 | HAS_IMM | HAS_IMM;
        case 0x3027:
            *name = "fsd";
            *rs1 = (ibits & 0x000F8000) >> 15;
            *rs2 = (ibits & 0x01F00000) >> 20;
            *imm |= ((ibits & 0x80000000) ? ((ibits & 0xFE000000) >> 25) | 0xFFFFFF80 : (ibits & 0xFE000000) >> 25);
            *imm |= ((ibits & 0x00000F80) >> 7);
            return 0 | HAS_RS1 | HAS_RS2 | HAS_IMM | HAS_IMM;
    }
    switch(ibits & 0x600007f){
        case 0x43:
            *name = "fmadd.s";
            *rd  = (ibits & 0x00000F80) >> 7;
            *rs1 = (ibits & 0x000F8000) >> 15;
            *rs2 = (ibits & 0x01F00000) >> 20;
            *rs3 = (ibits & 0xF8000000) >> 27;
            return 0 | HAS_RD | HAS_RS1 | HAS_RS2 | HAS_RS3;
        case 0x47:
            *name = "fmsub.s";
            *rd  = (ibits & 0x00000F80) >> 7;
            *rs1 = (ibits & 0x000F8000) >> 15;
            *rs2 = (ibits & 0x01F00000) >> 20;
            *rs3 = (ibits & 0xF8000000) >> 27;
            return 0 | HAS_RD | HAS_RS1 | HAS_RS2 | HAS_RS3;
        case 0x4b:
            *name = "fnmsub.s";
            *rd  = (ibits & 0x00000F80) >> 7;
            *rs1 = (ibits & 0x000F8000) >> 15;
            *rs2 = (ibits & 0x01F00000) >> 20;
            *rs3 = (ibits & 0xF8000000) >> 27;
            return 0 | HAS_RD | HAS_RS1 | HAS_RS2 | HAS_RS3;
        case 0x4f:
            *name = "fnmadd.s";
            *rd  = (ibits & 0x00000F80) >> 7;
            *rs1 = (ibits & 0x000F8000) >> 15;
            *rs2 = (ibits & 0x01F00000) >> 20;
            *rs3 = (ibits & 0xF8000000) >> 27;
            return 0 | HAS_RD | HAS_RS1 | HAS_RS2 | HAS_RS3;
        case 0x2000043:
            *name = "fmadd.d";
            *rd  = (ibits & 0x00000F80) >> 7;
            *rs1 = (ibits & 0x000F8000) >> 15;
            *rs2 = (ibits & 0x01F00000) >> 20;
            *rs3 = (ibits & 0xF8000000) >> 27;
            return 0 | HAS_RD | HAS_RS1 | HAS_RS2 | HAS_RS3;
        case 0x2000047:
            *name = "fmsub.d";
            *rd  = (ibits & 0x00000F80) >> 7;
            *rs1 = (ibits & 0x000F8000) >> 15;
            *rs2 = (ibits & 0x01F00000) >> 20;
            *rs3 = (ibits & 0xF8000000) >> 27;
            return 0 | HAS_RD | HAS_RS1 | HAS_RS2 | HAS_RS3;
        case 0x200004b:
            *name = "fnmsub.d";
            *rd  = (ibits & 0x00000F80) >> 7;
            *rs1 = (ibits & 0x000F8000) >> 15;
            *rs2 = (ibits & 0x01F00000) >> 20;
            *rs3 = (ibits & 0xF8000000) >> 27;
            return 0 | HAS_RD | HAS_RS1 | HAS_RS2 | HAS_RS3;
        case 0x200004f:
            *name = "fnmadd.d";
            *rd  = (ibits & 0x00000F80) >> 7;
            *rs1 = (ibits & 0x000F8000) >> 15;
            *rs2 = (ibits & 0x01F00000) >> 20;
            *rs3 = (ibits & 0xF8000000) >> 27;
            return 0 | HAS_RD | HAS_RS1 | HAS_RS2 | HAS_RS3;
    }
    switch(ibits & 0xf800707f){
        case 0x202f:
            *name = "amoadd.w";
            *rd  = (ibits & 0x00000F80) >> 7;
            *rs1 = (ibits & 0x000F8000) >> 15;
            *rs2 = (ibits & 0x01F00000) >> 20;
            return 0 | HAS_RD | HAS_RS1 | HAS_RS2;
        case 0x2000202f:
            *name = "amoxor.w";
            *rd  = (ibits & 0x00000F80) >> 7;
            *rs1 = (ibits & 0x000F8000) >> 15;
            *rs2 = (ibits & 0x01F00000) >> 20;
            return 0 | HAS_RD | HAS_RS1 | HAS_RS2;
        case 0x4000202f:
            *name = "amoor.w";
            *rd  = (ibits & 0x00000F80) >> 7;
            *rs1 = (ibits & 0x000F8000) >> 15;
            *rs2 = (ibits & 0x01F00000) >> 20;
            return 0 | HAS_RD | HAS_RS1 | HAS_RS2;
        case 0x6000202f:
            *name = "amoand.w";
            *rd  = (ibits & 0x00000F80) >> 7;
            *rs1 = (ibits & 0x000F8000) >> 15;
            *rs2 = (ibits & 0x01F00000) >> 20;
            return 0 | HAS_RD | HAS_RS1 | HAS_RS2;
        case 0x8000202f:
            *name = "amomin.w";
            *rd  = (ibits & 0x00000F80) >> 7;
            *rs1 = (ibits & 0x000F8000) >> 15;
            *rs2 = (ibits & 0x01F00000) >> 20;
            return 0 | HAS_RD | HAS_RS1 | HAS_RS2;
        case 0xa000202f:
            *name = "amomax.w";
            *rd  = (ibits & 0x00000F80) >> 7;
            *rs1 = (ibits & 0x000F8000) >> 15;
            *rs2 = (ibits & 0x01F00000) >> 20;
            return 0 | HAS_RD | HAS_RS1 | HAS_RS2;
        case 0xc000202f:
            *name = "amominu.w";
            *rd  = (ibits & 0x00000F80) >> 7;
            *rs1 = (ibits & 0x000F8000) >> 15;
            *rs2 = (ibits & 0x01F00000) >> 20;
            return 0 | HAS_RD | HAS_RS1 | HAS_RS2;
        case 0xe000202f:
            *name = "amomaxu.w";
            *rd  = (ibits & 0x00000F80) >> 7;
            *rs1 = (ibits & 0x000F8000) >> 15;
            *rs2 = (ibits & 0x01F00000) >> 20;
            return 0 | HAS_RD | HAS_RS1 | HAS_RS2;
        case 0x800202f:
            *name = "amoswap.w";
            *rd  = (ibits & 0x00000F80) >> 7;
            *rs1 = (ibits & 0x000F8000) >> 15;
            *rs2 = (ibits & 0x01F00000) >> 20;
            return 0 | HAS_RD | HAS_RS1 | HAS_RS2;
        case 0x1800202f:
            *name = "sc.w";
            *rd  = (ibits & 0x00000F80) >> 7;
            *rs1 = (ibits & 0x000F8000) >> 15;
            *rs2 = (ibits & 0x01F00000) >> 20;
            return 0 | HAS_RD | HAS_RS1 | HAS_RS2;
    }
    switch(ibits & 0xf9f0707f){
        case 0x1000202f:
            *name = "lr.w";
            *rd  = (ibits & 0x00000F80) >> 7;
            *rs1 = (ibits & 0x000F8000) >> 15;
            return 0 | HAS_RD | HAS_RS1;
    }
    switch(ibits & 0xfc00707f){
        case 0x1013:
            *name = "slli";
            *rd  = (ibits & 0x00000F80) >> 7;
            *rs1 = (ibits & 0x000F8000) >> 15;
            return 0 | HAS_RD | HAS_RS1;
        case 0x5013:
            *name = "srli";
            *rd  = (ibits & 0x00000F80) >> 7;
            *rs1 = (ibits & 0x000F8000) >> 15;
            return 0 | HAS_RD | HAS_RS1;
        case 0x40005013:
            *name = "srai";
            *rd  = (ibits & 0x00000F80) >> 7;
            *rs1 = (ibits & 0x000F8000) >> 15;
            return 0 | HAS_RD | HAS_RS1;
    }
    switch(ibits & 0xfe00007f){
        case 0x53:
            *name = "fadd.s";
            *rd  = (ibits & 0x00000F80) >> 7;
            *rs1 = (ibits & 0x000F8000) >> 15;
            *rs2 = (ibits & 0x01F00000) >> 20;
            return 0 | HAS_RD | HAS_RS1 | HAS_RS2;
        case 0x8000053:
            *name = "fsub.s";
            *rd  = (ibits & 0x00000F80) >> 7;
            *rs1 = (ibits & 0x000F8000) >> 15;
            *rs2 = (ibits & 0x01F00000) >> 20;
            return 0 | HAS_RD | HAS_RS1 | HAS_RS2;
        case 0x10000053:
            *name = "fmul.s";
            *rd  = (ibits & 0x00000F80) >> 7;
            *rs1 = (ibits & 0x000F8000) >> 15;
            *rs2 = (ibits & 0x01F00000) >> 20;
            return 0 | HAS_RD | HAS_RS1 | HAS_RS2;
        case 0x18000053:
            *name = "fdiv.s";
            *rd  = (ibits & 0x00000F80) >> 7;
            *rs1 = (ibits & 0x000F8000) >> 15;
            *rs2 = (ibits & 0x01F00000) >> 20;
            return 0 | HAS_RD | HAS_RS1 | HAS_RS2;
        case 0x2000053:
            *name = "fadd.d";
            *rd  = (ibits & 0x00000F80) >> 7;
            *rs1 = (ibits & 0x000F8000) >> 15;
            *rs2 = (ibits & 0x01F00000) >> 20;
            return 0 | HAS_RD | HAS_RS1 | HAS_RS2;
        case 0xa000053:
            *name = "fsub.d";
            *rd  = (ibits & 0x00000F80) >> 7;
            *rs1 = (ibits & 0x000F8000) >> 15;
            *rs2 = (ibits & 0x01F00000) >> 20;
            return 0 | HAS_RD | HAS_RS1 | HAS_RS2;
        case 0x12000053:
            *name = "fmul.d";
            *rd  = (ibits & 0x00000F80) >> 7;
            *rs1 = (ibits & 0x000F8000) >> 15;
            *rs2 = (ibits & 0x01F00000) >> 20;
            return 0 | HAS_RD | HAS_RS1 | HAS_RS2;
        case 0x1a000053:
            *name = "fdiv.d";
            *rd  = (ibits & 0x00000F80) >> 7;
            *rs1 = (ibits & 0x000F8000) >> 15;
            *rs2 = (ibits & 0x01F00000) >> 20;
            return 0 | HAS_RD | HAS_RS1 | HAS_RS2;
    }
    switch(ibits & 0xfe00707f){
        case 0x33:
            *name = "add";
            *rd  = (ibits & 0x00000F80) >> 7;
            *rs1 = (ibits & 0x000F8000) >> 15;
            *rs2 = (ibits & 0x01F00000) >> 20;
            return 0 | HAS_RD | HAS_RS1 | HAS_RS2;
        case 0x40000033:
            *name = "sub";
            *rd  = (ibits & 0x00000F80) >> 7;
            *rs1 = (ibits & 0x000F8000) >> 15;
            *rs2 = (ibits & 0x01F00000) >> 20;
            return 0 | HAS_RD | HAS_RS1 | HAS_RS2;
        case 0x1033:
            *name = "sll";
            *rd  = (ibits & 0x00000F80) >> 7;
            *rs1 = (ibits & 0x000F8000) >> 15;
            *rs2 = (ibits & 0x01F00000) >> 20;
            return 0 | HAS_RD | HAS_RS1 | HAS_RS2;
        case 0x2033:
            *name = "slt";
            *rd  = (ibits & 0x00000F80) >> 7;
            *rs1 = (ibits & 0x000F8000) >> 15;
            *rs2 = (ibits & 0x01F00000) >> 20;
            return 0 | HAS_RD | HAS_RS1 | HAS_RS2;
        case 0x3033:
            *name = "sltu";
            *rd  = (ibits & 0x00000F80) >> 7;
            *rs1 = (ibits & 0x000F8000) >> 15;
            *rs2 = (ibits & 0x01F00000) >> 20;
            return 0 | HAS_RD | HAS_RS1 | HAS_RS2;
        case 0x4033:
            *name = "xor";
            *rd  = (ibits & 0x00000F80) >> 7;
            *rs1 = (ibits & 0x000F8000) >> 15;
            *rs2 = (ibits & 0x01F00000) >> 20;
            return 0 | HAS_RD | HAS_RS1 | HAS_RS2;
        case 0x5033:
            *name = "srl";
            *rd  = (ibits & 0x00000F80) >> 7;
            *rs1 = (ibits & 0x000F8000) >> 15;
            *rs2 = (ibits & 0x01F00000) >> 20;
            return 0 | HAS_RD | HAS_RS1 | HAS_RS2;
        case 0x40005033:
            *name = "sra";
            *rd  = (ibits & 0x00000F80) >> 7;
            *rs1 = (ibits & 0x000F8000) >> 15;
            *rs2 = (ibits & 0x01F00000) >> 20;
            return 0 | HAS_RD | HAS_RS1 | HAS_RS2;
        case 0x6033:
            *name = "or";
            *rd  = (ibits & 0x00000F80) >> 7;
            *rs1 = (ibits & 0x000F8000) >> 15;
            *rs2 = (ibits & 0x01F00000) >> 20;
            return 0 | HAS_RD | HAS_RS1 | HAS_RS2;
        case 0x7033:
            *name = "and";
            *rd  = (ibits & 0x00000F80) >> 7;
            *rs1 = (ibits & 0x000F8000) >> 15;
            *rs2 = (ibits & 0x01F00000) >> 20;
            return 0 | HAS_RD | HAS_RS1 | HAS_RS2;
        case 0x101b:
            *name = "slliw";
            *rd  = (ibits & 0x00000F80) >> 7;
            *rs1 = (ibits & 0x000F8000) >> 15;
            return 0 | HAS_RD | HAS_RS1;
        case 0x501b:
            *name = "srliw";
            *rd  = (ibits & 0x00000F80) >> 7;
            *rs1 = (ibits & 0x000F8000) >> 15;
            return 0 | HAS_RD | HAS_RS1;
        case 0x4000501b:
            *name = "sraiw";
            *rd  = (ibits & 0x00000F80) >> 7;
            *rs1 = (ibits & 0x000F8000) >> 15;
            return 0 | HAS_RD | HAS_RS1;
        case 0x3b:
            *name = "addw";
            *rd  = (ibits & 0x00000F80) >> 7;
            *rs1 = (ibits & 0x000F8000) >> 15;
            *rs2 = (ibits & 0x01F00000) >> 20;
            return 0 | HAS_RD | HAS_RS1 | HAS_RS2;
        case 0x4000003b:
            *name = "subw";
            *rd  = (ibits & 0x00000F80) >> 7;
            *rs1 = (ibits & 0x000F8000) >> 15;
            *rs2 = (ibits & 0x01F00000) >> 20;
            return 0 | HAS_RD | HAS_RS1 | HAS_RS2;
        case 0x103b:
            *name = "sllw";
            *rd  = (ibits & 0x00000F80) >> 7;
            *rs1 = (ibits & 0x000F8000) >> 15;
            *rs2 = (ibits & 0x01F00000) >> 20;
            return 0 | HAS_RD | HAS_RS1 | HAS_RS2;
        case 0x503b:
            *name = "srlw";
            *rd  = (ibits & 0x00000F80) >> 7;
            *rs1 = (ibits & 0x000F8000) >> 15;
            *rs2 = (ibits & 0x01F00000) >> 20;
            return 0 | HAS_RD | HAS_RS1 | HAS_RS2;
        case 0x4000503b:
            *name = "sraw";
            *rd  = (ibits & 0x00000F80) >> 7;
            *rs1 = (ibits & 0x000F8000) >> 15;
            *rs2 = (ibits & 0x01F00000) >> 20;
            return 0 | HAS_RD | HAS_RS1 | HAS_RS2;
        case 0x2000033:
            *name = "mul";
            *rd  = (ibits & 0x00000F80) >> 7;
            *rs1 = (ibits & 0x000F8000) >> 15;
            *rs2 = (ibits & 0x01F00000) >> 20;
            return 0 | HAS_RD | HAS_RS1 | HAS_RS2;
        case 0x2001033:
            *name = "mulh";
            *rd  = (ibits & 0x00000F80) >> 7;
            *rs1 = (ibits & 0x000F8000) >> 15;
            *rs2 = (ibits & 0x01F00000) >> 20;
            return 0 | HAS_RD | HAS_RS1 | HAS_RS2;
        case 0x2002033:
            *name = "mulhsu";
            *rd  = (ibits & 0x00000F80) >> 7;
            *rs1 = (ibits & 0x000F8000) >> 15;
            *rs2 = (ibits & 0x01F00000) >> 20;
            return 0 | HAS_RD | HAS_RS1 | HAS_RS2;
        case 0x2003033:
            *name = "mulhu";
            *rd  = (ibits & 0x00000F80) >> 7;
            *rs1 = (ibits & 0x000F8000) >> 15;
            *rs2 = (ibits & 0x01F00000) >> 20;
            return 0 | HAS_RD | HAS_RS1 | HAS_RS2;
        case 0x2004033:
            *name = "div";
            *rd  = (ibits & 0x00000F80) >> 7;
            *rs1 = (ibits & 0x000F8000) >> 15;
            *rs2 = (ibits & 0x01F00000) >> 20;
            return 0 | HAS_RD | HAS_RS1 | HAS_RS2;
        case 0x2005033:
            *name = "divu";
            *rd  = (ibits & 0x00000F80) >> 7;
            *rs1 = (ibits & 0x000F8000) >> 15;
            *rs2 = (ibits & 0x01F00000) >> 20;
            return 0 | HAS_RD | HAS_RS1 | HAS_RS2;
        case 0x2006033:
            *name = "rem";
            *rd  = (ibits & 0x00000F80) >> 7;
            *rs1 = (ibits & 0x000F8000) >> 15;
            *rs2 = (ibits & 0x01F00000) >> 20;
            return 0 | HAS_RD | HAS_RS1 | HAS_RS2;
        case 0x2007033:
            *name = "remu";
            *rd  = (ibits & 0x00000F80) >> 7;
            *rs1 = (ibits & 0x000F8000) >> 15;
            *rs2 = (ibits & 0x01F00000) >> 20;
            return 0 | HAS_RD | HAS_RS1 | HAS_RS2;
        case 0x20000053:
            *name = "fsgnj.s";
            *rd  = (ibits & 0x00000F80) >> 7;
            *rs1 = (ibits & 0x000F8000) >> 15;
            *rs2 = (ibits & 0x01F00000) >> 20;
            return 0 | HAS_RD | HAS_RS1 | HAS_RS2;
        case 0x20001053:
            *name = "fsgnjn.s";
            *rd  = (ibits & 0x00000F80) >> 7;
            *rs1 = (ibits & 0x000F8000) >> 15;
            *rs2 = (ibits & 0x01F00000) >> 20;
            return 0 | HAS_RD | HAS_RS1 | HAS_RS2;
        case 0x20002053:
            *name = "fsgnjx.s";
            *rd  = (ibits & 0x00000F80) >> 7;
            *rs1 = (ibits & 0x000F8000) >> 15;
            *rs2 = (ibits & 0x01F00000) >> 20;
            return 0 | HAS_RD | HAS_RS1 | HAS_RS2;
        case 0x28000053:
            *name = "fmin.s";
            *rd  = (ibits & 0x00000F80) >> 7;
            *rs1 = (ibits & 0x000F8000) >> 15;
            *rs2 = (ibits & 0x01F00000) >> 20;
            return 0 | HAS_RD | HAS_RS1 | HAS_RS2;
        case 0x28001053:
            *name = "fmax.s";
            *rd  = (ibits & 0x00000F80) >> 7;
            *rs1 = (ibits & 0x000F8000) >> 15;
            *rs2 = (ibits & 0x01F00000) >> 20;
            return 0 | HAS_RD | HAS_RS1 | HAS_RS2;
        case 0x22000053:
            *name = "fsgnj.d";
            *rd  = (ibits & 0x00000F80) >> 7;
            *rs1 = (ibits & 0x000F8000) >> 15;
            *rs2 = (ibits & 0x01F00000) >> 20;
            return 0 | HAS_RD | HAS_RS1 | HAS_RS2;
        case 0x22001053:
            *name = "fsgnjn.d";
            *rd  = (ibits & 0x00000F80) >> 7;
            *rs1 = (ibits & 0x000F8000) >> 15;
            *rs2 = (ibits & 0x01F00000) >> 20;
            return 0 | HAS_RD | HAS_RS1 | HAS_RS2;
        case 0x22002053:
            *name = "fsgnjx.d";
            *rd  = (ibits & 0x00000F80) >> 7;
            *rs1 = (ibits & 0x000F8000) >> 15;
            *rs2 = (ibits & 0x01F00000) >> 20;
            return 0 | HAS_RD | HAS_RS1 | HAS_RS2;
        case 0x2a000053:
            *name = "fmin.d";
            *rd  = (ibits & 0x00000F80) >> 7;
            *rs1 = (ibits & 0x000F8000) >> 15;
            *rs2 = (ibits & 0x01F00000) >> 20;
            return 0 | HAS_RD | HAS_RS1 | HAS_RS2;
        case 0x2a001053:
            *name = "fmax.d";
            *rd  = (ibits & 0x00000F80) >> 7;
            *rs1 = (ibits & 0x000F8000) >> 15;
            *rs2 = (ibits & 0x01F00000) >> 20;
            return 0 | HAS_RD | HAS_RS1 | HAS_RS2;
        case 0xa0000053:
            *name = "fle.s";
            *rd  = (ibits & 0x00000F80) >> 7;
            *rs1 = (ibits & 0x000F8000) >> 15;
            *rs2 = (ibits & 0x01F00000) >> 20;
            return 0 | HAS_RD | HAS_RS1 | HAS_RS2;
        case 0xa0001053:
            *name = "flt.s";
            *rd  = (ibits & 0x00000F80) >> 7;
            *rs1 = (ibits & 0x000F8000) >> 15;
            *rs2 = (ibits & 0x01F00000) >> 20;
            return 0 | HAS_RD | HAS_RS1 | HAS_RS2;
        case 0xa0002053:
            *name = "feq.s";
            *rd  = (ibits & 0x00000F80) >> 7;
            *rs1 = (ibits & 0x000F8000) >> 15;
            *rs2 = (ibits & 0x01F00000) >> 20;
            return 0 | HAS_RD | HAS_RS1 | HAS_RS2;
        case 0xa2000053:
            *name = "fle.d";
            *rd  = (ibits & 0x00000F80) >> 7;
            *rs1 = (ibits & 0x000F8000) >> 15;
            *rs2 = (ibits & 0x01F00000) >> 20;
            return 0 | HAS_RD | HAS_RS1 | HAS_RS2;
        case 0xa2001053:
            *name = "flt.d";
            *rd  = (ibits & 0x00000F80) >> 7;
            *rs1 = (ibits & 0x000F8000) >> 15;
            *rs2 = (ibits & 0x01F00000) >> 20;
            return 0 | HAS_RD | HAS_RS1 | HAS_RS2;
        case 0xa2002053:
            *name = "feq.d";
            *rd  = (ibits & 0x00000F80) >> 7;
            *rs1 = (ibits & 0x000F8000) >> 15;
            *rs2 = (ibits & 0x01F00000) >> 20;
            return 0 | HAS_RD | HAS_RS1 | HAS_RS2;
    }
    switch(ibits & 0xfff0007f){
        case 0x58000053:
            *name = "fsqrt.s";
            *rd  = (ibits & 0x00000F80) >> 7;
            *rs1 = (ibits & 0x000F8000) >> 15;
            return 0 | HAS_RD | HAS_RS1;
        case 0x40100053:
            *name = "fcvt.s.d";
            *rd  = (ibits & 0x00000F80) >> 7;
            *rs1 = (ibits & 0x000F8000) >> 15;
            return 0 | HAS_RD | HAS_RS1;
        case 0x42000053:
            *name = "fcvt.d.s";
            *rd  = (ibits & 0x00000F80) >> 7;
            *rs1 = (ibits & 0x000F8000) >> 15;
            return 0 | HAS_RD | HAS_RS1;
        case 0x5a000053:
            *name = "fsqrt.d";
            *rd  = (ibits & 0x00000F80) >> 7;
            *rs1 = (ibits & 0x000F8000) >> 15;
            return 0 | HAS_RD | HAS_RS1;
        case 0xc0000053:
            *name = "fcvt.w.s";
            *rd  = (ibits & 0x00000F80) >> 7;
            *rs1 = (ibits & 0x000F8000) >> 15;
            return 0 | HAS_RD | HAS_RS1;
        case 0xc0100053:
            *name = "fcvt.wu.s";
            *rd  = (ibits & 0x00000F80) >> 7;
            *rs1 = (ibits & 0x000F8000) >> 15;
            return 0 | HAS_RD | HAS_RS1;
        case 0xc0200053:
            *name = "fcvt.l.s";
            *rd  = (ibits & 0x00000F80) >> 7;
            *rs1 = (ibits & 0x000F8000) >> 15;
            return 0 | HAS_RD | HAS_RS1;
        case 0xc0300053:
            *name = "fcvt.lu.s";
            *rd  = (ibits & 0x00000F80) >> 7;
            *rs1 = (ibits & 0x000F8000) >> 15;
            return 0 | HAS_RD | HAS_RS1;
        case 0xc2000053:
            *name = "fcvt.w.d";
            *rd  = (ibits & 0x00000F80) >> 7;
            *rs1 = (ibits & 0x000F8000) >> 15;
            return 0 | HAS_RD | HAS_RS1;
        case 0xc2100053:
            *name = "fcvt.wu.d";
            *rd  = (ibits & 0x00000F80) >> 7;
            *rs1 = (ibits & 0x000F8000) >> 15;
            return 0 | HAS_RD | HAS_RS1;
        case 0xc2200053:
            *name = "fcvt.l.d";
            *rd  = (ibits & 0x00000F80) >> 7;
            *rs1 = (ibits & 0x000F8000) >> 15;
            return 0 | HAS_RD | HAS_RS1;
        case 0xc2300053:
            *name = "fcvt.lu.d";
            *rd  = (ibits & 0x00000F80) >> 7;
            *rs1 = (ibits & 0x000F8000) >> 15;
            return 0 | HAS_RD | HAS_RS1;
        case 0xd0000053:
            *name = "fcvt.s.w";
            *rd  = (ibits & 0x00000F80) >> 7;
            *rs1 = (ibits & 0x000F8000) >> 15;
            return 0 | HAS_RD | HAS_RS1;
        case 0xd0100053:
            *name = "fcvt.s.wu";
            *rd  = (ibits & 0x00000F80) >> 7;
            *rs1 = (ibits & 0x000F8000) >> 15;
            return 0 | HAS_RD | HAS_RS1;
        case 0xd0200053:
            *name = "fcvt.s.l";
            *rd  = (ibits & 0x00000F80) >> 7;
            *rs1 = (ibits & 0x000F8000) >> 15;
            return 0 | HAS_RD | HAS_RS1;
        case 0xd0300053:
            *name = "fcvt.s.lu";
            *rd  = (ibits & 0x00000F80) >> 7;
            *rs1 = (ibits & 0x000F8000) >> 15;
            return 0 | HAS_RD | HAS_RS1;
        case 0xd2000053:
            *name = "fcvt.d.w";
            *rd  = (ibits & 0x00000F80) >> 7;
            *rs1 = (ibits & 0x000F8000) >> 15;
            return 0 | HAS_RD | HAS_RS1;
        case 0xd2100053:
            *name = "fcvt.d.wu";
            *rd  = (ibits & 0x00000F80) >> 7;
            *rs1 = (ibits & 0x000F8000) >> 15;
            return 0 | HAS_RD | HAS_RS1;
        case 0xd2200053:
            *name = "fcvt.d.l";
            *rd  = (ibits & 0x00000F80) >> 7;
            *rs1 = (ibits & 0x000F8000) >> 15;
            return 0 | HAS_RD | HAS_RS1;
        case 0xd2300053:
            *name = "fcvt.d.lu";
            *rd  = (ibits & 0x00000F80) >> 7;
            *rs1 = (ibits & 0x000F8000) >> 15;
            return 0 | HAS_RD | HAS_RS1;
    }
    switch(ibits & 0xfff0707f){
        case 0xe0000053:
            *name = "fmv.x.s";
            *rd  = (ibits & 0x00000F80) >> 7;
            *rs1 = (ibits & 0x000F8000) >> 15;
            return 0 | HAS_RD | HAS_RS1;
        case 0xe0001053:
            *name = "fclass.s";
            *rd  = (ibits & 0x00000F80) >> 7;
            *rs1 = (ibits & 0x000F8000) >> 15;
            return 0 | HAS_RD | HAS_RS1;
        case 0xe2000053:
            *name = "fmv.x.d";
            *rd  = (ibits & 0x00000F80) >> 7;
            *rs1 = (ibits & 0x000F8000) >> 15;
            return 0 | HAS_RD | HAS_RS1;
        case 0xe2001053:
            *name = "fclass.d";
            *rd  = (ibits & 0x00000F80) >> 7;
            *rs1 = (ibits & 0x000F8000) >> 15;
            return 0 | HAS_RD | HAS_RS1;
        case 0xf0000053:
            *name = "fmv.s.x";
            *rd  = (ibits & 0x00000F80) >> 7;
            *rs1 = (ibits & 0x000F8000) >> 15;
            return 0 | HAS_RD | HAS_RS1;
        case 0xf2000053:
            *name = "fmv.d.x";
            *rd  = (ibits & 0x00000F80) >> 7;
            *rs1 = (ibits & 0x000F8000) >> 15;
            return 0 | HAS_RD | HAS_RS1;
    }
    switch(ibits & 0xfff07fff){
        case 0x10100073:
            *name = "sfence.vm";
            *rs1 = (ibits & 0x000F8000) >> 15;
            return 0 | HAS_RS1;
    }
    switch(ibits & 0xffffffff){
        case 0x73:
            *name = "scall";
            return 0;
        case 0x100073:
            *name = "sbreak";
            return 0;
        case 0x10000073:
            *name = "sret";
            return 0;
        case 0x10200073:
            *name = "wfi";
            return 0;
        case 0x30600073:
            *name = "mrth";
            return 0;
        case 0x30500073:
            *name = "mrts";
            return 0;
        case 0x20500073:
            *name = "hrts";
            return 0;
    }
    return -1;
}
