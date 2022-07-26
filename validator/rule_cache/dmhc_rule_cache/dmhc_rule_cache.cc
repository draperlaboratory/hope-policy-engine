#include "dmhc_rule_cache.h"
#include "meta_cache.h"
#include "riscv_isa.h"

namespace policy_engine {

dmhc_rule_cache_t::dmhc_rule_cache_t(int capacity, int iwidth, int owidth, int k, bool no_evict, meta_set_cache_t* cache) {
  ms_cache = cache;

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
  the_rule_cache = new dmhc_t(capacity, k, 2, OPS_LEN, RES_LEN, ops_size, res_size, no_evict, cache);
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
  printf("ops - pc: %" PRIu32 ", ci: %" PRIu32, ops_copy[OP_PC].tags[0], ops_copy[OP_CI].tags[0]);
  if (consider[OP_OP1]) printf(", op1: %" PRIu32, ops_copy[OP_OP1].tags[0]);
  if (consider[OP_OP2]) printf(", op2: %" PRIu32, ops_copy[OP_OP2].tags[0]);
  if (consider[OP_OP3]) printf(", op3: %" PRIu32, ops_copy[OP_OP3].tags[0]);
  if (consider[OP_MEM]) printf(", mem: %" PRIu32, ops_copy[OP_MEM].tags[0]);
  printf("\n");
  printf("res - pc: %" PRItag ", rd: %" PRItag ", csr: %" PRItag ", pcRes: %" PRId32 ", rdRes: %"
         PRIu32 ", csrRes: %" PRIu32 "\n", res_copy.pc, res_copy.rd,  res_copy.csr,
         res_copy.pcResult, res_copy.rdResult,  res_copy.csrResult);
#endif
  the_rule_cache->insert(ops_copy, res_copy, consider);
}

bool dmhc_rule_cache_t::allow(const operands_t& ops, results_t& res) {
  ops_copy[OP_PC] = (*ms_cache)[ops.pc];
  ops_copy[OP_CI] = (*ms_cache)[ops.ci];
  if (ops.op1) {
    ops_copy[OP_OP1] = (*ms_cache)[ops.op1];
    consider[OP_OP1] = true;
  } else {
    consider[OP_OP1] = false;
  }  
  if (ops.op2) {
    ops_copy[OP_OP2] = (*ms_cache)[ops.op2];
    consider[OP_OP2] = true;
  } else {
    consider[OP_OP2] = false;
  }  
  if (ops.op3) {
    ops_copy[OP_OP3] = (*ms_cache)[ops.op3];
    consider[OP_OP3] = true;
  } else {
    consider[OP_OP3] = false;
  }
  if (ops.mem) {
    ops_copy[OP_MEM] = (*ms_cache)[ops.mem];
    consider[OP_MEM] = true;
  }
  else {
    consider[OP_MEM] = false;
  }

#ifdef DMHC_DEBUG
  printf("Allow\nops - pc: %" PRIu32 ", ci: %" PRIu32, ops_copy[OP_PC].tags[0], ops_copy[OP_CI].tags[0]);
  if (ops->op1) printf(", op1: %" PRIu32, ops_copy[OP_OP1].tags[0]);
  if (ops->op2) printf(", op2: %" PRIu32, ops_copy[OP_OP2].tags[0]);
  if (ops->op3) printf(", op3: %" PRIu32, ops_copy[OP_OP3].tags[0]);
  if (ops->mem) printf(", mem: %" PRIu32, ops_copy[OP_MEM].tags[0]);
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