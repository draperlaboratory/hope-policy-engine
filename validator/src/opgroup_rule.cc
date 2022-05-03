#include <algorithm>
#include <array>
#include <cstdint>
#include <vector>
#include "opgroup_rule.h"
#include "riscv_isa.h"

namespace policy_engine {

void opgroup_rule_t::add_operand_rule(std::vector<uint32_t> values, operand_rule_match_t match) {
  rules.push_back(operand_rule_t{.values=values, .match=match});
}

static bool operand_rule_match(operand_rule_t &rule, uint32_t value) {
  switch (rule.match) {
    case OPERAND_RULE_ANY: return true;
    case OPERAND_RULE_EQUAL: return std::any_of(rule.values.begin(), rule.values.end(), [=](uint32_t v){ return value == v; });
    case OPERAND_RULE_NOT: return std::none_of(rule.values.begin(), rule.values.end(), [=](uint32_t v){ return value == v; });
    case OPERAND_RULE_RANGE: return (value >= rule.values.front()) && (value <= rule.values.back());
    case OPERAND_RULE_NOT_RANGE: return ((value >= rule.values.front()) && (value <= rule.values.back())) == false;
    default: return false;
  }
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

}