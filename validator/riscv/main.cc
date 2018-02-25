#include <stdio.h>

#include "meta_cache.h"
#include "meta_set_factory.h"
#include "rv32_validator.h"
#include "metadata_memory_map.h"
#include "tag_file.h"
#include "validator_exception.h"

using namespace policy_engine;

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
				  std::string(getenv("GENERATED_POLICY_DIR")) + "/../soc_cfg/miv_cfg.yml");
    rv_validator = new rv32_validator_t(&ms_cache, ms_factory, soc_config, reg_reader);

    address_t base_address = 0x80000000; // FIXME - need to be able to query for this
    metadata_cache_t md_cache;
    metadata_memory_map_t map(base_address, &md_cache);
    std::string tags_file = std::string(getenv("GENERATED_POLICY_DIR")) + "/../application_tags.taginfo";
    if (!load_tags(&map, tags_file)) {
      printf("failed read\n");
    } else {
      rv_validator->apply_metadata(&map);
    }
  } catch (exception_t &e) {
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
