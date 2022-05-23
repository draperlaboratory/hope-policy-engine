#ifndef __MD_RANGE_H__
#define __MD_RANGE_H__

#include <string>

#include "metadata_factory.h"
#include "metadata_memory_map.h"
#include "range_map.h"
#include "reporter.h"

namespace policy_engine {

void md_range(metadata_factory_t& md_factory, metadata_memory_map_t& md_map, const range_map_t& range_map);
    
}

#endif // __MD_RANGE_H__