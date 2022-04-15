#ifndef __SOC_TAGGER_H__
#define __SOC_TAGGER_H__

#include <string>
#include <yaml-cpp/yaml.h>
#include "tagging_utils.h"

namespace policy_engine {

void generate_soc_range(std::string soc_file, RangeFile& range_file, YAML::Node policy_inits);

} // namespace policy_engine

#endif // __SOC_TAGGER_H__