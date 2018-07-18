#include <map>
#include "riscv_isa.h"

struct compare_ops {
  bool operator()(const operands_t &a, const operands_t &b) const {
    if (a.pc->tags[0]<b.pc->tags[0]) return true;
    else if (a.pc->tags[0]>b.pc->tags[0]) return false;

    if (a.ci->tags[0]<b.ci->tags[0]) return true;
    else if (a.ci->tags[0]>b.ci->tags[0]) return false;

    if (a.op1->tags[0]<b.op1->tags[0]) return true;
    else if (a.op1->tags[0]>b.op1->tags[0]) return false;

    if (a.op2->tags[0]<b.op2->tags[0]) return true;
    else if (a.op2->tags[0]>b.op2->tags[0]) return false;

    if (a.op3->tags[0]<b.op3->tags[0]) return true;
    else if (a.op3->tags[0]>b.op3->tags[0]) return false;

    if (a.mem->tags[0]<b.mem->tags[0]) return true;
    else if (a.mem->tags[0]>b.mem->tags[0]) return false;

    return false;
  }
};

class ideal_pipe_t 
{

public:

  ideal_pipe_t();
  ~ideal_pipe_t();
  
  void install_rule(operands_t *ops, results_t *res);
  bool allow(operands_t *ops, results_t *res);

private:
  std::map<operands_t, results_t, compare_ops> pump_table;
};
