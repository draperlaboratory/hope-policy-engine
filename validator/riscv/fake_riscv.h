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

#ifndef FAKE_RISCV_H
#define FAKE_RISCV_H

#include <string.h>
#include <stdint.h>
#include <vector>

#include "metadata_factory.h"
#include "metadata_memory_map.h"

namespace policy_engine {

/**
   Fake RISCV machine (32bit).  Used for manually feeding instructions to test apps.
 */
class fake_riscv_t {
// names a register and a new value for that register
public:
  struct register_change_t {
    int regno;
    uint32_t new_value;
    register_change_t(int regno, uint32_t new_value) : regno(regno), new_value(new_value) { }
  };

  // An op is a PC value, the actual instruction bits, and whatever register changes
  // we say that instruction should have.  Let's us hand code up a completely fake
  // simulator.
  struct op_t {
    address_t pc;
    uint32_t insn;
    std::vector<register_change_t> changes;
  };

private:
  reg_t regs[32];
  int current_op;
  std::vector<op_t> ops;

public:
  fake_riscv_t() { reset(); }
  fake_riscv_t(std::vector<op_t> ops) : ops(ops) { reset(); }
  
  void set_ops(std::vector<op_t> o) { ops = o; reset(); }
  
  void reset() {
    current_op = 0;
    memset(regs, 0, sizeof(regs));
  }
  
  reg_t read_register(uint32_t regno) {
    return regs[regno];
  }
  
  bool step();
  
  address_t get_pc() const {
    if (current_op == ops.size())
      return -1;
    return ops[current_op].pc;
  }
  
  uint32_t get_insn() const {
    if (current_op == ops.size())
      return -1;
    return ops[current_op].insn;
  }

  void apply_group_tags(metadata_factory_t *md_factory, metadata_memory_map_t *md_map);
  void apply_tag(metadata_factory_t *md_factory, metadata_memory_map_t *md_map, const char *tag_name);
};

#ifdef DEFINE_REGISTER_CHANGE_MACROS
#define RA(v) fake_riscv_t::register_change_t(1, (v))
#define SP(v) fake_riscv_t::register_change_t(2, (v))
#define GP(v) fake_riscv_t::register_change_t(3, (v))
#define TP(v) fake_riscv_t::register_change_t(4, (v))
#define T0(v) fake_riscv_t::register_change_t(5, (v))
#define T1(v) fake_riscv_t::register_change_t(6, (v))
#define T2(v) fake_riscv_t::register_change_t(7, (v))
#define S0(v) fake_riscv_t::register_change_t(8, (v))
#define FP(v) S0(v)
#define S1(v) fake_riscv_t::register_change_t(9, (v))
#define A0(v) fake_riscv_t::register_change_t(10, (v))
#define A1(v) fake_riscv_t::register_change_t(11, (v))
#define A2(v) fake_riscv_t::register_change_t(12, (v))
#define A3(v) fake_riscv_t::register_change_t(13, (v))
#define A4(v) fake_riscv_t::register_change_t(14, (v))
#define A5(v) fake_riscv_t::register_change_t(15, (v))
#define A6(v) fake_riscv_t::register_change_t(16, (v))
#define A7(v) fake_riscv_t::register_change_t(17, (v))
#endif // DEFINE_REGISTER_CHANGE_MACROS

} // namespace policy_engine

#endif
