#include <string.h>

#include "policy_utils.h"

#include "meta_set_factory.h"
#if 0
void meta_set_factory_t::init_encoding_map(YAML::Node &rawEnc) {
  YAML::Node root = rawEnc["Metadata"];
  YAML::Node node;
  
  for (size_t i = 0; i < root.size(); i++) {
    node = root[i];
    encoding_map.insert(std::make_pair(node["name"].as<std::string>(), node["id"].as<meta_t>()));
  }
}

std::vector<std::string> meta_set_factory_t::split_dotted_name(const std::string &name) {
  std::istringstream ss(name);
  
  std::vector<std::string> res;
  std::string elt;
  while (std::getline(ss, elt, '.'))
    res.push_back(elt);
//  using iter = std::istream_iterator<std::string>;
//  std::vector<std::string> res{iter(ss), iter()};
  return res;
}

#include <stdio.h>

static void dump_node(YAML::Node node) {
  printf("node: %p\n", node);
  switch (node.Type()) {
    case YAML::NodeType::Null: printf("  null\n"); break;
    case YAML::NodeType::Scalar: printf("  scalar\n"); break;
    case YAML::NodeType::Sequence: printf("  sequence\n"); break;
    case YAML::NodeType::Map: printf("  map\n"); break;
    case YAML::NodeType::Undefined: printf("  undefined\n"); break;
      default: printf("  unknown\n"); break;
  }
}

bool meta_set_factory_t::lookupMetadata(std::string dotted_path, std::map<std::string,meta_t> &meta_map) {
//std::map<std::string,meta_t> meta_set_factory_t::lookupMetadata(std::string dotted_path) {
  std::map<std::string, std::map<std::string, meta_t>>::iterator path_map_iter;
  path_map_iter = path_map.find(dotted_path);
  if (path_map_iter != path_map.end()) {
    meta_map = path_map_iter->second;
    return true;
//    return path_map_iter->second;
  }

  std::vector<std::string> path = split_dotted_name(dotted_path);
  
//  std::map<std::string, meta_t> metaMap;
//  std::vector<std::string> md = metadata.find_metadata(path);
  std::vector<std::string> md;
  if (metadata.find_metadata(path, md)) {
    for (auto name: md)
      meta_map.insert(std::make_pair(name, encoding_map[name]));
    path_map[dotted_path] = meta_map;
  } else {
//    printf("metadata not found\n");
    return false;
  }
//  return metaMap;
  return true;
}

void meta_set_factory_t::init_group_map(YAML::Node &n) {
  n = n["Groups"];
  for (YAML::const_iterator it = n.begin(); it != n.end(); ++it) {
    meta_set_t *ms = new meta_set_t();
    memset(ms, 0, sizeof(*ms));
    std::string key = it->first.as<std::string>();       // <- key
//    printf("key = %s\n", key.c_str());
    YAML::Node node = it->second;

    // iterate over names
    for (size_t i=0;i<node.size();i++) {
//      std::string name = "og." + node[i].as<std::string>();
      std::string name = node[i].as<std::string>();
      ms_bit_add(ms, encoding_map[name]);
//      printf("  name = %s\n", name.c_str());
    }
    char tag_name[1024];
    meta_set_to_string(ms, tag_name, sizeof(tag_name));
//    printf("  final tag: %s\n", tag_name);
    group_map[key] = ms;
  }
}

#include <linux/limits.h>
// temporary until we have a better way to initialize the policy code in renode
YAML::Node meta_set_factory_t::load_yaml(const char *yml_file) {
  char path_buff[PATH_MAX];
  strcpy(path_buff, policy_dir.c_str());
  strcat(path_buff, "/");
  strcat(path_buff, yml_file);
  return YAML::LoadFile(path_buff);
}

meta_set_factory_t::meta_set_factory_t(meta_set_cache_t *ms_cache, std::string policy_dir)
  : ms_cache(ms_cache), policy_dir(policy_dir) {
  // load up all the requirements for initialization
  YAML::Node reqsAST = load_yaml("policy_init.yml");
  // load up the individual tag encodings
  YAML::Node metaAST = load_yaml("policy_meta.yml");
  metadata.populate(reqsAST);
//  metadata = reqsAST;
  init_encoding_map(metaAST);
  YAML::Node groupAST = load_yaml("policy_group.yml");
  init_group_map(groupAST);
}
#endif

meta_set_factory_t::meta_set_factory_t(meta_set_cache_t *ms_cache, std::string policy_dir)
  : metadata_factory_t(policy_dir), ms_cache(ms_cache) {
}

meta_set_t const *meta_set_factory_t::get_meta_set(std::string dotted_path) {
  // FIXME: not so good if we can't find the path
//  std::map<std::string, meta_t> metadata = lookupMetadata(dotted_path);
  metadata_t const *metadata = lookup_metadata(dotted_path);
  if (metadata) {
    meta_set_t ms;
    memset(&ms, 0, sizeof(ms));
//    printf("get_meta_set: %s = ", dotted_path.c_str());
    for (auto &it: *metadata) {
      ms_bit_add(&ms, it);
//      printf("0x%lx ", it.second);
    }
//    printf("\n");
    return ms_cache->canonize(ms);
  } else {
    return nullptr;
  }
}
