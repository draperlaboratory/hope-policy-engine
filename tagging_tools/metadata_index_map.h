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
class metadata_index_map_t : private std::map<K, int> {
private:
  using parent = typename std::map<K, int>;

public:
  using iterator = typename parent::iterator;
  using const_iterator = typename parent::const_iterator;
  using size_type = typename parent::size_type;

  std::vector<const metadata_t*> metadata;

  metadata_index_map_t() {}

  metadata_index_map_t(const M& metadata_map, std::vector<const metadata_t*> initial_metadata) {
    metadata = std::vector(initial_metadata);
    for (const auto& [ key, value ] : metadata_map) {
      const auto it = std::find_if(metadata.begin(), metadata.end(), [&value](const metadata_t* v){ return *v == *value; });
      (*this)[key] = std::distance(metadata.begin(), it);
      if (it == metadata.end()) {
        metadata.push_back(value);
      }
    }
  }

  metadata_index_map_t(const M& metadata_map) {
    for (const auto& [ key, value ] : metadata_map) {
      const auto it = std::find_if(metadata.begin(), metadata.end(), [&value](const metadata_t* v){ return *v == *value; });
      (*this)[key] = std::distance(metadata.begin(), it);
      if (it == metadata.end())
        metadata.push_back(value);
    }
  }

  using parent::begin;
  using parent::end;
  using parent::cbegin;
  using parent::cend;
  using parent::find;
  using parent::insert;
  using parent::at;
  using parent::operator[];
  using parent::erase;
  using parent::size;
};

} // namespace policy_engine

#endif
