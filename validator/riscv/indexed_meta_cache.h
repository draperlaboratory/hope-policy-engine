#ifndef __INDEXED_META_CACHE_H__
#define __INDEXED_META_CACHE_H__

#include "meta_cache.h"
#include "metadata.h"
#include "policy_meta_set.h"
#include "tag_types.h"

namespace policy_engine {

class indexed_meta_set_cache_t : public meta_set_cache_t {
private:
  std::vector<meta_set_t> meta_sets;

public:
  indexed_meta_set_cache_t() { meta_sets.reserve(1024); }

  tag_t canonize(const meta_set_t& ts);
  using meta_set_cache_t::canonize;

  const meta_set_t& operator [](tag_t tag) const { return meta_sets.at(tag - 1); }
};

} // namespace policy_engine

#endif // __INDEXED_META_CACHE_H__