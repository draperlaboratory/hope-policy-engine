#include "finite_rule_cache.h"
#include "../rule_analysis/rule_analysis.h"
#include <string.h>

finite_rule_cache_t::finite_rule_cache_t(int capacity) : ideal_rule_cache_t() {
  this->capacity = capacity;
  entries = (operands_t*) calloc(capacity, sizeof(operands_t));
  next_entry = 0;
  cache_full = false;
}

finite_rule_cache_t::~finite_rule_cache_t() {
  delete entries;
}

void finite_rule_cache_t::install_rule(operands_t *ops, results_t *res) {

  // Added for rule analysis
  rule_inserted(std::make_pair(*ops, *res));
  
  if (cache_full) {
    auto existing_entry = rule_cache_table.find(entries[next_entry]) ;
    if ((existing_entry==rule_cache_table.end())) {
      printf("Internal error in rule cache - do not trust results.\n");
    } else {
      // Trigger eviction logic in rule_analysis.
      // Call rule_evicted() on each pair of this and rule and other rules
      // still in the cache.
      auto rule = *existing_entry;
      operands_t evicted_ops = rule.first;
      
      //for (int i = 0; i < this->capacity; i++){
      //rule_evicted(&evicted_ops, &entries[i]);
      //}

      // New for just getting rule frequencies (ignore pairs)
      operands_t dummy_ops;
      memset(&dummy_ops, '\0', sizeof(dummy_ops));
      rule_evicted(&evicted_ops, &dummy_ops);

		   
      rule_cache_table.erase(existing_entry);
    }
  }

  auto new_entry = rule_cache_table.insert(std::make_pair(*ops, *res));
  memcpy(entries+next_entry, ops, sizeof(operands_t));

  next_entry++;
  if(next_entry>=capacity) {
    cache_full = true;
    next_entry=0;
  }
}

bool finite_rule_cache_t::allow(operands_t *ops, results_t *res) {
  
  auto existing_entry = rule_cache_table.find(*ops);
  
  if (!(existing_entry == rule_cache_table.end())) {
    *res->pc = *existing_entry->second.pc;
    *res->rd = *existing_entry->second.rd;
    *res->csr = *existing_entry->second.csr;
    res->pcResult = existing_entry->second.pcResult;
    res->rdResult = existing_entry->second.rdResult;
    res->csrResult = existing_entry->second.csrResult;
  }

  return !(existing_entry == rule_cache_table.end());
}
