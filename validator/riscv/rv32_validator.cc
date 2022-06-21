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

#include <string>
#include "csr_list.h"
#include "platform_types.h"
#include "policy_eval.h"
#include "policy_utils.h"
#include "rv32_validator.h"
#include "soc_tag_configuration.h"
#include "tag_based_validator.h"
#include "validator_exception.h"

namespace policy_engine {

extern std::string render_metadata(const metadata_t* metadata);

static const char* tag_name(meta_set_t const* tag) {
  static char tag_name[1024];
  meta_set_to_string(tag, tag_name, sizeof(tag_name));
  return tag_name;
}

void rv32_validator_t::apply_metadata(const metadata_memory_map_t* md_map) {
  for (const auto [ range, metadata ]: *md_map) {
    for (address_t start = range.start; start < range.end; start += 4) {
      if (!tag_bus.store_insn_tag(start, ms_cache.to_tag(ms_cache.canonize(metadata)))) {
	      throw configuration_exception_t("unable to apply metadata");
      }
    }
  }
}

void rv32_validator_t::handle_violation(context_t* ctx, operands_t* ops){
  if (!failed) {
    failed = true;
    memcpy(&failed_ctx, ctx, sizeof(context_t));
    memcpy(&failed_ops, ops, sizeof(operands_t));
  }
}

void rv32_validator_t::setup_validation() {
  memset(ctx, 0, sizeof(*ctx));
  memset(ops, 0, sizeof(*ops));
  ctx->cached = true;

  if (res->pcResult) {
    memset(res->pc, 0, sizeof(meta_set_t));
    res->pcResult = false;
  }

  if (res->rdResult) {
    memset(res->rd, 0, sizeof(meta_set_t));
    res->rdResult = false;
  }

  if (res->csrResult) {
    memset(res->csr, 0, sizeof(meta_set_t));
    res->csrResult = false;
  }
}

rv32_validator_t::rv32_validator_t(const std::string& policy_dir, const std::string& soc_cfg, RegisterReader_t rr, AddressFixer_t af) : tag_based_validator_t(policy_dir, rr, af), watch_pc(false) {
  ctx = new context_t;
  ops = new operands_t;
  res = new results_t{
    .pc = new meta_set_t{0},
    .rd = new meta_set_t{0},
    .csr = new meta_set_t{0},
    .pcResult = true,
    .rdResult = true,
    .csrResult = true
  };

  const meta_set_t* ms;
  ms = ms_factory.get_meta_set("ISA.RISCV.Reg.Default");
  ireg_tags.reset(ms_cache.to_tag(ms));
  ms = ms_factory.get_meta_set("ISA.RISCV.Reg.RZero");
  ireg_tags[0] = ms_cache.to_tag(ms);
  ms = ms_factory.get_meta_set("ISA.RISCV.CSR.Default");
  csr_tags.reset(ms_cache.to_tag(ms));
  ms = ms_factory.get_meta_set("ISA.RISCV.Reg.Env");
  pc_tag = ms_cache.to_tag(ms);
  // set initial tags for specific CSRs
  ms = ms_factory.get_meta_set("ISA.RISCV.CSR.MEPC");
  csr_tags[CSR_MEPC] = ms_cache.to_tag(ms);
  ms = ms_factory.get_meta_set("ISA.RISCV.CSR.MTVal");
  csr_tags[CSR_MTVAL] = ms_cache.to_tag(ms);
  ms = ms_factory.get_meta_set("ISA.RISCV.CSR.MTVec");
  csr_tags[CSR_MTVEC] = ms_cache.to_tag(ms);

  soc_tag_configuration_t config(&ms_factory, soc_cfg);
  config.apply(&tag_bus, &ms_cache);
  failed = false;
  has_insn_mem_addr = false;
  rule_cache_hits = 0;
  rule_cache_misses = 0;
}

bool rv32_validator_t::validate(address_t pc, insn_bits_t insn,
                                address_t memory_addr, bool *hit) {
  has_insn_mem_addr = true;
  mem_addr = memory_addr;

  bool result = validate(pc, insn);
  if (rule_cache) {
    *hit = rule_cache_hit;
    if (rule_cache_hit) rule_cache_hits++;
    else rule_cache_misses++;
  }
  return result;
}

void rv32_validator_t::flush_rule_cache() {
  if (rule_cache)
    rule_cache->flush();
}

bool rv32_validator_t::validate(address_t pc, insn_bits_t insn) {
  int policy_result = POLICY_EXP_FAILURE;

  setup_validation();
  prepare_eval(pc, insn);
  if (rule_cache) {
    if (rule_cache->allow(ops, res)) {
      rule_cache_hits++;
      rule_cache_hit = true;
      return true;
    }
    else {
      rule_cache_misses++;
      rule_cache_hit = false;
    }
  }

  policy_result = eval_policy(ctx, ops, res);
  ctx->policy_result = policy_result;
  if (policy_result == POLICY_SUCCESS) {
    complete_eval();
  } else {
    printf("violation address: 0x%" PRIaddr "\n",pc);

    handle_violation(ctx, ops);
  }
  return policy_result == POLICY_SUCCESS;
}

bool rv32_validator_t::commit() {
  bool hit_watch = false;

  if (res->pcResult) {
    tag_t new_tag = ms_cache.to_tag(ms_cache.canonize(*res->pc));
    if(watch_pc && pc_tag != new_tag){
      printf("Watch tag pc\n");
      fflush(stdout);
      hit_watch = true;
    }
    pc_tag = new_tag;
  }

  if (has_pending_RD && res->rdResult) {
    tag_t new_tag = ms_cache.to_tag(ms_cache.canonize(*res->rd));
    for(std::vector<address_t>::iterator it = watch_regs.begin(); it != watch_regs.end(); ++it) {
      if(pending_RD == *it && ireg_tags[pending_RD] != new_tag){
        printf("Watch tag reg\n");
        fflush(stdout);
        hit_watch = true;
      }
    }

    // dont update metadata on regZero
    if(pending_RD)
        ireg_tags[pending_RD] = new_tag;
  }
  
  if (has_pending_mem && res->rdResult) {
    tag_t new_tag = ms_cache.to_tag(ms_cache.canonize(*res->rd));
    tag_t old_tag;
    address_t mem_paddr = addr_fixer(mem_addr);
    if (!tag_bus.load_tag(mem_paddr, old_tag)) {
        printf("failed to load MR tag @ 0x%" PRIaddr " (0x%" PRIaddr ")\n", mem_addr, mem_paddr);

      fflush(stdout);
      // might as well halt
      hit_watch = true;
    }
    for(std::vector<address_t>::iterator it = watch_addrs.begin(); it != watch_addrs.end(); ++it) {
      if(mem_addr == *it && old_tag != new_tag){
        address_t epc_addr = ctx->epc;

        printf("Watch tag mem at PC 0x%" PRIaddr "\n", epc_addr);

        fflush(stdout);
        hit_watch = true;
      }
    }
    if (!tag_bus.store_tag(mem_paddr, new_tag)) {
        printf("failed to store MR tag @ 0x%" PRIaddr " (0x%" PRIaddr ")\n", mem_addr, mem_paddr);

      fflush(stdout);
      // might as well halt
      hit_watch = true;
    }
  }
  if (has_pending_CSR && res->csrResult) {
    tag_t new_tag = ms_cache.to_tag(ms_cache.canonize(*res->csr));
    for(std::vector<address_t>::iterator it = watch_csrs.begin(); it != watch_csrs.end(); ++it) {
      if(pending_CSR == *it && csr_tags[pending_CSR] != new_tag){
        printf("Watch tag CSR\n");
        fflush(stdout);
        hit_watch = true;
      }
    }
    csr_tags[pending_CSR] = new_tag;
  }

  if (rule_cache) {
    results_t res_copy = {
      .pc = (meta_set_t *)ms_cache.canonize(*res->pc),
      .rd = (meta_set_t *)ms_cache.canonize(*res->rd),
      .csr = (meta_set_t *)ms_cache.canonize(*res->csr),
      .pcResult = res->pcResult,
      .rdResult = res->rdResult,
      .csrResult = res->csrResult
    };

    if (ctx->cached && !rule_cache->allow(ops, res)) {
      rule_cache->install_rule(ops, &res_copy);
    }
  }
  return hit_watch;
}

void rv32_validator_t::set_pc_watch(bool watching){
  watch_pc = watching;
}
void rv32_validator_t::set_reg_watch(address_t addr){
  watch_regs.push_back(addr);
}
void rv32_validator_t::set_csr_watch(address_t addr){
  watch_csrs.push_back(addr);
}
void rv32_validator_t::set_mem_watch(address_t addr){
  watch_addrs.push_back(addr);
}

void rv32_validator_t::prepare_eval(address_t pc, insn_bits_t insn) {
  tag_t ci_tag;
  address_t pc_paddr = addr_fixer(pc);;
  address_t mem_paddr;

  int32_t flags;

  failed = false;
  
  memset(ctx, 0, sizeof(*ctx));
  memset(ops, 0, sizeof(*ops));
  ctx->cached=true;

  if(res->pcResult){
    memset(res->pc, 0, sizeof(meta_set_t));
    res->pcResult = false;
  }

  if(res->rdResult){
    memset(res->rd, 0, sizeof(meta_set_t));
    res->rdResult = false;
  }

  if(res->csrResult){
    memset(res->csr, 0, sizeof(meta_set_t));
    res->csrResult = false;
  }

  decoded_instruction_t inst = decode(insn, ADDRESS_T_SIZE*8);
  if (!inst) {
    printf("Couldn't decode instruction at 0x%" PRIaddr " (0x%" PRIaddr "): 0x%08x   %s\n", pc, pc_paddr, insn, inst.name.c_str());
  }
  pending_RD = inst.rd.getOrElse(-1);

  if (inst.rs1.exists) ops->op1 = ms_cache[ireg_tags[inst.rs1]];
  if (inst.flags.has_csr_load || inst.flags.has_csr_store) ops->op2 = ms_cache[csr_tags[inst.imm]];
  if (inst.rs2.exists) ops->op2 = ms_cache[ireg_tags[inst.rs2]];
  if (inst.rs3.exists) ops->op3 = ms_cache[ireg_tags[inst.rs3]];
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
    mem_paddr = addr_fixer(mem_addr);
    ctx->bad_addr = mem_addr;
    tag_t mtag;
    if (!tag_bus.load_tag(mem_paddr, mtag)) {
      printf("failed to load MR tag -- pc: 0x%" PRIaddr " (0x%" PRIaddr ") addr: 0x%" PRIaddr " (0x%" PRIaddr ")\n", pc, pc_paddr, mem_addr, mem_paddr);
    } else {
      ops->mem = ms_cache[mtag];
      if (!ops->mem) {
        printf("Error: TMT miss for memory (0x%" PRIaddr " (0x%" PRIaddr ")) at instruction 0x%" PRIaddr ". TMT misses are fatal.\n", mem_addr, mem_paddr, pc);
        exit(1);
      }
    }
  }

