/*
 * Copyright © 2017-2018 Dover Microsystems, Inc.
 * All rights reserved. 
 *
 * Use and disclosure subject to the following license. 
 *
 * Permission is hereby granted, free of charge, to any person obtaining
 * a copy of this software and associated documentation files (the
 * "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, sublicense, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject to
 * the following conditions:
 * 
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE
 * LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION
 * OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
 * WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

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
  virtual bool commit() = 0;
};

/**
   A simple implementation of the abstract validator that always returns
   <code>true</code>.  Using this validator is the simplest functional equivalent
   of a NOP.  Instructions will always be permitted to execute.
*/
class always_ok_validator_t final : public abstract_validator_t {
  public:
  bool validate(address_t pc, insn_bits_t insn) { return true; }
  // Return false means no watchpoint hit
  bool commit() { return false; }
};

} // namespace policy_engine

#endif
