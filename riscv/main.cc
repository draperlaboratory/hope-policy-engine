#include <stdio.h>

#include "meta_cache.h"
#include "meta_set_factory.h"
#include "rv32_validator.h"
#include "validator_exception.h"

meta_set_cache_t ms_cache;
meta_set_factory_t *ms_factory;
rv32_validator_t *rv_validator;

static bool DOA = false;

extern "C" void e_v_set_callbacks(RegisterReader_t reg_reader, MemoryReader_t mem_reader) {
  try {
    printf("setting callbacks\n");
    ms_factory = new meta_set_factory_t(&ms_cache, getenv("GENERATED_POLICY_DIR"));
    soc_tag_configuration_t *soc_config =
      new soc_tag_configuration_t(ms_factory,
				  std::string(getenv("GENERATED_POLICY_DIR")) + "/../soc_cfg.yml");
    rv_validator = new rv32_validator_t(&ms_cache, ms_factory, soc_config, reg_reader);
  } catch (validator::exception_t &e) {
    printf("validator exception %s while setting callbacks - policy code DOA\n", e.what().c_str());
    DOA = true;
  } catch (std::exception &e) {
    printf("c++ exception %s while setting callbacks - policy code DOA\n", e.what());
    DOA = true;
  } catch (...) {
    printf("c++ exception while setting callbacks - policy code DOA\n");
    DOA = true;
  }
}

extern "C" uint32_t e_v_validate(uint32_t pc, uint32_t instr) {
//  printf("validating 0x%x: 0x%x\n", pc, instr);
  if (!DOA) {
    try {
      return rv_validator->validate(pc, instr);
    } catch (...) {
      printf("c++ exception while validating - policy code DOA\n");
      DOA = true;
    }
  }
  return 0;
}

extern "C" void e_v_commit() {
//  printf("committing\n");
  if (!DOA) {
    try {
      rv_validator->commit();
    } catch (...) {
      printf("c++ exception while commiting - policy code DOA\n");
      DOA = true;
    }
  }
}
