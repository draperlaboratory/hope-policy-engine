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
  void add_range(uint64_t start, uint64_t end, const std::string& tag);
  void merge_ranges();
  const std::vector<std::string>* get_tags(uint64_t addr);
  std::vector<std::pair<uint64_t, uint64_t>>* get_ranges(const std::string& tag);

  ~RangeMap() {
    for (auto& range : range_map)
      delete range.tags;
  }
};

size_t RangeFile::write_range(uint64_t start, uint64_t end, const std::string& tag) {
  char buf[tag.length() + 48];
  size_t size = std::sprintf(buf, "0x%016lx 0x%016lx %s\n", start, end, tag.c_str());
  return write(_fd, buf, size);
}

void RangeFile::finish() {
  close(_fd);
}

std::string RangeFile::name() {
  return _name;
}

void RangeFile::done() {
  if (_fd >= 0) {
    unlink(_name.c_str());
    _fd = -1;
  }
}

void RangeFile::print() {
  std::ifstream file(_name);
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

void RangeMap::sort() {
  std::sort(range_map.begin(), range_map.end(), [](range_t a, range_t b) { return a.start < b.start; });
}

void RangeMap::add_range(uint64_t start, uint64_t end, const std::string& tag) {
  for(auto& [ s, e, tags ] : range_map)
    if (s == start && e == end)
      tags->push_back(tag);
  auto tags = new std::vector<std::string>();
  tags->push_back(tag);
  range_map.push_back(range_t{start, end, tags});
}

void RangeMap::merge_ranges() {
  auto rangemap = std::vector<range_t>(range_map);
  std::sort(rangemap.begin(), rangemap.end(), [](range_t a, range_t b) { return a.start < b.start; });
  auto [ curr_s, curr_e, curr_tags ] = rangemap[0];
  for (int i = 1; i < rangemap.size(); i++) {
    auto [ s, e, tags ] = rangemap[i];
    if (s > curr_s) {
      if (e > curr_e) {
        rangemap[i - 1] = range_t{curr_s, s - 1, curr_tags};
        auto new_tags = new std::vector<std::string>(*curr_tags);
        new_tags->insert(new_tags->end(), tags->begin(), tags->end());
        delete rangemap[i].tags;
        rangemap[i] = range_t{s, curr_e, new_tags};
        rangemap.insert(rangemap.begin() + i + 1, range_t{curr_e + 1, e, tags});
      } else {
        auto new_tags = new std::vector<std::string>(*curr_tags);
        new_tags->insert(new_tags->end(), tags->begin(), tags->end());
        delete rangemap[i - 1].tags;
        rangemap[i - 1] = range_t{curr_s, e, new_tags};
        rangemap[i] = range_t{e + 1, curr_e, curr_tags};
      }
    } else if (s == curr_s) {
      if (e == curr_e) {
        auto new_tags = new std::vector<std::string>(*curr_tags);
        new_tags->insert(new_tags->end(), tags->begin(), tags->end());
        delete rangemap[i - 1].tags;
        rangemap[i - 1] = range_t{s, e, new_tags};
        rangemap.erase(rangemap.begin() + i);
      } else {
        auto new_tags = new std::vector<std::string>(*curr_tags);
        new_tags->insert(new_tags->end(), tags->begin(), tags->end());
        delete rangemap[i - 1].tags;
        rangemap[i - 1] = range_t{curr_s, curr_e, new_tags};
        rangemap[i] = range_t{curr_e + 1, e, tags};
      }
    }
    curr_s = rangemap[i].start;
    curr_e = rangemap[i].end;
    curr_tags = rangemap[i].tags;
  }
}

const std::vector<std::string> empty;

const std::vector<std::string>* RangeMap::get_tags(uint64_t addr) {
  for (auto& [ start, end, tags ] : range_map)
    if (addr >= start && addr <= end)
      return tags;
  return &empty;
}

std::vector<std::pair<uint64_t, uint64_t>>* RangeMap::get_ranges(const std::string& tag) {
  auto ranges = new std::vector<std::pair<uint64_t, uint64_t>>();
  for (auto& [ start, end, tags ] : range_map) {
    if (std::find(tags->begin(), tags->end(), tag) != tags->end() || tags->empty() && tag.empty())
      ranges->push_back(std::make_pair(start, end));
  }
  return ranges;
}

} // namespace policy_engine