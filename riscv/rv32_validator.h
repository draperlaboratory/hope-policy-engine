#ifndef RV32_VALIDATOR_H
#define RV32_VALIDATOR_H

#include <stdio.h>
#include <string>

#include "soc_tag_configuration.h"
#include "tag_based_validator.h"
#include "tag_converter.h"
#include "policy_eval.h"
#include "metadata_memory_map.h"

#define REG_SP 2
class rv32_validator_t : public tag_based_validator_t {
  context_t *ctx;
  operands_t *ops;
  results_t *res;
  tag_bus_t tag_bus;
  tag_file_t<32> ireg_tags;
  tag_file_t<0x1000> csr_tags;
  tag_t pc_tag;
  uint32_t pending_RD;
  bool has_pending_RD;
  meta_set_t temp_ci_tag;
  public:
//  rv32_validator_t(std::string policy_dir, std::string soc_config_file, RegisterReader_t rr);
  rv32_validator_t(meta_set_cache_t *ms_cache,
		   meta_set_factory_t *ms_factory,
		   soc_tag_configuration_t *tag_config,
		   RegisterReader_t rr);

  void apply_metadata(metadata_memory_map_t *md_map);
/*
 : reg_reader(rr), mem_reader(mr) {
    ctx = (context_t *)malloc(sizeof(context_t));
    ops = (operands_t *)malloc(sizeof(operands_t));
    res = (results_t *)malloc(sizeof(results_t));
*/
//    alloc_eval_params(&context, &ops, &res); // TODO: error handling
//    printf("context = 0x%p, ops = 0x%p, res = 0x%p\n", ctx, ops, res);
//  }
  virtual ~rv32_validator_t() {
    free(ctx);
    free(ops);
    free(res);
  }
  bool validate(address_t pc, insn_bits_t insn);
  void commit();
  
  void prepare_eval(address_t pc, insn_bits_t insn);
  void complete_eval();
};

#endif
