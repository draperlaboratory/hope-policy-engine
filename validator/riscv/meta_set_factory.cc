#include <string.h>

#include "policy_utils.h"

#include "meta_set_factory.h"

meta_set_factory_t::meta_set_factory_t(meta_set_cache_t *ms_cache, std::string policy_dir)
  : metadata_factory_t(policy_dir), ms_cache(ms_cache) {
}

meta_set_t const *meta_set_factory_t::get_meta_set(std::string dotted_path) {
  metadata_t const *metadata = lookup_metadata(dotted_path);
  if (metadata) {
    meta_set_t ms;
    memset(&ms, 0, sizeof(ms));
//    printf("get_meta_set: %s = ", dotted_path.c_str());
    for (auto &it: *metadata) {
      ms_bit_add(&ms, it);
//      printf("0x%lx ", it.second);
    }
//    printf("\n");
    return ms_cache->canonize(ms);
  } else {
    return nullptr;
  }
}
