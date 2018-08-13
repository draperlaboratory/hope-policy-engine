#ifndef __IDEAL_RULE_CACHE_H__
#define __IDEAL_RULE_CACHE_H__

#include <unordered_map>
#include <functional>
#include "base_rule_cache.h"

class ideal_rule_cache_t : public rule_cache_t
{

public:

  ideal_rule_cache_t();
  ~ideal_rule_cache_t();

  void install_rule(operands_t *ops, results_t *res);
  bool allow(operands_t *ops, results_t *res);
  void flush();

protected:
  std::unordered_map<operands_t, results_t, std::hash<operands_t>, compare_ops> rule_cache_table;
};

#endif// __IDEAL_RULE_CACHE_H__
