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

class range_file_t {
private:
  std::ofstream file;
public:
  range_file_t() : name("ranges") {
    file = std::ofstream(name);
  }

  size_t write_range(uint64_t start, uint64_t end, const std::string& tag);
  void finish();
  const std::string name;
  void done();
  void print();

  ~range_file_t() {
    if (file.is_open())
      file.close();
  }
};

class range_map_t {
private:
  std::vector<tagged_range_t> range_map;

public:
  using iterator = typename decltype(range_map)::iterator;

  range_map_t() {}

  bool contains(const tagged_range_t& key);
  tagged_range_t& operator [](int i) { return range_map[i]; }
  void add_range(uint64_t start, uint64_t end, const std::string& tag = "");
  const std::vector<std::string>& get_tags(uint64_t addr);
  std::vector<range_t> get_ranges(const std::string& tag);
  iterator begin() { return range_map.begin(); }
  iterator end() { return range_map.end(); }
};

}

#endif // __TAGGING_TOOLS_H__