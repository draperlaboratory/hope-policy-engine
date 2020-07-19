/*
 * This is the main rule analysis engine for SCALPEL.
 * It hooks the following operations: rule_validated(), rule_inserted() and rule_evicted().
 * Using these hooks, it computes various useful stats currently used for compartmentalization:
 * 1) A collection of working sets by epoch.
 * 2) A coresident graph of rule cache-locality relationships.
 * 3) A "prefetching policy" file for consumption by the *policy*, not in the validator.
 * Rule tracing (working sets and coresident graph) are enabled by creating a TRACE_ENABLED file.
 * Prefetching logic (create prefetching policy file) is enabled by creating a PREFETCH_ENABLED file.
 *
 * Due to the latency induced from calculating the full coresident graph, this now just periodically
 * dumps the cache for later sampling. Leaving code intact for possible future analysis.
 *
 * Both should not run simulatenously, as the prefetching insertions currently trigger the same
 * hooks that the tracing monitors, so it would pollute results.
 */

#include "rule_analysis.h"
#include "prefetching.cc"
#include <unordered_set>

using namespace policy_engine;

#define EPOCH_LENGTH 1000000
#define CACHE_DUMP_FREQ 1000000

const char * working_set_filename = "working_sets_latest.txt";
const char * coresidency_graph_filename = "coresidency_latest.txt";

bool prefetch_enabled = false;
bool tracing_enabled = false;

// Tracing data structures:
std::unordered_set<operands_t, std::hash<operands_t>, compare_ops> * rules_in_period;
std::unordered_map<std::pair<operands_t, operands_t>, long, pair_ops_hash, compare_ops_pair> * coresident_graph;
std::unordered_map<operands_t, long, std::hash<operands_t>, compare_ops> * cycle_inserted;
unsigned long current_instr = 0;
unsigned long evicted_rules = 0;
FILE * working_set_file;
FILE * coresidency_graph_file;
operands_t * rule_cache_ptr;
int rule_cache_size;

// Prefetching data structures:
std::unordered_map<std::pair<Rule, Rule>, long, pair_rule_hash, compare_rule_pair> * next_rule_graph;
Rule last_rule;
int last_rule_set = 0;

// Called by rv32_validator.cc upon validator initialization to initialize this rule analysis.
// This checks what kind of analysis to do, opens files and initializes all the relevant C++ maps.
void init_rule_analysis(){

  // I can't figure out how to get env vars passed to the validator...
  // So for now checking for the existance of PREFETCH_ENABLE and TRACING_ENABLE files.

  FILE * check_prefetch = fopen("PREFETCH_ENABLE", "r");
  if (check_prefetch){
    
    // Detect as enabled
    printf("Prefetching enabled!\n");
    prefetch_enabled = true;
    
    // Initialize data structures
    next_rule_graph = new std::unordered_map<std::pair<Rule, Rule>, long, pair_rule_hash, compare_rule_pair>();
    
  } else {
    printf("No prefetching!\n");
  }

  FILE * check_tracing = fopen("TRACING_ENABLE", "r");
  if (check_tracing){
    
    // Detect as enabled
    printf("Tracing enabled!\n");
    tracing_enabled = true;
    
    // Initialize data structures
    rules_in_period = new std::unordered_set<operands_t, std::hash<operands_t>, compare_ops>();
    //coresident_graph = new std::unordered_map<std::pair<operands_t, operands_t>, long, pair_ops_hash, compare_ops_pair>();
    cycle_inserted = new std::unordered_map<operands_t, long, std::hash<operands_t>, compare_ops>();
    working_set_file = fopen(working_set_filename, "w");
    coresidency_graph_file = fopen(coresidency_graph_filename, "w");    
    
  } else {
    printf("No tracing!\n");
  }
  
}

// Called from the finite rule cache initialization, gives us a pointer
// to the entries of the current cache so that we can access them.
void init_rule_cache_ptr(operands_t * entries, int size){
  printf("Setting rule_cache_ptr to %p\n", entries);
  rule_cache_ptr = entries;
  rule_cache_size = size;
}

