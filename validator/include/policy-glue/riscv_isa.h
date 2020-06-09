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


#ifndef RISCV_ISA_H
#define RISCV_ISA_H

#include <stdint.h>
#include <stdbool.h>
#include "policy_meta_set.h"

#ifdef __cplusplus 
extern "C" {
#endif

#define HAS_RS1   1
#define HAS_RS2   2
#define HAS_RS3   4
#define HAS_RD    8
#define HAS_IMM   16
#define HAS_LOAD  32
#define HAS_STORE 64
#define HAS_CSR_LOAD    128
#define HAS_CSR_STORE   256

  /**
   * Decode funtion takes instruction bits and returns flags for which fields
   * in the results are valid.
   */
  int32_t  decode(uint32_t ibits,   // the instruction bits to decode
                                    // decoded results only valid when HAS_* flag set
                  uint32_t *rs1,   // register id
                  uint32_t *rs2,   // register id
                  uint32_t *rs3,   // register id
                  uint32_t *rd,    // register id
                  int32_t *imm,    // signed immediate value
                  const char **name, // Instruction name
				  uint32_t *op);     // Opcode defined in inst_decode.h

  /**
   * Structure that holds any special evaluation context, for
   * things like debug or performance optimization.
   */  
typedef struct context {
  uintptr_t epc;
  uintptr_t bad_addr;
  int policy_result;
  const char *fail_msg;
  const char *rule_str;
  bool cached;
} context_t;

  /**
   * Structure that holds input operands for rule eval
   */  
typedef struct operands {
  meta_set_t const *pc;
  meta_set_t const *ci;
  meta_set_t const *op1;
  meta_set_t const *op2;
  meta_set_t const *op3;
  meta_set_t const *mem;
} operands_t;

  /**
   * Structure that holds results after rule eval
   */  
typedef struct results {
  meta_set_t *pc;
  meta_set_t *rd;
  meta_set_t *csr;
  // flags indicate results are present
  bool pcResult;
  bool rdResult;
  bool csrResult;
} results_t;

#ifdef __cplusplus
}
#endif

#endif // RISCV_ISA_H

