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
  std::string line;
  while (std::getline(file, line))
    std::cout << line << '\n';
}

bool range_map_t::contains(const tagged_range_t& key) {
  auto& [ start, end, tags ] = key;
  return std::any_of(range_map.begin(), range_map.end(), [&start, &end](tagged_range_t range) {
    auto& [ s, e, t ] = range;
    return start >= s && end <= e;
  });
}

void range_map_t::add_range(uint64_t start, uint64_t end, const std::string& tag) {
  for(auto& [ s, e, tags ] : range_map)
    if (s == start && e == end)
      tags.push_back(tag);
  std::vector<std::string> tags;
  tags.push_back(tag);
  range_map.push_back(tagged_range_t{start, end, tags});
}

const std::vector<std::string> empty;

const std::vector<std::string>& range_map_t::get_tags(uint64_t addr) {
  for (const tagged_range_t& range : range_map)
    if (addr >= range.start && addr <= range.end)
      return range.tags;
  return empty;
}

std::vector<std::pair<uint64_t, uint64_t>> range_map_t::get_ranges(const std::string& tag) {
  std::vector<std::pair<uint64_t, uint64_t>> ranges;
  for (const auto& [ start, end, tags ] : range_map) {
    if (std::find(tags.begin(), tags.end(), tag) != tags.end() || tags.empty() && tag.empty())
      ranges.push_back(std::make_pair(start, end));
  }
  return ranges;
}

} // namespace policy_engine