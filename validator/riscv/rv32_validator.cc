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

#include "soc_tag_configuration.h"
#include "rv32_validator.h"
#include "validator_exception.h"

#include "policy_utils.h"

using namespace policy_engine;

static const char *tag_name(meta_set_t const *tag) {
  static char tag_name[1024];
  meta_set_to_string(tag, tag_name, sizeof(tag_name));
  return tag_name;
}

rv32_validator_base_t::rv32_validator_base_t(meta_set_cache_t *ms_cache,
					     meta_set_factory_t *ms_factory,
					     RegisterReader_t rr)
  : tag_based_validator_t(ms_cache, ms_factory, rr) {
  
  ctx = (context_t *)malloc(sizeof(context_t));
  ops = (operands_t *)malloc(sizeof(operands_t));
  res = (results_t *)malloc(sizeof(results_t));
  res->pc = (meta_set_t *)malloc(sizeof(meta_set_t));
  res->rd = (meta_set_t *)malloc(sizeof(meta_set_t));
  res->csr = (meta_set_t *)malloc(sizeof(meta_set_t));
  memset(res->pc, 0, sizeof(meta_set_t));
  memset(res->rd, 0, sizeof(meta_set_t));
  memset(res->csr, 0, sizeof(meta_set_t));
  res->pcResult = false;
  res->rdResult = false;
  res->csrResult = false;
}

extern std::string render_metadata(metadata_t const *metadata);

void rv32_validator_base_t::apply_metadata(metadata_memory_map_t *md_map) {
  for (auto &e: *md_map) {
    for (address_t start = e.first.start; start < e.first.end; start += 4) {
//      std::string s = render_metadata(e.second);
//      printf("0x%08x: %s\n", start, s.c_str());
      if (!tag_bus.store_tag(start, m_to_t(ms_cache->canonize(e.second)))) {
	throw configuration_exception_t("unable to apply metadata");
      }
    }
  }
}

