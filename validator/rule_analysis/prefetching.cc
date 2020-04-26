#include "rule_analysis.h"
#include "prefetching.h"
#include <stdio.h>

const char * prefetching_filename = "prefetching_latest.pp";

#include "rule_IO.c"

// The simplest prefetching policy is always prefetching the rule with the highest probability.
// We'll start with this simple policy to get some simple infrastructure up.
void make_simple_prefetch_policy(std::unordered_map<std::pair<Rule, Rule>, long, pair_rule_hash, compare_rule_pair> * next_rule_graph){

  // For each rule, find the single highest successor
  printf("Making a simple prefetch policy (highest successor).\n");
  
  std::unordered_map<Rule, std::pair<Rule, long>, rule_hash, compare_rule> * best_successor = new std::unordered_map<Rule, std::pair<Rule, long>, rule_hash, compare_rule>();

  for (auto it = next_rule_graph -> begin(); it != next_rule_graph -> end(); it++){

    // Get the current operands_t and see if it's in the map
    std::pair<Rule, Rule> rule_pair = it -> first;
    long rule_hits = it -> second;

    Rule key = rule_pair.first;

    if (best_successor -> find(key) == best_successor -> end()){
      // We did not have this key yet, create it
      best_successor -> insert({key, std::make_pair(rule_pair.second, rule_hits)});
    } else {
      // We have this key, update it if the count is higher
      auto lookup_iterator = best_successor -> find(key);
      long existing_hits = lookup_iterator -> second.second;
      if (existing_hits > rule_hits){
	best_successor -> erase(lookup_iterator);
	best_successor -> insert({key, std::make_pair(rule_pair.second, rule_hits)});	
      }
    }
    
  }

  printf("Now writing out policy!\n");
  FILE * f = open_rule_file(prefetching_filename);  
  for (auto it = best_successor -> begin(); it != best_successor -> end(); it++){
    Rule key = it -> first;
    std::pair<Rule, long> prefetch_data = it -> second;
    long count = prefetch_data.second;
    //printf("For this rule, had prefetch successor with hits %lu\n", rule_pair.second);
    if (count > 1)
      write_rule(&key, &(prefetch_data.first), f);
  }
  
}
