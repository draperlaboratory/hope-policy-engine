#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include "dmhc.h"
#include "meta_cache.h"
#include "riscv_isa.h"
#include "tag_utils.h"

namespace policy_engine {

dmhc_t::dmhc_t(int cap, int newk, int newc, int infields, int outfields, int* ops_size,  int* res_size, bool new_no_evict) {
  k = newk;
  c = newc;
  input_fields = infields;
  output_fields = outfields;
  capacity = cap;
  no_evict = new_no_evict;

  assert(capacity > 1); // capacity 1 case doesn't work since we use the 
                        //0 entry for INVALID_LAST_USER

  hashes = (int*)malloc(sizeof(int)*k);
  victim_hashes = (int*)malloc(sizeof(int)*k);
  do_not_victimize_hashes = (int*)malloc(sizeof(int)*k);
  consider_evict = (bool*)malloc(sizeof(bool)*input_fields);

  // store fields
  input_field_widths = (int*)malloc(sizeof(int)*input_fields);
  output_field_widths = (int*)malloc(sizeof(int)*output_fields);
  for (int i = 0; i < input_fields; i++)
    input_field_widths[i] = ops_size[i];
  for (int i = 0; i < output_fields; i++)
    output_field_widths[i] = res_size[i];

  // initialize tables
  gtable = (int**)malloc(sizeof(int *)*k);
  gtable_cnt = (int**)malloc(sizeof(int *)*k);
  gtable_lastinsert = (int**)malloc(sizeof(int *)*k);
  for (int i = 0; i < k; i++) {
    gtable[i] = (int*)malloc(sizeof(int)*c*capacity);
    gtable_cnt[i] = (int*)malloc(sizeof(int)*c*capacity);
    gtable_lastinsert[i] = (int*)malloc(sizeof(int)*c*capacity);
  }
    
  mtable_use = (bool*)malloc(sizeof(bool)*capacity);
  mtable_inputs = new operands_t[capacity];
  mtable_outputs = new results_t[capacity];
  consider_mtable_input = (bool**)malloc(sizeof(bool *)*input_fields);
  for (int i = 0; i < input_fields; i++) {
    consider_mtable_input[i] = (bool*)malloc(sizeof(bool)*capacity);
  }
#ifdef DMHC_DIRECT_K_HASH_CONFLICTS 
  mtable_hashes = (int**)malloc(sizeof(int*)*k);
  for (int i = 0; i < k; i++)
    mtable_hashes[i] = (int*)malloc(sizeof(int)*capacity);
#else
  mtable_hashes=(int**)NULL;
#endif
  
  init_hashes();

  // need to call even if INIT_HASH_POSITIONS is false
  hasher = new compute_hash_t(input_fields,input_field_widths, k, c*capacity); // total capacity (so apply c)

  reset(); // to initialize all the fields
}

dmhc_t::~dmhc_t() {
  for (int i = 0; i < k; i++) {
    free(gtable[i]);
    free(gtable_cnt[i]);
    free(gtable_lastinsert[i]);
  }
  free(gtable);
  free(gtable_lastinsert);
  delete[] mtable_inputs;
  delete[] mtable_outputs;
  for (int i = 0; i < input_fields; i++)
    free(consider_mtable_input[i]);
  free(consider_mtable_input);
#ifdef DMHC_DIRECT_K_HASH_CONFLICTS 
  for (int i = 0; i < k; i++)
    free(mtable_hashes[i]);
#endif  
  free(input_field_widths);
  free(output_field_widths);
  free(mtable_hashes);
  free(mtable_use);
  free(consider_evict);
  free(do_not_victimize_hashes);
  free(victim_hashes);
  free(hashes);
}

void dmhc_t::reset() {
  next_entry = 1;  // skip 0 = INVALID_LAST_USER
  for (int i = 0; i < k; i++) {
    for (int j = 0; j < (c*capacity); j++) {
      gtable[i][j] = 0;
      gtable_cnt[i][j] = 0;
      gtable_lastinsert[i][j] = INVALID_LAST_USER;
    }
  }
  for (int j = 0; j < capacity; j++)
    mtable_use[j] = false;
  for (int i = 0; i < capacity; i++) {
    mtable_inputs[i] = {0};
    mtable_outputs[i] = {0};
  }

#ifdef DMHC_DIRECT_K_HASH_CONFLICTS   
  for (int i = 0; i < k; i++) {
    for (int j = 0; j < capacity; j++)
      mtable_hashes[i][j] = -1;
  }
#endif
  
  hits = 0; misses = 0; false_hits = 0; direct_conflicts = 0; inserts = 0;
}

void dmhc_t::check_direct_k_hash_conflicts(const operands_t& ops, bool* consider) {
  compute_hashes(ops, hashes, consider);
  for (int i = 0; i < capacity; i++) {
    if (mtable_use[i] == true) {
      bool conflict = true;
      for (int j = 0; j < k; j++)
        if (hashes[j] != mtable_hashes[j][i])
          conflict = false;
      bool real_conflict = false;
      if (conflict)
        real_conflict = ops != mtable_inputs[i];
      if (real_conflict) {
        printf("dMHC: (compute ) WARNING direct k-hash conflict ");
        for (int f = 0; f < k; f++)
          printf("%03x ", mtable_hashes[f][i]);
        printf(": [%" PRItag " %" PRItag " %" PRItag " %" PRItag " %" PRItag " %" PRItag "]",
               ops.pc, ops.ci, ops.op1, ops.op2, ops.op3, ops.mem);
        printf(" [%" PRItag " %" PRItag " %" PRItag " %" PRItag " %" PRItag " % " PRItag "]",
               mtable_inputs[i].pc, mtable_inputs[i].ci, mtable_inputs[i].op1, mtable_inputs[i].op2, mtable_inputs[i].op3, mtable_inputs[i].mem);
        printf("]\n");
        direct_conflicts++;
      }
    }
  }
}
  
void dmhc_t::compute_hashes(const operands_t& ops, int* hashes_to_fill, bool* consider) {
#ifdef INIT_HASH_POSITIONS
  hasher->compute_hash_set_from_precomputed_positions(k, ops, hashes_to_fill, consider);
#else
  hasher->compute_hash_set(k, ops, hashes_to_fill, input_fields, input_field_widths, c*capacity, consider);
#endif

#ifdef VERIFY_PRECOMPUTE_HASH
  int* verify_hashes = (int*)malloc(sizeof(int)*k);
  hasher->compute_hash_set(k, ops, verify_hashes, input_fields, input_field_widths, c*capacity, consider);
  int h;
  bool mismatch = false;
  for (h = 0; h < k; h++) {
    if (verify_hashes[h] != hashes_to_fill[h])
      mismatch = true;
  }
  if (mismatch) {
    printf("compute_hashes: VERIFY ERROR ");
    for (h = 0; h < k; h++) 
      printf(" [%x %x]", verify_hashes[h], hashes_to_fill[h]);
    printf(" <-from- ");
    printf(" %" PRItag " %" PRItag " %" PRItag " %" PRItag " %" PRItag " %" PRItag,
           ops.pc, ops.ci, ops.op1, ops.op2, ops.op3, ops.mem);
    printf("\n");
    printf("HALTING ON FAILED VERIFY OF PRECOMPUTED HASH\n");
    exit(37); 
  } else {
    printf("Compute hashes checking and passing\n");
  }
  free(verify_hashes);
#endif
}

bool dmhc_t::hit(const operands_t& ops, int address, bool* consider) {
  if (mtable_use[address] == false) {
    printf("mtable_use[address} is false\n");
    return false;
  }

  if (consider[0] && mtable_inputs[address].pc != ops.pc) {
    printf("mtable_inputs[address].pc is false\n");
    return false;
  }
   if (consider[1] && mtable_inputs[address].ci != ops.ci) {
     printf("mtable_inputs[address].ci is false\n");
     return false;
   }
  if (consider[2] && mtable_inputs[address].op1 != ops.op1) {
    printf("mtable_inputs[address].op1 is false\n");
    return false;
  }
  if (consider[3] && mtable_inputs[address].op2 != ops.op2) {
    printf("mtable_inputs[address].op2 is false\n");
    return false;
  }
  if (consider[4] && mtable_inputs[address].op3 != ops.op3) {
    printf("mtable_inputs[address].op3 is false\n");
    return false;
  }
  if (consider[5] && mtable_inputs[address].mem != ops.mem) {
    printf("mtable_inputs[address].mem is false\n");
    return false;
  }
  return true;
}

void dmhc_t::copy_victim_to_reinsert() {
  reinsert_ops = victim_ops;
  reinsert_res = victim_res;
}

void dmhc_t::evict_mtable_entry(int address, operands_t& victim_ops, results_t& victim_res, int* do_not_victimize) {

  int result;
  if (no_evict) {
    exit(100);
  }
  
  //bool consider_evict[input_fields];
  victim_ops = mtable_inputs[address];
  mtable_inputs[address] = {0};
  for (int i = 0; i < input_fields; i++) {
    consider_evict[i] = consider_mtable_input[i][address];
  }

  victim_res = mtable_outputs[address];
  mtable_outputs[address] = {0};

  mtable_use[address] = false;
  compute_hashes(victim_ops, victim_hashes, consider_evict);
  bool cleared_gtable = false;

  for (int i= 0; i < k; i++) {
    // because something may have gotten bashed and we did *not* know the user
    //   it is possible the mtable entry is an orphan, and the associated gtable entry is gone;
    // ...so, it is possible that this has already been decremented to zero -- catch that...
    if (gtable_cnt[i][victim_hashes[i]] > 0) {
      // because the counts can be wrong, we can have a case where
      //  this would erroneously count down to zero
      if ((do_not_victimize == NULL) || (do_not_victimize[i] != victim_hashes[i])) {
        gtable_cnt[i][victim_hashes[i]] = gtable_cnt[i][victim_hashes[i]] - 1;
      }
      // if it is a do_not_victimize, 
      //   we still want to remove the contribution of the victim,
      //   we just don't want to drop cnt to 1
      else if ((do_not_victimize != NULL) && (do_not_victimize[i] == victim_hashes[i])) {
    if (gtable_cnt[i][victim_hashes[i]] > 1)
      gtable_cnt[i][victim_hashes[i]] = gtable_cnt[i][victim_hashes[i]] - 1;
      }
    }
    if (gtable_cnt[i][victim_hashes[i]] == 0) {
      cleared_gtable = true;
      // (originally BSV not do this)
      gtable[i][victim_hashes[i]] = 0; 
      gtable_lastinsert[i][victim_hashes[i]] = INVALID_LAST_USER;
    } else if (gtable_lastinsert[i][victim_hashes[i]] == address) {
      gtable_lastinsert[i][victim_hashes[i]] = INVALID_LAST_USER;
    }
  }
}

void dmhc_t::real_insert(int address, const operands_t& ops, results_t& res, int* hashes, int free_slot, bool* consider) {
  for (int i = 0; i < input_fields; i++) {
    consider_mtable_input[i][address] = consider[i];
  }
  if (consider_mtable_input[0][address])
    mtable_inputs[address].pc = ops.pc;
  if (consider_mtable_input[1][address])
    mtable_inputs[address].ci = ops.ci;
  if (consider_mtable_input[2][address])
    mtable_inputs[address].op1 = ops.op1;
  if (consider_mtable_input[3][address])
    mtable_inputs[address].op2 = ops.op2;
  if (consider_mtable_input[4][address])
    mtable_inputs[address].op3 = ops.op3;
  if (consider_mtable_input[5][address])
    mtable_inputs[address].mem = ops.mem;
  mtable_outputs[address] = res;
#ifdef DMHC_DIRECT_K_HASH_CONFLICTS
  for (int i = 0; i < k; i++)
    mtable_hashes[i][address] = hashes[i];
#endif
  mtable_use[address] = true;
  
  assert(gtable_cnt[free_slot][hashes[free_slot]] == 0); // this should only be called after evicting rules to make space

  int current_value=0;
  for (int i = 0; i < k; i++)
    current_value = current_value^gtable[i][hashes[i]];
  // xor with current_value to bring it to zero
  // xor with address so the result will be address
  gtable[free_slot][hashes[free_slot]] = gtable[free_slot][hashes[free_slot]]^current_value^address;
  for (int i = 0; i < k; i++) { // could do this in loop above, but intent probably clearer here
    if (gtable_cnt[i][hashes[i]] != GTABLE_MAX_COUNT) {
      //printf("gtable: %d\n", gtable_cnt[i][hashes[i]]);
      gtable_cnt[i][hashes[i]]++;
      //printf("hashes[%d]: %d, gtable: %d\n", i, hashes[i], gtable_cnt[i][hashes[i]]);
    }
      // record who caused the insert (in all)
      //  (previously only updated free slot,
      //     but (a) not match BSV
      //         (b) at beginning filling in all k
      //         (c) may end up conflicting with multiple hashes in victim
      //               so may be replacing in multiple
    gtable_lastinsert[i][hashes[i]] = address;
  }  
}

void dmhc_t::insert(const operands_t& ops, results_t& res, bool* consider) {
  inserts++;
#ifdef DMHC_DIRECT_K_HASH_CONFLICTS
  check_direct_k_hash_conflicts(ops, consider);
#endif
  
  int mtable_entry = next_entry;
  next_entry++;
  if (next_entry >= capacity)
    next_entry = 1; // skip 0=INVALID_LAST_USER   

  if (mtable_use[mtable_entry] == true)
    evict_mtable_entry(mtable_entry, victim_ops, victim_res, NULL); // capacity eviction -- we cannot save
  
  compute_hashes(ops, do_not_victimize_hashes, consider);

  insert(mtable_entry, ops, res, 0, do_not_victimize_hashes, consider);
#ifdef DMHC_DEBUG
  printf("ops - pc: %" PRItag ", ci: %" PRItag, ops.pc, ops.ci);
  if (consider[OP_OP1]) printf(", op1: %" PRItag, ops.op1);
  if (consider[OP_OP2]) printf(", op2: %" PRItag, ops.op2);
  if (consider[OP_OP3]) printf(", op3: %" PRItag, ops.op3);
  if (consider[OP_OP1]) printf(", mem: %" PRItag, ops.mem);
  printf("\n");
  printf("res - pc: %" PRItag ", rd: %" PRItag ", csr: %" PRItag ", pcRes: %" PRItag ", rdRes: %"
         PRItag ", csrRes: %" PRItag "\n", victim_res.pc, victim_res.rd, victim_res.csr,
         victim_res.pcResult, victim_res.rdResult, victim_res.csrResult);
#endif
  if (lookup(ops, victim_res, consider) != true) {
    printf("lookup is not true\n");
    exit(1);
  } else if (res != victim_res) {
    printf("victim res is not true\n");
    exit(1);
  }
}

void dmhc_t::insert(int mtable_address, const operands_t& ops, results_t& res, int hops, int* do_not_victimize, bool* consider) {
  // with care, this could use lookup as a subroutine
  //  would need to make address a return value (or part of the object representation)
  int free_slot = -1;
  bool victim = false;

  int victim_address = -1;

  compute_hashes(ops, hashes, consider);  

  // 2/13/15 -- swapped this around so will take the lowest with 0 to match BSV
  for (int i = k - 1; i > -1; i--) {
    if (gtable_cnt[i][hashes[i]] == 0)
      free_slot = i;
  }
  if (free_slot == -1) {
    // try to identify thing to evict
    int minuse = -1;
    int minusers = -1;
    for (int i = 0; i < k; i++) {
      if ((hops == 0) || (hashes[i] != do_not_victimize[i])) {
        if (minuse == -1) {
          minuse = i;
          minusers = gtable_cnt[i][hashes[i]];
        } else if (gtable_cnt[i][hashes[i]] < minusers) {
        minuse = i;
        minusers = gtable_cnt[i][hashes[i]];
      }
    }
  }
    
#ifdef DMHC_DIRECT_K_HASH_CONFLICTS_FROM_REINSERT 
    if ((free_slot < 0) && (minuse < 0)) {
      int tmp_mslot = INVALID_LAST_USER;
      for (int f = 0; f < k; f++)
    if (gtable_lastinsert[f][hashes[f]] != INVALID_LAST_USER)
      tmp_mslot = gtable_lastinsert[f][hashes[f]];
      bool real_conflict = false;
      for (int j = 0; j < input_fields; j++)
    if (ops[j].tags[0] != mtable_inputs[j][tmp_mslot].tags[0])
      real_conflict = true;
    }
#endif

    if ((hops > 0) && (minuse == -1)) {
      // give up on re-inserting to avoid victimizing the thing that started the miss
      // go ahead and declare the victim slot we were trying to replace as dead
      mtable_use[mtable_address] = false;     
      return;
    } else if ((hops == 0) && (minuse == -1)) {
      exit(3);
    } else {// we have a minuse>-1
      if (gtable_lastinsert[minuse][hashes[minuse]] != INVALID_LAST_USER) {
        victim_address = gtable_lastinsert[minuse][hashes[minuse]];  
        victim = true;
        if (hops > 0)
          evict_mtable_entry(victim_address, victim_ops, victim_res, do_not_victimize); // conflict case -- try to reinsert
        else // have not, yet, inserted the thing not to victimize
          evict_mtable_entry(victim_address, victim_ops, victim_res, NULL); // conflict case -- try to reinsert
      } else {  
        // give up on evicting -- we will need to bash it anyway
        victim = false; // should not be necessary, but making intent clear
      }
    }

    // 11/5/15: the above can fail
    //   (a) if none of the entires has count 1, the above will fail to clear out.
    //   (b) may not be a valid lastuser, so fail to clear something out
    //  So, in the case, we need to pick a slot and victimize it.
    // this time, we've aborted above if do_not_victimize prevents us from selecting an entry
    //   so there must be some entry to find.
    minuse = -1;
    minusers = -1;
    for (int i = 0; i < k; i++)
      if ((hops == 0) || (hashes[i] != do_not_victimize[i])) {
        if (minuse == -1) {
          minuse = i;
          minusers = gtable_cnt[i][hashes[i]];
        } else if (gtable_cnt[i][hashes[i]] < minusers) {
          minuse = i;
          minusers = gtable_cnt[i][hashes[i]];
        }
      }
      free_slot = minuse;
  }
#ifdef DMHC_DEBUG
  printf("mtable_address: %d, free_slot: %d\n", mtable_address, free_slot);
  printf("ops - pc: %" PRItag ", ci: %" PRItag, ops.pc, ops.ci);
  if (consider[OP_OP1]) printf(", op1: %" PRItag, ops.op1);
  if (consider[OP_OP2]) printf(", op2: %" PRItag, ops.op2);
  if (consider[OP_OP3]) printf(", op3: %" PRItag, ops.op3);
  if (consider[OP_OP1]) printf(", mem: %" PRItag, ops.mem);
  printf("\n");
  printf("res - pc: %" PRItag ", rd: %" PRItag ", csr: %" PRItag ", pcRes: %" PRItag ", rdRes: %"
         PRItag ", csrRes: %" PRItag "\n", res.pc, res.rd, res.csr,
         res.pcResult, res.rdResult, res.csrResult);
#endif
  real_insert(mtable_address, ops, res, hashes, free_slot, consider);
  if (victim == true) {
    if (hops < HOP_LIMIT) {
      copy_victim_to_reinsert();
      insert(victim_address, reinsert_ops, reinsert_res, hops + 1, do_not_victimize, consider_evict);
    }
  } // if victim
}

bool dmhc_t::lookup(const operands_t& ops, results_t& res, bool* consider) {
  compute_hashes(ops, hashes, consider); 
#ifdef MISS_ON_ZERO_GTABLE_COUNT
  // see if it is a miss
  for (int i = 0; i < k; i++)
    if (gtable_cnt[i][hashes[i]] == 0) {      
      misses++;
      //printf("It is a miss\n");
      //printf(" -> k: %d, i: %d, hashes[i]: %d\n", k, i, hashes[i]);
      return false;
    }
#endif

  // get the address
  int addr = 0;
  for (int i = 0; i < k; i++)
    addr = addr^gtable[i][hashes[i]];   
    
  if (hit(ops, addr, consider)) { // check if real hit
    // grab values
    res = mtable_outputs[addr];
    hits++;
    return true;
  } else { // false hit      
    false_hits++;
    //printf("It is a false hit\n");
    return false;
  }    
}

}
