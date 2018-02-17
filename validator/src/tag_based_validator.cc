#include "tag_based_validator.h"

tag_based_validator_t::tag_based_validator_t(meta_set_cache_t *ms_cache,
					     meta_set_factory_t *ms_factory,
					     RegisterReader_t rr)
  : abstract_renode_validator_t(rr), ms_cache(ms_cache), ms_factory(ms_factory) {
}
