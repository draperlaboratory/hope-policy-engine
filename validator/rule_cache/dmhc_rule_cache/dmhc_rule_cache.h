#include <functional>
#include "base_rule_cache.h"
#include "dmhc.h"

//#define DMHC_DEBUG 1

class dmhc_rule_cache_t : public rule_cache_t {

public:
  dmhc_rule_cache_t(int capacity, int iwidth, int owidth, int k, bool no_evict);
  ~dmhc_rule_cache_t();

  void install_rule(operands_t *ops, results_t *res); //Not used
  bool allow(operands_t *ops, results_t *res);
  void flush();

private:
  meta_set_t ops_copy[OPS_LEN];
  meta_set_t res_copy[RES_LEN];
  bool consider[OPS_LEN];
  dmhc_t *the_rule_cache;
};
