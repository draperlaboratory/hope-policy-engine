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

#include <linux/limits.h>
#include <string.h>
#include <stdio.h>
#include <sstream>
#include <exception>

#include "validator_exception.h"
#include "metadata_factory.h"

using namespace policy_engine;

std::string metadata_factory_t::abbreviate(std::string const &dotted_string) {
  size_t last = dotted_string.rfind('.');
  if (last == std::string::npos)
    return dotted_string;
  return dotted_string.substr(last + 1, std::string::npos);
}

void metadata_factory_t::init_entity_initializers(YAML::Node const &reqsAST, std::string prefix) {
  for (YAML::const_iterator it = reqsAST.begin(); it != reqsAST.end(); ++it) {
    std::string key = it->first.as<std::string>();
    if (key == "metadata") {
      entity_init_t init;
      init.entity_name = prefix;
      YAML::Node mnode = it->second;
      for (size_t i = 0; i < mnode.size(); i++) {
        std::string name = mnode[i]["name"].as<std::string>();
        init.meta_names.push_back(name);
      }
      entity_initializers[prefix] = init;
//      printf("adding: %s\n", prefix.c_str());
    } else {
      init_entity_initializers(it->second, prefix == "" ? key : prefix + "." + key);
    }
  }
}

void metadata_factory_t::update_entity_initializers(YAML::Node const &metaAST, std::string prefix) {
  YAML::Node metadata = metaAST; //[0]; //["Metadata"];
  // for each node in Metadata
  for (YAML::const_iterator seq_it = metadata.begin(); seq_it != metadata.end(); ++seq_it) {
          YAML::Node metadata_node = *seq_it;
          std::string mname = metadata_node["name"].as<std::string>();
          entity_init_t init;
          init.entity_name = mname;
          init.meta_names.push_back(mname);
          entity_initializers[mname] = init;
  }
}

void metadata_factory_t::init_encoding_map(YAML::Node &rawEnc) {
  YAML::Node root = rawEnc["Metadata"];
  YAML::Node node;
  
  for (size_t i = 0; i < root.size(); i++) {
    node = root[i];
    encoding_map[node["name"].as<std::string>()] = node["id"].as<meta_t>();
    reverse_encoding_map[node["id"].as<meta_t>()] = node["name"].as<std::string>();
    abbrev_reverse_encoding_map[node["id"].as<meta_t>()] = abbreviate(node["name"].as<std::string>());
  }
}

