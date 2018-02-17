
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
                  const char **name); // Instruction name


  /**
   * Structure that holds any special evaluation context, for
   * things like debug or performance optimization.
   */  
typedef struct context {
  uintptr_t epc;
  uintptr_t bad_addr;
  bool cached;
} context_t;

  /**
   * Structure that holds input operands for rule eval
   */  
typedef struct operands {
  meta_set_t *pc;
  meta_set_t *ci;
  meta_set_t *op1;
  meta_set_t *op2;
  meta_set_t *op3;
  meta_set_t *mem;
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

