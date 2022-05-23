#ifndef __MD_CODE_H__
#define __MD_CODE_H__

#include <cstdint>
#include <string>
#include "metadata_factory.h"
#include "metadata_memory_map.h"
#include "reporter.h"

namespace policy_engine {

void md_code(metadata_factory_t& md_factory, metadata_memory_map_t& map, uint64_t code_address, void* bytes, int n, reporter_t& err);
    
}

#endif // __MD_CODE_H__