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

#include <string.h>
#include <list>
#include <yaml-cpp/yaml.h>

#include "platform_types.h"
#include "meta_set_factory.h"
#include "tag_converter.h"
#include "tag_utils.h"

namespace policy_engine {
class soc_tag_configuration_t {
  public:
  struct soc_element_t {
    address_t start;
    address_t end;
    bool heterogeneous;
    size_t tag_granularity = 32;
    meta_set_t const *meta_set;
  };

  typedef std::list<soc_element_t>::iterator iterator;

  private:
  std::list<soc_element_t> elements;
  meta_set_factory_t *factory;
  void process_element(std::string element_name, const YAML::Node &n);

  public:

  soc_tag_configuration_t(meta_set_factory_t *factory, std::string file_name);
  void apply(tag_bus_t *tag_bus, tag_converter_t *converter);
  iterator begin() { return elements.begin(); }
  iterator end() { return elements.end(); }
};

} // namespace policy_engine

#endif
