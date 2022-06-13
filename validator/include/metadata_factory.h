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

#include <cstdint>
#include <cstdio>
#include <map>
#include <memory>
#include <unordered_map>
#include <vector>
#include <yaml-cpp/yaml.h>
#include "metadata.h"
#include "metadata_memory_map.h"
#include "opgroup_rule.h"
#include "policy_types.h"
#include "range_map.h"
#include "riscv_isa.h"

namespace policy_engine {

struct entity_init_t {
  std::string entity_name;
  std::vector<std::string> meta_names;
};

class metadata_factory_t {
private:
  const std::string policy_dir;

  std::unordered_map<meta_t, std::string> reverse_encoding_map; // for rendering
  std::unordered_map<meta_t, std::string> abbrev_reverse_encoding_map; // for rendering
  std::unordered_map<std::string, meta_t> encoding_map;
  std::unordered_map<std::string, std::unique_ptr<const metadata_t>> path_map;
  std::unordered_map<std::string, std::unique_ptr<const metadata_t>> group_map;
  std::unordered_map<std::string, opgroup_rule_t> opgroup_rule_map;

  std::map<std::string, entity_init_t> entity_initializers;

  std::string abbreviate(const std::string& dotted_string);

  void init_entity_initializers(const YAML::Node& reqsAST, const std::string& prefix);
  void update_entity_initializers(const YAML::Node& metaAST, const std::string& prefix);
  void init_encoding_map(const YAML::Node& rawEnc);
  void init_group_map(const YAML::Node& groupAST);
  void update_rule_map(std::string key, const YAML::Node& node);

  YAML::Node load_yaml(const std::string& yml_file);

public:
  metadata_factory_t(const std::string& policy_dir);

  const metadata_t* lookup_metadata(const std::string& dotted_path);
  std::map<std::string, const metadata_t*> lookup_metadata_map(const std::string& dotted_path);
  const metadata_t* lookup_group_metadata(const std::string& opgroup, const decoded_instruction_t& inst);

  bool apply_tag(metadata_memory_map_t& map, uint64_t start, uint64_t end, const std::string& tag_name);
  template<class RangeMap=range_map_t>
  void apply_tags(metadata_memory_map_t& map, const RangeMap& range_map) {
    for (const auto& [ range, tags ] : range_map)
      for (const std::string& tag : tags)
        if (!apply_tag(map, range.start, range.end, tag))
          throw std::out_of_range("could not find tag " + tag);
  }

  void tag_opcodes(metadata_memory_map_t& map, uint64_t code_address, int xlen, void* bytes, int n, reporter_t& err);
  void tag_entities(metadata_memory_map_t& md_map, const elf_image_t& img, const std::vector<std::string>& yaml_files, reporter_t& err);
  std::vector<std::string> enumerate();

  std::string render(meta_t meta, bool abbrev=false) const;

  template<class MetadataPtr>
  std::string render(MetadataPtr metadata, bool abbrev=false) const {
    std::ostringstream os;
    bool first = true;
    for (const meta_t& meta: *metadata) {
      if (first)
        first = false;
      else
        os << ", ";
      os << render(meta, abbrev);
    }
    return os.str();
  }
};

} // namespace policy_engine

#endif
