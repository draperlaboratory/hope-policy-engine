#include <algorithm>
#include <cstdint>
#include <gelf.h>
#include <map>
#include <string>
#include <vector>
#include "symbol_table.h"

namespace policy_engine {

void symbol_table_t::push_back(const symbol_t& sym){
  sym_list.push_back(sym);
  addr_map[sym.address] = sym_list.size() - 1;
  name_map[sym.name] = sym_list.size() - 1;
}

symbol_table_t::const_iterator symbol_table_t::find(const std::string& name) const {
  if (name_map.find(name) == name_map.end())
    return end();
  return begin() + name_map.at(name);
}

symbol_table_t::const_iterator symbol_table_t::find(uint64_t addr) const {
  if (addr_map.find(addr) == addr_map.end())
    return end();
  return begin() + addr_map.at(addr);
}

symbol_table_t::const_iterator symbol_table_t::lower_bound(uint64_t addr) const {
  auto low = addr_map.lower_bound(addr);
  if (low == addr_map.end())
    return end();
  return begin() + low->second;
}

symbol_table_t::const_iterator symbol_table_t::upper_bound(uint64_t addr) const {
  auto low = addr_map.upper_bound(addr);
  if (low == addr_map.end())
    return end();
  return begin() + low->second;
}

symbol_table_t::const_iterator symbol_table_t::find_nearest(uint64_t addr) const {
  auto low = lower_bound(addr);
  auto high = upper_bound(addr);
  if (low == end() && high == end())
    return end();
  else if (low == end() && high != end())
    return high;
  else if (low != end() && high == end())
    return low;
  else if ((addr - low->address) < (high->address - addr))
    return low;
  else
    return high;
}

}