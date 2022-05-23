#include <string>
#include "elf_loader.h"
#include "metadata_factory.h"
#include "metadata_memory_map.h"
#include "reporter.h"

namespace policy_engine {

void tag_op_codes(metadata_factory_t& md_factory, metadata_memory_map_t& md_map, const elf_image_t& ef, reporter_t& err);

} // namespace policy_engine