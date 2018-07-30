#include "dmhc_pipe.h"

dmhc_pipe_t::dmhc_pipe_t(int capacity, int iwidth, int owidth, int k, bool no_evict) {
  for (meta_set_t *meta_set = ops; meta_set < ops + OPS_LEN; meta_set++) {
    meta_set = new meta_set_t();
    meta_set->tags[0] = iwidth;
  }
  for (meta_set_t *meta_set = res; meta_set < res + RES_LEN; meta_set++) {
    meta_set = new meta_set_t();
    meta_set->tags[0] = owidth;
  }
  the_pipe = new dmhc_t(capacity, k, 2, 9, 2, ops, res, no_evict);
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
