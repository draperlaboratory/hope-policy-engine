#ifndef METADATA_INDEX_MAP_H
#define METADATA_INDEX_MAP_H

#include <map>
#include "metadata_memory_map.h"

namespace policy_engine {

struct range_compare {
  bool operator() (const range_t &lhs, const range_t &rhs) const {
    return (lhs.start < rhs.start) ||
           (lhs.start == rhs.start && lhs.end < rhs.end);
  }
};

typedef std::map<range_t, uint32_t, range_compare> metadata_index_map_t;

} // namespace policy_engine

#endif
