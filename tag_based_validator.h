#ifndef TAG_BASED_VALIDATOR_H
#define TAG_BASED_VALIDATOR_H

#include <string>

#include "renode_validator.h"
#include "tag_utils.h"
#include "meta_set_factory.h"

class tag_based_validator_t : public abstract_renode_validator_t {
  protected:

  meta_set_t *t_to_m(tag_t t) { return (meta_set_t *)t; }
  tag_t m_to_t(meta_set_t *ms) { return (tag_t)ms; }
  meta_set_cache_t ms_cache;
  meta_set_factory_t ms_factory;
  
  public:
  tag_based_validator_t(std::string policy_directory, std::string soc_config_file, RegisterReader_t rr);
  virtual ~tag_based_validator_t() { }
  virtual bool validate(address_t pc, insn_bits_t insn) = 0;
  virtual void commit() = 0;
};

#endif