// This function hooks rule *validation*, so it runs each instruction.
// It's used for tracing to track working sets of rules.
void rule_validated(operands_t * ops){
  
  // This logic is only relevant to tracing
  if (!tracing_enabled)
    return;
      
  // If tracing, then add to our working set map if we don't have it
  if (rules_in_period -> find(*ops) == rules_in_period -> end()){
    rules_in_period -> insert(*ops);
  }

  // If this is an epoch boundary, then print out current working set then clear
  // TODO: does anything in the map need to be freed?...
  // I think everything is passed by value currently, so I don't think it's holding heap mem
  current_instr += 1;
  if (current_instr % CACHE_DUMP_FREQ == 0){
    printf("Dumping cache contents...\n");
    dump_cache_contents();
  }
  
  if (current_instr % EPOCH_LENGTH == 0){
    printf("Epoch boundary! Had %lu rules. Total evicts: %lu.\n", rules_in_period -> size(), evicted_rules);
    output_working_set();
    rules_in_period -> clear();
  }
 
}

// This function hooks rule *insertion*. It's used by both tracing and prefetching:
// 1) We set the insertion cycle for the coresidency graph.
// 2) We update the next-rule graph for prefetching.
void rule_inserted(Rule rule){
  
  // *** Tracing logic ***
  if (tracing_enabled){

    // New rule is inserted: set it's cycle inserted to current cycle
    operands_t * ops = &rule.first;
    auto it = cycle_inserted -> find(*ops);    
    if (it == cycle_inserted -> end()){
      cycle_inserted -> insert(std::pair<operands_t, int>(*ops, current_instr));
    } else {
      it -> second = current_instr;
    }  
  }

  // *** Prefetching logic ***
  if (prefetch_enabled){

    // *** Update next_rule_graph ***
    if (last_rule_set == 0){
      last_rule_set = 1;
      last_rule = rule;
      return;
    }
    
    std::pair<Rule, Rule> rule_pair = std::make_pair(last_rule, rule);
    auto rule_graph_it = next_rule_graph -> find(rule_pair);
    if (rule_graph_it == next_rule_graph -> end()){
      results_t * res = &(rule_pair.second.second);
      next_rule_graph -> insert(std::make_pair(rule_pair, 1));
    } else {
      rule_graph_it -> second += 1;
    }

    // Update to new last rule
    last_rule = rule;
  }
}

// Determine an ordering of rules. This assumes the actual
// metadata_ts will be canonized so the pointer comparison
// is meaningful for differentiating them.
bool rule_compare(operands_t * r1, operands_t * r2){
  
  if (r1 -> op1 < r2 -> op1){
    return true;
  } else {
    return false;
  }

  if (r1 -> op2 < r2 -> op2){
    return true;
  } else {
    return false;
  }

  if (r1 -> pc < r2 -> pc){
    return true;
  } else {
    return false;
  }
  
  if (r1 -> ci < r2 -> ci){
    return true;
  } else {
    return false;
  }

  if (r1 -> mem < r2 -> mem){
    return true;
  } else {
    return false;
  }

  return true;
}

// This function hooks rule *evictions*.
// It's used by the tracing policy to track coresidency lifetimes. 
// It's called for each old_rule currently in the cache combined with
// the new_rule that is coming in.
// TODO: Change finite_rule_cache.cc back to calling on each pair
//       This code is currently in a hacky state: cores graph too big
void rule_evicted(operands_t * evicted_rule, operands_t * cores_rule){

  // Now that we are using cache sampling instead of coresidency graph
  // calculation, don't need this logic.
  return;
  
  if (!tracing_enabled)
    return;
  
  // Compute overlap of these two rules
  long old_cycle = cycle_inserted -> find(*evicted_rule) -> second;
  //long resident_cycle = cycle_inserted -> find(*cores_rule) -> second;
  //long weight_increase = current_instr - std::max(old_cycle, resident_cycle);

  /*  
  // Put the pair into a canonical ordering before storing to save space
  operands_t * r1, * r2;  
  if (rule_compare(evicted_rule, cores_rule)){
    r1 = evicted_rule;
    r2 = cores_rule;
  } else {
    r1 = cores_rule;
    r2 = evicted_rule;
  }
  std::pair<operands_t, operands_t> p  = std::make_pair(*r1, *r2); */

  // Generating the actual coresident graph is quite expensive
  // This is a quick hack to just get rule frequencies...
  std::pair<operands_t, operands_t> p = std::make_pair(*evicted_rule, *cores_rule);
  long weight_increase = current_instr - old_cycle;
  // End hack
  
  // Lookup this pair, create new if we don't have it
  auto it = coresident_graph -> find(p);
  if (it == coresident_graph -> end()){
    std::pair<std::pair<operands_t, operands_t>, long> val = std::make_pair(p, weight_increase);
    coresident_graph -> insert(val);
  } else {
    it -> second += weight_increase;
  }
}

