#ifndef METADATA_INDEX_MAP_H
#define METADATA_INDEX_MAP_H

#include <algorithm>
#include <cstdint>
#include <iterator>
#include <map>
#include <memory>
#include <utility>
#include <vector>
#include "metadata_memory_map.h"

#include <iostream>

namespace policy_engine {

/**
 * Container for mapping memory/registers to indexes in a
 * collection of metadata values.
 * If provided metadata_values and metadata_map on initialization,
 * updates metadata_values with any new metadata found in metadata_map.
 */

// XXX: K is always the key type of M, but two generics are needed to support
// metadata_memory_map_t as M
template <class M, class K>
class metadata_index_map_t {
private:
  std::map<K, int> index_map;

public:
  using iterator = typename decltype(index_map)::iterator;

  std::vector<std::shared_ptr<metadata_t>> metadata;

  metadata_index_map_t() {}

  metadata_index_map_t(const M& metadata_map) {
    for (const auto& [ key, value ] : metadata_map) {
      const auto it = std::find_if(metadata.begin(), metadata.end(), [&value](std::shared_ptr<const metadata_t> v){ return *v == *value; });
      index_map[key] = std::distance(metadata.begin(), it);
      if (it == metadata.end())
        metadata.push_back(value);
    }
  }

  iterator begin() { return index_map.begin(); }
  iterator end() { return index_map.end(); }
  size_t size() { return index_map.size(); }

  void insert(const std::pair<K, int>& p) { index_map.insert(p); }
  int& at(const K& k) { return index_map.at(k); }
  const int& at(const K& k) const { return index_map.at(k); }
  size_t erase(const K& k) { return index_map.erase(k); }
};

} // namespace policy_engine

#endif
