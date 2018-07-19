#include <map>
#include "riscv_isa.h"

struct compare_ops {
  bool operator()(const operands_t &a, const operands_t &b) const {
    if (a.op1 && b.op1) {
      if (a.op1->tags[0]!=b.op1->tags[0])
        return false;
    }
    if (a.op2 && b.op2) {
      if (a.op2->tags[0]!=b.op2->tags[0])
        return false;
    }
    if (a.op3 && b.op3) {
      if (a.op3->tags[0]!=b.op3->tags[0])
        return false;
    }
    if (a.mem && b.mem) {
      if (a.mem->tags[0]!=b.mem->tags[0])
        return false;
    }
    if ((a.pc->tags[0]==b.pc->tags[0]) && 
      (a.ci->tags[0]==b.ci->tags[0]))
      return true;

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