// Write out the working set data for this epoch into output file
void output_working_set(){

  printf("Writing out working set...\n");  
  char rule[2048];
  fprintf(working_set_file, "BEGIN\n");
  
  for (auto it = rules_in_period -> begin(); it != rules_in_period -> end(); it++){
    operands_t ops = *it;
    pretty_print_rule(rule, ops.ci, ops.op1, ops.op2, ops.mem, ops.pc);
    fprintf(working_set_file, "%s\n", rule);
  }
}

static unsigned int sample_no = 1;
void dump_cache_contents(){
  printf("Dumping cache to fd %d\n", coresidency_graph_file);
  char rule[2048];
  fprintf(coresidency_graph_file, "%d\n", sample_no);
  for (int i = 0; i < rule_cache_size; i++){
    operands_t * ops = &rule_cache_ptr[i];

    // If this entry is all zeroes, it's unset so skip
    char * p = (char *) ops;
    bool all_zeroes = true;
    for (int j = 0; j < sizeof(operands_t); j++)
      if (p[j] != '\0')
	all_zeroes = false;

    if (all_zeroes)
      continue;
	
    pretty_print_rule(rule, ops->ci, ops->op1, ops->op2, ops->mem, ops->pc);
    fprintf(coresidency_graph_file, "%s\n", rule);
  }
  fflush(coresidency_graph_file);
  printf("Wrote out cache sample %d\n", sample_no);  
  sample_no++;
}

// Write out the working set data for this epoch into output file
/*
void output_coresidency_graph(){

  printf("Writing out coresidency graph...\n");
  FILE * f = fopen(coresidency_graph_filename, "w");
  char rule1[2048], rule2[2048];

  int num_pairs = 0;
  for (auto it = coresident_graph -> begin(); it != coresident_graph -> end(); it++){
    std::pair<std::pair<operands_t, operands_t>, long> cores_element = *it;
    std::pair<operands_t, operands_t> * rule_pair = &cores_element.first;
    operands_t * ops1 = &(rule_pair -> first);
    operands_t * ops2 = &(rule_pair -> second);
    long cores_cycles = cores_element.second;
    pretty_print_rule(rule1, ops1 -> ci, ops1 -> op1, ops1 -> op2, ops1 -> mem, ops1 -> pc);
    //pretty_print_rule(rule2, ops2 -> ci, ops2 -> op1, ops2 -> op2, ops2 -> mem, ops2 -> pc);    
    //fprintf(f, "%s %s %lu\n", rule1, rule2, cores_cycles);
    fprintf(f, "%s <none> %lu\n", rule1, cores_cycles);    
    num_pairs++;
  }
  printf("Number of pairs in coresidency graph: %d\n", num_pairs);
  } */

// At the end of a run, create a prefetching policy file if prefetch enabled.
extern "C" void rule_analysis_end(){

  printf("Rule analysis detected termination.\n");

  //if (tracing_enabled){
  //printf("Tracing enabled, creating coresidency graph.\n");
  //output_coresidency_graph();
  //}
  
  if (prefetch_enabled){
    printf("Prefetching enabled, creating prefetching policy file.\n");
    make_simple_prefetch_policy(next_rule_graph);
  }
}
