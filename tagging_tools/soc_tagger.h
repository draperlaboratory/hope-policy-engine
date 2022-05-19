#ifndef __SOC_TAGGER_H__
#define __SOC_TAGGER_H__

#include <string>
#include <yaml-cpp/yaml.h>
#include "range_map.h"
#include "reporter.h"

namespace policy_engine {

void add_soc_ranges(range_map_t& range_map, const std::string& soc_file, const YAML::Node& policy_inits, reporter_t& err);

} // namespace policy_engine

#endif // __SOC_TAGGER_H__