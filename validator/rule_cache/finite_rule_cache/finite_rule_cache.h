#ifndef __FINITE_RULE_CACHE_H__
#define __FINITE_RULE_CACHE_H__

#include <functional>
#include <unordered_map>
#include <vector>
#include "ideal_rule_cache.h"
#include "riscv_isa.h"

namespace policy_engine {

class finite_rule_cache_t : public ideal_rule_cache_t {
public:
  finite_rule_cache_t(int capacity) : ideal_rule_cache_t(), capacity(capacity), cache_full(false), entries(std::vector<operands_t>(capacity)), next_entry(0) {}
  ~finite_rule_cache_t() {}

  void install_rule(const operands_t& ops, const results_t& res);
  bool allow(const operands_t& ops, results_t& res);

private:
  // the number of rules the cache can hold.
  int capacity;

  // We track whether the cache is full so that we know when to begin evicting
  // elements upon insertion.  The cache is ideal in the sense that the oldest
  // element is always the one evicted.
  bool cache_full;

  // The keys of the elements currently in the cache.
  // The oldest entry in the cache is:
  //   - entries[0] if cache_full is false
  //   - entries[(next_entry+1)%capacity] if cache_full is true
  std::vector<operands_t> entries;

  // the next location into which we will insert an entry in the "entries"
  // array.
  int next_entry;
};

} // namespace policy_engine

#endif// __FINITE_RULE_CACHE_H__
