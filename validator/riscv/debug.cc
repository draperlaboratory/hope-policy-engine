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

#include <stdio.h>

#include <stdarg.h>

#include "policy_eval.h"
#include "policy_utils.h"
#include "riscv_isa.h"

extern "C" void debug_msg(context_t *ctx, const char *msg) {
  printf("%s", msg);
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

void dump_tag(meta_set_t const *ms) {
  char tag_name[1024];
  if (ms) {
    meta_set_to_string(ms, tag_name, sizeof(tag_name));
    printf("%s", tag_name);
  } else {
    printf("<null>");
  }
}

extern "C" void debug_operands(context_t *ctx, operands_t *ops) {
  printf("  pc = "); dump_tag(ops->pc); printf("\n");
  printf("  ci = "); dump_tag(ops->ci); printf("\n");
  printf("  op1 = "); dump_tag(ops->op1); printf("\n");
  printf("  op2 = "); dump_tag(ops->op2); printf("\n");
  printf("  op3 = "); dump_tag(ops->op3); printf("\n");
  printf("  mem = "); dump_tag(ops->mem); printf("\n");
}

extern "C" void debug_results(context_t *ctx, results_t *res) {
  if (res->pcResult) {
    printf("  pc = "); dump_tag(res->pc); printf("\n");
  }
  if (res->rdResult) {
    printf("  rd = "); dump_tag(res->rd); printf("\n");
  }
  if (res->csrResult) {
    printf("  csr = "); dump_tag(res->csr); printf("\n");
  }
}

// referenced by meta_set_t rendering code in policy_utils, but not used
extern "C" void printm(const char* s, ...)
{
  char buf[256];
  va_list vl;

  va_start(vl, s);
  vsnprintf(buf, sizeof buf, s, vl);
  va_end(vl);

  puts(buf);
}
