#ifndef METADATA_FACTORY_H
#define METADATA_FACTORY_H

#include <stdint.h>
#include <map>
#include <unordered_map>
#include <yaml-cpp/yaml.h>

#include "policy_types.h" // meta_t
#include "metadata.h"

struct meta_tree_t {
  struct meta_node_t;
  struct meta_node_t {
    std::vector<std::string> meta_names;
    std::unordered_map<std::string, meta_node_t *> children;
    meta_node_t *add_node(std::string name) {
      meta_node_t *res = new meta_node_t();
      children[name] = res;
      return res;
    }
  };
  meta_node_t *root;
  bool find_metadata(std::vector<std::string> path, std::vector<std::string> &md) {
    meta_node_t *n = root;
//    printf("searching: ");
    for (auto name: path) {
//      printf("%s ", name.c_str());
      n = n->children[name];
      if (!n) {
	return false;
      }
    }
//    printf("\n");
    md = n->meta_names;
    return true;
  }
  meta_tree_t() : root(new meta_node_t()) { }
  void populate(meta_node_t *child, YAML::Node node) {
    for (YAML::const_iterator it = node.begin(); it != node.end(); ++it) {
      std::string key = it->first.as<std::string>();
      if (key == "metadata") {
	YAML::Node mnode = it->second;
	for (size_t i = 0; i < mnode.size(); i++) {
	  std::string name = mnode[i]["name"].as<std::string>();
	  child->meta_names.push_back(name);
	}
      } else {
	meta_node_t *new_child = child->add_node(key);
	populate(new_child, it->second);
      }
    }
  }
  void populate(YAML::Node n) {
    populate(root, n);
  }
};

class metadata_factory_t {
  std::unordered_map<meta_t, std::string> reverse_encoding_map; // for rendering
  std::unordered_map<std::string, meta_t> encoding_map;
  std::unordered_map<std::string, metadata_t*> path_map;
  std::unordered_map<std::string, metadata_t *> group_map;

  meta_tree_t meta_tree;

  void init_encoding_map(YAML::Node &rawEnc);
  void init_group_map(YAML::Node &groupAST);
  YAML::Node load_yaml(const char *yml_file);
  
  std::string policy_dir;

  static std::vector<std::string> split_dotted_name(const std::string &name);

  public:
  metadata_factory_t(std::string policy_dir);
  metadata_t const *lookup_metadata(std::string dotted_path);
  metadata_t const*lookup_group_metadata(std::string const &opgroup) {
    auto const &it = group_map.find(opgroup);
    if (it == group_map.end()) {
      return nullptr;
    }
    return it->second;
  }

  std::string render(metadata_t const *metadata);
};

#endif
