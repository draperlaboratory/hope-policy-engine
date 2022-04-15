#ifndef __TAGGING_TOOLS_H__
#define __TAGGING_TOOLS_H__

#include <cstdint>
#include <string>
#include <vector>

namespace policy_engine {

struct range_t {
  uint64_t start;
  uint64_t end;
  std::vector<std::string>* tags;
};

class RangeFile {
private:
  int _fd;
  std::string _name;
public:
  RangeFile() : _name("/tmp/range_XXXXXX") {
    _fd = mkstemp((char*)_name.data());
  }

  size_t write_range(uint64_t start, uint64_t end, const std::string& tag);
  void finish();
  std::string name();
  int fd();
  void done();
  void print();

  ~RangeFile() {
    if (_fd >= 0) {
      done();
      finish();
    }
  }
};

class RangeMap {
private:
  std::vector<range_t> range_map;
public:
  RangeMap() {}

  bool contains(const range_t& key);
  range_t& operator [](int i);
  void sort();
  void add_range(uint64_t start, uint64_t end, const std::string& tag = "");
  void merge_ranges();
  const std::vector<std::string>* get_tags(uint64_t addr);
  std::vector<std::pair<uint64_t, uint64_t>>* get_ranges(const std::string& tag);
  std::vector<range_t>::iterator begin();
  std::vector<range_t>::iterator end();

  ~RangeMap() {
    for (auto& range : range_map)
      delete range.tags;
  }
};

}

#endif // __TAGGING_TOOLS_H__