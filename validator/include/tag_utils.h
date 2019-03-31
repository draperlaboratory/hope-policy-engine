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
#include <vector>
#include <map>

#include "platform_types.h"

#include "policy_debug.h"

namespace policy_engine {

/**
   This type is what we will maintain internally in the functional, in-process simulator for the PIPE.
   The size of the type will allow for conversion to a native pointer in policy code.  It is not
   necessarily the case that the actual data for the tag_t would be the size of a pointer on a
   particular platform, in which case, there would be some conversion to policy data structures
   required.
*/
typedef uintptr_t tag_t;

struct tag_provider_t {
  virtual ~tag_provider_t() { }
  virtual bool get_tag(address_t addr, tag_t &tag) = 0;
  virtual bool set_tag(address_t addr, tag_t tag) = 0;
};

class uniform_tag_provider_t : public tag_provider_t {
  private:
  address_t size;
  tag_t tag;

  public:
  uniform_tag_provider_t(address_t size, tag_t tag) : size(size), tag(tag) { }
  bool get_tag(address_t addr, tag_t &t) {
    if (addr < size) {
      t = tag;
      return true;
    }
    return false;
  }

  bool set_tag(address_t addr, tag_t t) {
    if (addr < size) {
      tag = t;
      return true;
    }
    return false;
  }
};

class platform_ram_tag_provider_t : public tag_provider_t {
  private:
  address_t size;
  unsigned word_size;
  std::vector<tag_t> tags;

  public:
  platform_ram_tag_provider_t(address_t size, unsigned word_size, tag_t tag) :
  size(size), word_size(word_size), tags(size / word_size, tag) {  }
  bool get_tag(address_t addr, tag_t &t) {
    if (addr < size) {
      t = tags[addr / word_size];
      return true;
    }
    return false;
  }
  bool set_tag(address_t addr, tag_t t) {
    if (addr < size) {
      tags[addr / word_size] = t;
      return true;
    }
    return false;
  }
};

class tag_provider_map_t {
  public:

  void add_provider(address_t start_addr, address_t end_addr, tag_provider_t *provider) {
    providers[-start_addr] = provider;
  }

  tag_provider_t *get_provider(address_t addr, address_t &offset) {
    auto it = providers.lower_bound(-addr);
    if (it == providers.end()) {
      return nullptr;
    }
    offset = addr - -it->first;
    return it->second;
  }

  private:
  std::map<int64_t, tag_provider_t *> providers;
};

class tag_bus_t {
  tag_provider_map_t provider_map;
  public:
  void add_provider(address_t start_addr, address_t end_addr, tag_provider_t *provider) {
    provider_map.add_provider(start_addr, end_addr, provider);
  }
  bool load_tag(address_t addr, tag_t &tag) {
    tag_provider_t *tp;
    address_t offset;
    tp = provider_map.get_provider(addr, offset);
    if (tp)
      return tp->get_tag(offset, tag);
    return false;
  }
  bool store_tag(address_t addr, tag_t tag) {
    tag_provider_t *tp;
    address_t offset;
    tp = provider_map.get_provider(addr, offset);
    if (tp)
      return tp->set_tag(offset, tag);
    return false;
  }
};

#define tassert(b) if (!(b)) { printf("bad register index: %lx\n", i); return bad_value; }
#define BAD_TAG_VALUE (-1)
template <size_t N> class tag_file_t {
  tag_t tags[N];

  tag_t bad_value;
  public:

  tag_file_t() : bad_value(BAD_TAG_VALUE) { }

  tag_file_t(tag_t initial_tag) : bad_value(BAD_TAG_VALUE) {
    reset(initial_tag);
  }

  void reset(tag_t initial_tag) {
    for (size_t i = 0; i < N; i++)
      tags[i] = initial_tag;
  }

  tag_t &operator[](size_t i) {
    tassert(i >= 0 && i < N);
    return tags[i];
  }

  const tag_t &operator[](size_t i) const {
    tassert(i >= 0 && i < N);
    return tags[i];
  }
};

} // namespace policy_engine

#endif
