#include <stdio.h>

#include "validator_exception.h"
#include "soc_tag_configuration.h"
#include "rv32_validator.h"
#include "tag_file.h"
#include "metadata_memory_map.h"
#include "meta_cache.h"
#include "meta_set_factory.h"

using namespace policy_engine;

meta_set_cache_t ms_cache;
meta_set_factory_t *ms_factory;
metadata_factory_t *md_factory;
rv32_validator_t *rv_validator;

static uint32_t regs[32];

uint32_t reg_reader(uint32_t regno) { return regs[regno]; }

extern void init_metadata_renderer(metadata_factory_t *md_factory);

void init() {
  try {
    ms_factory = new meta_set_factory_t(&ms_cache, getenv("GENERATED_POLICY_DIR"));
    md_factory = new metadata_factory_t(getenv("GENERATED_POLICY_DIR"));
    init_metadata_renderer(md_factory);
    soc_tag_configuration_t *soc_config =
      new soc_tag_configuration_t(ms_factory,
				  std::string(getenv("GENERATED_POLICY_DIR")) + "/../soc_cfg/miv_cfg.yml");
    rv_validator = new rv32_validator_t(&ms_cache, ms_factory, soc_config, reg_reader);
  } catch (validator::exception_t &e) {
    printf("exception: %s\n", e.what().c_str());
  }
}

struct register_change_t {
  int regno;
  uint32_t new_value;
  register_change_t(int regno, uint32_t new_value) : regno(regno), new_value(new_value) { }
};

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


std::vector<op_t> ops= {
  { 0x80000200, 0x00000093, { RA(0), SP(0x80000200) }},
  { 0x80000204, 0x00512023, {}},
};

extern std::string render_metadata(metadata_t const *metadata);

int main(int argc, char **argv) {
  init();
  address_t base_address = 0x80000000; // FIXME - need to be able to query for this
  metadata_cache_t md_cache;
  metadata_memory_map_t map(base_address, &md_cache);
  if (!load_tags(&map, argv[1])) {
    printf("failed read\n");
  } else {
    rv_validator->apply_metadata(&map);
  }
  
  for (auto &op: ops) {
    tag_t ci_tag;
    if (!rv_validator->get_tag(op.pc, ci_tag)) {
      printf("could not load tag for PC 0x%08x\n", op.pc);
    } else {
      // we can print the tag here
    }
#if 1
    metadata_t const *metadata = map.get_metadata(op.pc);
    if (!metadata) {
      printf("could not load metadata for PC 0x%08x\n", op.pc);
    } else {
      std::string s = render_metadata(metadata);
      printf("0x%08x: %s\n", op.pc, s.c_str());
    }
#endif
    rv_validator->validate(op.pc, op.insn);
    for (auto &rc: op.changes) {
      regs[rc.regno] = rc.new_value;
    }
  }
}
