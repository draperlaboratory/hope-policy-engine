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

size_t range_file_t::write_range(uint64_t start, uint64_t end, const std::string& tag) {
  char buf[tag.length() + 48];
  size_t size = std::sprintf(buf, "0x%016lx 0x%016lx %s\n", start, end, tag.c_str());
  file << buf;
  return size;
}

void range_file_t::finish() {
  file.close();
}

void range_file_t::done() {}

void range_file_t::print() {
  std::ifstream file(name);
  for (std::string line; std::getline(file, line);)
    std::cout << line << std::endl;
}

bool range_map_t::contains(const tagged_range_t& key) const {
  const auto& [ range, tags ] = key;
  return std::any_of(range_map.begin(), range_map.end(), [&](const tagged_range_t& tagged) {
    const auto& [ r, t ] = tagged;
    return range.start >= r.start && range.end <= r.end;
  });
}

void range_map_t::add_range(uint64_t start, uint64_t end, const std::string& tag) {
  for (auto& [ range, tags ] : range_map) {
    if (range.start == start && range.end == end) {
      tags.push_back(tag);
      return;
    }
  }
  range_map.push_back(tagged_range_t{{start, end}, {tag}});
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