  if (!tag_bus.load_insn_tag(pc_paddr, ci_tag)) {
    printf("failed to load CI tag for PC 0x%" PRIaddr " (0x%" PRIaddr ")\n", pc, pc_paddr);
  }
  ctx->epc = pc;
  ops->ci = ms_cache[ci_tag];
  ops->pc = ms_cache[pc_tag];
}

void rv32_validator_t::complete_eval() {}

void rv32_validator_t::config_rule_cache(const std::string& rule_cache_name, int capacity) {
  printf("%s rule cache with capacity %d!\n", rule_cache_name.c_str(), capacity);
  for (auto s : rule_cache_name)
    s = tolower(s);
  if (rule_cache_name == "ideal") {
    rule_cache = new ideal_rule_cache_t();
  } else if (rule_cache_name == "finite") {
    rule_cache = new finite_rule_cache_t(capacity);
  } else if (rule_cache_name == "dmhc") {
    rule_cache = new dmhc_rule_cache_t(capacity, DMHC_RULE_CACHE_IWIDTH, DMHC_RULE_CACHE_OWIDTH, DMHC_RULE_CACHE_K, DMHC_RULE_CACHE_NO_EVICT);
  } else if (rule_cache_name.size() != 0) {
    throw configuration_exception_t("Invalid rule cache name");
  }
}

void rv32_validator_t::rule_cache_stats() {
  if (rule_cache) {
    fprintf(stderr, "rule cache: hits %ld misses %ld total %ld\n",
	rule_cache_hits, rule_cache_misses, rule_cache_hits + rule_cache_misses);
    double hit_rate = (double)rule_cache_hits / (rule_cache_hits + rule_cache_misses);
    fprintf(stderr, "rule cache hit rate was %f%%!\n", hit_rate * 100);
  }
};

} // namespace policy_engine