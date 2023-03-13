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

#ifndef TAG_UTILS_H
#define TAG_UTILS_H

#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <vector>
#include <map>
#include <memory>
#include <stdexcept>
#include <string>
#include <vector>
#include "platform_types.h"

namespace policy_engine {

/**
 * This type is what we will maintain internally in the functional, in-process simulator for the PIPE.
 * The size of the type will allow for conversion to a native pointer in policy code.  It is not
 * necessarily the case that the actual data for the tag_t would be the size of a pointer on a
 * particular platform, in which case, there would be some conversion to policy data structures
 * required.
 */
typedef uintptr_t tag_t;
#define PRItag PRIuPTR

static const tag_t BAD_TAG_VALUE = -1;

struct tag_provider_t {
  virtual ~tag_provider_t() {}

  virtual tag_t& data_tag_at(address_t addr) = 0;
  virtual tag_t& insn_tag_at(address_t addr) = 0;
};

class uniform_tag_provider_t : public tag_provider_t {
private:
  size_t tag_granularity; // number of bytes a tag applies to
  address_t size;
  tag_t tag;

  tag_t& tag_at(address_t addr) {
    if (addr < size)
      return tag;
    else {
      char buf[64];
      std::sprintf(buf, "bad address %#lx", addr);
      throw std::out_of_range(buf);
    }
  }

public:
  uniform_tag_provider_t(address_t size, tag_t tag, size_t tag_granularity) : size(size), tag(tag), tag_granularity(tag_granularity) {}

  tag_t& data_tag_at(address_t addr) { return tag_at(addr); }
  tag_t& insn_tag_at(address_t addr) { return tag_at(addr); }
};

class platform_ram_tag_provider_t : public tag_provider_t {
private:
  address_t size;
  size_t tag_granularity; // number of bytes a tag applies to
  unsigned word_size;
  std::vector<tag_t> tags;

public:
  platform_ram_tag_provider_t(address_t size, tag_t tag, size_t word_size, size_t tag_granularity) :
    size(size), word_size(word_size), tags(size/MIN_TAG_GRANULARITY + 1, tag), tag_granularity(tag_granularity) {}
  
  tag_t& data_tag_at(address_t addr) {
    if (addr < size)
      return tags[(addr & -tag_granularity)/MIN_TAG_GRANULARITY];
    else {
      char buf[64];
      std::sprintf(buf, "bad address %#lx", addr);
      throw std::out_of_range(buf);
    }
  }

  tag_t& insn_tag_at(address_t addr) {
    if (addr < size)
      return tags[addr/MIN_TAG_GRANULARITY];
    else {
      char buf[64];
      std::sprintf(buf, "bad address %#lx", addr);
      throw std::out_of_range(buf);
    }
  }
};

class tag_bus_t : public tag_provider_t {
private:
  std::map<int64_t, std::unique_ptr<tag_provider_t>> provider_map;

  decltype(provider_map)::reference get_provider(address_t addr) {
    if (auto it = provider_map.lower_bound(-addr); it != provider_map.end()) {
      //return tf(it->second, addr - -it->first);
      return *it;
    } else {
      char buf[64];
      std::sprintf(buf, "bad address %#lx", addr);
      throw std::out_of_range(buf);
    }
  }

public:
  void add_provider(address_t start_addr, address_t end_addr, std::unique_ptr<tag_provider_t>&& provider) {
    provider_map[-start_addr] = std::move(provider);
  }

  tag_t& data_tag_at(address_t addr) {
    auto& [ base, tp ] = get_provider(addr);
    return tp->data_tag_at(addr - -base);
  }

  tag_t& insn_tag_at(address_t addr) {
    auto& [ base, tp ] = get_provider(addr);
    return tp->insn_tag_at(addr - -base);
  }
};

} // namespace policy_engine

#endif
