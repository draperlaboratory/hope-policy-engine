#ifndef __LLVM_METADATA_TAGGER_H__
#define __LLVM_METADATA_TAGGER_H__

#include <cstdint>
#include <map>
#include <string>
#include <yaml-cpp/yaml.h>
#include "elf_loader.h"
#include "range_map.h"
#include "reporter.h"

namespace policy_engine {

class llvm_metadata_tagger_t {
private:
  std::map<std::string, bool> needs_tag_cache;
  reporter_t& err;
public:
  static constexpr int PTR_SIZE = 4;
  static const std::map<std::string, uint8_t> metadata_ops;
  static const std::map<std::string, uint8_t> tag_specifiers;
  static const std::map<std::string, std::map<std::string, std::string>> policy_map;

  llvm_metadata_tagger_t(reporter_t& err) : err(err) {}

  bool policy_needs_tag(const YAML::Node& policy_inits, const std::string& tag);
  void add_code_section_ranges(const elf_image_t& ef, range_map_t& range_map);
  void check_and_add_range(range_map_t& range_map, uint64_t start, uint64_t end, uint8_t tag_specifier, const YAML::Node& policy_inits);
  range_map_t generate_policy_ranges(const elf_image_t& ef, const YAML::Node& policy_inits);
};

}

#endif // __LLVM_METADATA_TAGGER_H__