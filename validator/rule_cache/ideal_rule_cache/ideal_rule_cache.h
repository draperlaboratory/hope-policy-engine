#ifndef __IDEAL_RULE_CACHE_H__
#define __IDEAL_RULE_CACHE_H__

#include <functional>
#include <unordered_map>
#include "base_rule_cache.h"
#include "riscv_isa.h"

namespace policy_engine {

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

} // namespace policy_engine

#endif// __IDEAL_RULE_CACHE_H__
