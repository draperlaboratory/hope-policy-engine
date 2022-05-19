#ifndef __TAGGING_TOOLS_H__
#define __TAGGING_TOOLS_H__

#include <cstdint>
#include <cstdio>
#include <fstream>
#include <string>
#include <vector>
#include "metadata_memory_map.h"

namespace policy_engine {

struct tagged_range_t {
  range_t range;
  std::vector<std::string> tags;
};

class range_map_t {
private:
  std::vector<tagged_range_t> range_map;

public:
  using iterator = typename decltype(range_map)::iterator;
  using const_iterator = typename decltype(range_map)::const_iterator;

  range_map_t() {}

  bool contains(const tagged_range_t& key) const;

  tagged_range_t& operator [](int i) { return range_map[i]; }
  const tagged_range_t& operator [](int i) const { return range_map[i]; }
  tagged_range_t& at(int i) { return range_map.at(i); }
  const tagged_range_t& at(int i) const { return range_map.at(i); }

  iterator begin() noexcept { return range_map.begin(); }
  iterator end() noexcept { return range_map.end(); }
  const_iterator begin() const noexcept { return range_map.begin(); }
  const_iterator end() const noexcept { return range_map.end(); }
  const_iterator cbegin() const noexcept { return range_map.cbegin(); }
  const_iterator cend() const noexcept { return range_map.cend(); }

  void add_range(uint64_t start, uint64_t end, const std::string& tag="") { add_range(start, end, std::vector<std::string>{tag}); }
  void add_range(uint64_t start, uint64_t end, const std::vector<std::string>& tags);
  const std::vector<std::string>& get_tags(uint64_t addr) const;
  std::vector<range_t> get_ranges(const std::string& tag) const;
};

}

#endif // __TAGGING_TOOLS_H__