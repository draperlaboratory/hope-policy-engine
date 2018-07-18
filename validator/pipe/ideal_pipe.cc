#include "ideal_pipe.h"

ideal_pipe_t::ideal_pipe_t() {
}

ideal_pipe_t::~ideal_pipe_t() {
}

void ideal_pipe_t::install_rule(operands_t *ops, results_t *res) {
  pump_table.insert(std::make_pair(*ops, *res));
}

bool ideal_pipe_t::allow(operands_t *ops, results_t *res) {
  auto entries = pump_table.find(*ops);
  if (!(entries == pump_table.end())) {
    res->pc = entries->second.pc;
    res->rd = entries->second.rd;
    res->csr = entries->second.csr;
    res->pcResult = entries->second.pcResult;
    res->rdResult = entries->second.rdResult;
    res->csrResult = entries->second.csrResult;
  }
  return !(entries == pump_table.end());
}
