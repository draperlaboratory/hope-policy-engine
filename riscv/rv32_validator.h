#ifndef RV32_VALIDATOR_H
#define RV32_VALIDATOR_H

#include <stdio.h>

#include "validator.h"
#include "policy_eval.h"
#include "tag_utils.h"
#include "meta_set_factory.h"

#define REG_SP 2
class rv32_validator_t : public abstract_renode_validator_t {
  context_t *ctx;
  operands_t *ops;
  results_t *res;
  tag_bus_t tag_bus;
  tag_file_t<32> ireg_tags;
  tag_file_t<0x1000> csr_tags;
  tag_t pc_tag;
  uint32_t pending_RD;
  bool has_pending_RD;
  meta_set_t *t_to_m(tag_t t) { return (meta_set_t *)t; }
  tag_t m_to_t(meta_set_t *ms) { return (tag_t)ms; }
  meta_set_cache_t ms_cache;
  meta_set_factory_t ms_factory;
  public:
  rv32_validator_t(RegisterReader_t rr, MemoryReader_t mr);
/*
 : reg_reader(rr), mem_reader(mr) {
    ctx = (context_t *)malloc(sizeof(context_t));
    ops = (operands_t *)malloc(sizeof(operands_t));
    res = (results_t *)malloc(sizeof(results_t));
*/
//    alloc_eval_params(&context, &ops, &res); // TODO: error handling
//    printf("context = 0x%p, ops = 0x%p, res = 0x%p\n", ctx, ops, res);
//  }
  virtual ~rv32_validator_t() {
    free(ctx);
    free(ops);
    free(res);
  }
  bool validate(address_t pc, insn_bits_t insn);
  void commit();
  
  void prepare_eval(address_t pc, insn_bits_t insn);
  void complete_eval();
};

#endif
