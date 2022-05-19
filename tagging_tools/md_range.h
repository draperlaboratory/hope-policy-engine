#ifndef __MD_RANGE_H__
#define __MD_RANGE_H__

#include <string>
#include "reporter.h"
#include "tagging_utils.h"

namespace policy_engine {

void md_range(const std::string& policy_dir, const range_map_t& range_map, const std::string& file_name, reporter_t& err);
    
}

#endif // __MD_RANGE_H__