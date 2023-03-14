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

#ifndef SOC_TAG_CONFIGURATION_H
#define SOC_TAG_CONFIGURATION_H

#include <list>
#include <string>
#include <yaml-cpp/yaml.h>
#include "meta_cache.h"
#include "meta_set_factory.h"
#include "platform_types.h"
#include "tag_utils.h"

namespace policy_engine {

class soc_tag_configuration_t {
public:
  struct soc_element_t {
    address_t start;
    address_t end;
    bool heterogeneous;
    size_t tag_granularity = 4;
    size_t word_size = 4;
    tag_t tag;
  };

private:
  std::list<soc_element_t> elements;
  meta_set_factory_t* factory;

  void process_element(const std::string& element_name, const YAML::Node& n, int xlen);

public:
  using iterator = typename decltype(elements)::iterator;
  using const_iterator = typename decltype(elements)::const_iterator;

  soc_tag_configuration_t(meta_set_factory_t* factory, const std::string& file_name, int xlen);

  void apply(tag_bus_t* tag_bus, meta_set_cache_t* ms_cache);

  iterator begin() { return elements.begin(); }
  iterator end() { return elements.end(); }
  const_iterator begin() const { return elements.begin(); }
  const_iterator end() const { return elements.end(); }
  const_iterator cbegin() const { return elements.cbegin(); }
  const_iterator cend() const { return elements.cend(); }
};

} // namespace policy_engine

#endif
