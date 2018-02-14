#ifndef TAG_FILE_H
#define TAG_FILE_H

#include <string>
#include "metadata_memory_map.h"

bool load_tags(metadata_memory_map_t *map, std::string file_name);
bool save_tags(metadata_memory_map_t *map, std::string file_name);

#endif
