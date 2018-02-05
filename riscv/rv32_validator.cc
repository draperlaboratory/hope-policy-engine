#include "rv32_validator.h"


rv32_validator_t::rv32_validator_t(RegisterReader_t rr, MemoryReader_t mr) :
  abstract_renode_validator_t(rr, mr) {
  ctx = (context_t *)malloc(sizeof(context_t));
  ops = (operands_t *)malloc(sizeof(operands_t));
  res = (results_t *)malloc(sizeof(results_t));
}

bool rv32_validator_t::validate(address_t pc, insn_bits_t insn) {
  int policy_result = POLICY_EXP_FAILURE;
  
  prepare_eval(pc, insn);
  
  policy_result = eval_policy(ctx, ops, res);

  if (policy_result == POLICY_SUCCESS) {
    complete_eval();
  }

//  if (policy_result != POLICY_SUCCESS)
//    handle_violation(ctx, ops, res);

  return policy_result == POLICY_SUCCESS;
//  return true;
}

void rv32_validator_t::commit() {
}

void rv32_validator_t::prepare_eval(address_t pc, insn_bits_t insn) {
  memset(ctx, 0, sizeof(*ctx));
  memset(ops, 0, sizeof(*ops));
  memset(res, 0, sizeof(*res));
}

void rv32_validator_t::complete_eval() {
}
