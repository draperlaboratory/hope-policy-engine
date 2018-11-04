#ifndef __FINITE_RULE_CACHE_H__
#define __FINITE_RULE_CACHE_H__

#include <unordered_map>
#include <functional>
#include "ideal_rule_cache.h"

class finite_rule_cache_t : public ideal_rule_cache_t
{

public:

  finite_rule_cache_t(int capacity);
  ~finite_rule_cache_t();

  void install_rule(operands_t *ops, results_t *res);
  bool allow(operands_t *ops, results_t *res);

private:
  int capacity;
  operands_t **entries;
  bool *entry_used;
  int next_entry;
};

#endif// __FINITE_RULE_CACHE_H__
