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

#include <cstdarg>
#include <cstdio>
#include "policy_eval.h"
#include "policy_utils.h"
#include "tag_types.h"
#include "tag_utils.h"
#include "riscv_isa.h"

namespace policy_engine {

void dump_tag(tag_t tag) {
  if (tag != BAD_TAG_VALUE) {
    char tag_name[1024];
    meta_set_to_string(get_ms(tag), tag_name, sizeof(tag_name));
    std::printf(tag_name);
  } else {
    std::printf("<invalid>");
  }
}

extern "C" {

void debug_msg(const context_t* ctx, const char* msg) {
  std::printf("%s", msg);
}

void debug_status(const context_t *ctx, int status) {
  switch (status) {
  case POLICY_ERROR_FAILURE :
    puts("Internal Policy Error");
    break;
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

void debug_operands(const context_t* ctx, const operands_t* ops) {
  std::printf("    pc = (%ld) ", ops->pc); dump_tag(ops->pc); std::printf("\n");
  std::printf("    ci = (%ld) ", ops->ci); dump_tag(ops->ci); std::printf("\n");
  std::printf("    op1 = (%ld) ", ops->op1); dump_tag(ops->op1); std::printf("\n");
  std::printf("    op2 = (%ld) ", ops->op2); dump_tag(ops->op2); std::printf("\n");
  std::printf("    op3 = (%ld) ", ops->op3); dump_tag(ops->op3); std::printf("\n");
  std::printf("    mem = (%ld) ", ops->mem); dump_tag(ops->mem); std::printf("\n");
}

void debug_results(const context_t* ctx, const results_t* res) {
  if (res->pcResult) {
    std::printf("      pc = "); dump_tag(res->pc); std::printf("\n");
  }
  if (res->rdResult) {
    std::printf("      rd = "); dump_tag(res->rd); std::printf("\n");
  }
  if (res->csrResult) {
    std::printf("      csr = "); dump_tag(res->csr); std::printf("\n");
  }
}

// referenced by meta_set_t rendering code in policy_utils, but not used
void printm(const char* s, ...)
{
  char buf[256];
  va_list vl;

  va_start(vl, s);
  vsnprintf(buf, sizeof buf, s, vl);
  va_end(vl);

  puts(buf);
}

} // extern "C"
} // namespace policy_engine
