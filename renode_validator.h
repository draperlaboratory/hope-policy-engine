#ifndef RENODE_VALIDATOR_H
#define RENODE_VALIDATOR_H

#include "validator.h"
#include "renode_interface.h"

/**
   The Renode validator is a slightly more specific validator that expresses
   some of the connection to Renode.  Specifically, when Renode calls an
   external validator, it provides APIs to read registers and memory
   on the assumption that the validator needs to inquire of some SOC
   state in order to evaluate an operation.
*/
class abstract_renode_validator_t : abstract_validator_t {
  protected:
  RegisterReader_t reg_reader;
  MemoryReader_t mem_reader;
  public:
  abstract_renode_validator_t(RegisterReader_t rr, MemoryReader_t mr) : reg_reader(rr), mem_reader(mr) {
  }
  virtual ~abstract_renode_validator_t() { }
  virtual bool validate(address_t pc, insn_bits_t insn) = 0;
  virtual void commit() = 0;
};


#endif
