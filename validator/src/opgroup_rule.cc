#include <algorithm>
#include <array>
#include <cstdint>
#include <vector>
#include <utility>
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

bool opgroup_rule_t::matches(const decoded_instruction_t& inst) {
  static constexpr int NUM_FIELDS = 5;
  const std::array<std::pair<bool, int>, NUM_FIELDS> fields{
    std::make_pair((inst.flags & HAS_RD) != 0, inst.rd),
    std::make_pair((inst.flags & HAS_RS1) != 0, inst.rs1),
    std::make_pair((inst.flags & HAS_RS2) != 0, inst.rs2),
    std::make_pair((inst.flags & HAS_RS3) != 0, inst.rs3),
    std::make_pair((inst.flags & HAS_IMM) != 0, inst.imm)
  };
  for (int i = 0; i < NUM_FIELDS; i++)
    if (fields[i].first)
      if (i >= rules.size() || !operand_rule_match(rules[i], fields[i].second))
        return false;
  return true;
}

}