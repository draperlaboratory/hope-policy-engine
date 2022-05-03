#include <array>
#include "opgroup_rule.h"
#include "riscv_isa.h"

using namespace policy_engine;

void
opgroup_rule_t::add_operand_rule(std::vector<uint32_t> values,
                                operand_rule_match_t match) {
  operand_rule_t rule;

  rule.values = values;
  rule.match = match;

  rules.push_back(rule);
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
  static constexpr int NUM_FIELDS = 5;
  std::array<int32_t, NUM_FIELDS> fields{rd, rs1, rs2, rs3, imm};
  std::array<uint32_t, NUM_FIELDS> field_flags{HAS_RD, HAS_RS1, HAS_RS2, HAS_RS3, HAS_IMM};
  for (int i = 0; i < NUM_FIELDS; i++)
    if (flags & field_flags[i])
      if (i >= rules.size() || !operand_rule_match(rules[i], fields[i]))
        return false;
  return true;
}
