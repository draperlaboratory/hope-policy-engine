#include "riscv_isa.h"
#include "fake_riscv.h"

using namespace policy_engine;

void fake_riscv_t::apply_group_tags(metadata_factory_t *md_factory, metadata_memory_map_t *md_map) {
  for (auto &op: ops) {
    uint32_t rs1, rs2, rs3, rd;
    int32_t imm;
    const char *name;
	uint32_t opdef;
    int32_t flags = decode(op.insn, &rs1, &rs2, &rs3, &rd, &imm, &name, &opdef);
    metadata_t const *metadata = md_factory->lookup_group_metadata(name);
    if (!metadata) {
      printf("0x%08x: 0x%08x  %s - no group found for instruction\n", op.pc, op.insn, name);
    } else {
      md_map->add_range(op.pc, op.pc + 4, metadata);
    }
  }
}

void fake_riscv_t::apply_tag(metadata_factory_t *md_factory,
			     metadata_memory_map_t *md_map,
			     const char *tag_name) {
  metadata_t const *metadata = md_factory->lookup_metadata(tag_name);
  if (!metadata) {
    printf("tag name %s not found\n", tag_name);
    return;
  }
  
  for (auto &op: ops)
    md_map->add_range(op.pc, op.pc + 4, metadata);
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
