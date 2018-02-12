#include <stdio.h>

#include "rv32_validator.h"

rv32_validator_t *validator;

static bool DOA = false;

extern "C" void e_v_set_callbacks(RegisterReader_t reg_reader, MemoryReader_t mem_reader) {
  try {
    printf("setting callbacks\n");
    validator = new rv32_validator_t(getenv("GENERATED_POLICY_DIR"),
				     "soc_conf.yml",
				     reg_reader);
  } catch (...) {
    printf("c++ exception while setting callbacks - policy code DOA\n");
    DOA = true;
  }
}

extern "C" uint32_t e_v_validate(uint32_t pc, uint32_t instr) {
//  printf("validating 0x%x: 0x%x\n", pc, instr);
  if (!DOA) {
    try {
      return validator->validate(pc, instr);
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
      validator->commit();
    } catch (...) {
      printf("c++ exception while commiting - policy code DOA\n");
      DOA = true;
    }
  }
}
