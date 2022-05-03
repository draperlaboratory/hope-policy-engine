#include <string>
#include <yaml-cpp/yaml.h>
#include "tagging_utils.h"

namespace policy_engine {

int generate_tag_array(const std::string& elfname, range_file_t& range_file, const std::string& policy_name, YAML::Node policy_meta_info, bool rv64);

}