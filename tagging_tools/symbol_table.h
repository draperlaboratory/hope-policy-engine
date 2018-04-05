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

#include <map>
#include <list>
#include <algorithm>

#include "platform_types.h"

namespace policy_engine {

class symbol_t {
  public:
  enum visibility_t {
    PUBLIC, PRIVATE
  };
  enum kind_t {
    CODE, DATA
  };
  symbol_t(const char *name) : name(name), addr(0), size(0), visibility(PUBLIC), kind(CODE) { }
  symbol_t(const char *name, address_t addr) : name(name), addr(addr), size(0), visibility(PUBLIC), kind(CODE) { }
  symbol_t(const char *name, address_t addr, size_t size) : name(name), addr(addr), size(size), visibility(PUBLIC), kind(CODE) { }
  symbol_t(const char *name, address_t addr, size_t size, visibility_t visibility, kind_t kind) :
  name(name), addr(addr), size(size), visibility(visibility), kind(kind) { }
  address_t get_address() const { return addr; }
  size_t get_size() const { return size; }
  std::string get_name() const { return name; }
  visibility_t get_visibility() { return visibility; }
  kind_t get_kind() { return kind; }

  bool operator <=(const symbol_t &rhs) const { return addr <= rhs.addr; }
  bool operator <(const symbol_t &rhs) const { return addr < rhs.addr; }
  private:

  std::string name;
  address_t addr;
  size_t size;
  visibility_t visibility;
  kind_t kind;
};

class symbol_table_t {
  typedef std::map<address_t, symbol_t *> symbol_map_t;
  symbol_map_t symbols;
  std::map<std::string, symbol_t *> symbols_by_name;

  public:

  void add_symbol(symbol_t *sym) {
    symbols[sym->get_address()] = sym;
    symbols_by_name[sym->get_name()] = sym;
  }

  symbol_t *find_symbol(std::string name) const {
    auto it = symbols_by_name.find(name);
    if (it == symbols_by_name.end())
      return nullptr;
    return it->second;
  }

  symbol_t *find_nearest_symbol(address_t addr) const {
    auto low = symbols.lower_bound(addr);
    if (low == symbols.end())
      return nullptr;
    if (low == symbols.begin())
      return nullptr;
    --low;
    return low->second;
  }

  symbol_t *find_next_symbol(address_t addr) const {
    auto low = symbols.lower_bound(addr);
    if (low == symbols.end())
      return nullptr;
    ++low;
    return low->second;
  }

  typedef std::list<symbol_t *> symbol_list_t;
  symbol_list_t sorted_symbols() {
    symbol_list_t res;
    std::transform(symbols.begin(), symbols.end(), std::back_inserter(res),
		   [](const symbol_map_t::value_type &val) { return val.second; });
    res.sort([](const symbol_t * const &lhs, const symbol_t * const &rhs) { return *lhs <= *rhs; });
    return res;
  }
 
  static void next_nearest_symbol(symbol_list_t::const_iterator &iter,
				  symbol_list_t::const_iterator const &end,
				  address_t addr) {
    while (iter != end && (*iter)->get_address() < addr) {
//      printf("nns: 0x%x - %s\n", (*iter)->get_address(), (*iter)->name.c_str());
      iter++;
    }
  }
};

} // namespace policy_engine

#endif
