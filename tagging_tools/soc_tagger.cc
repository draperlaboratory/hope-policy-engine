#include <cstdint>
#include <iostream>
#include <map>
#include <string>
#include <utility>
#include <yaml-cpp/yaml.h>
#include "reporter.h"
#include "tagging_utils.h"

namespace policy_engine {

void generate_soc_ranges(std::string soc_file, range_file_t& range_file, const YAML::Node& policy_inits, reporter_t& err) {
  YAML::Node soc_cfg = YAML::LoadFile(soc_file);

  std::map<std::string, std::pair<uint64_t, uint64_t>> soc_ranges;
  for (const auto& elem : soc_cfg["SOC"]) {
    soc_ranges[elem.second["name"].as<std::string>()] = std::make_pair(elem.second["start"].as<uint64_t>(), elem.second["end"].as<uint64_t>() + 1);
  }
  
  for (const auto& device : policy_inits["Require"]["SOC"]) {
    for (const auto& elem : device.second) {
      std::string name = "SOC." + device.first.as<std::string>() + "." + elem.first.as<std::string>();
      if (soc_ranges.find(name) != soc_ranges.end()) {
        err.info("%s: %#lx - %#lx\n", name, soc_ranges[name].first, soc_ranges[name].second);
        range_file.write_range(soc_ranges[name].first, soc_ranges[name].second, name);
      }
    }
  }
}
    
} // namespace policy_engine