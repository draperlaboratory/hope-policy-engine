#ifndef __BASE_RULE_CACHE_H__
#define __BASE_RULE_CACHE_H__

#include "riscv_isa.h"

namespace policy_engine {

struct compare_ops {
  bool operator()(const operands_t& a, const operands_t& b) const {
    return (a.op1 == b.op1 &&
            a.op2 == b.op2 &&
            a.op3 == b.op3 &&
            a.mem == b.mem &&
            a.pc == b.pc &&
            a.ci == b.ci);
  }
};

class rule_cache_t {
public:
  virtual void flush() = 0;
  virtual void install_rule(const operands_t* ops, results_t* res) = 0;
  virtual bool allow(const operands_t* ops, results_t* res) = 0;
};

} // namespace policy_engine

namespace std
{

template <>
struct hash<policy_engine::operands_t>
{
  size_t operator()(const policy_engine::operands_t& ops) const
  {
    // XOR all meta_set_t pointers (operands) together.
    // Shift pointers slightly so that two identical
    // tags don't cancel out to 0.
    size_t hash = (size_t) ops.pc;
    hash ^= (size_t) ops.ci  << 1;
    hash ^= (size_t) ops.op1 << 2;
    hash ^= (size_t) ops.op2 << 3;
    hash ^= (size_t) ops.op3 << 4;
    hash ^= (size_t) ops.mem << 5;
    return hash;
  }
};

} // namespace std

#endif// __BASE_RULE_CACHE_H__
