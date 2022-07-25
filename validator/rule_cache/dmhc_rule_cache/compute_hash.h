#ifndef _COMPUTE_HASH_H
#define _COMPUTE_HASH_H

#include <cinttypes>
#include <unordered_map>
#include <vector>
#include "meta_cache.h"
#include "riscv_isa.h"

//#define INIT_HASH_POSITIONS 1
#define HASH_HASH 1
//#define DMHC_DEBUG 1

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

namespace policy_engine {

void init_hashes();

class compute_hash_t {

 public:
  compute_hash_t(int num_fields, int* field_widths,  int k, int capacity, meta_set_cache_t* cache);
  ~compute_hash_t();
  void compute_hash_set_from_precomputed_positions(int k, meta_set_t* ops, int* hashes, bool* consider);
  void compute_hash_set(int k, meta_set_t* ops, int* hashes, int num_fields,  int* field_widths, int capacity, bool* consider);

 private:
  int compute_hash_from_precomputed_positions(int which, meta_set_t* ops, bool* consider);
  int compute_hash(int which, int num_fields, int* field_widths, meta_set_t* fields,  meta_set_t* permute_field, int hash_table_size, int ones_cnt);
  int fold(int num_fields, int* field_widths, meta_set_t* fields, int hash_table_size);
  void convert_to_bit_fields(int orig_num_fields, int* orig_field_widths,  meta_set_t* orig_fields, int* field_widths,  meta_set_t* fields, bool* consider);

  meta_set_cache_t* ms_cache;

  int num_fields;
  int* ops_index;
  int* bit_index;
  int*** hash_input_position;
  int** sized_perm;
  int k_hash_position;
  int total_ops_bits;
  bool hash_positions_initialized;
  //operands_t *static_ops;
  std::unordered_map<operands_t, std::vector<int>> hash_table;
};

} // namespace policy_engine

#endif
