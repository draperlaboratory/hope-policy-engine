#ifndef OPGROUP_RULE_H
#define OPGROUP_RULE_H

#include <cstdint>
#include <memory>
#include <vector>
#include "metadata.h"
#include "riscv_isa.h"

namespace policy_engine {

typedef enum {
  OPERAND_RULE_UNKNOWN,
  OPERAND_RULE_ANY,
  OPERAND_RULE_EQUAL,
  OPERAND_RULE_NOT,
  OPERAND_RULE_RANGE,
  OPERAND_RULE_NOT_RANGE,
} operand_rule_match_t;

struct operand_rule_t {
  std::vector<uint32_t> values;
  operand_rule_match_t match;
};

class opgroup_rule_t {
private:
  std::vector<operand_rule_t> rules;
  
public:
  std::unique_ptr<const metadata_t> metadata;

  opgroup_rule_t() {}
  opgroup_rule_t(std::unique_ptr<metadata_t>& metadata) : metadata(std::move(metadata)) {}
  void add_operand_rule(std::vector<uint32_t> values, operand_rule_match_t match);
  bool matches(const decoded_instruction_t& inst);
};

} // namespace policy_engine

#endif // OPGROUP_RULE_H
