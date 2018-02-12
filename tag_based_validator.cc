#include "tag_based_validator.h"

tag_based_validator_t::tag_based_validator_t(std::string policy_directory,
					     std::string soc_config_file,
					     RegisterReader_t rr)
: abstract_renode_validator_t(rr), ms_factory(policy_directory, &ms_cache) {
}
