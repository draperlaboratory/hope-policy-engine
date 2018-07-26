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

#ifndef RV32_VALIDATOR_H
#define RV32_VALIDATOR_H

#include <stdio.h>
#include <string>
#include <vector>

#include "soc_tag_configuration.h"
#include "tag_based_validator.h"
#include "tag_converter.h"
#include "policy_eval.h"
#include "metadata_memory_map.h"
#ifdef ENABLE_PIPE
#include "base_pipe.h"
#endif
#ifdef ENABLE_IDEAL_PIPE
#include "ideal_pipe.h"
#endif
#ifdef ENABLE_FINITE_PIPE
#include "finite_pipe.h"
#endif

namespace policy_engine {

class rv32_validator_base_t : public tag_based_validator_t {
protected: 
  tag_bus_t tag_bus;

public:
  context_t *ctx;
  operands_t *ops;
  results_t *res;

  rv32_validator_base_t(meta_set_cache_t *ms_cache,
			meta_set_factory_t *ms_factory,
			RegisterReader_t rr);

  void apply_metadata(metadata_memory_map_t *md_map);
  
  // called before we call the policy code - initializes ground state of input/output structures
  void setup_validation();
  
  // Provides the tag for a given address.  Used for debugging.
  virtual bool get_tag(address_t addr, tag_t &tag) {
    return tag_bus.load_tag(addr, tag);
  }
};

#define REG_SP 2
class rv32_validator_t : public rv32_validator_base_t {
  uint32_t pending_RD;
  address_t mem_addr;
  uint32_t pending_CSR;
  bool has_pending_RD;
  bool has_pending_mem;
  bool has_pending_CSR;
  int logIdx;
  bool has_insn_mem_addr;
#ifdef ENABLE_PIPE
  bool pipe_hit;
#endif

//  meta_set_t temp_ci_tag;

 public:
  tag_t pc_tag;
  tag_file_t<32> ireg_tags;
  tag_file_t<0x1000> csr_tags;

  void handle_violation(context_t *ctx, operands_t *ops);
  
  bool watch_pc;
  std::vector<address_t> watch_regs;
  std::vector<address_t> watch_csrs;
  std::vector<address_t> watch_addrs;

  rv32_validator_t(meta_set_cache_t *ms_cache,
		   meta_set_factory_t *ms_factory,
		   soc_tag_configuration_t *tag_config,
		   RegisterReader_t rr);

  virtual ~rv32_validator_t() {
    free(ctx);
    free(ops);
    free(res);
  }

  bool validate(address_t pc, insn_bits_t insn);
  bool validate(address_t pc, insn_bits_t insn, address_t mem_addr, bool *hit);
  bool commit();

  // Provides the tag for a given address.  Used for debugging.
  virtual bool get_tag(address_t addr, tag_t &tag) {
    return tag_bus.load_tag(addr, tag);
  }

  void set_pc_watch(bool watching);
  void set_reg_watch(address_t addr);
  void set_csr_watch(address_t addr);
  void set_mem_watch(address_t addr);
  const char* get_first_rule_descr();
  const char* get_next_rule_descr();

  void prepare_eval(address_t pc, insn_bits_t insn);
  void complete_eval();

  // fields used by main.cc
  bool failed;
  context_t failed_ctx;
  operands_t failed_ops;
  #ifdef ENABLE_PIPE
  pipe_t *pipe;
  #endif
};

} // namespace policy_engine

#endif
