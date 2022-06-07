#include <algorithm>
#include <cstdint>
#include <gelf.h>
#include <iostream>
#include <map>
#include <sstream>
#include <stdexcept>
#include <string>
#include <unistd.h>
#include <yaml-cpp/yaml.h>
#include "elf_loader.h"
#include "llvm_metadata_tagger.h"
#include "range_map.h"

namespace policy_engine {

uint64_t round_up(uint64_t x, uint64_t align) {
  return ((x + align - 1)/align)*align;
}

const std::map<std::string, uint8_t> llvm_metadata_tagger_t::metadata_ops = {
  {"DMD_SET_BASE_ADDRESS_OP", 0x1},
  {"DMD_TAG_ADDRESS_OP", 0x2},
  {"DMD_TAG_ADDRESS_RANGE_OP", 0x3},
  {"DMD_TAG_POLICY_SYMBOL", 0x4},
  {"DMD_TAG_POLICY_RANGE", 0x5},
  {"DMD_TAG_POLICY_SYMBOL_RANKED", 0x6},
  {"DMD_TAG_POLICY_RANGE_RANKED", 0x7},
  {"DMD_END_BLOCK", 0x8},
  {"DMD_END_BLOCK_WEAK_DECL_HACK", 0x9},
  {"DMD_FUNCTION_RANGE", 0xa}
};

const std::map<std::string, uint8_t> llvm_metadata_tagger_t::tag_specifiers = {
  {"DMT_CFI3L_VALID_TGT", 0x1},
  {"DMT_STACK_PROLOGUE_AUTHORITY", 0x2},
  {"DMT_STACK_EPILOGUE_AUTHORITY", 0x3},
  {"DMT_FPTR_STORE_AUTHORITY", 0x4},
  {"DMT_BRANCH_VALID_TGT", 0x5},
  {"DMT_RET_VALID_TGT", 0x6},
  {"DMT_RETURN_INSTR", 0x7},
  {"DMT_CALL_INSTR", 0x8},
  {"DMT_BRANCH_INSTR", 0x9},
  {"DMT_FPTR_CREATE_AUTHORITY", 0xa}
};

const std::map<std::string, std::map<std::string, std::string>> llvm_metadata_tagger_t::policy_map = {
  {"call-tgt", {
    {"tag_specifier", "DMT_CFI3L_VALID_TGT"},
    {"name", "llvm.CFI_Call-Tgt"}
  }},
  {"branch-tgt", {
    {"tag_specifier", "DMT_BRANCH_VALID_TGT"},
    {"name", "llvm.CFI_Branch-Tgt"}
  }},
  {"return-tgt", {
    {"tag_specifier", "DMT_RET_VALID_TGT"},
    {"name", "llvm.CFI_Return-Tgt"}
  }},
  {"call-instr", {
    {"tag_specifier", "DMT_CALL_INSTR"},
    {"name", "llvm.CFI_Call-Instr"}
  }},
  {"branch-instr", {
    {"tag_specifier", "DMT_BRANCH_INSTR"},
    {"name", "llvm.CFI_Branch-Instr"}
  }},
  {"return-instr", {
    {"tag_specifier", "DMT_RETURN_INSTR"},
    {"name", "llvm.CFI_Return-Instr"}
  }},
  {"fptrcreate", {
    {"tag_specifier", "DMT_FPTR_CREATE_AUTHORITY"},
    {"name", "llvm.CPI.FPtrCreate"}
  }},
  {"fptrstore", {
    {"tag_specifier", "DMT_FPTR_STORE_AUTHORITY"},
    {"name", "llvm.CPI.FPtrStore"}
  }},
  {"prologue", {
    {"tag_specifier", "DMT_STACK_PROLOGUE_AUTHORITY"},
    {"name", "llvm.Prologue"}
  }},
  {"epilogue", {
    {"tag_specifier", "DMT_STACK_EPILOGUE_AUTHORITY"},
    {"name", "llvm.Epilogue"}
  }}
};

bool llvm_metadata_tagger_t::policy_needs_tag(const YAML::Node& node, const std::string& tag) {
  if (needs_tag_cache.find(tag) != needs_tag_cache.end())
    return needs_tag_cache[tag];
  
  std::string::size_type pos = tag.find('.');
  std::string item = tag.substr(0, pos);
  if (!node[item]) {
    needs_tag_cache[tag] = false;
    return false;
  } else if (pos == std::string::npos) {
    needs_tag_cache[tag] = true;
    return true;
  } else {
    bool needs_tag = policy_needs_tag(node[item], tag.substr(pos + 1, std::string::npos));
    needs_tag_cache[tag] = needs_tag;
    return needs_tag;
  }
}

void llvm_metadata_tagger_t::add_code_section_ranges(const elf_image_t& ef, range_map_t& range_map) {
  for (int i = 0; i < ef.sections.size(); i++) {
    if (ef.sections[i].flags & SHF_ALLOC) {
      uint64_t start = ef.sections[i].address;
      uint64_t end = round_up(start + ef.sections[i].size, PTR_SIZE);
      if ((ef.sections[i].flags & (SHF_ALLOC | SHF_WRITE | SHF_EXECINSTR)) == (SHF_ALLOC | SHF_EXECINSTR)) {
        err.info("saw code range = %#lx:%#lx\n", start, end);
        range_map.add_range(start, end);
      }
    }
  }
}

void llvm_metadata_tagger_t::check_and_add_range(range_map_t& range_map, uint64_t start, uint64_t end, uint8_t tag_specifier, const YAML::Node& policy_inits) {
  for (const auto& [ policy, tags ] : policy_map) {
    if (policy_needs_tag(policy_inits["Require"], tags.at("name"))) {
      if (tag_specifiers.at(tags.at("tag_specifier")) == tag_specifier) {
        err.info("saw tag %s = %#lx:%#lx\n", tags.at("name").c_str(), start, end);
        range_map.add_range(start, end, tags.at("name"));
      }
    }
  }
}

void llvm_metadata_tagger_t::add_policy_ranges(range_map_t& range_map, const elf_image_t& ef, const YAML::Node& policy_inits) {
  auto metadata_section = std::find_if(ef.sections.begin(), ef.sections.end(), [](const elf_section_t& s){ return s.name == ".dover_metadata"; });
  if (metadata_section == ef.sections.end())
    throw std::runtime_error("no metadata found in ELF file");
  uint8_t* metadata = reinterpret_cast<uint8_t*>(metadata_section->data);
  if (metadata[0] != metadata_ops.at("DMD_SET_BASE_ADDRESS_OP"))
    throw std::runtime_error("invalid metadata found in ELF file");
  
  uint64_t base_address = 0;
  for (int i = 0; i < metadata_section->size;) {
    // Don't increment i at the end of the loop because it should point to the next op after the if block
    uint8_t op = metadata[i++];
    if (op == metadata_ops.at("DMD_SET_BASE_ADDRESS_OP")) {
      base_address = 0;
      for (int j = 0; j < 8; j++, i++)
        base_address += metadata[i] << (j*8);
      err.info("new base address is %#x\n", base_address);
    } else if (op == metadata_ops.at("DMD_TAG_ADDRESS_OP")) {
      uint64_t address = base_address;
      for (int j = 0; j < PTR_SIZE; j++, i++)
        address += metadata[i] << (j*8);
      uint8_t tag_specifier = metadata[i++];
      err.info("tag is %#x at address %#lx\n", tag_specifier, address);
      check_and_add_range(range_map, address, address + PTR_SIZE, tag_specifier, policy_inits);
    } else if (op == metadata_ops.at("DMD_TAG_ADDRESS_OP")) {
      uint64_t start_address = base_address, end_address = base_address;
      for (int j = 0; j < PTR_SIZE; j++, i++)
        start_address += metadata[i] << (j*8);
      for (int j = 0; j < PTR_SIZE; j++, i++)
        end_address += metadata[i] << (j*8);
      uint8_t tag_specifier = metadata[i++];
      err.info("tag is %#x for address range %#lx:%#lx\n", tag_specifier, start_address, end_address);
      check_and_add_range(range_map, start_address, end_address, tag_specifier, policy_inits);
    } else if (op == metadata_ops.at("DMD_END_BLOCK")) {
      uint64_t end_address = 0;
      for (int j = 0; j < PTR_SIZE; j++, i++)
        end_address += metadata[i] << (j*8);
      err.info("saw end block tag range = %#lx:%#lx\n", base_address, base_address + end_address);
    } else if (op == metadata_ops.at("DMD_FUNCTION_RANGE")) {
      uint64_t start_address = base_address, end_address = base_address;
      for (int j = 0; j < PTR_SIZE; j++, i++)
        start_address += metadata[i] << (j*8);
      for (int j = 0; j < PTR_SIZE; j++, i++)
        end_address += metadata[i] << (j*8);
      err.info("saw function range = %#lx:%#lx\n", start_address, end_address);
    } else if (op == metadata_ops.at("DMD_TAG_POLICY_SYMBOL")) {
      throw std::runtime_error("saw policy symbol");
    } else if (op == metadata_ops.at("DMD_TAG_POLICY_RANGE")) {
      throw std::runtime_error("saw policy range");
    } else if (op == metadata_ops.at("DMD_TAG_POLICY_SYMBOL_RANKED")) {
      throw std::runtime_error("saw policy symbol ranked");
    } else if (op == metadata_ops.at("DMD_TAG_POLICY_RANGE_RANKED")) {
      throw std::runtime_error("saw policy range ranked");
    } else if (op == metadata_ops.at("DMD_END_BLOCK_WEAK_DECL_HACK")) {
      throw std::runtime_error("saw end weak decl tag");
    } else {
      throw std::runtime_error("found unknown byte in metadata: " + std::to_string(metadata[i]));
    }
  }

  if (policy_inits["Require"]["llvm"]["NoCFI"]) {
    range_map_t code_range_map;
    add_code_section_ranges(ef, code_range_map);
    for (auto& [ range, tags ] : code_range_map) {
      for (uint64_t s = range.start; s < range.end; s += PTR_SIZE) {
        uint64_t e = s + PTR_SIZE;
        if (!range_map.contains(tagged_range_t{{s, e}, tags})) {
          err.info("llvm.NoCFI range = %lx:%lx\n", s, e);
          range_map.add_range(s, e, "llvm.NoCFI");
        }
      }
    }
  }
}

} // namespace policy_engine