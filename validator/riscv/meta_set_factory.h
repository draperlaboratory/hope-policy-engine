#ifndef META_SET_FACTORY_H
#define META_SET_FACTORY_H

#include <map>
#include <unordered_map>
#include <yaml-cpp/yaml.h>

#include "metadata_factory.h"
#include "meta_cache.h"
#include "policy_meta_set.h"

class meta_set_factory_t : public metadata_factory_t {
  meta_set_cache_t *ms_cache;

  public:
  meta_set_factory_t(meta_set_cache_t *ms_cache, std::string policy_dir);
  meta_set_t const*get_meta_set(std::string dotted_path);
  meta_set_t const*get_group_meta_set(std::string opgroup) {
    return nullptr;
  }
};

#endif
