/*
 * Copyright © 2017-2018 Dover Microsystems, Inc.
 * All rights reserved. 
 *
 * Use and disclosure subject to the following license. 
 *
 * Permission is hereby granted, free of charge, to any person obtaining
 * a copy of this software and associated documentation files (the
 * "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, sublicense, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject to
 * the following conditions:
 * 
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE
 * LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION
 * OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
 * WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

#ifndef SYMBOL_TABLE_H
#define SYMBOL_TABLE_H

#include <algorithm>
#include <cstdint>
#include <gelf.h>
#include <list>
#include <map>
#include <stdexcept>
#include <string>
#include <vector>

namespace policy_engine {

struct symbol_t {
  enum visibility_t { PUBLIC, PRIVATE };
  static visibility_t get_visibility(const GElf_Sym& symbol) {
    switch (GELF_ST_BIND(symbol.st_info)) {
      case STB_LOCAL:  return PRIVATE;
      case STB_GLOBAL: return PUBLIC;
      default: return PRIVATE;
    }
  }

  enum kind_t { CODE, DATA };
  static kind_t get_kind(const GElf_Sym& symbol) {
    switch (GELF_ST_TYPE(symbol.st_info)) {
      case STT_FUNC: return CODE;
      default: return DATA;
    }
  }

  const std::string name;
  const uint64_t address = 0;
  const size_t size = 0;
  const visibility_t visibility = PUBLIC;
  const kind_t kind = CODE;

  bool operator <(const symbol_t& rhs) const { return address < rhs.address; }
  bool operator <=(const symbol_t& rhs) const { return address <= rhs.address; }
  bool operator >(const symbol_t& rhs) const { return address > rhs.address; }
  bool operator >=(const symbol_t& rhs) const { return address >= rhs.address; }
};

class symbol_table_t {
private:
  std::vector<symbol_t> sym_list;
  std::map<uint64_t, int> addr_map;
  std::map<std::string, int> name_map;

public:
  typedef std::vector<symbol_t>::const_iterator const_iterator;

  symbol_table_t() {}

  const_iterator begin() const { return sym_list.begin(); }
  const_iterator end() const { return sym_list.end(); }

  void add_symbol(const symbol_t& sym) {
    sym_list.push_back(sym);
    addr_map[sym.address] = sym_list.size() - 1;
    name_map[sym.name] = sym_list.size() - 1;
  }

  const symbol_t& at(const std::string& name) const { return sym_list[name_map.at(name)]; }
  const symbol_t& at(uint64_t addr) const { return sym_list[addr_map.at(addr)]; }

  const_iterator find(const std::string& name) const {
    if (name_map.find(name) == name_map.end())
      return end();
    return begin() + name_map.at(name);
  }

  const_iterator find(uint64_t addr) const {
    if (addr_map.find(addr) == addr_map.end())
      return end();
    return begin() + addr_map.at(addr);
  }

  const_iterator lower_bound(uint64_t addr) const {
    auto low = addr_map.lower_bound(addr);
    if (low == addr_map.end())
      return end();
    return begin() + low->second;
  }

  const_iterator upper_bound(uint64_t addr) const {
    auto low = addr_map.upper_bound(addr);
    if (low == addr_map.end())
      return end();
    return begin() + low->second;
  }

  const_iterator find_nearest(uint64_t addr) const {
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
};

} // namespace policy_engine

#endif
