#include <stdio.h>

#include "rv32_validator.h"

rv32_validator_t *validator;

extern "C" void e_v_set_callbacks(RegisterReader_t reg_reader, MemoryReader_t mem_reader) {
  printf("setting callbacks\n");
  validator = new rv32_validator_t(reg_reader, mem_reader);
}

extern "C" uint32_t e_v_validate(uint32_t pc, uint32_t instr) {
//  printf("validating 0x%x: 0x%x\n", pc, instr);
  return validator->validate(pc, instr);
//  return 1;
}

extern "C" void e_v_commit() {
//  printf("committing\n");
  validator->commit();
}
