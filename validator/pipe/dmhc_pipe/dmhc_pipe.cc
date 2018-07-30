#include "dmhc_pipe.h"

dmhc_pipe_t::dmhc_pipe_t(int capacity, int iwidth, int owidth, int k, bool no_evict) {
  the_pipe = new dmhc_t(capacity, k, 2, OPS_LEN, RES_LEN, no_evict);
}

dmhc_pipe_t::~dmhc_pipe_t() {
}

void dmhc_pipe_t::install_rule(operands_t *ops, results_t *res) {
}

bool dmhc_pipe_t::allow(operands_t *ops, results_t *res) {
  return false;
}

void dmhc_pipe_t::flush() {
  the_pipe->reset();
}
