#include <cstring>
#include "finite_rule_cache.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include "riscv_isa.h"

namespace policy_engine {

void finite_rule_cache_t::install_rule(const operands_t& ops, const results_t& res) {
  if (cache_full) {
    auto existing_entry = rule_cache_table.find(entries[next_entry]);
    if (existing_entry == rule_cache_table.end()) {
      printf("Internal error in rule cache - do not trust results.\n");
    } else {
      rule_cache_table.erase(existing_entry);
    }
  }
  rule_cache_table[ops] = res;
  entries[next_entry] = ops;

  next_entry++;
  if (next_entry >= capacity) {
    cache_full = true;
    next_entry = 0;
  }
}

bool finite_rule_cache_t::allow(const operands_t& ops, results_t& res) {
  auto existing_entry = rule_cache_table.find(ops);
  if (existing_entry != rule_cache_table.end()) {
    res.pc = existing_entry->second.pc;
    res.rd = existing_entry->second.rd;
    res.csr = existing_entry->second.csr;
    res.pcResult = existing_entry->second.pcResult;
    res.rdResult = existing_entry->second.rdResult;
    res.csrResult = existing_entry->second.csrResult;
  }
  return existing_entry != rule_cache_table.end();
}

} // namespace policy_engine
