#ifndef __DMHC_H__
#define __DMHC_H__

#include "compute_hash.h"
#include "meta_cache.h"
#include "riscv_isa.h"

#define GTABLE_MAX_COUNT 3
#define HOP_LIMIT 1

#define INVALID_LAST_USER 0x0

//#define VERIFY_PRECOMPUTE_HASH 1
//Q: how often do we get a direct conflict in all hashes? -- it should be extremely rare
//   will only catch conflicts with things in the dMHC...
#define DMHC_DIRECT_K_HASH_CONFLICTS 1
// this one is much cheaper...but there are rare cases where it may miss a conflict that the above will catch
//#define DMHC_DIRECT_K_HASH_CONFLICTS_FROM_REINSERT 1 
// 2/25/16 -- BSV DMHC didn't originally do this, but revised it to do so.
#define MISS_ON_ZERO_GTABLE_COUNT 1

namespace policy_engine {

class dmhc_t {
public:
  dmhc_t(int cap, int newk, int newc, int infields, int outfields, int* ops_size, int* res_size, bool new_no_evict);
  ~dmhc_t();
  void reset();
  void insert(const operands_t& ops, results_t& res, bool* consider);
  bool lookup(const operands_t& ops, results_t& res, bool* consider);
  void compute_hashes(const operands_t& ops, int* hashes, bool* consider);

private:
  void copy_victim_to_reinsert();
  void insert(int address, const operands_t& ops, results_t& res, int hops, int* do_not_victimize, bool* consider);
  void real_insert(int address, const operands_t& ops, results_t& res, int* hashes, int free_slot, bool* consider);
  bool hit(const operands_t& ops, int address, bool* consider);
  void evict_mtable_entry(int address, operands_t& victim_ops, results_t& victim_res, int* do_not_victimize);
  void check_direct_k_hash_conflicts(const operands_t& ops, bool* consider); // goes with DMHC_DIRECT_K_HASH_CONFLICTS

  int capacity;
  int k;
  int c;
  int input_fields;
  int output_fields;
  int* input_field_widths;
  int* output_field_widths;

  compute_hash_t* hasher;

  bool no_evict;
  int next_entry;

  int* hashes; // (perf. optimization to avoid reallocating)
  int* victim_hashes; // similar optimization
  int* do_not_victimize_hashes; // similar optimization
  
  int** gtable;
  int** gtable_cnt;
  int** gtable_lastinsert;
  bool* mtable_use;
  operands_t* mtable_inputs;
  results_t* mtable_outputs;
  bool** consider_mtable_input; //Used to keep track of don't care ops

  int **mtable_hashes; // not something we use in a real dMHC, but something to use to investigate a particular question
  //Q: how often do we get a direct conflict in all hashes? -- it should be extremely rare

  operands_t victim_ops; // place to hold victims before reinsert
  results_t victim_res; //  (perf. optimization to avoid reallocating)
  operands_t reinsert_ops; // place to hold victim during reinsertion (and not conflict with the victim it may produce)
  results_t reinsert_res; //  (perf. optimization to avoid reallocating)
  bool* consider_evict;

  int misses, hits, false_hits, direct_conflicts, inserts; // statistics
};

} // namespace policy_engine

#endif // __DMHC_H__