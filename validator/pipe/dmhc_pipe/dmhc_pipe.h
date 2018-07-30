#include <functional>
#include "base_pipe.h"
#include "dmhc.h"

#define OP_PC 0
#define OP_CI 1
#define OP_OP1 2
#define OP_OP2 3
#define OP_OP3 4
#define OP_MEM 5
#define RES_PC 0
#define RES_RD 1
#define RES_CSR 2
#define PC_RES 0
#define RD_RES 1
#define CSR_RES 2
#define OPS_LEN 6
#define RES_LEN 6

class dmhc_pipe_t : public pipe_t {

public:
  dmhc_pipe_t(int capacity, int iwidth, int owidth, int k, bool no_evict);
  ~dmhc_pipe_t();

  void install_rule(operands_t *ops, results_t *res);
  bool allow(operands_t *ops, results_t *res);
  void flush();

private:
  meta_set_t ops_copy[OPS_LEN];
  meta_set_t res_copy[RES_LEN];
  dmhc_t *the_pipe;
};
