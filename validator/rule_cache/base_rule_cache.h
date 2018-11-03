#ifndef __BASE_RULE_CACHE_H__
#define __BASE_RULE_CACHE_H__

#include "riscv_isa.h"
//#include <inttypes.h>

struct compare_ops {
  bool operator()(const operands_t &a, const operands_t &b) const {
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
  virtual void install_rule(operands_t *ops, results_t *res) = 0;
  virtual bool allow(operands_t *ops, results_t *res) = 0;
};

namespace std
{
  inline void hash_combine(std::size_t& seed) { }

  template <typename T, typename... Rest>
      inline void hash_combine(std::size_t& seed, const T& v, Rest... rest) {
    std::hash<T> hasher;
    seed ^= hasher(v) + 0x9e3779b9 + (seed<<6) + (seed>>2);
    hash_combine(seed, rest...);
  }

  template <>
      struct hash<operands_t>
  {
    size_t operator()(const operands_t& ops) const
    {
      size_t res = 0;
      hash_combine(res, *ops.pc->tags, *ops.ci->tags);
      if (ops.op1)
        hash_combine(res, *ops.op1->tags);
      if (ops.op2)
        hash_combine(res, *ops.op2->tags);
      if (ops.op3)
        hash_combine(res, *ops.op3->tags);
      if (ops.mem)
        hash_combine(res, *ops.mem->tags);

      return res;
    }
  };

}

#endif// __BASE_RULE_CACHE_H__
