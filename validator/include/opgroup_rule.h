#ifndef OPGROUP_RULE_H
#define OPGROUP_RULE_H

#include <stdint.h>
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
    metadata_t *metadata;
    
  public:
    opgroup_rule_t(metadata_t *metadata);
    void add_operand_rule(std::vector<uint32_t> values,
                          operand_rule_match_t match);
    metadata_t *match(int32_t flags, uint32_t rs1, uint32_t rs2,
               uint32_t rs3, uint32_t rd, int32_t imm);
};

} // namespace policy_engine

#endif // OPGROUP_RULE_H
