#include "dmhc_rule_cache.h"
#include "meta_cache.h"
#include "riscv_isa.h"

namespace policy_engine {

dmhc_rule_cache_t::dmhc_rule_cache_t(int capacity, int iwidth, int owidth, int k, bool no_evict) {
  int ops_size[OPS_LEN];
  ops_size[OP_PC] = iwidth;
  ops_size[OP_CI] = iwidth;
  ops_size[OP_OP1] = iwidth;
  ops_size[OP_OP2] = iwidth;
  ops_size[OP_OP3] = iwidth;
  ops_size[OP_MEM] = iwidth;
  int res_size[RES_LEN];
  res_size[RES_PC] = owidth;
  res_size[RES_RD] = owidth;
  res_size[RES_CSR] = owidth;
  res_size[PC_RES] = owidth;
  res_size[RD_RES] = owidth;
  res_size[CSR_RES] = owidth;
  the_rule_cache = new dmhc_t(capacity, k, 2, OPS_LEN, RES_LEN, ops_size, res_size, no_evict);
  consider[OP_PC] = true;
  consider[OP_CI] = true;
  consider[OP_OP1] = false;
  consider[OP_OP2] = false;
  consider[OP_OP3] = false;
  consider[OP_MEM] = false;
}

void dmhc_rule_cache_t::install_rule(const operands_t& ops, const results_t& res) {
  res_copy = res;
#ifdef DMHC_DEBUG
  printf("Install\n");
  printf("ops - pc: %" PRItag ", ci: %" PRItag, ops_copy.pc, ops_copy.ci);
  if (consider[OP_OP1]) printf(", op1: %" PRItag, ops_copy.op1);
  if (consider[OP_OP2]) printf(", op2: %" PRItag, ops_copy.op2);
  if (consider[OP_OP3]) printf(", op3: %" PRItag, ops_copy.op3);
  if (consider[OP_MEM]) printf(", mem: %" PRItag, ops_copy.mem);
  printf("\n");
  printf("res - pc: %" PRItag ", rd: %" PRItag ", csr: %" PRItag ", pcRes: %" PRId32 ", rdRes: %"
         PRIu32 ", csrRes: %" PRIu32 "\n", res_copy.pc, res_copy.rd,  res_copy.csr,
         res_copy.pcResult, res_copy.rdResult,  res_copy.csrResult);
#endif
  the_rule_cache->insert(ops_copy, res_copy, consider);
}

bool dmhc_rule_cache_t::allow(const operands_t& ops, results_t& res) {
  ops_copy = ops;
  consider[OP_OP1] = ops.op1 != BAD_TAG_VALUE;
  consider[OP_OP2] = ops.op2 != BAD_TAG_VALUE;
  consider[OP_OP3] = ops.op3 != BAD_TAG_VALUE;
  consider[OP_MEM] = ops.mem != BAD_TAG_VALUE;

#ifdef DMHC_DEBUG
  printf("Allow\nops - pc: %" PRItag ", ci: %" PRItag, ops_copy.pc, ops_copy.ci);
  if (ops->op1) printf(", op1: %" PRItag, ops_copy.op1);
  if (ops->op2) printf(", op2: %" PRItag, ops_copy.op2);
  if (ops->op3) printf(", op3: %" PRItag, ops_copy.op3);
  if (ops->mem) printf(", mem: %" PRItag, ops_copy.mem);
  printf("\n");
#endif

  bool found = the_rule_cache->lookup(ops_copy, res_copy, consider);
  if (found == false) {
#ifdef DMHC_DEBUG
    printf("Not found\n");
#endif
    return false;
  } else {
#ifdef DMHC_DEBUG
    printf("Found\n");
#endif
    res = res_copy;
    return true;
  }
}

void dmhc_rule_cache_t::flush() {
  the_rule_cache->reset();
}

} // namespace policy_engine