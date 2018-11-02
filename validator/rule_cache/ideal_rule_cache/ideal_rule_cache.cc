#include "ideal_rule_cache.h"

ideal_rule_cache_t::ideal_rule_cache_t() {
  rule_cache_table.clear();
}

void ideal_rule_cache_t::flush() {
  rule_cache_table.clear();
}

ideal_rule_cache_t::~ideal_rule_cache_t() {
  flush();
}

void ideal_rule_cache_t::install_rule(operands_t *ops, results_t *res) {
  rule_cache_table.insert(std::make_pair(*ops, *res));
}

bool ideal_rule_cache_t::allow(operands_t *ops, results_t *res) {
  auto entries = rule_cache_table.find(*ops);
  if (!(entries == rule_cache_table.end())) {
    *res->pc = *entries->second.pc;
    *res->rd = *entries->second.rd;
    *res->csr = *entries->second.csr;
    res->pcResult = entries->second.pcResult;
    res->rdResult = entries->second.rdResult;
    res->csrResult = entries->second.csrResult;
  }
  return !(entries == rule_cache_table.end());
}
