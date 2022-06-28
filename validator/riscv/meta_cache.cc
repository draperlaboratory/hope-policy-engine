#include <functional>
#include <iostream>
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

meta_set_t& meta_set_cache_t::canonize(const meta_set_t& ts) {
  for (meta_set_t& ms : canon)
    if (ms == ts)
      return ms;
  canon.push_back(ts);

  m2t[&canon.back()] = reinterpret_cast<tag_t>(&canon.back());
  t2m[m2t[&canon.back()]] = &canon.back();

  return canon.back();
}

meta_set_t& meta_set_cache_t::canonize(const metadata_t& md) {
  meta_set_t ms{0};
  for (const meta_t& e : md)
    ms_bit_add(&ms, e);
  return canonize(ms);
}

meta_set_t* meta_set_cache_t::operator [](tag_t tag) const {
  if (tag == 0)
    return nullptr;
  return t2m.at(tag);
}

tag_t meta_set_cache_t::to_tag(meta_set_t* msp) const {
  if (m2t.find(msp) == m2t.end())
    return 0;
  return m2t.at(msp);
}

}