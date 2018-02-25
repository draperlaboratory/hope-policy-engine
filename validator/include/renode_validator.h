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

#ifndef RENODE_VALIDATOR_H
#define RENODE_VALIDATOR_H

#include "validator.h"
#include "renode_interface.h"

namespace policy_engine {

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
//  MemoryReader_t mem_reader;
  public:
//  abstract_renode_validator_t(RegisterReader_t rr, MemoryReader_t mr) : reg_reader(rr), mem_reader(mr) {
  abstract_renode_validator_t(RegisterReader_t rr) : reg_reader(rr) {
  }
  virtual ~abstract_renode_validator_t() { }
  virtual bool validate(address_t pc, insn_bits_t insn) = 0;
  virtual void commit() = 0;
};


} // namespace policy_engine

#endif
