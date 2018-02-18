#include "soc_tag_configuration.h"
#include "rv32_validator.h"
#include "validator_exception.h"

#include "policy_utils.h"

using namespace policy_engine;

rv32_validator_t::rv32_validator_t(meta_set_cache_t *ms_cache,
				   meta_set_factory_t *ms_factory,
				   soc_tag_configuration_t *config,
				   RegisterReader_t rr) :
  tag_based_validator_t(ms_cache, ms_factory, rr) {
  ctx = (context_t *)malloc(sizeof(context_t));
  ops = (operands_t *)malloc(sizeof(operands_t));
  res = (results_t *)malloc(sizeof(results_t));
  res->pc = (meta_set_t *)malloc(sizeof(meta_set_t));
  res->rd = (meta_set_t *)malloc(sizeof(meta_set_t));
  res->csr = (meta_set_t *)malloc(sizeof(meta_set_t));
  res->pcResult = false;
  res->rdResult = false;
  res->csrResult = false;

//  soc_tag_configuration_t soc_cfg(&ms_factory, soc_config_file);

  meta_set_t const *ms;

  ms = ms_factory->get_meta_set("requires.dover.riscv.Mach.Reg");
  ireg_tags.reset(m_to_t(ms));
  ms = ms_factory->get_meta_set("requires.dover.riscv.Mach.RegZero");
  ireg_tags[0] = m_to_t(ms);
  ms = ms_factory->get_meta_set("requires.dover.SOC.CSR.Default");
  csr_tags.reset(m_to_t(ms));
  ms = ms_factory->get_meta_set("requires.dover.riscv.Mach.PC");
  pc_tag = m_to_t(ms);

  config->apply(&tag_bus, this);
}

extern std::string render_metadata(metadata_t const *metadata);

void rv32_validator_t::apply_metadata(metadata_memory_map_t *md_map) {
  for (auto &e: *md_map) {
    for (address_t start = e.first.start; start < e.first.end; start += 4) {
//      std::string s = render_metadata(e.second);
//      printf("0x%08x: %s\n", start, s.c_str());
      if (!tag_bus.store_tag(start, m_to_t(ms_cache->canonize(e.second)))) {
	throw validator::configuration_exception_t("unable to apply metadata");
      }
    }
  }
}

bool rv32_validator_t::validate(address_t pc, insn_bits_t insn) {
  int policy_result = POLICY_EXP_FAILURE;
  
  prepare_eval(pc, insn);
  
  policy_result = eval_policy(ctx, ops, res);
//  policy_result = POLICY_SUCCESS;

  if (policy_result == POLICY_SUCCESS) {
    complete_eval();
  }

//  if (policy_result != POLICY_SUCCESS)
//    handle_violation(ctx, ops, res);

  return policy_result == POLICY_SUCCESS;
}

void rv32_validator_t::commit() {
}

void rv32_validator_t::prepare_eval(address_t pc, insn_bits_t insn) {
  uint32_t rs1, rs2, rs3;
  int32_t imm;
  const char *name;
  address_t offset;
  tag_t ci_tag;
  char tag_name[1024];

  int32_t flags;
  
  memset(ctx, 0, sizeof(*ctx));
  memset(ops, 0, sizeof(*ops));

  if(res->pcResult){
    memset(res->pc, 0, sizeof(meta_set_t));
    res->pcResult = false;
  }

  if(res->rdResult){
    memset(res->rd, 0, sizeof(meta_set_t));
    res->rdResult = false;
  }

  if(res->csrResult){
    memset(res->csr, 0, sizeof(meta_set_t));
    res->csrResult = false;
  }
  flags = decode(insn, &rs1, &rs2, &rs3, &pending_RD, &imm, &name);
//  printf("0x%x: 0x%08x   %s\n", pc, insn, name);

  if (flags & HAS_RS1) ops->op1 = t_to_m(ireg_tags[rs1]);
  if (flags & HAS_RS2) ops->op2 = t_to_m(ireg_tags[rs2]);
  if (flags & HAS_RS3) ops->op3 = t_to_m(ireg_tags[rs3]);
  has_pending_RD = (flags & HAS_RD) != 0;
  if (flags & (HAS_LOAD | HAS_STORE)) {
    address_t maddr = reg_reader(rs1);
    if (flags & HAS_IMM)
      maddr += imm;
    ctx->bad_addr = maddr;
//    printf("maddr = 0x%08x\n", maddr);
    tag_t mtag;
    if (!tag_bus.load_tag(maddr, mtag)) {
      printf("failed to load MR tag\n");
    } else {
      ops->mem = t_to_m(mtag);
//      printf("mr tag = 0x%p\n", ops->mem);
    }
  }

  if (!tag_bus.load_tag(pc, ci_tag))
    printf("failed to load CI tag\n");
//    printf("ci_tag: 0x%lx\n", ci_tag);
  ctx->epc = pc;
//  ctx->bad_addr = 0;
//  ctx->cached = false;

  temp_ci_tag = *t_to_m(ci_tag);
  ops->ci = &temp_ci_tag;
//  meta_set_to_string(ops->ci, tag_name, sizeof(tag_name));
//  printf("ci tag name before merge: %s\n", tag_name);
  ops->pc = t_to_m(pc_tag);
}

void rv32_validator_t::complete_eval() {
//  printf("complete eval\n");
}
