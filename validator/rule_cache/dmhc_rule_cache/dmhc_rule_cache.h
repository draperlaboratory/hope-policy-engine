#ifndef __DMHC_RULE_CACHE_H__
#define __DMHC_RULE_CACHE_H__

#include <functional>
#include "base_rule_cache.h"
#include "dmhc.h"
#include "meta_cache.h"
#include "riscv_isa.h"

//#define DMHC_DEBUG 1

#ifndef DMHC_RULE_CACHE_IWIDTH
#define DMHC_RULE_CACHE_IWIDTH 32
#endif

#ifndef DMHC_RULE_CACHE_OWIDTH
#define DMHC_RULE_CACHE_OWIDTH 32
#endif

#ifndef DMHC_RULE_CACHE_OWIDTH
#define DMHC_RULE_CACHE_OWIDTH 32
#endif

#ifndef DMHC_RULE_CACHE_K
#define DMHC_RULE_CACHE_K 4
#endif

#ifndef DMHC_RULE_CACHE_NO_EVICT
#define DMHC_RULE_CACHE_NO_EVICT 0
#endif

namespace policy_engine {

class dmhc_rule_cache_t : public rule_cache_t {

public:
  dmhc_rule_cache_t(int capacity, int iwidth, int owidth, int k, bool no_evict);
  ~dmhc_rule_cache_t() {}

  void install_rule(const operands_t& ops, const results_t& res); //Not used
  bool allow(const operands_t& ops, results_t& res);
  void flush();

private:
  operands_t ops_copy;
  results_t res_copy;
  bool consider[OPS_LEN];
  dmhc_t* the_rule_cache;
};

} // namespace policy_engine

#endif// __DMHC_RULE_CACHE_H__