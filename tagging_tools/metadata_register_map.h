#ifndef METADATA_REGISTER_MAP_H
#define METADATA_REGISTER_MAP_H

#include <map>
#include "metadata_memory_map.h"

namespace policy_engine {

/**
  Register equivalent to metadata_memory_map_t.
  Maps register names to metadata structures
*/
typedef std::map<std::string, metadata_t const *> metadata_register_map_t;

}; // namespace policy_engine

#endif
