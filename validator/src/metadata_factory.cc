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

#include <algorithm>
#include <cstring>
#include <cstdint>
#include <cstdio>
#include <exception>
#include <linux/limits.h>
#include <memory>
#include <sstream>
#include <string>
#include <unordered_map>
#include <vector>
#include <yaml-cpp/yaml.h>
#include "entity_binding.h"
#include "metadata_factory.h"
#include "metadata_memory_map.h"
#include "opgroup_rule.h"
#include "platform_types.h"
#include "policy_meta_set.h"
#include "policy_types.h"
#include "riscv_isa.h"
#include "validator_exception.h"

namespace policy_engine {

std::string metadata_factory_t::abbreviate(const std::string& dotted_string) {
  size_t last = dotted_string.rfind('.');
  if (last == std::string::npos)
    return dotted_string;
  return dotted_string.substr(last + 1, std::string::npos);
}

void metadata_factory_t::init_entity_initializers(const YAML::Node& reqsAST, const std::string& prefix) {
  for (const auto& it : reqsAST) {
    std::string key = it.first.as<std::string>();
    if (key == "metadata") {
      entity_init_t init;
      init.entity_name = prefix;
      for (const YAML::Node& m : it.second)
        init.meta_names.push_back(m["name"].as<std::string>());
      entity_initializers[prefix] = init;
    } else {
      init_entity_initializers(it.second, prefix == "" ? key : prefix + "." + key);
    }
  }
}

void metadata_factory_t::update_entity_initializers(const YAML::Node& metaAST, const std::string& prefix) {
  for (const YAML::Node& metadata_node : metaAST) {
    std::string mname = metadata_node["name"].as<std::string>();
    entity_init_t init;
    init.entity_name = mname;
    init.meta_names.push_back(mname);
    entity_initializers[mname] = init;
  }
}

void metadata_factory_t::init_encoding_map(const YAML::Node& rawEnc) {
  for (const YAML::Node& node: rawEnc["Metadata"]) {
    encoding_map[node["name"].as<std::string>()] = node["id"].as<meta_t>();
    reverse_encoding_map[node["id"].as<meta_t>()] = node["name"].as<std::string>();
    abbrev_reverse_encoding_map[node["id"].as<meta_t>()] = abbreviate(node["name"].as<std::string>());
  }
}

static std::vector<std::string> split_dotted_name(const std::string &name) {
  std::vector<std::string> res;
  std::string elt;
  for (std::istringstream ss(name); std::getline(ss, elt, '.');)
    res.push_back(elt);
  return res;
}

static void dump_node(YAML::Node node) {
  printf("node: %p\n", &node);
  switch (node.Type()) {
    case YAML::NodeType::Null: printf("  null\n"); break;
    case YAML::NodeType::Scalar: printf("  scalar\n"); break;
    case YAML::NodeType::Sequence: printf("  sequence\n"); break;
    case YAML::NodeType::Map: printf("  map\n"); break;
    case YAML::NodeType::Undefined: printf("  undefined\n"); break;
    default: printf("  unknown\n"); break;
  }
}

const metadata_t* metadata_factory_t::lookup_metadata(const std::string& dotted_path) {
  if (const auto& it = path_map.find(dotted_path); it != path_map.end())
    return it->second.get();

  if (const auto& it = entity_initializers.find(dotted_path); it != entity_initializers.end()) {
    std::unique_ptr<metadata_t> md = std::make_unique<metadata_t>();
    for (const std::string& name : it->second.meta_names)
      md->insert(encoding_map[name]);
    return (path_map[dotted_path] = std::move(md)).get();
  }
  return nullptr;
}

std::map<std::string, const metadata_t*> metadata_factory_t::lookup_metadata_map(const std::string& dotted_path) {
  std::map<std::string, const metadata_t*> results;
  for (const auto& [ name, init ] : entity_initializers)
    if (name.rfind(dotted_path) == 0)
      results[name] = lookup_metadata(name);
  return results;
}

const metadata_t* metadata_factory_t::lookup_group_metadata(const std::string& opgroup, const decoded_instruction_t& inst) {
  const auto& it_opgroup_rule = opgroup_rule_map.find(opgroup);
  if (it_opgroup_rule != opgroup_rule_map.end()) {
    if (it_opgroup_rule->second.matches(inst))
      return it_opgroup_rule->second.metadata.get();
  }

  const auto& it_group = group_map.find(opgroup);
  if (it_group == group_map.end()) {
    return nullptr;
  }
  return it_group->second.get();
}

static const std::unordered_map<std::string, operand_rule_match_t> operand_match_yaml_ids {
  {"match_all", OPERAND_RULE_ANY},
  {"match_equal", OPERAND_RULE_EQUAL},
  {"match_not_equal", OPERAND_RULE_NOT},
  {"match_in_range", OPERAND_RULE_RANGE},
  {"match_not_in_range", OPERAND_RULE_NOT_RANGE},
};

void metadata_factory_t::update_rule_map(std::string key, const YAML::Node& node) {
  std::string name;
  YAML::Node operand_rules;
  std::unique_ptr<metadata_t> metadata = std::make_unique<metadata_t>();
  for (const auto& it : node) {
    name = it.first.as<std::string>();
    operand_rules = it.second;
  }
  metadata->insert(encoding_map[name]);
  opgroup_rule_map[key] = opgroup_rule_t(metadata);

  for (const YAML::Node& operand_rule : operand_rules) {
    if (operand_rule.IsScalar()) {
      opgroup_rule_map[key].add_operand_rule(std::vector<uint32_t>(), OPERAND_RULE_ANY);
    } else if (operand_rule.IsMap()) {
      std::string operand_id;
      std::vector<uint32_t> values;
      for (const auto& it : operand_rule) {
        operand_id = it.first.as<std::string>();
        for (const YAML::Node& n : it.second)
          values.push_back(n.as<uint32_t>());
      }

      auto it = std::find_if(
        operand_match_yaml_ids.begin(), operand_match_yaml_ids.end(),
        [&](const std::pair<std::string, operand_rule_match_t>& e){ return operand_id == e.first; }
      );
      if (it == operand_match_yaml_ids.end())
        throw std::runtime_error("Invalid operand rule type: " + operand_id);
      else
        opgroup_rule_map[key].add_operand_rule(values, it->second);
    }
  }
}

void metadata_factory_t::init_group_map(const YAML::Node& node) {
  for (const auto& it : node["Groups"]) {
    const std::string key = it.first.as<std::string>();
    std::unique_ptr<metadata_t> md = std::make_unique<metadata_t>();
    const YAML::Node& instruction_node = it.second;

    for (const YAML::Node& opgroup_node : instruction_node) {
      if (opgroup_node.IsScalar()) {
        std::string name = opgroup_node.as<std::string>();
        md->insert(encoding_map[name]);
      }

      if (opgroup_node.IsMap()) {
        update_rule_map(key, opgroup_node);
      }
    }
    group_map[key] = std::move(md);
  }
}

YAML::Node metadata_factory_t::load_yaml(const std::string& yml_file) {
  const std::string path = policy_dir + "/" + yml_file;
  try {
    return YAML::LoadFile(path);
  } catch (const std::exception& e) {
    throw configuration_exception_t("while parsing " + path + ": " + e.what());
  }
}

metadata_factory_t::metadata_factory_t(const std::string& policy_dir) : policy_dir(policy_dir) {
  // load up all the requirements for initialization
  YAML::Node reqsAST = load_yaml("policy_init.yml");
  // load up the individual tag encodings
  YAML::Node metaAST = load_yaml("policy_meta.yml");
  // meta_tree.populate(reqsAST);
  init_entity_initializers(reqsAST["Require"], "");
  update_entity_initializers(metaAST["Metadata"], "");
  init_encoding_map(metaAST);
  YAML::Node groupAST = load_yaml("policy_group.yml");
  init_group_map(groupAST);
}

bool metadata_factory_t::apply_tag(metadata_memory_map_t& map, uint64_t start, uint64_t end, const std::string& tag_name) {
  if (const metadata_t* md = lookup_metadata(tag_name)) {
    map.add_range(start, end, *md);
    return true;
  }
  return false;
}

void metadata_factory_t::tag_opcodes(metadata_memory_map_t& map, uint64_t code_address, int xlen, void* bytes, int n, reporter_t& err) {
  insn_bits_t* bits = reinterpret_cast<insn_bits_t*>(bytes);
  for (int i = 0; i < n/sizeof(insn_bits_t); i++) {
    decoded_instruction_t inst = decode(bits[i], xlen);
    if (!inst) {
      err.warning("Failed to decode instruction 0x%08x at address %#x\n", bits[i], code_address);
      code_address += 4;
      continue;
    }

    if (const metadata_t* metadata = lookup_group_metadata(inst.name, inst))
      map.add_range(code_address, code_address + 4, *metadata);
    else
      err.warning("0x%016lx: 0x%08x  %s - no group found for instruction\n", code_address, bits[i], inst.name);
    code_address += inst.flags.is_compressed ? 2 : 4;
  }
}

void metadata_factory_t::tag_entities(metadata_memory_map_t& md_map, const elf_image_t& img, const std::vector<std::string>& yaml_files, reporter_t& err) {
  std::list<std::unique_ptr<entity_binding_t>> bindings;
  for (const std::string& yaml_file : yaml_files)
    bindings.splice(bindings.end(), entity_binding_t::load(yaml_file, err));
  for (const std::string& s : enumerate()) {
    auto it = std::find_if(bindings.begin(), bindings.end(), [&](std::unique_ptr<entity_binding_t>& peb) { return peb->entity_name == s; });
    if (it == bindings.end()) {
      err.warning("Entity %s has no binding\n", s);
    }
  }

  for (const std::unique_ptr<entity_binding_t>& e: bindings) {
    if (const auto sb = dynamic_cast<entity_symbol_binding_t*>(e.get())) {
      if (auto sym = img.symtab.find(sb->elf_name, !sb->is_singularity, sb->optional, err); sym != img.symtab.end()) {
        // go ahead and mark it
        uint64_t end_addr;
        if (sb->is_singularity)
          end_addr = sym->address + img.word_bytes();
        else
          end_addr = sym->address + sym->size; // TODO: align to platform word boundary?
        if (!apply_tag(md_map, sym->address, end_addr, sb->entity_name)) {
          err.warning("Unable to apply tag %s\n", sb->entity_name);
        }
      }
    } else if (const auto rb = dynamic_cast<entity_range_binding_t*>(e.get())) {
      auto sym = img.symtab.find(rb->elf_start_name, false, false, err);
      auto end = img.symtab.find(rb->elf_end_name, false, false, err);
      if (sym != img.symtab.end() && end != img.symtab.end()) {
        if (!apply_tag(md_map, sym->address, end->address, rb->entity_name)) {
          err.warning("Unable to apply tag %s\n", rb->entity_name);
        }
      }
    }
  }
}

std::vector<std::string> metadata_factory_t::enumerate() {
  std::vector<std::string> elts;
  for (const auto& [ name, init ] : entity_initializers)
    elts.push_back(name);
  return elts;
}

std::string metadata_factory_t::render(meta_t meta, bool abbrev) const {
  if (abbrev) {
    const auto iter = abbrev_reverse_encoding_map.find(meta);
    if (iter == abbrev_reverse_encoding_map.end())
      return "<unknown: " + std::to_string(meta) + ">";
    else
      return iter->second;
  } else {
    const auto iter = reverse_encoding_map.find(meta);
    if (iter == reverse_encoding_map.end())
      return "<unknown: " + std::to_string(meta) + ">";
    else
      return iter->second;
  }
}

}