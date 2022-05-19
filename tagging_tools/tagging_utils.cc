#include <algorithm>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <string>
#include <tuple>
#include <unistd.h>
#include <utility>
#include <vector>
#include "metadata_memory_map.h"
#include "tagging_utils.h"

namespace policy_engine {

bool range_map_t::contains(const tagged_range_t& key) const {
  const auto& [ range, tags ] = key;
  return std::any_of(range_map.begin(), range_map.end(), [&](const tagged_range_t& tagged) {
    const auto& [ r, t ] = tagged;
    return range.start >= r.start && range.end <= r.end;
  });
}

void range_map_t::add_range(uint64_t start, uint64_t end, const std::vector<std::string>& tags) {
  for (auto& [ r, t ] : range_map) {
    if (r.start == start && r.end == end) {
      t.insert(t.end(), tags.begin(), tags.end());
      return;
    }
  }
  range_map.push_back(tagged_range_t{{start, end}, tags});
}

const std::vector<std::string> empty;

const std::vector<std::string>& range_map_t::get_tags(uint64_t addr) const {
  for (const tagged_range_t& tagged : range_map)
    if (tagged.range.contains(addr))
      return tagged.tags;
  return empty;
}

std::vector<range_t> range_map_t::get_ranges(const std::string& tag) const {
  std::vector<range_t> ranges;
  for (const auto& [ range, tags ] : range_map) {
    if (std::find(tags.begin(), tags.end(), tag) != tags.end() || (tags.empty() && tag.empty()))
      ranges.push_back(range);
  }
  return ranges;
}

} // namespace policy_engine