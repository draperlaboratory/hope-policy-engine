#include <algorithm>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <string>
#include <tuple>
#include <unistd.h>
#include <utility>
#include <vector>
#include <yaml-cpp/yaml.h>
#include "elf_loader.h"
#include "metadata_memory_map.h"
#include "range.h"
#include "range_map.h"
#include "reporter.h"
#include "yaml_tools.h"

namespace policy_engine {

const std::string range_map_t::RWX_X = "elf.Section.SHF_EXECINSTR";
const std::string range_map_t::RWX_R = "elf.Section.SHF_ALLOC";
const std::string range_map_t::RWX_W = "elf.Section.SHF_WRITE";

bool range_map_t::contains(const tagged_range_t& key) const {
  const auto& [ range, tags ] = key;
  return std::any_of(range_map.begin(), range_map.end(), [&](const tagged_range_t& tagged) {
    const auto& [ r, t ] = tagged;
    return range.start >= r.start && range.end <= r.end;
  });
}

void range_map_t::add_range(uint64_t start, uint64_t end, const std::vector<std::string>& tags) {
  for (auto& [ r, t ] : range_map) {
    if (r.start == start && r.end == end) {
      t.insert(t.end(), tags.begin(), tags.end());
      return;
    }
  }
  range_map.push_back(tagged_range_t{{start, end}, tags});
}

void range_map_t::add_rwx_ranges(const elf_image_t& ef, reporter_t& err) {
  for (const elf_section_t& section : ef.sections) {
    if (section.flags & SHF_EXECINSTR) {
      add_range(section.address, section.end_address(), {RWX_R, RWX_X});
      err.info("X %s: %#lx - %#lx\n", section.name, section.address, section.end_address());
    } else if (section.flags & SHF_WRITE) {
      add_range(section.address, section.end_address(), RWX_W);
      err.info("W %s: %#lx - %#lx\n", section.name, section.address, section.end_address());
    } else if (section.flags & SHF_ALLOC) {
      add_range(section.address, section.end_address(), RWX_R);
      err.info("R %s: %#lx - %#lx\n", section.name, section.address, section.end_address());
    }
  }
}

void range_map_t::add_soc_ranges(const std::string& soc_file, const YAML::Node& policy_inits, reporter_t& err) {
  YAML::Node soc_cfg = YAML::LoadFile(soc_file);

  std::map<std::string, range_t> soc_ranges;
  for (const auto& elem : soc_cfg["SOC"])
    soc_ranges[elem.second["name"].as<std::string>()] = elem.second.as<range_t>();
  for (const auto& device : policy_inits["Require"]["SOC"]) {
    for (const auto& elem : device.second) {
      std::string name = "SOC." + device.first.as<std::string>() + "." + elem.first.as<std::string>();
      if (soc_ranges.find(name) != soc_ranges.end()) {
        err.info("%s: %#lx - %#lx\n", name, soc_ranges[name].start, soc_ranges[name].end);
        add_range(soc_ranges[name].start, soc_ranges[name].end, name);
      }
    }
  }
}

const std::vector<std::string> empty;

const std::vector<std::string>& range_map_t::get_tags(uint64_t addr) const {
  for (const tagged_range_t& tagged : range_map)
    if (tagged.range.contains(addr))
      return tagged.tags;
  return empty;
}

std::vector<range_t> range_map_t::get_ranges(const std::string& tag) const {
  std::vector<range_t> ranges;
  for (const auto& [ range, tags ] : range_map) {
    if (std::find(tags.begin(), tags.end(), tag) != tags.end() || (tags.empty() && tag.empty()))
      ranges.push_back(range);
  }
  return ranges;
}

} // namespace policy_engine