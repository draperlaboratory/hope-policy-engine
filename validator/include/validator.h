#ifndef VALIDATOR_H
#define VALIDATOR_H

#include <stdlib.h>
#include <stdint.h>
#include <string.h>

// platform_types specifies things specific to the general architecture we are
// building for.  This includes things like address_t and insn_bits_t.
#include "platform_types.h"

namespace policy_engine {

/**
   Highest level validator interface class.  No structure of input information
   is presumed at all.  The interface is restricted to a validate/commit pair of
   API calls.
*/
class abstract_validator_t {
  public:
  virtual ~abstract_validator_t() { }

  /**
     Called before an instruction is executed.  Parameters are the PC of the
     instruction to be executed, and the actual bits of the instruction.

     Returns <code>true</code> on success, meaning the instruction should be
     executed, and <code>false</code> on failure.
   */
  virtual bool validate(address_t pc, insn_bits_t insn) = 0;
  virtual void commit() = 0;
};

/**
   A simple implementation of the abstract validator that always returns
   <code>true</code>.  Using this validator is the simplest functional equivalent
   of a NOP.  Instructions will always be permitted to execute.
*/
class always_ok_validator_t final : public abstract_validator_t {
  public:
  bool validate(address_t pc, insn_bits_t insn) { return true; }
  void commit() { }
};

} // namespace policy_engine

#endif
