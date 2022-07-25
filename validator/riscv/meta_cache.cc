#include <functional>
#include <stdexcept>
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
  for (const auto& [ tag, ms ] : tags)
    if (*ms == ts)
      return tag;
  canon.push_back(ts);
  tags[canon.size() - 1] = &canon.back();
  return canon.size() - 1;
}

tag_t meta_set_cache_t::canonize(const metadata_t& md) {
  meta_set_t ms{0};
  for (const meta_t& e : md)
    ms_bit_add(&ms, e);
  return canonize(ms);
}

} // namespace policy_engine