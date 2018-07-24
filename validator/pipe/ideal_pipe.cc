#include "ideal_pipe.h"

ideal_pipe_t::ideal_pipe_t() {
  pipe_table.clear();
}

ideal_pipe_t::~ideal_pipe_t() {
}

void ideal_pipe_t::install_rule(operands_t *ops, results_t *res) {
  operands_t *ops_copy = new operands_t();
  ops_copy->pc = new meta_set_t{*ops->pc};
  ops_copy->ci = new meta_set_t{*ops->ci};
  if (ops->op1)
    ops_copy->op1 = new meta_set_t{*ops->op1};
  if (ops->op2)
    ops_copy->op2 = new meta_set_t{*ops->op2};
  if (ops->op3)
    ops_copy->op3 = new meta_set_t{*ops->op3};
  if (ops->mem)
    ops_copy->mem = new meta_set_t{*ops->mem};
  results_t *res_copy = new results_t{new meta_set_t{*res->pc}, 
  new meta_set_t{*res->rd}, new meta_set_t{*res->csr}, res->pcResult, 
  res->rdResult, res->csrResult};
  pipe_table.insert(std::make_pair(*ops_copy, *res_copy));
  delete ops_copy;
  delete res_copy;
}

bool ideal_pipe_t::allow(operands_t *ops, results_t *res) {
  auto entries = pipe_table.find(*ops);
  if (!(entries == pipe_table.end())) {
    res->pc = entries->second.pc;
    res->rd = entries->second.rd;
    res->csr = entries->second.csr;
    res->pcResult = entries->second.pcResult;
    res->rdResult = entries->second.rdResult;
    res->csrResult = entries->second.csrResult;
  }
  return !(entries == pipe_table.end());
}
