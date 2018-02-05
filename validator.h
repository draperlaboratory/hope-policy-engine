#ifndef VALIDATOR_H
#define VALIDATOR_H

#include <stdlib.h>
#include <stdint.h>
#include <string.h>

typedef uint32_t address_t;
typedef uint32_t insn_bits_t;

class abstract_validator_t {
  public:
  virtual ~abstract_validator_t() { }
  virtual bool validate(address_t pc, insn_bits_t insn) = 0;
  virtual void commit() = 0;
};

//  virtual get_mvec();
//  virtual install_rule();
class always_ok_validator_t final : public abstract_validator_t {
  public:
  bool validate(address_t pc, insn_bits_t insn) { return true; }
  void commit() { }
};

#include "renode_validator.h"
class abstract_renode_validator_t : abstract_validator_t {
  RegisterReader_t reg_reader;
  MemoryReader_t mem_reader;
  public:
  abstract_renode_validator_t(RegisterReader_t rr, MemoryReader_t mr) : reg_reader(rr), mem_reader(mr) { }
  virtual ~abstract_renode_validator_t() { }
  virtual bool validate(address_t pc, insn_bits_t insn) = 0;
  virtual void commit() = 0;
};


// brings in ISA specific bits defining contexts, etc
#include "policy_eval.h"
#include <stdio.h>
class basic_renode_validator_t {
  RegisterReader_t reg_reader;
  MemoryReader_t mem_reader;
  context_t *ctx;
  operands_t *ops;
  results_t *res;
  public:
  basic_renode_validator_t(RegisterReader_t rr, MemoryReader_t mr) : reg_reader(rr), mem_reader(mr) {
    ctx = (context_t *)malloc(sizeof(context_t));
    ops = (operands_t *)malloc(sizeof(operands_t));
    res = (results_t *)malloc(sizeof(results_t));
//    alloc_eval_params(&context, &ops, &res); // TODO: error handling
//    printf("context = 0x%p, ops = 0x%p, res = 0x%p\n", ctx, ops, res);
  }
  virtual ~basic_renode_validator_t() {
    free(ctx);
    free(ops);
    free(res);
//    free_eval_params(&context, &ops, &res);
  }
  bool validate(address_t pc, insn_bits_t insn);
  void commit();
  
  void prepare_eval(address_t pc, insn_bits_t insn);
  void complete_eval();
};

#endif
