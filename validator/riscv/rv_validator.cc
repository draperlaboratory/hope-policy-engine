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

#include <algorithm>
#include <cctype>
#include <iostream>
#include <stdexcept>
#include <string>
#include <utility>
#include "csr_list.h"
#include "platform_types.h"
#include "policy_eval.h"
#include "policy_utils.h"
#include "rv_validator.h"
#include "soc_tag_configuration.h"
#include "tag_based_validator.h"
#include "validator_exception.h"

namespace policy_engine {

static std::string tag_name(const meta_set_t* tag) {
  static char tag_name[1024];
  meta_set_to_string(tag, tag_name, sizeof(tag_name));
  return std::string(tag_name);
}

void rv_validator_t::apply_metadata(const metadata_memory_map_t* md_map) {
  for (const auto [ range, metadata ]: *md_map) {
    for (address_t start = range.start; start < range.end; start += 4) {
      try {
        tag_bus.insn_tag_at(start) = ms_cache.to_tag(ms_cache.canonize(metadata));
      } catch (const std::out_of_range& e) {
        throw configuration_exception_t("unable to apply metadata");
      }
    }
  }
}

void rv_validator_t::handle_violation(context_t* ctx, operands_t* ops){
  if (!failed) {
    failed = true;
    memcpy(&failed_ctx, ctx, sizeof(context_t));
    memcpy(&failed_ops, ops, sizeof(operands_t));
  }
}

void rv_validator_t::setup_validation() {
  ctx = {0, 0, 0, "", "", true};
  ops = {nullptr, nullptr, nullptr, nullptr, nullptr, nullptr};

  if (res.pcResult) {
    memset(res.pc, 0, sizeof(meta_set_t));
    res.pcResult = false;
  }

  if (res.rdResult) {
    memset(res.rd, 0, sizeof(meta_set_t));
    res.rdResult = false;
  }

  if (res.csrResult) {
    memset(res.csr, 0, sizeof(meta_set_t));
    res.csrResult = false;
  }
}

rv_validator_t::rv_validator_t(const std::string& policy_dir, const std::string& soc_cfg, RegisterReader_t rr, AddressFixer_t af) :
    sim_validator_t(rr, af), tag_based_validator_t(policy_dir), res({new meta_set_t{0}, new meta_set_t{0}, new meta_set_t{0}, true, true, true}),
    watch_pc(false), rule_cache(nullptr), failed(false), has_insn_mem_addr(false), rule_cache_hits(0), rule_cache_misses(0) {
  ireg_tags.fill(ms_cache.to_tag(ms_factory.get_meta_set("ISA.RISCV.Reg.Default")));
  ireg_tags[0] = ms_cache.to_tag(ms_factory.get_meta_set("ISA.RISCV.Reg.RZero"));
  csr_tags.fill(ms_cache.to_tag(ms_factory.get_meta_set("ISA.RISCV.CSR.Default")));
  pc_tag = ms_cache.to_tag(ms_factory.get_meta_set("ISA.RISCV.Reg.Env"));
  // set initial tags for specific CSRs
  csr_tags[CSR_MEPC] = ms_cache.to_tag(ms_factory.get_meta_set("ISA.RISCV.CSR.MEPC"));
  csr_tags[CSR_MTVAL] = ms_cache.to_tag(ms_factory.get_meta_set("ISA.RISCV.CSR.MTVal"));
  csr_tags[CSR_MTVEC] = ms_cache.to_tag(ms_factory.get_meta_set("ISA.RISCV.CSR.MTVec"));

  soc_tag_configuration_t config(&ms_factory, soc_cfg);
  config.apply(&tag_bus, &ms_cache);
}

rv_validator_t::~rv_validator_t() {
  delete res.pc;
  delete res.rd;
  delete res.csr;
  if (rule_cache) {
    delete rule_cache;
  }
}

std::pair<bool, bool> rv_validator_t::validate(address_t pc, insn_bits_t insn, address_t memory_addr) {
  has_insn_mem_addr = true;
  mem_addr = memory_addr;

  bool result = validate(pc, insn);
  if (rule_cache) {
    if (rule_cache_hit)
      rule_cache_hits++;
    else
      rule_cache_misses++;
  }
  return std::make_pair(result, rule_cache_hit);
}

void rv_validator_t::flush_rule_cache() {
  if (rule_cache)
    rule_cache->flush();
}

bool rv_validator_t::validate(address_t pc, insn_bits_t insn) {
  int policy_result = POLICY_EXP_FAILURE;

  setup_validation();
  prepare_eval(pc, insn);
  if (rule_cache) {
    if (rule_cache->allow(&ops, &res)) {
      rule_cache_hits++;
      rule_cache_hit = true;
      return true;
    } else {
      rule_cache_misses++;
      rule_cache_hit = false;
    }
  }

  policy_result = eval_policy(&ctx, &ops, &res);
  ctx.policy_result = policy_result;
  if (policy_result == POLICY_SUCCESS) {
    complete_eval();
  } else {
    std::printf("violation address: 0x%" PRIaddr "\n",pc);
    handle_violation(&ctx, &ops);
  }
  return policy_result == POLICY_SUCCESS;
}

bool rv_validator_t::commit() {
  bool hit_watch = false;

  if (res.pcResult) {
    tag_t new_tag = ms_cache.to_tag(ms_cache.canonize(*res.pc));
    if (watch_pc && pc_tag != new_tag) {
      std::cout << "Watch tag pc" << std::endl;
      hit_watch = true;
    }
    pc_tag = new_tag;
  }

  if (has_pending_RD && res.rdResult) {
    tag_t new_tag = ms_cache.to_tag(ms_cache.canonize(*res.rd));
    for (const address_t& reg : watch_regs) {
      if (pending_RD == reg && ireg_tags[pending_RD] != new_tag) {
        std::cout << "Watch tag reg" << std::endl;
        hit_watch = true;
      }
    }

    // dont update metadata on regZero
    if(pending_RD)
        ireg_tags[pending_RD] = new_tag;
  }
  
  if (has_pending_mem && res.rdResult) {
    tag_t new_tag = ms_cache.to_tag(ms_cache.canonize(*res.rd));
    tag_t old_tag;
    address_t mem_paddr = addr_fixer(mem_addr);
    try {
      old_tag = tag_bus.data_tag_at(mem_paddr);
    } catch (const std::out_of_range& e) {
      std::printf("failed to load MR tag @ 0x%" PRIaddr " (0x%" PRIaddr ")\n", mem_addr, mem_paddr);
      hit_watch = true; // might as well halt
    }

    for (const address_t& addr : watch_addrs) {
      if (mem_addr == addr && old_tag != new_tag){
        address_t epc_addr = ctx.epc;
        std::printf("Watch tag mem at PC 0x%" PRIaddr "\n", epc_addr);
        hit_watch = true;
      }
    }

    try {
      tag_bus.data_tag_at(mem_paddr) = new_tag;
    } catch (const std::out_of_range& e) {
      printf("failed to store MR tag @ 0x%" PRIaddr " (0x%" PRIaddr ")\n", mem_addr, mem_paddr);
      fflush(stdout);
      hit_watch = true; // might as well halt
    }
  }

  if (has_pending_CSR && res.csrResult) {
    tag_t new_tag = ms_cache.to_tag(ms_cache.canonize(*res.csr));
    for (const address_t& csr : watch_csrs) {
      if (pending_CSR == csr && csr_tags[pending_CSR] != new_tag){
        printf("Watch tag CSR\n");
        fflush(stdout);
        hit_watch = true;
      }
    }
    csr_tags[pending_CSR] = new_tag;
  }

  if (rule_cache) {
    results_t res_copy = {
      .pc = ms_cache.canonize(*res.pc),
      .rd = ms_cache.canonize(*res.rd),
      .csr = ms_cache.canonize(*res.csr),
      .pcResult = res.pcResult,
      .rdResult = res.rdResult,
      .csrResult = res.csrResult
    };

    if (ctx.cached && !rule_cache->allow(&ops, &res)) {
      rule_cache->install_rule(&ops, &res_copy);
    }
  }
  return hit_watch;
}

void rv_validator_t::prepare_eval(address_t pc, insn_bits_t insn) {
  failed = false;
  setup_validation();

  address_t pc_paddr = addr_fixer(pc);
  decoded_instruction_t inst = decode(insn, ADDRESS_T_SIZE*8);
  if (!inst) {
    printf("Couldn't decode instruction at 0x%" PRIaddr " (0x%" PRIaddr "): 0x%08x   %s\n", pc, pc_paddr, insn, inst.name.c_str());
  }
  pending_RD = inst.rd.getOrElse(-1);

  if (inst.rs1.exists) ops.op1 = ms_cache[ireg_tags[inst.rs1]];
  if (inst.flags.has_csr_load || inst.flags.has_csr_store) ops.op2 = ms_cache[csr_tags[inst.imm]];
  if (inst.rs2.exists) ops.op2 = ms_cache[ireg_tags[inst.rs2]];
  if (inst.rs3.exists) ops.op3 = ms_cache[ireg_tags[inst.rs3]];
  has_pending_CSR = inst.flags.has_csr_store;
  has_pending_RD = inst.rd.exists;
  has_pending_mem = inst.flags.has_store;
  pending_CSR = inst.imm.getOrElse(-1);

  // Handle memory address calculation
  if (inst.flags.has_load || inst.flags.has_store) {
    if (has_insn_mem_addr) {
      //mem_addr has already been set
      has_insn_mem_addr = false;
    }
    else {
      uint64_t reg_val = reg_reader(inst.rs1);

      /* mask off upper bits, just in case */
      mem_addr = (address_t)(reg_val);

      if (inst.imm.exists)
        mem_addr += inst.imm;

      /* mask off unaligned bits, just in case */
    }
    address_t mem_paddr = addr_fixer(mem_addr);
    ctx.bad_addr = mem_addr;
    try {
      tag_t mtag = tag_bus.data_tag_at(mem_paddr);
      ops.mem = ms_cache[mtag];
      if (!ops.mem) {
        char buf[128];
        sprintf(buf, "TMT miss for memory (0x%" PRIaddr " (0x%" PRIaddr ")) at instruction 0x%" PRIaddr ". TMT misses are fatal.\n", mem_addr, mem_paddr, pc);
        throw std::runtime_error(buf);
      }
    } catch (const std::out_of_range& e) {
      printf("failed to load MR tag -- pc: 0x%" PRIaddr " (0x%" PRIaddr ") addr: 0x%" PRIaddr " (0x%" PRIaddr ")\n", pc, pc_paddr, mem_addr, mem_paddr);
    }
  }

  tag_t ci_tag;
  try {
    ci_tag = tag_bus.insn_tag_at(pc_paddr);
  } catch (const std::out_of_range& e) {
    printf("failed to load CI tag for PC 0x%" PRIaddr " (0x%" PRIaddr ")\n", pc, pc_paddr);
  }
  ctx.epc = pc;
  ops.ci = ms_cache[ci_tag];
  ops.pc = ms_cache[pc_tag];
}

void rv_validator_t::complete_eval() {}

void rv_validator_t::config_rule_cache(const std::string& rule_cache_name, int capacity) {
  printf("%s rule cache with capacity %d!\n", rule_cache_name.c_str(), capacity);
  std::string name_lower;
  std::transform(rule_cache_name.begin(), rule_cache_name.end(), name_lower.begin(), [](char c){ return std::tolower(c); });
  if (name_lower == "ideal") {
    rule_cache = new ideal_rule_cache_t();
  } else if (name_lower == "finite") {
    rule_cache = new finite_rule_cache_t(capacity);
  } else if (name_lower == "dmhc") {
    rule_cache = new dmhc_rule_cache_t(capacity, DMHC_RULE_CACHE_IWIDTH, DMHC_RULE_CACHE_OWIDTH, DMHC_RULE_CACHE_K, DMHC_RULE_CACHE_NO_EVICT);
  } else if (rule_cache_name.size() != 0) {
    throw configuration_exception_t("Invalid rule cache name");
  }
}

void rv_validator_t::rule_cache_stats() {
  if (rule_cache) {
    std::cerr << "rule cache: hits " << rule_cache_hits << " misses " << rule_cache_misses << " total " << (rule_cache_hits + rule_cache_misses) << std::endl;
    double hit_rate = (double)rule_cache_hits/(rule_cache_hits + rule_cache_misses);
    std::cerr << "rule cache hit rate was " << (hit_rate*100) << "%!" << std::endl;
  }
};

} // namespace policy_engine