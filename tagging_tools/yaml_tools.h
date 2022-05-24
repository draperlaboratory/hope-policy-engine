#ifndef __YAML_TOOLS_H__
#define __YAML_TOOLS_H__

#include "range.h"

namespace YAML {

template<>
struct convert<policy_engine::range_t> {
  static bool decode(const Node& node, policy_engine::range_t& rhs) {
    if (!node.IsMap() || !node["start"] || !node["end"])
      return false;
    
    rhs.start = node["start"].as<uint64_t>();
    rhs.end = node["end"].as<uint64_t>();
    return true;
  }
};

}

#endif // __YAML_TOOLS_H__