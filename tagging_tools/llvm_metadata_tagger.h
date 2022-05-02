#ifndef __LLVM_METADATA_TAGGER_H__
#define __LLVM_METADATA_TAGGER_H__

#include <cstdint>
#include <map>
#include <string>
#include <yaml-cpp/yaml.h>
#include "elf_loader.h"
#include "reporter.h"
#include "tagging_utils.h"

namespace policy_engine {

class llvm_metadata_tagger_t {
private:
  std::map<std::string, bool> needs_tag_cache;
  reporter_t& err;
public:
  static const int PTR_SIZE = 4;
  static const std::map<std::string, uint8_t> metadata_ops;
  static const std::map<std::string, uint8_t> tag_specifiers;
  static const std::map<std::string, std::map<std::string, std::string>> policy_map;

  llvm_metadata_tagger_t(reporter_t& err) : err(err) {}

  bool policy_needs_tag(const YAML::Node& policy_inits, const std::string& tag);
  void add_code_section_ranges(const elf_image_t& ef, RangeMap& range_map);
  void check_and_write_range(RangeFile& range_file, uint64_t start, uint64_t end, uint8_t tag_specifier, const YAML::Node& policy_inits, RangeMap& range_map);
  RangeMap generate_policy_ranges(elf_image_t& elf_file, RangeFile& range_file, const YAML::Node& policy_inits);
};

}

#endif // __LLVM_METADATA_TAGGER_H__