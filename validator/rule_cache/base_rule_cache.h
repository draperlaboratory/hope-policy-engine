#ifndef __BASE_RULE_CACHE_H__
#define __BASE_RULE_CACHE_H__

#include "riscv_isa.h"

namespace policy_engine {

class rule_cache_t {
public:
  virtual void flush() = 0;
  virtual void install_rule(const operands_t& ops, const results_t& res) = 0;
  virtual bool allow(const operands_t& ops, results_t& res) = 0;
};

} // namespace policy_engine

#endif// __BASE_RULE_CACHE_H__
