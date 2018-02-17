#ifndef TAG_BASED_VALIDATOR_H
#define TAG_BASED_VALIDATOR_H

#include <string>

#include "renode_validator.h"
#include "tag_utils.h"
#include "tag_converter.h"
#include "meta_set_factory.h"

class tag_based_validator_t : public abstract_renode_validator_t, public tag_converter_t {
  protected:

  meta_set_cache_t *ms_cache;
  meta_set_factory_t *ms_factory;
  
  public:
  tag_based_validator_t(meta_set_cache_t *ms_cache,
			meta_set_factory_t *ms_factory,
			RegisterReader_t rr);
  virtual ~tag_based_validator_t() { }
  virtual bool validate(address_t pc, insn_bits_t insn) = 0;
  virtual void commit() = 0;

  // Provides the tag for a given address.  Used for debugging.
  virtual bool get_tag(address_t addr, tag_t &tag) = 0;
};

#endif
