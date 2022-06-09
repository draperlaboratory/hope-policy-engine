#include "riscv_isa.h"
#include "fake_riscv.h"

using namespace policy_engine;

void fake_riscv_t::apply_group_tags(metadata_factory_t *md_factory, metadata_memory_map_t *md_map) {
  for (auto &op: ops) {
    decoded_instruction_t inst = decode(op.insn);
    const metadata_t* metadata = md_factory->lookup_group_metadata(inst.name, inst);
    if (!metadata) {
      printf("0x%" PRIaddr_pad
             ": 0x%08x  %s - no group found for instruction\n",
             op.pc, op.insn, inst.name.c_str());
    } else {
      md_map->add_range(op.pc, op.pc + 4, *metadata);
    }
  }
}

void fake_riscv_t::apply_tag(metadata_factory_t *md_factory,
			     metadata_memory_map_t *md_map,
			     const char *tag_name) {
  const metadata_t* metadata = md_factory->lookup_metadata(tag_name);
  if (!metadata) {
    printf("tag name %s not found\n", tag_name);
    return;
  }
  
  for (auto &op: ops)
    md_map->add_range(op.pc, op.pc + 4, *metadata);
}

bool fake_riscv_t::step() {
  for (auto &rc: ops[current_op].changes) {
    regs[rc.regno] = rc.new_value;
  }
  
  current_op++;
  
  if (current_op == ops.size())
    return false;
  
  return true;
}
