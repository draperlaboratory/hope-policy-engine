#ifndef __MD_INDEX_H__
#define __MD_INDEX_H__

#include <string>
#include "metadata_factory.h"
#include "metadata_memory_map.h"
#include "reporter.h"

namespace policy_engine {

void md_index(metadata_factory_t& metadata_factory, metadata_memory_map_t& metadata_memory_map, const std::string& tag_filename, reporter_t& err);

}

#endif // __MD_INDEX_H__