#include <stdio.h>

#include "validator_exception.h"
#include "soc_tag_configuration.h"
#include "rv32_validator.h"

meta_set_cache_t ms_cache;
meta_set_factory_t *ms_factory;
rv32_validator_t *rv_validator;

static uint32_t regs[32];

uint32_t reg_reader(uint32_t regno) { return regs[regno]; }

void init() {
  try {
    ms_factory = new meta_set_factory_t(&ms_cache, getenv("GENERATED_POLICY_DIR"));
    soc_tag_configuration_t *soc_config =
      new soc_tag_configuration_t(ms_factory,
				  std::string(getenv("GENERATED_POLICY_DIR")) + "/../soc_cfg.yml");
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

std::vector<op_t> ops= {
  { 0x80000200, 0x00000093, { RA(0) }}
};

int main() {
  init();
  for (auto &op: ops) {
    rv_validator->validate(op.pc, op.insn);
    for (auto &rc: op.changes) {
      regs[rc.regno] = rc.new_value;
    }
  }
}
