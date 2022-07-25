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

#include <iostream>
#include <memory>
#include <string>
#include <yaml-cpp/yaml.h>
#include "meta_cache.h"
#include "policy_utils.h"
#include "platform_types.h"
#include "soc_tag_configuration.h"
#include "validator_exception.h"

namespace policy_engine {

static void dump_node(const YAML::Node& node) {
  switch (node.Type()) {
    case YAML::NodeType::Null: std::cout << "  null" << std::endl; break;
    case YAML::NodeType::Scalar: std::cout << "  scalar" << std::endl; break;
    case YAML::NodeType::Sequence: std::cout << "  sequence" << std::endl; break;
    case YAML::NodeType::Map: std::cout << "  map" << std::endl; break;
    case YAML::NodeType::Undefined: std::cout << "  undefined" << std::endl; break;
    default: std::cout << "  unknown" << std::endl; break;
  }
}

void soc_tag_configuration_t::process_element(const std::string& element_name, const YAML::Node& n, int xlen) {
  std::string elt_path;
  soc_element_t elt;
  elt.heterogeneous = false;

  if (n["name"]) {
    elt_path = n["name"].as<std::string>();
  } else {
    throw configuration_exception_t("'name' field not present for element " + element_name);
  }
  
  if (n["tag_granularity"]) {
    elt.tag_granularity = n["tag_granularity"].as<size_t>();
  } else {
    elt.tag_granularity = xlen/8;
  }
  elt.word_size = xlen/8;

  if (n["start"]) {
    elt.start = n["start"].as<address_t>();
  } else {
    throw configuration_exception_t("'start' field not present for element " + element_name);
  }
  
  if (n["end"]) {
    elt.end = n["end"].as<address_t>();
  } else {
    throw configuration_exception_t("'end' field not present for element " + element_name);
  }
  if (n["heterogeneous"]) {
    elt.heterogeneous = n["heterogeneous"].as<bool>();
  }
  elt.tag = factory->get_tag(elt_path);
  elements.push_back(elt);
}

soc_tag_configuration_t::soc_tag_configuration_t(meta_set_factory_t* factory, const std::string& file_name, int xlen) : factory(factory) {
  YAML::Node n = YAML::LoadFile(file_name);
  if (n["SOC"]) {
    for (const auto& it : n["SOC"]) {
      process_element(it.first.as<std::string>(), it.second, xlen);
    }
  } else {
    throw configuration_exception_t("Expected a root SOC node");
  }
}

void soc_tag_configuration_t::apply(tag_bus_t* tag_bus, meta_set_cache_t* ms_cache) {
  for (const auto& e: elements) {
    if (e.heterogeneous) {
      tag_bus->add_provider(e.start, e.end, std::make_unique<platform_ram_tag_provider_t>(e.end - e.start, e.tag, e.word_size, e.tag_granularity));
    } else {
      tag_bus->add_provider(e.start, e.end, std::make_unique<uniform_tag_provider_t>(e.end - e.start, e.tag, e.tag_granularity));
    }
  }
}

} // namespace policy_engine