#ifndef RV32_VALIDATOR_H
#define RV32_VALIDATOR_H

#include <stdio.h>

#include "validator.h"
#include "policy_eval.h"

class rv32_validator_t : public abstract_renode_validator_t {
  context_t *ctx;
  operands_t *ops;
  results_t *res;
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
