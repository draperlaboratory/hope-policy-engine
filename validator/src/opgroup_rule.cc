#include "opgroup_rule.h"
#include "riscv_isa.h"

using namespace policy_engine;

void
opgroup_rule_t::add_operand_rule(std::vector<uint32_t> values,
                                operand_rule_match_t match) {
  operand_rule_t rule;

  rule.values = values;
  rule.match = match;

  this->rules.push_back(rule);
}

static bool
operand_rule_match(operand_rule_t &rule, uint32_t value) {
  switch(rule.match) {
    case OPERAND_RULE_ANY:
      return true;
    case OPERAND_RULE_EQUAL:
      for(auto const& rule_value: rule.values) {
        if (value == rule_value) {
          return true;
        }
      }
      break;
    case OPERAND_RULE_NOT:
      for(auto const& rule_value: rule.values) {
        if (value == rule_value) {
          return false;
        }
      }
      return true;
    case OPERAND_RULE_RANGE:
      return (value >= rule.values.front()) && (value <= rule.values.back());
    case OPERAND_RULE_NOT_RANGE:
      return ((value >= rule.values.front()) && (value <= rule.values.back())) == false;
    default:
      break;
  }
  
  return false;
}

bool opgroup_rule_t::matches(int32_t flags, uint32_t rs1, uint32_t rs2, uint32_t rs3, uint32_t rd, int32_t imm) {
  std::vector<operand_rule_t>::iterator it = this->rules.begin();

  if ((flags & HAS_RD) != 0) {
    if ((operand_rule_match(*it, rd) == false) || (it == this->rules.end())) {
      return false;
    }
    it++;
  }

  if ((flags & HAS_RS1) != 0) {
    if ((operand_rule_match(*it, rs1) == false) || (it == this->rules.end())) {
      return false;
    } 
    it++;
  }

  if ((flags & HAS_RS2) != 0) {
    if ((operand_rule_match(*it, rs2) == false) || (it == this->rules.end())) {
      return false;
    } 
    it++;
  }

  if ((flags & HAS_RS3) != 0) {
    if ((operand_rule_match(*it, rs3) == false) || (it == this->rules.end())) {
      return false;
    } 
    it++;
  }

  if ((flags & HAS_IMM) != 0) {
    if ((operand_rule_match(*it, imm) == false) || (it == this->rules.end())) {
      return false;
    } 
    it++;
  }

  return true;
}
