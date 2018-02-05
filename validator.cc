#include "validator.h"

bool basic_renode_validator_t::validate(address_t pc, insn_bits_t insn) {
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

void basic_renode_validator_t::commit() {
}

void basic_renode_validator_t::prepare_eval(address_t pc, insn_bits_t insn) {
  memset(ctx, 0, sizeof(*ctx));
  memset(ops, 0, sizeof(*ops));
  memset(res, 0, sizeof(*res));
}

void basic_renode_validator_t::complete_eval() {
}
