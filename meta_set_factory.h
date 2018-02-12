#ifndef META_SET_FACTORY_H
#define META_SET_FACTORY_H

#include <map>
#include <unordered_map>
#include <yaml-cpp/yaml.h>

#include "meta_cache.h"
#include "policy_meta_set.h"

struct meta_tree_t {
  struct meta_node_t;
  struct meta_node_t {
    std::vector<std::string> metadata;
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
//	throw "not found";
      }
    }
//    printf("\n");
    md = n->metadata;
    return true;
//    return n->metadata;
  }
  meta_tree_t() : root(new meta_node_t()) { }
  void populate(meta_node_t *child, YAML::Node node) {
    for (YAML::const_iterator it = node.begin(); it != node.end(); ++it) {
      std::string key = it->first.as<std::string>();       // <- key
      if (key == "metadata") {
	YAML::Node mnode = it->second;
	for (size_t i = 0; i < mnode.size(); i++) {
	  std::string name = mnode[i]["name"].as<std::string>();
	  child->metadata.push_back(name);
//	  metaMap.insert(std::make_pair(name, encoding_map[name]));
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

class meta_set_factory_t {
  std::map<std::string, meta_t> encoding_map;
  std::map<std::string, std::map<std::string, meta_t>> path_map;
  std::unordered_map<std::string, meta_set_t *> group_map;

  meta_tree_t metadata;

  meta_set_cache_t *ms_cache;

  void init_encoding_map(YAML::Node &rawEnc);
  void init_group_map(YAML::Node &groupAST);
  YAML::Node load_yaml(const char *yml_file);
  
//  std::map<std::string,meta_t> lookupMetadata(std::string dotted_path);
  bool lookupMetadata(std::string dotted_path, std::map<std::string,meta_t> &md_map);

  std::string policy_dir;

  public:
  static std::vector<std::string> split_dotted_name(const std::string &name);
  meta_set_factory_t(meta_set_cache_t *ms_cache, std::string policy_dir);
  meta_set_t *get_meta_set(std::string dotted_path);
  meta_set_t *get_group_meta_set(std::string opgroup) {
    return group_map[opgroup];
  }
};

#endif
