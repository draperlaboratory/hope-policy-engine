#ifndef METADATA_INDEX_MAP_H
#define METADATA_INDEX_MAP_H

#include <cstdint>
#include <map>
#include <utility>
#include <vector>
#include "metadata_memory_map.h"

namespace policy_engine {

/**
 * Container for mapping memory/registers to indexes in a
 * collection of metadata values.
 * If provided metadata_values and metadata_map on initialization,
 * updates metadata_values with any new metadata found in metadata_map.
 */

// XXX: TKey is always the key type of TMap, but two generics are needed to support
// metadata_memory_map_t as TMap
template <typename TMap, typename TKey>
class metadata_index_map_t {
  private:
    std::map<TKey, uint32_t> index_map;
    TMap& metadata_map;
    std::vector<const metadata_t *>& metadata_values;

    void extract_metadata_values() {
      for (const auto& it : metadata_map) {
        size_t i;
        for (i = 0; i < metadata_values.size(); i++) {
          if(*metadata_values.at(i) == *it.second) {
            break;
          }
        }
        if (i == metadata_values.size()) {
          metadata_values.push_back(it.second);
        }

        std::pair<TKey, uint32_t> p(it.first, i);
        index_map.insert(p);
      }
    }
  public:
    metadata_index_map_t() {}

    metadata_index_map_t(TMap& metadata_map, std::vector<const metadata_t *>& metadata_values):
      metadata_map(metadata_map), metadata_values(metadata_values) {
      extract_metadata_values();
    }

    using iterator = typename std::map<TKey, uint32_t>::iterator;

    iterator begin()        { return index_map.begin(); }
    iterator end()          { return index_map.end(); }
    size_t size()           { return index_map.size(); }

    void insert(std::pair<TKey, uint32_t> p) {
      index_map.insert(p);
    }

    uint32_t &at(TKey k) {
      return index_map.at(k);
    }

    size_t erase(const TKey k) {
      return index_map.erase(k);
    }
};

} // namespace policy_engine

#endif