std::vector<std::string> metadata_factory_t::split_dotted_name(const std::string &name) {
  std::istringstream ss(name);
  
  std::vector<std::string> res;
  std::string elt;
  while (std::getline(ss, elt, '.'))
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

metadata_t const *metadata_factory_t::lookup_metadata(std::string dotted_path) {
  metadata_t *metadata = nullptr;
  auto const &path_map_iter = path_map.find(dotted_path);
  if (path_map_iter != path_map.end()) {
    return path_map_iter->second;
  }

  auto const &entity_init_iter = entity_initializers.find(dotted_path);
  if (entity_init_iter != entity_initializers.end()) {
          std::vector<std::string> const &meta_names = entity_init_iter->second.meta_names;
          metadata = new metadata_t();
          for (auto name: meta_names) {
                  metadata->insert(encoding_map[name]);
          }
          path_map[dotted_path] = metadata;
          return metadata;
  }
  return NULL;
#if 0
  std::vector<std::string> path = split_dotted_name(dotted_path);
  
  std::vector<std::string> md;
  if (meta_tree.find_metadata(path, md)) {
    metadata = new metadata_t();
    for (auto name: md)
      metadata->insert(encoding_map[name]);
    path_map[dotted_path] = metadata;
  }
  return metadata;
#endif
}

std::map<std::string, metadata_t const *> *metadata_factory_t::lookup_metadata_map(std::string dotted_path) {
  std::map<std::string, metadata_t const *> *results = new std::map<std::string, metadata_t const *>();

  for (auto &it : entity_initializers) {
    if (it.first.rfind(dotted_path) == 0) {
      (*results)[it.first] = lookup_metadata(it.first);
    }
  }

  return results;
}

static const std::unordered_map<std::string, operand_rule_match_t> operand_match_yaml_ids {
  {"match_all", OPERAND_RULE_ANY},
  {"match_equal", OPERAND_RULE_EQUAL},
  {"match_not_equal", OPERAND_RULE_NOT},
  {"match_in_range", OPERAND_RULE_RANGE},
  {"match_not_in_range", OPERAND_RULE_NOT_RANGE},
};

void metadata_factory_t::update_rule_map(std::string key, YAML::Node &node) {
  std::string name;
  YAML::Node operand_rules;
  metadata_t *metadata;
  opgroup_rule_t *opgroup_rule;

  metadata = new metadata_t();

  for (const auto &it : node) {
    name = it.first.as<std::string>();
    operand_rules = it.second;
  }

  metadata->insert(encoding_map[name]);

  opgroup_rule = new opgroup_rule_t(metadata);

  for (size_t i = 0; i < operand_rules.size(); i++) {
    std::vector<uint32_t> values;
    operand_rule_match_t match = OPERAND_RULE_UNKNOWN;

    if (operand_rules[i].IsScalar()) {
      opgroup_rule->add_operand_rule(values, OPERAND_RULE_ANY);
      continue;
    }

    if (operand_rules[i].IsMap()) {
      std::string operand_id;

      for (const auto &it : operand_rules[i]) {
        operand_id = it.first.as<std::string>();

        for (size_t j = 0; j < it.second.size(); j++) {
          values.push_back(it.second[j].as<uint32_t>());
        }
      }

      for (const auto &id : operand_match_yaml_ids) {
        if (operand_id.compare(id.first) == 0) {
          match = id.second;
        }
      }

      if (match == OPERAND_RULE_UNKNOWN) {
        throw std::runtime_error("Invalid operand rule type: " + operand_id);
      }
    }

    opgroup_rule->add_operand_rule(values, match);
  }

  opgroup_rule_map[key] = opgroup_rule;
}

void metadata_factory_t::init_group_map(YAML::Node &node) {
  node = node["Groups"];

  for (YAML::const_iterator it = node.begin(); it != node.end(); ++it) {
    metadata_t *metadata = new metadata_t();
    std::string key = it->first.as<std::string>();

    YAML::Node instruction_node = it->second;

    for (size_t i = 0; i < instruction_node.size(); i++) {
      YAML::Node opgroup_node = instruction_node[i];

      if (opgroup_node.IsScalar()) {
        std::string name = opgroup_node.as<std::string>();
        metadata->insert(encoding_map[name]);
      }

      if (opgroup_node.IsMap()) {
        update_rule_map(key, opgroup_node);
      }
    }

    group_map[key] = metadata;
  }
}

YAML::Node metadata_factory_t::load_yaml(const char *yml_file) {
  char path_buff[PATH_MAX];
  try {
    strcpy(path_buff, policy_dir.c_str());
    strcat(path_buff, "/");
    strcat(path_buff, yml_file);
    return YAML::LoadFile(path_buff);
  } catch (std::exception &e) {
    throw configuration_exception_t(std::string("while parsing ") + path_buff + std::string(": ") + e.what());
  }
}

metadata_factory_t::metadata_factory_t(std::string policy_dir)
  : policy_dir(policy_dir) {
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

std::string metadata_factory_t::render(meta_t meta, bool abbrev) {
  if (abbrev) {
    auto const &iter = abbrev_reverse_encoding_map.find(meta);
    if (iter == abbrev_reverse_encoding_map.end())
      return "<unknown: " + std::to_string(meta) + ">";
    else
      return iter->second;
  } else {
    auto const &iter = reverse_encoding_map.find(meta);
    if (iter == reverse_encoding_map.end())
      return "<unknown: " + std::to_string(meta) + ">";
    else
      return iter->second;
  }
}

std::string metadata_factory_t::render(metadata_t const *metadata, bool abbrev) {
  std::ostringstream os;
  bool first = true;
  for (auto &meta: *metadata) {
    if (first)
      first = false;
    else
      os << ", ";
    os << render(meta, abbrev);
  }
  return os.str();
}
