#ifndef METADATA_CACHE_H
#define METADATA_CACHE_H

#include <unordered_map>
#include "metadata.h"

class metadata_cache_t {
  private:
  std::unordered_map<metadata_t, metadata_t *, metadata_t::hasher_t, metadata_t::equal_t> map;
  public:
  metadata_t const *canonize(metadata_t const *md) {
    if (map.find(*md) == map.end()) {
      map[*md] = new metadata_t(*md);
    }
    return map[*md];
  }
};

#endif
