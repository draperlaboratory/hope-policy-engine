#include "dmhc_rule_cache.h"

dmhc_rule_cache_t::dmhc_rule_cache_t(int capacity, int iwidth, int owidth, int k, bool no_evict) {
  int ops_size[OPS_LEN];
  ops_size[OP_PC]=iwidth;
  ops_size[OP_CI]=iwidth;
  ops_size[OP_OP1]=iwidth;
  ops_size[OP_OP2]=iwidth;
  ops_size[OP_OP3]=iwidth;
  ops_size[OP_MEM]=iwidth;
  int res_size[RES_LEN];
  res_size[RES_PC]=owidth;
  res_size[RES_RD]=owidth;
  res_size[RES_CSR]=owidth;
  res_size[PC_RES]=owidth;
  res_size[RD_RES]=owidth;
  res_size[CSR_RES]=owidth;
  the_rule_cache = new dmhc_t(capacity, k, 2, OPS_LEN, RES_LEN, ops_size, res_size, no_evict);
  consider[OP_PC]=true;
  consider[OP_CI]=true;
  consider[OP_OP1]=false;
  consider[OP_OP2]=false;
  consider[OP_OP3]=false;
  consider[OP_MEM]=false;
}

dmhc_rule_cache_t::~dmhc_rule_cache_t() {
}

void dmhc_rule_cache_t::install_rule(operands_t *ops, results_t *res) {
#ifdef DMHC_DEBUG
  printf("Install\n");
  printf("ops - pc: %" PRIu32 ", ci: %" PRIu32, ops_copy[OP_PC].tags[0], ops_copy[OP_CI].tags[0]);
  if (consider[OP_OP1]) printf(", op1: %" PRIu32, ops_copy[OP_OP1].tags[0]);
  if (consider[OP_OP2]) printf(", op2: %" PRIu32, ops_copy[OP_OP2].tags[0]);
  if (consider[OP_OP3]) printf(", op3: %" PRIu32, ops_copy[OP_OP3].tags[0]);
  if (consider[OP_MEM]) printf(", mem: %" PRIu32, ops_copy[OP_MEM].tags[0]);
  printf("\n");
  printf("res - pc: %" PRIu32 ", rd: %" PRIu32 ", csr: %" PRIu32 ", pcRes: %" PRIu32 ", rdRes: %"
         PRIu32 ", csrRes: %" PRIu32 "\n", res_copy[RES_PC].tags[0], res_copy[RES_RD].tags[0], 
         res_copy[RES_CSR].tags[0], res_copy[PC_RES].tags[0], res_copy[RD_RES].tags[0], 
         res_copy[CSR_RES].tags[0]);
#endif
  the_rule_cache->insert(ops_copy, res_copy, consider);
}

bool dmhc_rule_cache_t::allow(operands_t *ops, results_t *res) {
  ops_copy[OP_PC]=*ops->pc;
  ops_copy[OP_CI]=*ops->ci;
  if (ops->op1) {
    ops_copy[OP_OP1]=*ops->op1;
    consider[OP_OP1]=true;
  } else {
    consider[OP_OP1]=false;
  }  
  if (ops->op2) {
    ops_copy[OP_OP2]=*ops->op2;
    consider[OP_OP2]=true;
  } else {
    consider[OP_OP2]=false;
  }  
  if (ops->op3) {
    ops_copy[OP_OP3]=*ops->op3;
    consider[OP_OP3]=true;
  }
  else {
    consider[OP_OP3]=false;
  }
  if (ops->mem) {
    ops_copy[OP_MEM]=*ops->mem;
    consider[OP_MEM]=true;
  }
  else {
    consider[OP_MEM]=false;
  }

  res_copy[RES_PC]=*res->pc;
  res_copy[RES_RD]=*res->rd;
  res_copy[RES_CSR]=*res->csr;
  meta_set_t *pc_res=new meta_set_t();
  pc_res->tags[0]=res->pcResult;
  res_copy[PC_RES]=*pc_res;
  meta_set_t *rd_res=new meta_set_t();
  rd_res->tags[0]=res->rdResult;
  res_copy[RD_RES]=*rd_res;
  meta_set_t *csr_res=new meta_set_t();
  csr_res->tags[0]=res->csrResult;
  res_copy[CSR_RES]=*csr_res;

#ifdef DMHC_DEBUG
  printf("Allow\nops - pc: %" PRIu32 ", ci: %" PRIu32, ops_copy[OP_PC].tags[0], ops_copy[OP_CI].tags[0]);
  if (ops->op1) printf(", op1: %" PRIu32, ops_copy[OP_OP1].tags[0]);
  if (ops->op2) printf(", op2: %" PRIu32, ops_copy[OP_OP2].tags[0]);
  if (ops->op3) printf(", op3: %" PRIu32, ops_copy[OP_OP3].tags[0]);
  if (ops->mem) printf(", mem: %" PRIu32, ops_copy[OP_MEM].tags[0]);
  printf("\n");

  printf("res - pc: %" PRIu32 ", rd: %" PRIu32 ", csr: %" PRIu32 ", pcRes: %" PRIu32 ", rdRes: %"
         PRIu32 ", csrRes: %" PRIu32 "\n", res_copy[RES_PC].tags[0], res_copy[RES_RD].tags[0], 
         res_copy[RES_CSR].tags[0], res_copy[PC_RES].tags[0], res_copy[RD_RES].tags[0], 
         res_copy[CSR_RES].tags[0]);
#endif

  delete pc_res, rd_res, csr_res;

  bool found=the_rule_cache->lookup(ops_copy, res_copy, consider);
  if (found == false) {
#ifdef DMHC_DEBUG
    printf("Not found\n");
#endif
    return false;
  } else {
#ifdef DMHC_DEBUG
    printf("Found\n");
#endif
    res->pc = new meta_set_t{res_copy[RES_PC]};
    res->rd = new meta_set_t{res_copy[RES_RD]};
    res->csr = new meta_set_t{res_copy[RES_CSR]};
    res->pcResult = res_copy[PC_RES].tags[0];
    res->rdResult = res_copy[RD_RES].tags[0];
    res->csrResult = res_copy[CSR_RES].tags[0];
    return true;
  }
}

void dmhc_rule_cache_t::flush() {
  the_rule_cache->reset();
}
