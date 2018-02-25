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

#include <stdio.h>

#include "validator_exception.h"
#include "soc_tag_configuration.h"
#include "rv32_validator.h"
#include "tag_file.h"
#include "metadata_memory_map.h"
#include "meta_cache.h"
#include "meta_set_factory.h"
#include "riscv_isa.h"

using namespace policy_engine;

static metadata_cache_t md_cache;
static meta_set_cache_t ms_cache;
static meta_set_factory_t *ms_factory;
static metadata_factory_t *md_factory;
static rv32_validator_t *rv_validator;
static metadata_memory_map_t *md_map;

extern std::string render_metadata(metadata_t const *metadata);

// so we can do debugging output with string representations of tags
extern void init_metadata_renderer(metadata_factory_t *md_factory);

// fake up the RV32 register values for the validator
static uint32_t regs[32];
static uint32_t reg_reader(uint32_t regno) { return regs[regno]; }

static void init(const char *policy_dir, const char *soc_cfg) {
  try {
    ms_factory = new meta_set_factory_t(&ms_cache, policy_dir);
    md_factory = new metadata_factory_t(policy_dir);
    init_metadata_renderer(md_factory);
    soc_tag_configuration_t *soc_config =
      new soc_tag_configuration_t(ms_factory, soc_cfg);
    rv_validator = new rv32_validator_t(&ms_cache, ms_factory, soc_config, reg_reader);
  } catch (exception_t &e) {
    printf("exception: %s\n", e.what().c_str());
  }
}

// names a register and a new value for that register
struct register_change_t {
  int regno;
  uint32_t new_value;
  register_change_t(int regno, uint32_t new_value) : regno(regno), new_value(new_value) { }
};

// An op is a PC value, the actual instruction bits, and whatever register changes
// we say that instruction should have.  Let's us hand code up a completely fake
// simulator.
struct op_t {
  uint32_t pc;
  uint32_t insn;
  std::vector<register_change_t> changes;
};

#define RA(v) register_change_t(1, (v))
#define SP(v) register_change_t(2, (v))
#define GP(v) register_change_t(3, (v))
#define TP(v) register_change_t(4, (v))
#define T0(v) register_change_t(5, (v))
#define T1(v) register_change_t(6, (v))
#define T2(v) register_change_t(7, (v))
#define S0(v) register_change_t(8, (v))
#define FP(v) S0(v)
#define S1(v) register_change_t(9, (v))
#define A0(v) register_change_t(10, (v))
#define A1(v) register_change_t(11, (v))
#define A2(v) register_change_t(12, (v))
#define A3(v) register_change_t(13, (v))
#define A4(v) register_change_t(14, (v))
#define A5(v) register_change_t(15, (v))
#define A6(v) register_change_t(16, (v))
#define A7(v) register_change_t(17, (v))


static std::vector<op_t> ops= {
  // two instructions - first is a load immediate to RA, and we lie and say it modifies
  // SP as well.  The next instruction is a store through SP, which will then trip a
  // policy violation because it's pointing at code, which is not marked writable.
  { 0x80000200, 0x00000093, { RA(0), SP(0x80000200) }},
  { 0x80000204, 0x00512023, {}},
};

// utility function to apply a named tag to an address range
static void apply_tag(address_t start, address_t end, const char *tag_name) {
  metadata_t const *metadata = md_factory->lookup_metadata(tag_name);
  if (!metadata) {
    printf("tag %s not found\n", tag_name);
  } else {
    md_map->add_range(start, end, metadata);
  }
}

// put group tags on the ops, and apply whatever other tags we
// want on the instructions, etc
static void tag_stuff() {
  for (auto &op: ops) {
    uint32_t rs1, rs2, rs3, rd;
    int32_t imm;
    const char *name;
    int32_t flags = decode(op.insn, &rs1, &rs2, &rs3, &rd, &imm, &name);
    metadata_t const *metadata = md_factory->lookup_group_metadata(name);
    if (!metadata) {
      printf("0x%08x: 0x%08x  %s - no group found for instruction\n", op.pc, op.insn, name);
    } else {
      md_map->add_range(op.pc, op.pc + 4, metadata);
    }

    // now we'll just stick an RWX tag on the instruction
    apply_tag(op.pc, op.pc + 4, "requires.dover.Kernel.Code.ElfSection.SHF_EXECINSTR");
  }

  rv_validator->apply_metadata(md_map);
}

static void usage() {
  printf("usage: standalone <generated policy directory> <SOC config YAML file>\n");
  printf("  e.g. ./standalone ../policy ../soc_cfg/miv_cfg.yml\n");
}

int main(int argc, char **argv) {
  if (argc != 3) {
    usage();
    return 0;
  }
  const char *policy_dir = argv[1];
  const char *soc_cfg = argv[2];
  // load up our YAML files, setup validator, etc
  init(policy_dir, soc_cfg);
  address_t base_address = 0x80000000; // FIXME - need to be able to query for this
  md_map = new metadata_memory_map_t(base_address, &md_cache);
 
  // set up the initial tag state
  tag_stuff();

  for (auto &op: ops) {
    tag_t ci_tag;

    // get the CI tag
    if (!rv_validator->get_tag(op.pc, ci_tag)) {
      printf("could not load tag for PC 0x%08x\n", op.pc);
    } else {
      // we can print the tag here
    }

#if 0
    // for debugging things
    metadata_t const *metadata = md_map->get_metadata(op.pc);
    if (!metadata) {
      printf("could not load metadata for PC 0x%08x\n", op.pc);
    } else {
      std::string s = render_metadata(metadata);
      printf("0x%08x: %s\n", op.pc, s.c_str());
    }
#endif

    // call the validator
    rv_validator->validate(op.pc, op.insn);

    // Apply changs to registers for the next instruction.
    // This is so that effective addresses can be calculated
    // by the validator to find tag addresses
    for (auto &rc: op.changes) {
      regs[rc.regno] = rc.new_value;
    }
  }
}
