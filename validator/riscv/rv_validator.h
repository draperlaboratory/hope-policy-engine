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

#include <array>
#include <iostream>
#include <string>
#include <utility>
#include <vector>
#include "dmhc_rule_cache.h"
#include "finite_rule_cache.h"
#include "ideal_rule_cache.h"
#include "metadata_memory_map.h"
#include "policy_eval.h"
#include "reader.h"
#include "sim_validator.h"
#include "soc_tag_configuration.h"
#include "tag_based_validator.h"

namespace policy_engine {

class rv_validator_t : public sim_validator_t<RegisterReader_t, AddressFixer_t>, public tag_based_validator_t {
private:
  tag_bus_t tag_bus;

  uint32_t pending_RD;
  address_t mem_addr;
  uint32_t pending_CSR;
  bool has_pending_RD;
  bool has_pending_mem;
  bool has_pending_CSR;
  int logIdx;
  bool has_insn_mem_addr;
  bool rule_cache_hit;

public:
  const int xlen;

  context_t ctx;
  operands_t ops;
  results_t res;

  tag_t pc_tag;
  std::array<tag_t, 32> ireg_tags;
  std::array<tag_t, 0x1000> csr_tags;
  
  bool watch_pc;
  std::vector<address_t> watch_regs;
  std::vector<address_t> watch_csrs;
  std::vector<address_t> watch_addrs;

  rv_validator_t(int xlen, const std::string& policy_dir, const std::string& soc_cfg, RegisterReader_t rr, AddressFixer_t af);
  virtual ~rv_validator_t();

  constexpr uint64_t address_max() { if (xlen < 64) return (1ULL << xlen) - 1; else return -1; }

  // called before we call the policy code - initializes ground state of input/output structures
  void setup_validation();

  void apply_metadata(const metadata_memory_map_t* md_map);

  void handle_violation(context_t* ctx, const operands_t* ops);

  bool validate(address_t pc, insn_bits_t insn);
  std::pair<bool, bool> validate(address_t pc, insn_bits_t insn, address_t mem_addr);
  bool commit();

  // Provides the tag for a given address.  Used for debugging.
  tag_t& get_tag(address_t addr) { return tag_bus.data_tag_at(addr); }
  const meta_set_t* get_meta_set(address_t addr) {
    try {
      return ms_cache[get_tag(addr)];
    } catch (...) {
      return nullptr;
    }
  }

  const meta_set_t* get_pc_meta_set() { return ms_cache[pc_tag]; }
  const meta_set_t* get_csr_meta_set(address_t csr) { return ms_cache[csr_tags[csr]]; }
  const meta_set_t* get_ireg_meta_set(address_t reg) { return ms_cache[ireg_tags[reg]]; }

  void set_pc_watch(bool watching) { watch_pc = watching; }
  void set_reg_watch(address_t addr) { watch_regs.push_back(addr); }
  void set_csr_watch(address_t addr) { watch_csrs.push_back(addr); }
  void set_mem_watch(address_t addr) { watch_addrs.push_back(addr); }

  void prepare_eval(address_t pc, insn_bits_t insn);
  void complete_eval();

  void flush_rule_cache();
  void config_rule_cache(const std::string& cache_name, int capacity);
  void rule_cache_stats();

  // fields used by main.cc
  bool failed;
  context_t failed_ctx;
  operands_t failed_ops;
  rule_cache_t* rule_cache;
  uint64_t rule_cache_hits;
  uint64_t rule_cache_misses;
};

} // namespace policy_engine

#endif
