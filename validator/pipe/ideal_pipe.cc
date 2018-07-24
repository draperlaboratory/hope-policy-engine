#include "ideal_pipe.h"
#include <inttypes.h>

ideal_pipe_t::ideal_pipe_t() {
  pipe_table.clear();
  instruction_count = 0;
}

ideal_pipe_t::~ideal_pipe_t() {
  for (std::map<pipe_operands_t, results_t>::iterator itr = pipe_table.begin();
    itr!=pipe_table.end(); itr++) {
    delete &itr->first;
    delete &itr->second;
    pipe_table.erase(itr);
  }
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
  pipe_operands_t *pipe_ops = new pipe_operands_t();
  pipe_ops->ops = ops_copy;
  pipe_table.insert(std::make_pair(*pipe_ops, *res));
}

bool ideal_pipe_t::allow(operands_t *ops, results_t *res) {
  printf("Entered Allow. IC: %d\n", instruction_count);
  pipe_operands_t *pipe_ops = new pipe_operands_t();
  pipe_ops->ops=ops;
  if (instruction_count == 71316) {
    printf("pc: %" PRIu32 ", ci: %" PRIu32, ops->pc->tags[0], ops->ci->tags[0]);
    if (ops->op1) printf(", op1: %" PRIu32, ops->op1->tags[0]);
    if (ops->op2) printf(", op2: %" PRIu32, ops->op2->tags[0]);
    if (ops->op3) printf(", op3: %" PRIu32, ops->op3->tags[0]);
    if (ops->mem) printf(", mem: %" PRIu32, ops->mem->tags[0]);
    printf("\n");
  }
  auto entries = pipe_table.find(*pipe_ops);
  if (!(entries == pipe_table.end())) {
    res->pc = entries->second.pc;
    res->rd = entries->second.rd;
    res->csr = entries->second.csr;
    res->pcResult = entries->second.pcResult;
    res->rdResult = entries->second.rdResult;
    res->csrResult = entries->second.csrResult;
  } 
  delete pipe_ops;
  instruction_count++;
  return !(entries == pipe_table.end());
}
