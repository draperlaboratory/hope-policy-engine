#include "meta_cache.h"
#include "metadata.h"
#include "policy_meta_set.h"
#include "tag_utils.h"

namespace policy_engine {

bool operator ==(const meta_set_t& lhs, const meta_set_t& rhs) {
  for (int i = 0; i < META_SET_WORDS; i++)
    if (lhs.tags[i] != rhs.tags[i])
      return false;
  return true;
}

bool operator !=(const meta_set_t& lhs, const meta_set_t& rhs) { return !(lhs == rhs); }

tag_t meta_set_cache_t::canonize(const meta_set_t& ts) {
  for (int i = 0; i < meta_sets.size(); i++)
    if (meta_sets[i] == ts)
      return i;
  meta_sets.push_back(ts);
  return meta_sets.size() - 1;
}

tag_t meta_set_cache_t::canonize(const metadata_t& md) {
  meta_set_t ms{0};
  for (const meta_t& e : md)
    ms_bit_add(&ms, e);
  return canonize(ms);
}

} // namespace policy_engine