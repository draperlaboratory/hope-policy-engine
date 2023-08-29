#include <iostream>
#include "indexed_meta_cache.h"
#include "policy_meta_set.h"
#include "tag_types.h"

namespace policy_engine {

tag_t indexed_meta_set_cache_t::canonize(const meta_set_t& ts) {
  for (int i = 0; i < meta_sets.size(); i++)
    if (meta_sets[i] == ts)
      return i + 1;
  if (meta_sets.size() == meta_sets.capacity())
    std::cout << "reallocating meta set vector to increase capacity may invalidate tags that are pointers" << std::endl;
  meta_sets.push_back(ts);
  return meta_sets.size();
}

} // namespace policy_engine