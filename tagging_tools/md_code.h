#ifndef __MD_CODE_H__
#define __MD_CODE_H__

#include <cstdint>
#include <string>
#include "reporter.h"

namespace policy_engine {

int md_code(const std::string& policy_dir, uint64_t code_address, const std::string& file_name, void* bytes, int n, reporter_t& err);
    
}

#endif // __MD_CODE_H__