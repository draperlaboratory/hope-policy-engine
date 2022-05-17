#ifndef __MD_RANGE_H__
#define __MD_RANGE_H__

#include <string>
#include "reporter.h"

namespace policy_engine {

void md_range(const std::string& policy_dir, const std::string& range_file_name, const std::string& file_name, reporter_t& err);
    
}

#endif // __MD_RANGE_H__