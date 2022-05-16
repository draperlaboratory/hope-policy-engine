#ifndef METADATA_REGISTER_MAP_H
#define METADATA_REGISTER_MAP_H

#include <map>
#include <memory>
#include <string>
#include "metadata.h"

namespace policy_engine {

/**
 * Register equivalent to metadata_memory_map_t.
 * Maps register names to metadata structures
 */
using metadata_register_map_t = std::map<std::string, std::shared_ptr<metadata_t>>;

}; // namespace policy_engine

#endif
