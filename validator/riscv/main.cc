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

#include <cstdint>
#include <cstdio>
#include <cstring>
#include <limits>
#include <memory>
#include "meta_cache.h"
#include "meta_set_factory.h"
#include "rv_validator.h"
#include "metadata_memory_map.h"
#include "tag_file.h"
#include "validator_exception.h"
#include "policy_utils.h"
#include "platform_types.h"
#include <yaml-cpp/yaml.h>
#include <cinttypes>

// hack to correctly assign address max while the validator expects it to be statically determined
#ifdef RV64_VALIDATOR
uint64_t ADDRESS_T_MAX = std::numeric_limits<uint64_t>::max();
size_t ADDRESS_T_SIZE = sizeof(uint64_t);
#else
uint64_t ADDRESS_T_MAX = std::numeric_limits<uint32_t>::max();
size_t ADDRESS_T_SIZE = sizeof(uint32_t);
#endif

static std::unique_ptr<policy_engine::rv_validator_t> rv_validator = nullptr;
static std::string policy_dir;
static std::string tags_file;
static std::string soc_cfg_path;
static std::string rule_cache_name;
static int rule_cache_capacity;

static bool DOA = false;

extern "C" {

void e_v_set_callbacks(RegisterReader_t reg_reader, MemoryReader_t mem_reader, AddressFixer_t addr_fixer) {
  if (!DOA) {
    try {
      std::printf("setting callbacks\n");
      rv_validator = std::make_unique<policy_engine::rv_validator_t>(policy_dir, soc_cfg_path, reg_reader, addr_fixer);
      
      policy_engine::metadata_memory_map_t map;
      if (!policy_engine::load_tags(map, tags_file)) {
        std::printf("failed read\n");
      } else {
        rv_validator->apply_metadata(&map);
      }
      if (rule_cache_name.size() != 0)
        rv_validator->config_rule_cache(rule_cache_name, rule_cache_capacity);
    } catch (const policy_engine::exception_t& e) {
      std::printf("validator exception %s while setting callbacks - policy code DOA\n", e.what());
      DOA = true;
    } catch (const std::exception& e) {
      std::printf("c++ exception %s while setting callbacks - policy code DOA\n", e.what());
      DOA = true;
    } catch (...) {
      std::printf("c++ exception while setting callbacks - policy code DOA\n");
      DOA = true;
    }
  }
}

void e_v_set_metadata(const char* validator_cfg_path) {
  try {
    YAML::Node cfg = YAML::LoadFile(validator_cfg_path);
    if (!cfg) {
      throw policy_engine::configuration_exception_t("Unable to load validator yaml configuration!");
    }
    if (cfg["policy_dir"]) {
      policy_dir = cfg["policy_dir"].as<std::string>();
    } else {
      throw policy_engine::configuration_exception_t("Must provide policy directory in validator yaml configuration");
    }
    if (cfg["tags_file"]) {
      tags_file = cfg["tags_file"].as<std::string>();
    } else {
      throw policy_engine::configuration_exception_t("Must provide taginfo file path in validator yaml configuration");
    }
    if (cfg["soc_cfg_path"]) {
      soc_cfg_path = cfg["soc_cfg_path"].as<std::string>();
    } else {
      throw policy_engine::configuration_exception_t("Must provide soc_cfg file path in validator yaml configuration");
    }
    if (cfg["rule_cache"]) {
      for (const auto& element: cfg["rule_cache"]) {
        std::string element_string = element.first.as<std::string>();
        if (element_string == "name")
          rule_cache_name = element.second.as<std::string>();
        if (element_string == "capacity")
          rule_cache_capacity = element.second.as<int>();
      }
    }
    std::printf("set policy dir: %s\n", policy_dir.c_str());
    std::printf("set taginfo file: %s\n", tags_file.c_str());
    std::printf("set soc cfg file: %s\n", soc_cfg_path.c_str());
  } catch (const std::exception& e) {
      std::printf("c++ exception %s while setting metadata - policy code DOA\n", e.what());
      DOA = true;
  } catch (...) {
    std::printf("c++ exception while setting metadata - policy code DOA\n");
    DOA = true;
  }
}

uint32_t e_v_validate(uint64_t pc, uint32_t instr) {
  if (pc > ADDRESS_T_MAX) {
    std::printf("Validate PC (0x%lx) Out of Range.\n", pc);
    DOA = true;
    return 0;
  }

  if (!DOA) {
    try {
      return rv_validator->validate(static_cast<address_t>(pc), instr);
    } catch (...) {
      std::printf("c++ exception while validating - policy code DOA\n");
      DOA = true;
    }
  }
  return 0;
}

uint32_t e_v_validate_cached(uint64_t pc, uint32_t instr, uint64_t mem_addr, bool* hit) {
  if (pc > ADDRESS_T_MAX || mem_addr > ADDRESS_T_MAX) {
    std::printf("Cached validate PC (0x%lx) or Mem Address (0x%lx) Out of Range.\n", pc, mem_addr);
    DOA = true;
    return 0;
  }

  if (!DOA) {
    try {
      auto [ success, h ] = rv_validator->validate(static_cast<address_t>(pc), instr, static_cast<address_t>(mem_addr));
      *hit = h;
      return success ? 1 : 0;
    } catch (...) {
      std::printf("c++ exception while validating - policy code DOA\n");
      DOA = true;
    }
  }
  return 0;
}

uint32_t e_v_commit() {
  if (!DOA) {
    try {
      return rv_validator->commit();
    } catch (...) {
      std::printf("c++ exception while commiting - policy code DOA\n");
      DOA = true;
    }
  }
  return false;
}

void e_v_flush_rule_cache() {
  rv_validator->flush_rule_cache();
}

void e_v_rule_cache_stats() {
  rv_validator->rule_cache_stats();
}

void e_v_pc_tag(char* dest, int n) {
  meta_set_to_string(rv_validator->get_pc_meta_set(), dest, n);
}

void e_v_csr_tag(char* dest, int n, uint64_t addr) {
  if (addr < 0x1000) {
    meta_set_to_string(rv_validator->get_csr_meta_set(addr), dest, n);
  } else
    std::strncpy(dest, "Out of range", n);
}

void e_v_reg_tag(char* dest, int n, uint64_t addr) {
  if (addr < 32) {
    meta_set_to_string(rv_validator->get_ireg_meta_set(addr), dest, n);
  } else
    std::strncpy(dest, "Out of range", n);
}

void e_v_mem_tag(char* dest, int n, uint64_t addr) {
  if (addr <= ADDRESS_T_MAX) {
    if (meta_set_t* ms = rv_validator->get_meta_set(static_cast<address_t>(addr))) {
      meta_set_to_string(ms, dest, n);
    } else {
      std::snprintf(dest, n, "Bad Address: %lx\n", addr);
    }
  } else
    std::strncpy(dest, "Out of range", n);
}

const char* eval_status(int status) {
  switch(status) {
    case policy_engine::POLICY_ERROR_FAILURE: return "Internal Policy Error";
    case policy_engine::POLICY_EXP_FAILURE:   return "Explicit Failure";
    case policy_engine::POLICY_IMP_FAILURE:   return "Implicit Failure";
    case policy_engine::POLICY_SUCCESS:       return "Success";
    default: return "INVALID POLICY RESULT";
  }
}



void e_v_violation_msg(char* dest, int n) {
  // Maybe this belongs inside the validator?
  constexpr int s = 512;
  char tmp[s];

  if (rv_validator->failed) {
    std::string msg = "Policy Violation:\n";
    std::snprintf(tmp, s, "    PC = %lx", rv_validator->failed_ctx.epc);
    msg = msg + tmp;
    if (rv_validator->failed_ctx.bad_addr) {
      std::snprintf(tmp, s, "    MEM = %lx", rv_validator->failed_ctx.bad_addr);
      msg = msg + tmp;
    }
    
    msg = msg + "\nMetadata:\n";
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
    msg = msg + eval_status(rv_validator->ctx.policy_result) + "\n";

    if(rv_validator->failed_ctx.fail_msg)
      msg = msg + rv_validator->failed_ctx.fail_msg + "\n";
      
    std::strncpy(dest, msg.c_str(), n);
  }
  else {
    std::strncpy(dest, "No Policy Violation", n);
  }
}

void e_v_meta_log_short(char* dest, int n) {
  constexpr int s = 512;
  char tmp[s];
  std::string msg;

  meta_set_to_string(rv_validator->ops.ci, tmp, s);
  msg = msg + "C " + tmp;
  meta_set_to_string(rv_validator->ops.pc, tmp, s);
  msg = msg + " E " + tmp;
  meta_set_to_string(rv_validator->res.pc, tmp, s);
  msg = msg + " -> E " + tmp;
  
  std::strncpy(dest, msg.c_str(), n);
}

void e_v_rule_eval_log(char* dest, int n) {
  const int s = 512;
  char tmp[s];
  std::string msg = "\nMetadata:\n";

  meta_set_to_string(rv_validator->ops.pc, tmp, s);
  msg = msg + "    Env   : " + tmp + "\n";
  meta_set_to_string(rv_validator->ops.ci, tmp, s);
  msg = msg + "    Code  : " + tmp + "\n";
  meta_set_to_string(rv_validator->ops.op1, tmp, s);
  msg = msg + "    Op1   : " + tmp + "\n";
  meta_set_to_string(rv_validator->ops.op2, tmp, s);
  msg = msg + "    Op2   : " + tmp + "\n";
  meta_set_to_string(rv_validator->ops.op3, tmp, s);
  msg = msg + "    Op3   : " + tmp + "\n";
  meta_set_to_string(rv_validator->ops.mem, tmp, s);
  msg = msg + "    Mem   : " + tmp + "\n";

  msg = msg + "\nResults:\n";
  meta_set_to_string(rv_validator->res.pc, tmp, s);
  msg = msg + "    Env   : " + tmp + "\n";
  if (rv_validator->res.rdResult){
    meta_set_to_string(rv_validator->res.rd, tmp, s);
    msg = msg + "    RD    : " + tmp + "\n";
  }
  if (rv_validator->res.csrResult){
    meta_set_to_string(rv_validator->res.csr, tmp, s);
    msg = msg + "    CSR   : " + tmp + "\n";
  }
  
  std::strncpy(dest, msg.c_str(), n);
}

void e_v_set_pc_watch(bool watching){
  rv_validator->set_pc_watch(watching);
}

void e_v_set_reg_watch(uint64_t addr){
  if(addr <= ADDRESS_T_MAX) {
    rv_validator->set_reg_watch(static_cast<address_t>(addr));
  } else
    std::printf("Reg Watch Address Out of Range: 0x%lx\n", addr);
}

void e_v_set_csr_watch(uint64_t addr){
  if (addr <= ADDRESS_T_MAX) {
    rv_validator->set_csr_watch(static_cast<address_t>(addr));
  } else
    std::printf("CSR Watch Address Out of Range: 0x%lx\n", addr);
}

void e_v_set_mem_watch(uint64_t addr){
  if(addr <= ADDRESS_T_MAX) {
    rv_validator->set_mem_watch(static_cast<address_t>(addr));
  }
  else
    std::printf("Mem Watch Address Out of Range: 0x%lx\n", addr);
}

} // extern "C"