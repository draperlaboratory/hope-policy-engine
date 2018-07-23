#include <map>
#include "base_pipe.h"

class ideal_pipe_t : public pipe_t
{

public:

  ideal_pipe_t();
  ~ideal_pipe_t();

  void install_rule(operands_t *ops, results_t *res);
  bool allow(operands_t *ops, results_t *res);

private:
  std::map<operands_t, results_t, compare_ops> pipe_table;
};
