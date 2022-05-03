#ifndef __SOC_TAGGER_H__
#define __SOC_TAGGER_H__

#include <string>
#include <yaml-cpp/yaml.h>
#include "reporter.h"
#include "tagging_utils.h"

namespace policy_engine {

void generate_soc_ranges(std::string soc_file, range_file_t& range_file, const YAML::Node& policy_inits, reporter_t& err);

} // namespace policy_engine

#endif // __SOC_TAGGER_H__