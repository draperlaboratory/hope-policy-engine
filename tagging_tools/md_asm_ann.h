#ifndef __MD_ASM_ANN_H__
#define __MD_ASM_ANN_H__

#include <string>
#include "reporter.h"

namespace policy_engine {

int md_asm_ann(const std::string& policy_dir, const std::string& taginfo_file, const std::string& asm_file, reporter_t& err, const std::string& output_file="");

}

#endif // __MD_ASM_ANN_H__