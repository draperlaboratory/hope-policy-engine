
#ifndef RISCV_ISA_H
#define RISCV_ISA_H

#include <stdbool.h>
#include "policy_meta_set.h"

#ifdef __cplusplus 
extern "C" {
#endif

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

