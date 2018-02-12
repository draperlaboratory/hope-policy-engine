#ifndef META_CACHE_H
#define META_CACHE_H

#include <unordered_map>
#include "policy_meta_set.h"

class meta_set_cache_t {
  public:
  struct meta_set_hasher_t {
    std::size_t operator()(const meta_set_t &k) const {
//      return std::hash<std::bitset<CACHE_KEY_BITS>>()(k.bits);
      size_t hash = 0;
      for (int i = 0; i < META_SET_WORDS; i++)
	hash += k.tags[i];
      return hash;
    }
  };
  struct meta_set_equal_t {
    bool operator()(meta_set_t const &l, meta_set_t const &r) const {
//      return memcmp(&l.tags, &r.tags, sizeof(l)) == 0;
      for (int i = 0; i < META_SET_WORDS; i++)
	if (l.tags[i] != r.tags[i])
	  return false;
      return true;
    }
  };
  private:
  std::unordered_map<meta_set_t, meta_set_t *, meta_set_hasher_t, meta_set_equal_t> map;
  public:
  meta_set_t *canonize(meta_set_t const &ts) {
    if (map.find(ts) == map.end()) {
      meta_set_t *ms = new meta_set_t();
      *ms = ts;
      map[ts] = ms;
    }
    return map[ts];
  }
};

#endif
