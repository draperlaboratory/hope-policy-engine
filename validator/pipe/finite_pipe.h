#include <unordered_map>
#include <functional>
#include "base_pipe.h"

class finite_pipe_t : public pipe_t
{

public:

  finite_pipe_t(int capacity);
  ~finite_pipe_t();

  void install_rule(operands_t *ops, results_t *res);
  bool allow(operands_t *ops, results_t *res);

private:
  int capacity;
  operands_t **entries;
  bool *entry_used;
  int next_entry;
  std::unordered_map<operands_t, results_t, std::hash<operands_t>, compare_ops> pipe_table;
};
