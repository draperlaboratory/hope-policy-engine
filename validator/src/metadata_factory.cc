#include <string.h>
#include <sstream>

#include "metadata_factory.h"

void metadata_factory_t::init_encoding_map(YAML::Node &rawEnc) {
  YAML::Node root = rawEnc["Metadata"];
  YAML::Node node;
  
  for (size_t i = 0; i < root.size(); i++) {
    node = root[i];
    encoding_map[node["name"].as<std::string>()] = node["id"].as<meta_t>();
    reverse_encoding_map[node["id"].as<meta_t>()] = node["name"].as<std::string>();
//    encoding_map.insert(std::make_pair(node["name"].as<std::string>(), node["id"].as<meta_t>()));
//    encoding_map.insert(std::make_pair(node["name"].as<std::string>(), node["id"].as<meta_t>()));
  }
}

std::vector<std::string> metadata_factory_t::split_dotted_name(const std::string &name) {
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

metadata_t const *metadata_factory_t::lookup_metadata(std::string dotted_path) {
  metadata_t *metadata = nullptr;
//std::map<std::string,meta_t> meta_set_factory_t::lookupMetadata(std::string dotted_path) {
//  std::map<std::string, std::map<std::string, meta_t>>::iterator path_map_iter;
  auto const &path_map_iter = path_map.find(dotted_path);
  if (path_map_iter != path_map.end()) {
//    meta_map = path_map_iter->second;
//    return true;
    return path_map_iter->second;
  }

  std::vector<std::string> path = split_dotted_name(dotted_path);
  
//  std::map<std::string, meta_t> metaMap;
//  std::vector<std::string> md = metadata.find_metadata(path);
  std::vector<std::string> md;
  if (meta_tree.find_metadata(path, md)) {
    metadata = new metadata_t();
    for (auto name: md)
//      *metadata += encoding_map[name];
      metadata->insert(encoding_map[name]);
//      metadata->insert(std::make_pair(name, encoding_map[name]));
//    path_map[dotted_path] = meta_map;
    path_map[dotted_path] = metadata;
  }
  return metadata;
}

void metadata_factory_t::init_group_map(YAML::Node &n) {
  n = n["Groups"];
  for (YAML::const_iterator it = n.begin(); it != n.end(); ++it) {
    metadata_t *metadata = new metadata_t();
    std::string key = it->first.as<std::string>();       // <- key
//    printf("key = %s\n", key.c_str());
    YAML::Node node = it->second;

    // iterate over names
    for (size_t i = 0; i < node.size(); i++) {
//      std::string name = "og." + node[i].as<std::string>();
      std::string name = node[i].as<std::string>();
//      *metadata += encoding_map[name];
      metadata->insert(encoding_map[name]);
//      ms_bit_add(ms, encoding_map[name]);
//      printf("  name = %s\n", name.c_str());
    }
//    char tag_name[1024];
//    meta_set_to_string(ms, tag_name, sizeof(tag_name));
//    printf("  final tag: %s\n", tag_name);
//    printf("%s -> %p\n", key.c_str(), metadata);
    group_map[key] = metadata;
  }
}

#include <linux/limits.h>
// temporary until we have a better way to initialize the policy code in renode
YAML::Node metadata_factory_t::load_yaml(const char *yml_file) {
  char path_buff[PATH_MAX];
  strcpy(path_buff, policy_dir.c_str());
  strcat(path_buff, "/");
  strcat(path_buff, yml_file);
  return YAML::LoadFile(path_buff);
}

metadata_factory_t::metadata_factory_t(std::string policy_dir)
  : policy_dir(policy_dir) {
  // load up all the requirements for initialization
  YAML::Node reqsAST = load_yaml("policy_init.yml");
  // load up the individual tag encodings
  YAML::Node metaAST = load_yaml("policy_meta.yml");
  meta_tree.populate(reqsAST);
//  metadata = reqsAST;
  init_encoding_map(metaAST);
  YAML::Node groupAST = load_yaml("policy_group.yml");
  init_group_map(groupAST);
}

std::string metadata_factory_t::render(metadata_t const *metadata) {
  std::ostringstream os;
  bool first = true;
  for (auto &meta: *metadata) {
    if (first)
      first = false;
    else
      os << ", ";
    auto const &iter = reverse_encoding_map.find(meta);
    if (iter == reverse_encoding_map.end())
      os << "<unknown: " << meta << ">";
    else
      os << iter->second;
  }
  return os.str();
}
