#include <string>
#include "elf_loader.h"
#include "reporter.h"

namespace policy_engine {

void tag_op_codes(const std::string& policy_dir, elf_image_t& ef, const std::string& taginfo_file_name, reporter_t& err);

} // namespace policy_engine