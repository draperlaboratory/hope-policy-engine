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

#include <cstdint>
#include <string>
#include "platform_types.h"
#include "policy_meta_set.h"

namespace policy_engine {

static constexpr int HAS_RS1       = 0x1;
static constexpr int HAS_RS2       = 0x2;
static constexpr int HAS_RS3       = 0x4;
static constexpr int HAS_RD        = 0x8;
static constexpr int HAS_IMM       = 0x10;
static constexpr int HAS_LOAD      = 0x20;
static constexpr int HAS_STORE     = 0x40;
static constexpr int HAS_CSR_LOAD  = 0x80;
static constexpr int HAS_CSR_STORE = 0x100;

struct decoded_instruction_t {
  const std::string name; // instruction name
  const uint32_t op;      // opcode defined in inst_decode.h
  const int rd;           // register id
  const int rs1;          // register id
  const int rs2;          // register id
  const int rs3;          // register id
  const int imm;          // signed immediate value
  const uint32_t flags;   // fields only valid when HAS_* flag is set

  explicit operator bool() const { return !name.empty(); }
};

decoded_instruction_t decode(insn_bits_t bits);

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

} // namespace policy_engine

#endif // RISCV_ISA_H