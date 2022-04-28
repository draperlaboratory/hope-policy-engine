#ifndef __MD_INDEX_H__
#define __MD_INDEX_H__

#include <string>
#include "reporter.h"

namespace policy_engine {

int md_index(const std::string& tag_filename, const std::string& policy_dir, reporter_t& err);

}

#endif // __MD_INDEX_H__