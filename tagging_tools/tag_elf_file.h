#include <string>
#include <yaml-cpp/yaml.h>
#include "range_map.h"

namespace policy_engine {

bool add_tag_array(range_map_t& range_map, const std::string& elfname, const std::string& policy_name, const YAML::Node& policy_meta_info, int address_bytes);

}