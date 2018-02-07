#include <stdio.h>

#include "policy_eval.h"
#include "riscv_isa.h"

extern "C" void debug_msg(context_t *ctx, const char *msg) {
  printf("@0x%p: %s", ctx->epc, msg);
}

extern "C" void debug_status(context_t *ctx, int status) {
  switch(status) {
  case POLICY_EXP_FAILURE :
    puts("Explicit Failure");
    break;
  case POLICY_IMP_FAILURE :
    puts("Implicit Failure");
    break;
  case POLICY_SUCCESS :
    puts("Success");
    break;
  default:
    puts("INVALID POLICY RESULT");
  }
}

extern "C" void debug_operands(context_t *ctx, operands_t *ops) {
  printf("operands stuff\n");
}

extern "C" void debug_results(context_t *ctx, results_t *res) {
  printf("results stuff\n");
}

// referenced by meta_set_t rendering code in policy_utils, but not used
extern "C" void printm(const char *fmt, ...) {
  printf("printm: %s\n", fmt);
}
