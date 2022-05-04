#ifndef OPGROUP_RULE_H
#define OPGROUP_RULE_H

#include <cstdint>
#include <memory>
#include <vector>
#include "metadata.h"

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
  std::shared_ptr<metadata_t> metadata;

  opgroup_rule_t() {}
  opgroup_rule_t(std::shared_ptr<metadata_t> metadata) : metadata(metadata) {}
  void add_operand_rule(std::vector<uint32_t> values, operand_rule_match_t match);
  bool matches(int32_t flags, uint32_t rs1, uint32_t rs2, uint32_t rs3, uint32_t rd, int32_t imm);
};

} // namespace policy_engine

#endif // OPGROUP_RULE_H
