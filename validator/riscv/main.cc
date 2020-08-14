/*
 * Copyright © 2017-2018 Dover Microsystems, Inc.
 * All rights reserved. 
 *
 * Use and disclosure subject to the following license. 
 *
 * Permission is hereby granted, free of charge, to any person obtaining
 * a copy of this software and associated documentation files (the
 * "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, sublicense, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject to
 * the following conditions:
 * 
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE
 * LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION
 * OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
 * WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

#include <stdio.h>
#include <string>

#include "meta_cache.h"
#include "meta_set_factory.h"
#include "metadata_factory.h"
#include "rv32_validator.h"
#include "metadata_memory_map.h"
#include "tag_file.h"
#include "validator_exception.h"
#include "policy_utils.h"
#include "platform_types.h"
#include <yaml-cpp/yaml.h>

using namespace policy_engine;

meta_set_cache_t ms_cache;
meta_set_factory_t *ms_factory;
metadata_factory_t *md_factory;
rv32_validator_t *rv_validator;
std::string policy_dir;
std::string tags_file;
std::string soc_cfg_path;
std::string rule_cache_name;
int rule_cache_capacity;

static bool DOA = false;

extern "C" void e_v_set_callbacks(RegisterReader_t reg_reader, MemoryReader_t mem_reader, AddressFixer_t addr_fixer) {
  if (!DOA) {
    try {
      printf("setting callbacks\n");
      ms_factory = new meta_set_factory_t(&ms_cache, policy_dir);
      md_factory = new metadata_factory_t(policy_dir);
      soc_tag_configuration_t *soc_config =
        new soc_tag_configuration_t(ms_factory, soc_cfg_path);
      rv_validator = new rv32_validator_t(&ms_cache, md_factory, ms_factory, soc_config, reg_reader, addr_fixer);
      
      metadata_memory_map_t map;
      //      std::string tags_file = std::string(getenv("GENERATED_POLICY_DIR")) + "/../application_tags.taginfo";
      if (!load_tags(&map, tags_file)) {
        printf("failed read\n");
      } else {
        rv_validator->apply_metadata(&map);
      }
      if (rule_cache_name.size() != 0)
        rv_validator->config_rule_cache(rule_cache_name, rule_cache_capacity);
    } catch (exception_t &e) {
      printf("validator exception %s while setting callbacks - policy code DOA\n", e.what());
      DOA = true;
    } catch (std::exception &e) {
      printf("c++ exception %s while setting callbacks - policy code DOA\n", e.what());
      DOA = true;
    } catch (...) {
      printf("c++ exception while setting callbacks - policy code DOA\n");
      DOA = true;
    }
  }
}

extern "C" void e_v_set_metadata(const char* validator_cfg_path) {
  try {
    YAML::Node cfg = YAML::LoadFile(validator_cfg_path);
    if (!cfg) {
      throw configuration_exception_t("Unable to load validator yaml configuration!");
    }
    if (cfg["policy_dir"]) {
      policy_dir = cfg["policy_dir"].as<std::string>();
    }
    else {
      throw configuration_exception_t("Must provide policy directory in validator yaml configuration");
    }
    if (cfg["tags_file"]) {
      tags_file = cfg["tags_file"].as<std::string>();
    }
    else {
      throw configuration_exception_t("Must provide taginfo file path in validator yaml configuration");
    }
    if (cfg["soc_cfg_path"]) {
      soc_cfg_path = cfg["soc_cfg_path"].as<std::string>();
    }
    else {
      throw configuration_exception_t("Must provide soc_cfg file path in validator yaml configuration");
    }
    if (cfg["rule_cache"]) {
      for (auto element: cfg["rule_cache"]) {
        std::string element_string = element.first.as<std::string>();
        if (element_string == "name")
          rule_cache_name = element.second.as<std::string>();
        if (element_string == "capacity")
          rule_cache_capacity = element.second.as<int>();
      }
    }
    printf("set policy dir: %s\n", policy_dir.c_str());
    printf("set taginfo file: %s\n", tags_file.c_str());
    printf("set soc cfg file: %s\n", soc_cfg_path.c_str());
  } catch (std::exception &e) {
      printf("c++ exception %s while setting metadata - policy code DOA\n", e.what());
      DOA = true;
  } catch (...) {
    printf("c++ exception while setting metadata - policy code DOA\n");
    DOA = true;
  }
}

extern "C" uint32_t e_v_validate(uint64_t pc, uint32_t instr) {
  if (pc > ADDRESS_T_MAX)
  {
    printf("Validate PC (0x%lx) Out of Range.\n", pc);
    DOA = true;
    return 0;
  }

  address_t a = (address_t)pc;

//  printf("uncached: validating 0x%x: 0x%x\n", pc, instr);
  if (!DOA) {
    try {
      return rv_validator->validate(a, instr);
    } catch (...) {
      printf("c++ exception while validating - policy code DOA\n");
      DOA = true;
    }
  }
  return 0;
}

extern "C" uint32_t e_v_validate_cached(uint64_t pc, uint32_t instr, uint64_t mem_addr, bool* hit) {
  if ((pc > ADDRESS_T_MAX) || (mem_addr > ADDRESS_T_MAX))
  {
    printf("Cached validate PC (0x%lx) or Mem Address (0x%lx) "
           "Out of Range.\n", pc, mem_addr);
    DOA = true;
    return 0;
  }

  address_t ap = (address_t)pc;
  address_t am = (address_t)mem_addr;

//  printf("cached: validating 0x%x: 0x%x\n", pc, instr);
  if (!DOA) {
    try {
      return rv_validator->validate(ap, instr, am, hit);
    } catch (...) {
      printf("c++ exception while validating - policy code DOA\n");
      DOA = true;
    }
  }
  return 0;
}

extern "C" uint32_t e_v_commit() {
//  printf("committing\n");
  bool hit_watch = false;
  if (!DOA) {
    try {
      hit_watch = rv_validator->commit();
    } catch (...) {
      printf("c++ exception while commiting - policy code DOA\n");
      DOA = true;
    }
  }
  return hit_watch;
}

extern "C" void e_v_flush_rule_cache() {
  rv_validator->flush_rule_cache();
}

extern "C" void e_v_rule_cache_stats() {
  rv_validator->rule_cache_stats();
}

extern "C" void e_v_pc_tag(char* dest, int n) {
  const meta_set_t *ms = (const meta_set_t*) rv_validator->pc_tag;
  meta_set_to_string(ms, dest, n);
}

extern "C" void e_v_csr_tag(char* dest, int n, uint64_t addr) {
  if(addr < 0x1000){
    const meta_set_t *ms = (const meta_set_t*) rv_validator->csr_tags[addr];
    meta_set_to_string((const meta_set_t*)ms, dest, n);
  }
  else
    strncpy(dest, "Out of range", n);
}

extern "C" void e_v_reg_tag(char* dest, int n, uint64_t addr) {
  if(addr < 32){
    const meta_set_t *ms = (const meta_set_t*) rv_validator->ireg_tags[addr];
    meta_set_to_string((const meta_set_t*)ms, dest, n);
  }
  else
    strncpy(dest, "Out of range", n);
}

extern "C" void e_v_mem_tag(char* dest, int n, uint64_t addr) {
  if(addr <= ADDRESS_T_MAX){
    address_t a = (address_t)addr;
    tag_t t;
    if(rv_validator->get_tag(a, t)){
      const meta_set_t *ms = (const meta_set_t*) t;
      meta_set_to_string((const meta_set_t*)ms, dest, n);
    }
    else {
      char tmp[128];
      snprintf(&tmp[0], 128, "Bad Address: %lx\n", addr);
      strncpy(dest, &tmp[0], n);
    }
  }
  else
    strncpy(dest, "Out of range", n);
}

extern "C" const char* eval_status(int status) {
  switch(status) {
  case POLICY_ERROR_FAILURE :
    return "Internal Policy Error";
    break;
  case POLICY_EXP_FAILURE :
    return "Explicit Failure";
    break;
  case POLICY_IMP_FAILURE :
    return "Implicit Failure";
    break;
  case POLICY_SUCCESS :
    return "Success";
    break;
  default:
    return "INVALID POLICY RESULT";
  }
}



extern "C" void e_v_violation_msg(char* dest, int n) {
  // Maybe this belongs inside the validator?
  const int s = 512;
  char tmp[s];

  if(rv_validator->failed){
    std::string msg = "Policy Violation:\n";
    snprintf(tmp, s, "    PC = %lx", rv_validator->failed_ctx.epc);
    msg = msg + tmp;
    if(rv_validator->failed_ctx.bad_addr){
      snprintf(tmp, s, "    MEM = %lx", rv_validator->failed_ctx.bad_addr);
      msg = msg + tmp;
    }
    
    msg = msg + "\n" + "Metadata:\n";
    meta_set_to_string(rv_validator->failed_ops.pc, tmp, s);
    msg = msg + "    Env   : " + tmp + "\n";
    meta_set_to_string(rv_validator->failed_ops.ci, tmp, s);
    msg = msg + "    Code  : " + tmp + "\n";
    meta_set_to_string(rv_validator->failed_ops.op1, tmp, s);
    msg = msg + "    Op1   : " + tmp + "\n";
    meta_set_to_string(rv_validator->failed_ops.op2, tmp, s);
    msg = msg + "    Op2   : " + tmp + "\n";
    meta_set_to_string(rv_validator->failed_ops.op3, tmp, s);
    msg = msg + "    Op3   : " + tmp + "\n";
    meta_set_to_string(rv_validator->failed_ops.mem, tmp, s);
    msg = msg + "    Mem   : " + tmp + "\n";
    msg = msg + eval_status(rv_validator->ctx->policy_result) + "\n";

    if(rv_validator->failed_ctx.fail_msg)
      msg = msg + rv_validator->failed_ctx.fail_msg + "\n";
      
    strncpy(dest, msg.c_str(), n);
  }
  else {
    strncpy(dest, "No Policy Violation", n);
  }
}

extern "C" void e_v_meta_log_short(char* dest, int n) {
    const int s = 512;
    char tmp[s];
    std::string msg = "";
    const char* rule;

    meta_set_to_string(rv_validator->ops->ci, tmp, s);
    msg = msg + "C " + tmp;
    meta_set_to_string(rv_validator->ops->pc, tmp, s);
    msg = msg + " E " + tmp;
    meta_set_to_string(rv_validator->res->pc, tmp, s);
    msg = msg + " -> E " + tmp;
    
    strncpy(dest, msg.c_str(), n);
}

extern "C" void e_v_rule_eval_log(char* dest, int n) {
    const int s = 512;
    char tmp[s];
    std::string msg = "";
    const char* rule;

    msg = msg + "\n" + "Metadata:\n";
    meta_set_to_string(rv_validator->ops->pc, tmp, s);
    msg = msg + "    Env   : " + tmp + "\n";
    meta_set_to_string(rv_validator->ops->ci, tmp, s);
    msg = msg + "    Code  : " + tmp + "\n";
    meta_set_to_string(rv_validator->ops->op1, tmp, s);
    msg = msg + "    Op1   : " + tmp + "\n";
    meta_set_to_string(rv_validator->ops->op2, tmp, s);
    msg = msg + "    Op2   : " + tmp + "\n";
    meta_set_to_string(rv_validator->ops->op3, tmp, s);
    msg = msg + "    Op3   : " + tmp + "\n";
    meta_set_to_string(rv_validator->ops->mem, tmp, s);
    msg = msg + "    Mem   : " + tmp + "\n";

    msg = msg + "\n" + "Results:\n";
    meta_set_to_string(rv_validator->res->pc, tmp, s);
    msg = msg + "    Env   : " + tmp + "\n";
    if(rv_validator->res->rdResult){
        meta_set_to_string(rv_validator->res->rd, tmp, s);
        msg = msg + "    RD    : " + tmp + "\n";
    }
    if(rv_validator->res->csrResult){
        meta_set_to_string(rv_validator->res->csr, tmp, s);
        msg = msg + "    CSR   : " + tmp + "\n";
    }
    
    strncpy(dest, msg.c_str(), n);
}

extern "C" void e_v_set_pc_watch(bool watching){
  rv_validator->set_pc_watch(watching);
}
extern "C" void e_v_set_reg_watch(uint64_t addr){
  if(addr <= ADDRESS_T_MAX)
  {
    address_t a = (address_t)addr;
    rv_validator->set_reg_watch(a);
  }
  else
    printf("Reg Watch Address Out of Range: 0x%lx\n", addr);
}
extern "C" void e_v_set_csr_watch(uint64_t addr){
  if(addr <= ADDRESS_T_MAX)
  {
    address_t a = (address_t)addr;
    rv_validator->set_csr_watch(a);
  }
  else
    printf("CSR Watch Address Out of Range: 0x%lx\n", addr);
}
extern "C" void e_v_set_mem_watch(uint64_t addr){
  if(addr <= ADDRESS_T_MAX)
  {
    address_t a = (address_t)addr;
    rv_validator->set_mem_watch(a);
  }
  else
    printf("Mem Watch Address Out of Range: 0x%lx\n", addr);
}
