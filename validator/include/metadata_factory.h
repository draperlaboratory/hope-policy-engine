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

#ifndef METADATA_FACTORY_H
#define METADATA_FACTORY_H

#include <stdint.h>
#include <map>
#include <unordered_map>
#include <yaml-cpp/yaml.h>
#include <stdio.h>

#include "policy_types.h" // meta_t
#include "metadata.h"
#include "opgroup_rule.h"

namespace policy_engine {

struct entity_init_t {
  std::string entity_name;
  std::vector<std::string> meta_names;
};

class metadata_factory_t {
  std::unordered_map<meta_t, std::string> reverse_encoding_map; // for rendering
  std::unordered_map<meta_t, std::string> abbrev_reverse_encoding_map; // for rendering
  std::unordered_map<std::string, meta_t> encoding_map;
  std::unordered_map<std::string, metadata_t const *> path_map;
  std::unordered_map<std::string, metadata_t const *> group_map;
  std::unordered_map<std::string, opgroup_rule_t *> opgroup_rule_map;

  std::map<std::string, entity_init_t> entity_initializers;

  std::string abbreviate(std::string const &dotted_string);

  void init_entity_initializers(YAML::Node const &reqsAST, std::string prefix);
  void init_encoding_map(YAML::Node &rawEnc);
  void init_group_map(YAML::Node &groupAST);
  YAML::Node load_yaml(const char *yml_file);
  
  std::string policy_dir;

  static std::vector<std::string> split_dotted_name(const std::string &name);

  private:
  void update_rule_map(std::string key, YAML::Node &node);

  public:
  metadata_factory_t(std::string policy_dir);
  metadata_t const *lookup_metadata(std::string dotted_path);

  metadata_t const *lookup_group_metadata(std::string const &opgroup,
                                          int32_t flags, uint32_t rs1, uint32_t rs2,
                                          uint32_t rs3, uint32_t rd, int32_t imm) {
    metadata_t *metadata;
    auto const &it_opgroup_rule = opgroup_rule_map.find(opgroup);
    if (it_opgroup_rule != opgroup_rule_map.end()) {
      metadata = it_opgroup_rule->second->match(flags, rs1, rs2, rs3, rd, imm);
      if (metadata != nullptr) {
        return metadata;
      }
    }

    auto const &it_group = group_map.find(opgroup);
    if (it_group == group_map.end()) {
      return nullptr;
    }
    return it_group->second;
  }

  std::string render(meta_t meta, bool abbrev = false);
  std::string render(metadata_t const *metadata, bool abbrev = false);
  void enumerate(std::list<std::string> &elts) {
    for (auto const &p: entity_initializers) {
      elts.push_back(p.first);
    }
  }
};

} // namespace policy_engine

#endif
