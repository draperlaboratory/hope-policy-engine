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

#ifdef __cplusplus
  #include <cstdint>
  #include <string>
#else
  #include "stdint.h"
  #include "string.h"
#endif
#include "inst_decoder.h"
#include "option.h"
#include "platform_types.h"
#include "policy_meta_set.h"

#ifdef __cplusplus

namespace policy_engine {

struct flags_t {
  bool has_load = false;
  bool has_store = false;
  bool has_csr_load = false;
  bool has_csr_store = false;
  bool is_compressed = false;

  flags_t operator |(const flags_t& other) const { return flags_t{
    .has_load=has_load || other.has_load,
    .has_store=has_store || other.has_store,
    .has_csr_load=has_csr_load || other.has_csr_load,
    .has_csr_store=has_csr_store || other.has_csr_store
  }; }
};

static const flags_t has_load{true, false, false, false, false};
static const flags_t has_store{false, true, false, false, false};
static const flags_t has_csr_load{false, false, true, false, false};
static const flags_t has_csr_store{false, false, false, true, false};
static const flags_t is_compressed{false, false, false, false, true};

struct decoded_instruction_t {
  const std::string name; // instruction name
  const op_t op;          // opcode defined in inst_decoder.h
  const option<int> rd;   // register id
  const option<int> rs1;  // register id
  const option<int> rs2;  // register id
  const option<int> rs3;  // register id
  const option<int> imm;  // signed immediate value
  const flags_t flags;

  explicit operator bool() const { return !name.empty(); }
};

decoded_instruction_t decode(insn_bits_t bits, int xlen);

#endif // __cplusplus

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
} // namespace policy_engine
#endif

#endif // RISCV_ISA_H