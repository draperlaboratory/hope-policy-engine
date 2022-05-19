#include <cstdint>
#include <iostream>
#include <map>
#include <string>
#include <utility>
#include <yaml-cpp/yaml.h>
#include "metadata_memory_map.h"
#include "range_map.h"
#include "reporter.h"

namespace policy_engine {

void add_soc_ranges(range_map_t& range_map, const std::string& soc_file, const YAML::Node& policy_inits, reporter_t& err) {
  YAML::Node soc_cfg = YAML::LoadFile(soc_file);

  std::map<std::string, range_t> soc_ranges;
  for (const auto& elem : soc_cfg["SOC"])
    soc_ranges[elem.second["name"].as<std::string>()] = range_t{elem.second["start"].as<uint64_t>(), elem.second["end"].as<uint64_t>()};
  for (const auto& device : policy_inits["Require"]["SOC"]) {
    for (const auto& elem : device.second) {
      std::string name = "SOC." + device.first.as<std::string>() + "." + elem.first.as<std::string>();
      if (soc_ranges.find(name) != soc_ranges.end()) {
        err.info("%s: %#lx - %#lx\n", name, soc_ranges[name].start, soc_ranges[name].end);
        range_map.add_range(soc_ranges[name].start, soc_ranges[name].end, name);
      }
    }
  }
}
    
} // namespace policy_engine