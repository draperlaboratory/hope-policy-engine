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

using namespace policy_engine;

static void expect_field(YAML::Node const *n, const char *field, std::string entity_name) {
  if (!(*n)[field])
    throw configuration_exception_t("expected field " + std::string(field) + " in entity " + entity_name);
}

static void process_element(const YAML::Node &n,
			    std::list<std::unique_ptr<entity_binding_t>> &bindings) {
  std::string elt_path;
  
  if (!n["name"])
    throw configuration_exception_t("binding must have an entity name");
  std::string entity_name = n["name"].as<std::string>();
  expect_field(&n, "kind", entity_name);
  std::string element_name = n["kind"].as<std::string>();
  if (element_name == "symbol") {
    entity_symbol_binding_t *binding = new entity_symbol_binding_t(
      entity_name,
      n["elf_name"].as<std::string>(),
      n["optional"] && n["optional"].as<bool>(),
      n["tag_all"] && !n["tag_all"].as<bool>()
    );
    expect_field(&n, "elf_name", entity_name);
    std::unique_ptr<entity_binding_t> u = std::unique_ptr<entity_binding_t>(binding);
    bindings.push_back(std::move(u));
  } else if (element_name == "range") {
    entity_range_binding_t *binding = new entity_range_binding_t(
      entity_name,
      n["elf_start"].as<std::string>(),
      n["elf_end"].as<std::string>(),
      n["optional"] && n["optional"].as<bool>()
    );
    expect_field(&n, "elf_start", entity_name);
    expect_field(&n, "elf_end", entity_name);
    std::unique_ptr<entity_binding_t> u = std::unique_ptr<entity_binding_t>(binding);
    bindings.push_back(std::move(u));
  } else if (element_name == "soc") {
    entity_soc_binding_t *binding = new entity_soc_binding_t(entity_name, n["optional"] && n["optional"].as<bool>());
    std::unique_ptr<entity_binding_t> u = std::unique_ptr<entity_binding_t>(binding);
    bindings.push_back(std::move(u));
  } else if (element_name == "isa") {
    entity_isa_binding_t *binding = new entity_isa_binding_t(entity_name, n["optional"] && n["optional"].as<bool>());
    std::unique_ptr<entity_binding_t> u = std::unique_ptr<entity_binding_t>(binding);
    bindings.push_back(std::move(u));
  } else if (element_name == "image") {
    entity_image_binding_t *binding = new entity_image_binding_t(entity_name, n["optional"] && n["optional"].as<bool>());
    std::unique_ptr<entity_binding_t> u = std::unique_ptr<entity_binding_t>(binding);
    bindings.push_back(std::move(u));
  } else {
    throw configuration_exception_t("unexpected kind " + element_name);
  }
}

void policy_engine::load_entity_bindings(const std::string& file_name, std::list<std::unique_ptr<entity_binding_t>>& bindings, reporter_t& err) {
//  try {
    for (const YAML::Node& node : YAML::LoadFile(file_name))
      process_element(node, bindings);
//  } catch (const std::exception &e) {
//    err.error("while parsing %s: %s\n", file_name.c_str(), e.what());
//  }
}

