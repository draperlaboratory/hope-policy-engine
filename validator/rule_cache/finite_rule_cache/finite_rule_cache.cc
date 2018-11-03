#include "finite_rule_cache.h"

finite_rule_cache_t::finite_rule_cache_t(int capacity) : ideal_rule_cache_t() {
  this->capacity = capacity;
  entries = (operands_t**) malloc(sizeof(operands_t*) *capacity);
  next_entry = 0;
  entry_used = (bool*) malloc(sizeof(bool) *capacity);
  for (int i=0;i<capacity;i++){
    entry_used[i]=false;
  }
}

finite_rule_cache_t::~finite_rule_cache_t() {
  for (int i=0;i<capacity;i++) {
    delete entries[i];
  }
  delete entries;
  delete entry_used;
}

void finite_rule_cache_t::install_rule(operands_t *ops, results_t *res) {
  if (entry_used[next_entry]==true) {
    auto existing_entry = rule_cache_table.find(*entries[next_entry]);
    if ((existing_entry==rule_cache_table.end())) {
      printf("failed to find operand\n");
    } else {
      rule_cache_table.erase(existing_entry);
    }
  }
  rule_cache_table.insert(std::make_pair(*ops, *res));
  entries[next_entry]=ops;
  entry_used[next_entry]=true;

  // circular buffer of things to remove
  next_entry++;
  if (next_entry>=capacity)
    next_entry=0;
}

bool finite_rule_cache_t::allow(operands_t *ops, results_t *res) {
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
