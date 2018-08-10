#include "finite_rule_cache.h"

finite_rule_cache_t::finite_rule_cache_t(int capacity) : ideal_rule_cache_t() {
  this->capacity = capacity;
  entries = (operands_t**) malloc(sizeof(operands_t*) *capacity);
  next_entry = 0;
  entry_used = (bool*) malloc(sizeof(bool) *capacity);
  for (int i=0;i<capacity;i++){
    entry_used[i]=false;
  }
}

finite_rule_cache_t::~finite_rule_cache_t() {
  for (int i=0;i<capacity;i++) {
    delete entries[i];
  }
  delete entries;
  delete entry_used;
}

void finite_rule_cache_t::install_rule(operands_t *ops, results_t *res) {
  operands_t *ops_copy = new operands_t();
  ops_copy->pc = new meta_set_t{*ops->pc};
  ops_copy->ci = new meta_set_t{*ops->ci};
  if (ops->op1)
    ops_copy->op1 = new meta_set_t{*ops->op1};
  if (ops->op2)
    ops_copy->op2 = new meta_set_t{*ops->op2};
  if (ops->op3)
    ops_copy->op3 = new meta_set_t{*ops->op3};
  if (ops->mem)
    ops_copy->mem = new meta_set_t{*ops->mem};
  results_t *res_copy = new results_t{new meta_set_t{*res->pc},
                                      new meta_set_t{*res->rd},
                                      new meta_set_t{*res->csr},
                                      res->pcResult,
                                      res->rdResult, res->csrResult};

  if (entry_used[next_entry]==true) {
    auto existing_entry = rule_cache_table.find(*entries[next_entry]);
    if ((existing_entry==rule_cache_table.end())) {
      printf("failed to find operand\n");
    } else {
      rule_cache_table.erase(existing_entry);
    }
  }
  rule_cache_table.insert(std::make_pair(*ops_copy, *res_copy));
  /**printf("Table contents:\n");
  for (auto it = rule_cache_table.begin(); it != rule_cache_table.end(); it++) {
    printf("pc: %" PRIu32 ", ci: %" PRIu32, it->first.pc->tags[0], it->first.ci->tags[0]);
    if (it->first.op1) printf(", op1: %" PRIu32, it->first.op1->tags[0]);
    if (it->first.op2) printf(", op2: %" PRIu32, it->first.op2->tags[0]);
    if (it->first.op3) printf(", op3: %" PRIu32, it->first.op3->tags[0]);
    if (it->first.mem) printf(", mem: %" PRIu32, it->first.mem->tags[0]);
    printf("\n");
  }*/
  entries[next_entry]=ops_copy;
  entry_used[next_entry]=true;

  // circular buffer of things to remove
  next_entry++;
  if (next_entry>=capacity)
    next_entry=0;

  //delete ops_copy;
  delete res_copy;
}

bool finite_rule_cache_t::allow(operands_t *ops, results_t *res) {
  /**printf("Table contents:\n");
  for (auto it = rule_cache_table.begin(); it != rule_cache_table.end(); it++) {
    printf("pc: %" PRIu32 ", ci: %" PRIu32, it->first.pc->tags[0], it->first.ci->tags[0]);
    if (it->first.op1) printf(", op1: %" PRIu32, it->first.op1->tags[0]);
    if (it->first.op2) printf(", op2: %" PRIu32, it->first.op2->tags[0]);
    if (it->first.op3) printf(", op3: %" PRIu32, it->first.op3->tags[0]);
    if (it->first.mem) printf(", mem: %" PRIu32, it->first.mem->tags[0]);
    printf("\n");
  } 
  printf("Current instruction - pc: %" PRIu32 ", ci: %" PRIu32, ops->pc->tags[0], ops->ci->tags[0]);
  if (ops->op1) printf(", op1: %" PRIu32, ops->op1->tags[0]);
  if (ops->op2) printf(", op2: %" PRIu32, ops->op2->tags[0]);
  if (ops->op3) printf(", op3: %" PRIu32, ops->op3->tags[0]);
  if (ops->mem) printf(", mem: %" PRIu32, ops->mem->tags[0]);
  printf("\n");*/
  auto entries = rule_cache_table.find(*ops);
  if (!(entries == rule_cache_table.end())) {
    *res->pc = *entries->second.pc;
    *res->rd = *entries->second.rd;
    *res->csr = *entries->second.csr;
    res->pcResult = entries->second.pcResult;
    res->rdResult = entries->second.rdResult;
    res->csrResult = entries->second.csrResult;
    //printf("In table\n");
  } else {
    //printf("Not in table\n");
    /**printf("Table contents:\n");
    for (auto it = rule_cache_table.begin(); it != rule_cache_table.end(); it++) {
      printf("pc: %" PRIu32 ", ci: %" PRIu32, it->first.pc->tags[0], it->first.ci->tags[0]);
      if (it->first.op1) printf(", op1: %" PRIu32, it->first.op1->tags[0]);
      if (it->first.op2) printf(", op2: %" PRIu32, it->first.op2->tags[0]);
      if (it->first.op3) printf(", op3: %" PRIu32, it->first.op3->tags[0]);
      if (it->first.mem) printf(", mem: %" PRIu32, it->first.mem->tags[0]);
      printf("\n");
    } 
    printf("Current instruction - pc: %" PRIu32 ", ci: %" PRIu32, ops->pc->tags[0], ops->ci->tags[0]);
    if (ops->op1) printf(", op1: %" PRIu32, ops->op1->tags[0]);
    if (ops->op2) printf(", op2: %" PRIu32, ops->op2->tags[0]);
    if (ops->op3) printf(", op3: %" PRIu32, ops->op3->tags[0]);
    if (ops->mem) printf(", mem: %" PRIu32, ops->mem->tags[0]);
    printf("\n");*/
  }
  return !(entries == rule_cache_table.end());
}
