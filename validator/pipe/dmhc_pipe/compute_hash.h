#ifndef _COMPUTE_HASH_H
#define _COMPUTE_HASH_H

#define OP_PC 0
#define OP_CI 1
#define OP_OP1 2
#define OP_OP2 3
#define OP_OP3 4
#define OP_MEM 5
#define RES_PC 0
#define RES_RD 1
#define RES_CSR 2
#define PC_RES 3
#define RD_RES 4
#define CSR_RES 5
#define OPS_LEN 6
#define RES_LEN 6
#define BOOL_WIDTH 8

#include <map>
#include <vector>
#include "riscv_isa.h"
#include <inttypes.h>
//#define INIT_HASH_POSITIONS 1
#define HASH_HASH 1
//#define DMHC_DEBUG 1

void init_hashes();

struct compare_op {
  bool operator()(const operands_t &a, const operands_t &b) const {
    if (a.op1 && !b.op1) return false;
    if (b.op1 && !a.op1) return false;
    if (a.op2 && !b.op2) return false;
    if (b.op2 && !a.op2) return false;
    if (a.op3 && !b.op3) return false;
    if (b.op3 && !a.op3) return false;
    if (a.mem && !b.mem) return false;
    if (b.mem && !a.mem) return false;

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

class compute_hash_t {

 public:
  compute_hash_t(int num_fields, int *field_widths, 
	         int k, int capacity);
  ~compute_hash_t();
  void compute_hash_set_from_precomputed_positions(int k, meta_set_t *ops, int *hashes, bool *consider);
  void compute_hash_set(int k, meta_set_t *ops, int *hashes, int num_fields, 
                        int *field_widths, int capacity, bool *consider);

 private:
  int compute_hash_from_precomputed_positions(int which, meta_set_t *ops, bool *consider);
  int compute_hash(int which, int num_fields, int *field_widths, meta_set_t *fields, 
                   meta_set_t *permute_field, int hash_table_size, int ones_cnt);
  int fold(int num_fields, int *field_widths, meta_set_t *fields, int hash_table_size);
  void convert_to_bit_fields(int orig_num_fields, int *orig_field_widths, 
			     meta_set_t *orig_fields, int *field_widths, 
			     meta_set_t *fields, bool *consider);

  int num_fields;
  int * ops_index;
  int * bit_index;
  int ***hash_input_position;
  int ** sized_perm;
  int k_hash_position;
  int total_ops_bits;
  bool hash_positions_initialized;
  //operands_t *static_ops;
  std::map<operands_t, std::vector<int>, struct compare_op> hash_table;
};
#endif
