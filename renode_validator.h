#ifndef RENODE_VALIDATOR_H
#define RENODE_VALIDATOR_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef uint32_t (*RegisterReader_t)(uint32_t);
typedef uint32_t (*MemoryReader_t)(uint32_t);

void e_v_set_callbacks(RegisterReader_t reg_reader, MemoryReader_t mem_reader);
uint32_t e_v_validate(uint32_t pc, uint32_t instr);
void e_v_commit();
  
#ifdef __cplusplus
}
#endif

#endif