void rv32_validator_base_t::setup_validation() {
  memset(ctx, 0, sizeof(*ctx));
  memset(ops, 0, sizeof(*ops));

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

rv32_validator_t::rv32_validator_t(meta_set_cache_t *ms_cache,
				   meta_set_factory_t *ms_factory,
				   soc_tag_configuration_t *config,
				   RegisterReader_t rr) :
  rv32_validator_base_t(ms_cache, ms_factory, rr) {
  // true causes initial clear of results
  res->pcResult = true;
  res->rdResult = true;
  res->csrResult = true;

  meta_set_t const *ms;

  ms = ms_factory->get_meta_set("requires.dover.riscv.Mach.Reg");
  ireg_tags.reset(m_to_t(ms));
  ms = ms_factory->get_meta_set("requires.dover.riscv.Mach.RegZero");
  ireg_tags[0] = m_to_t(ms);
  ms = ms_factory->get_meta_set("requires.dover.SOC.CSR.Default");
  csr_tags.reset(m_to_t(ms));
  ms = ms_factory->get_meta_set("requires.dover.riscv.Mach.PC");
  pc_tag = m_to_t(ms);

  config->apply(&tag_bus, this);

void rv32_validator_t::handle_violation(context_t *ctx, operands_t *ops){
  failed = true;

  memcpy(&failed_ctx, ctx, sizeof(context_t));
  memcpy(&failed_ops, ops, sizeof(operands_t));
}

void rv32_validator_t::handle_violation(context_t *ctx, operands_t *ops){
  failed = true;

  memcpy(&failed_ctx, ctx, sizeof(context_t));
  memcpy(&failed_ops, ops, sizeof(operands_t));
}

bool rv32_validator_t::validate(address_t pc, insn_bits_t insn) {
  int policy_result = POLICY_EXP_FAILURE;

  setup_validation();
  
  prepare_eval(pc, insn);
  
  policy_result = eval_policy(ctx, ops, res);
//  policy_result = POLICY_SUCCESS;

  if (policy_result == POLICY_SUCCESS) {
    complete_eval();
  }

  if (policy_result != POLICY_SUCCESS)
    handle_violation(ctx, ops);

  return policy_result == POLICY_SUCCESS;
}

bool rv32_validator_t::commit() {
  bool hit_watch = false;
  if (res->pcResult) {
    tag_t new_tag = m_to_t(ms_cache->canonize(*res->pc));
    if(watch_pc && pc_tag != new_tag){
      printf("Watch tag pc");
      fflush(stdout);
      hit_watch = true;
    }
    pc_tag = new_tag;
  }
  if (has_pending_RD && res->rdResult) {
    tag_t new_tag = m_to_t(ms_cache->canonize(*res->rd));
    for(std::vector<address_t>::iterator it = watch_regs.begin(); it != watch_regs.end(); ++it) {
      if(pending_RD == *it && ireg_tags[pending_RD] != new_tag){
        printf("Watch tag reg");
        fflush(stdout);
        hit_watch = true;
      }
    }
    ireg_tags[pending_RD] = new_tag;
  }
  if (has_pending_mem && res->rdResult) {
    tag_t new_tag = m_to_t(ms_cache->canonize(*res->rd));
    tag_t old_tag;
    if (!tag_bus.load_tag(mem_addr, old_tag)) {
      printf("failed to load MR tag\n");
      fflush(stdout);
      // might as well halt
      hit_watch = true;
    }
//    printf("  committing tag '%s' to 0x%08x\n", tag_name(res->rd), mem_addr);
    for(std::vector<address_t>::iterator it = watch_addrs.begin(); it != watch_addrs.end(); ++it) {
      if(pending_RD == *it && old_tag != new_tag){
        printf("Watch tag mem");
        fflush(stdout);
        hit_watch = true;
      }
    }
    if (!tag_bus.store_tag(mem_addr, new_tag)) {
      printf("failed to store MR tag\n");
      fflush(stdout);
     // might as well halt
      hit_watch = true;
    }
  }
  if (has_pending_CSR && res->csrResult) {
    tag_t new_tag = m_to_t(ms_cache->canonize(*res->csr));
    for(std::vector<address_t>::iterator it = watch_csrs.begin(); it != watch_csrs.end(); ++it) {
      if(pending_CSR == *it && csr_tags[pending_CSR] != new_tag){
        printf("Watch tag CSR");
        fflush(stdout);
        hit_watch = true;
      }
    }
    csr_tags[pending_CSR] = new_tag;
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
  uint32_t rs1, rs2, rs3;
  int32_t imm;
  const char *name;
  address_t offset;
  tag_t ci_tag;
//  char tag_name[1024];

  int32_t flags;

  failed = false;
  
  memset(ctx, 0, sizeof(*ctx));
  memset(ops, 0, sizeof(*ops));

  if(res->pcResult){
    memset(res->pc, 0, sizeof(meta_set_t));
    res->pcResult = false;
  }

  if(res->rdResult){
    memset(res->rd, 0, sizeof(meta_set_t));
    res->rdResult = false;
  }
  
  flags = decode(insn, &rs1, &rs2, &rs3, &pending_RD, &imm, &name);
//  printf("0x%x: 0x%08x   %s\n", pc, insn, name);

  if (flags & HAS_RS1) ops->op1 = t_to_m(ireg_tags[rs1]);
  if (flags & HAS_RS2) ops->op2 = t_to_m(ireg_tags[rs2]);
  if (flags & HAS_RS3) ops->op3 = t_to_m(ireg_tags[rs3]);
  has_pending_CSR = (flags & HAS_CSR_STORE) != 0;
  has_pending_RD = (flags & HAS_RD) != 0;
  has_pending_mem = (flags & HAS_STORE) != 0;
  pending_CSR = rs3;
  if (flags & (HAS_LOAD | HAS_STORE)) {
//    address_t maddr = reg_reader(rs1);
    mem_addr = reg_reader(rs1);
    if (flags & HAS_IMM)
      mem_addr += imm;
    ctx->bad_addr = mem_addr;
//    printf("  mem_addr = 0x%08x\n", mem_addr);
    tag_t mtag;
    if (!tag_bus.load_tag(mem_addr, mtag)) {
      printf("failed to load MR tag\n");
    } else {
      ops->mem = t_to_m(mtag);
//      printf("  mr tag = '%s'\n", tag_name(ops->mem));
//      printf("mr tag = 0x%p\n", ops->mem);
    }
  }

  if (!tag_bus.load_tag(pc, ci_tag))
    printf("failed to load CI tag\n");
//    printf("ci_tag: 0x%lx\n", ci_tag);
  ctx->epc = pc;
//  ctx->bad_addr = 0;
//  ctx->cached = false;

//  temp_ci_tag = *t_to_m(ci_tag);
//  ops->ci = &temp_ci_tag;
  ops->ci = t_to_m(ci_tag);
//  meta_set_to_string(ops->ci, tag_name, sizeof(tag_name));
//  printf("ci tag name before merge: %s\n", tag_name);
  ops->pc = t_to_m(pc_tag);
}

void rv32_validator_t::complete_eval() {
//  printf("complete eval\n");
}
