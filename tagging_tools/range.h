#ifndef __RANGE_H__
#define __RANGE_H__

#include <cstdint>

namespace policy_engine {

struct range_t {
  uint64_t start; // inclusive
  uint64_t end;   // exclusive!

  bool contains(uint64_t address) const { return address >= start && address < end; }

  bool operator ==(const range_t& other) const { return start == other.start && end == other.end; }
  bool operator <(const range_t& other) const { return (start < other.start) || (start == other.start && end < other.end); }
};

}

#endif // __RANGE_H__