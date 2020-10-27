// A simple wrapper to the set-associative cache.
// Import this file in a validator to produce cache statistics from an execution.
// Interface is to call icache_access(addr) and dcache_access(addr) on each validate().
// Call init_data_caches(l1_size, l2_size)

#include "sacache.c"

int cache_sim_init = 0;

struct SACache * l1icache, * l1dcache, *l2cache;

unsigned long dram_accesses;
unsigned long dcache_reads = 0;
unsigned long dcache_writes = 0;

void init_data_caches(int l1_size, int l2_size){

  printf("Init data caches! L1 size = %d, L2 size = %d\n", l1_size, l2_size);
  
  // Create caches
  l1icache = create_sacache(l1_size, 4);
  l1dcache = create_sacache(l1_size, 4);
  l2cache = create_sacache(l2_size, 8);
  
  // Clear variables
  dram_accesses = 0;
  
  // Mark init as complete
  cache_sim_init = 1;
}

void icache_access(unsigned long pc){

  if (cache_sim_init == 0)
    return;
  
  unsigned long wb_addr;
  int l1icache_hit = lookup_sacache(l1icache, pc, 0, &wb_addr);
  if (l1icache_hit == 0){
    int l2cache_hit = lookup_sacache(l2cache, pc, 0, &wb_addr);
    if (l2cache_hit == 0){
      dram_accesses += 1;
    }
  }
}

void dcache_access(unsigned long addr, int rw){
  
  if (cache_sim_init == 0)
    return;
  
  unsigned long wb_addr;
  int l1dcache_hit = lookup_sacache(l1dcache, addr, rw, &wb_addr);
  if (l1dcache_hit == 0){
    int l2cache_hit = lookup_sacache(l2cache, addr, rw, &wb_addr);
    if (l2cache_hit == 0){
      dram_accesses += 1;
    }
  }

  if (rw)
    dcache_writes += 1;
  else
    dcache_reads += 1;
}


void print_data_cache_stats(){
  printf("Number of DRAM hits: %lu\n", dram_accesses);
  printf("icache hits: %d\n", get_hits(l1icache));
  printf("dcache hits: %d\n", get_hits(l1dcache));
  printf("l2cache hits: %d\n", get_hits(l2cache));
  printf("Total dcache read/write: %lu/%lu\n", dcache_reads, dcache_writes);
}
