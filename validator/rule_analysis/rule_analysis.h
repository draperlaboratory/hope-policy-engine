#ifndef RULE_ANALYSIS_H
#define RULE_ANALYSIS_H

// Define a convenient type for a rule (it's a pair of ops/res)
typedef std::pair<operands_t, results_t> Rule;

struct compare_ops_pair {
  bool operator()(std::pair<operands_t, operands_t> p1, std::pair<operands_t, operands_t> p2) const {
    operands_t a1 = p1.first;
    operands_t a2 = p1.second;
    operands_t b1 = p2.first;
    operands_t b2 = p2.second;
    
    return (a1.op1 == b1.op1 &&
            a1.op2 == b1.op2 &&
            a1.op3 == b1.op3 &&
            a1.mem == b1.mem &&
            a1.pc == b1.pc &&
            a1.ci == b1.ci &&
	    a2.op2 == b2.op2 &&
            a2.op2 == b2.op2 &&
            a2.op3 == b2.op3 &&
            a2.mem == b2.mem &&
            a2.pc == b2.pc &&
            a2.ci == b2.ci	    
	    );
  }
};

struct compare_rule_pair {
  bool operator()(std::pair<Rule, Rule> p1, std::pair<Rule, Rule> p2) const {
    
    operands_t a1 = p1.first.first;
    operands_t a2 = p1.second.first;
    operands_t b1 = p2.first.first;
    operands_t b2 = p2.second.first;
    
    return (a1.op1 == b1.op1 &&
            a1.op2 == b1.op2 &&
            a1.op3 == b1.op3 &&
            a1.mem == b1.mem &&
            a1.pc == b1.pc &&
            a1.ci == b1.ci &&
	    a2.op2 == b2.op2 &&
            a2.op2 == b2.op2 &&
            a2.op3 == b2.op3 &&
            a2.mem == b2.mem &&
            a2.pc == b2.pc &&
            a2.ci == b2.ci	    
	    );
  }
};

struct compare_rule {
  bool operator()(const Rule &r1, const Rule &r2) const {
    operands_t a = r1.first;
    operands_t b = r2.first;
    return (a.op1 == b.op1 &&
            a.op2 == b.op2 &&
            a.op3 == b.op3 &&
            a.mem == b.mem &&
            a.pc == b.pc &&
            a.ci == b.ci);
  }
};

struct pair_ops_hash {
  std::size_t operator()(const std::pair<operands_t, operands_t> &p) const {
    auto h1 = std::hash<operands_t>{}(p.first);
    auto h2 = std::hash<operands_t>{}(p.second);
    return h1 ^ h2;
  }
};

struct pair_rule_hash {
  std::size_t operator()(const std::pair<Rule, Rule> &p) const {
    auto h1 = std::hash<operands_t>{}(p.first.first);
    auto h2 = std::hash<operands_t>{}(p.second.first);
    return h1 ^ h2;
  }
};

struct rule_hash {
  std::size_t operator()(const Rule &p) const {
    auto h1 = std::hash<operands_t>{}(p.first);
    return h1;
  }
};



void rule_evicted(operands_t * old_rule, operands_t * new_rule);
void rule_inserted(Rule rule);
void output_working_set();
void output_coresidency_graph();
void dump_cache_contents();
void init_rule_cache_ptr(operands_t * ptr, int size);
#endif
