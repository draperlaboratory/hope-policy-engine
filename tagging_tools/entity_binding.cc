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

#include <string>
#include <yaml-cpp/yaml.h>
#include "entity_binding.h"
#include "reporter.h"
#include "validator_exception.h"

namespace policy_engine {

template <class T=std::string>
static T expect_field(const YAML::Node& n, const std::string& field, const std::string& entity_name) {
  if (!n[field])
    throw configuration_exception_t("expected field " + field + " in entity " + entity_name);
  return n[field].as<T>();
}

static std::unique_ptr<entity_binding_t> process_element(const YAML::Node& n) {
  std::string entity_name = expect_field(n, "name", "");
  std::string element_name = expect_field(n, "kind", entity_name);
  if (element_name == "symbol") {
    return std::make_unique<entity_symbol_binding_t>(
      entity_name,
      expect_field(n, "elf_name", entity_name),
      n["optional"] && n["optional"].as<bool>(),
      n["tag_all"] && !n["tag_all"].as<bool>()
    );
  } else if (element_name == "range") {
    return std::make_unique<entity_range_binding_t>(
      entity_name,
      expect_field(n, "elf_start", entity_name),
      expect_field(n, "elf_end", entity_name),
      n["optional"] && n["optional"].as<bool>()
    );
  } else if (element_name == "soc") {
    return std::make_unique<entity_soc_binding_t>(entity_name, n["optional"] && n["optional"].as<bool>());
  } else if (element_name == "isa") {
    return std::make_unique<entity_isa_binding_t>(entity_name, n["optional"] && n["optional"].as<bool>());
  } else if (element_name == "image") {
    return std::make_unique<entity_image_binding_t>(entity_name, n["optional"] && n["optional"].as<bool>());
  } else {
    throw configuration_exception_t("unexpected kind " + element_name);
  }
}

std::list<std::unique_ptr<entity_binding_t>> entity_binding_t::load(const std::string& file_name, reporter_t& err) {
  std::list<std::unique_ptr<entity_binding_t>> bindings;
  try {
    for (const YAML::Node& node : YAML::LoadFile(file_name))
      bindings.push_back(process_element(node));
  } catch (const std::exception &e) {
    err.error("while parsing %s: %s\n", file_name, e.what());
  }
  return bindings;
}

}