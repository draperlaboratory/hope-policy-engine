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

size_t RangeFile::write_range(uint64_t start, uint64_t end, const std::string& tag) {
  char buf[tag.length() + 48];
  size_t size = std::sprintf(buf, "0x%016lx 0x%016lx %s\n", start, end, tag.c_str());
  file << buf;
  return size;
}

void RangeFile::finish() {
  file.close();
}

void RangeFile::done() {}

void RangeFile::print() {
  std::ifstream file(name);
  std::string line;
  while (std::getline(file, line))
    std::cout << line << '\n';
}

bool RangeMap::contains(const range_t& key) {
  auto& [ start, end, tags ] = key;
  return std::any_of(range_map.begin(), range_map.end(), [&start, &end](range_t range) {
    auto& [ s, e, t ] = range;
    return start >= s && end <= e;
  });
}

range_t& RangeMap::operator [](int i) {
  return range_map[i];
}

void RangeMap::add_range(uint64_t start, uint64_t end, const std::string& tag) {
  for(auto& [ s, e, tags ] : range_map)
    if (s == start && e == end)
      tags.push_back(tag);
  std::vector<std::string> tags;
  tags.push_back(tag);
  range_map.push_back(range_t{start, end, tags});
}

const std::vector<std::string> empty;

const std::vector<std::string>& RangeMap::get_tags(uint64_t addr) {
  for (const auto& range : range_map) {
    if (addr >= range.start && addr <= range.end)
      return range.tags;
  }
  return empty;
}

std::vector<std::pair<uint64_t, uint64_t>> RangeMap::get_ranges(const std::string& tag) {
  std::vector<std::pair<uint64_t, uint64_t>> ranges;
  for (auto& [ start, end, tags ] : range_map) {
    if (std::find(tags.begin(), tags.end(), tag) != tags.end() || tags.empty() && tag.empty())
      ranges.push_back(std::make_pair(start, end));
  }
  return ranges;
}

std::vector<range_t>::iterator RangeMap::begin() {
  return range_map.begin();
}

std::vector<range_t>::iterator RangeMap::end() {
  return range_map.end();
}

} // namespace policy_engine