#ifndef __BASE_PIPE_H__
#define __BASE_PIPE_H__

#include "riscv_isa.h"

typedef struct pipe_operands {
  operands_t *ops;

  bool operator<(const struct pipe_operands &b) const {
    if (this->ops->pc->tags[0]<b.ops->pc->tags[0]) return true;
    if (this->ops->pc->tags[0]>b.ops->pc->tags[0]) return false;
    if (this->ops->ci->tags[0]<b.ops->ci->tags[0]) return true;
    if (this->ops->ci->tags[0]>b.ops->ci->tags[0]) return false;

    if (b.ops->op1 && !this->ops->op1) return true;
    if (this->ops->op1 && !b.ops->op1) return false;
    if (b.ops->op2 && !this->ops->op2) return true;
    if (this->ops->op2 && !b.ops->op2) return false;
    if (b.ops->op3 && !this->ops->op3) return true;
    if (this->ops->op3 && !b.ops->op3) return false;
    if (b.ops->mem && !this->ops->mem) return true;
    if (this->ops->mem && !b.ops->mem) return false;

    if (this->ops->op1 && b.ops->op1) {
      if (this->ops->op1->tags[0]<b.ops->op1->tags[0]) return true;
      if (this->ops->op1->tags[0]>b.ops->op1->tags[0]) return false;
    }
    if (this->ops->op2 && b.ops->op2) {
      if (this->ops->op2->tags[0]<b.ops->op2->tags[0]) return true;
      if (this->ops->op2->tags[0]>b.ops->op2->tags[0]) return false;
    }
    if (this->ops->op3 && b.ops->op3) {
      if (this->ops->op3->tags[0]<b.ops->op3->tags[0]) return true;
      if (this->ops->op3->tags[0]>b.ops->op3->tags[0]) return false;
    }
    if (this->ops->mem && b.ops->mem) {
      if (this->ops->mem->tags[0]<b.ops->mem->tags[0]) return true;
      if (this->ops->mem->tags[0]>b.ops->mem->tags[0]) return false;
    }
    
    return false;
  }
} pipe_operands_t;

class pipe_t {
public:
  virtual void install_rule(operands_t *ops, results_t *res) = 0;
  virtual bool allow(operands_t *ops, results_t *res) = 0;
  int instruction_count; //For debugging
};

#endif// __BASE_PIPE_H__
