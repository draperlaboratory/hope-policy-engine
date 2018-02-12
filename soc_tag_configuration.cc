#include <stdio.h>

#include <yaml-cpp/yaml.h>

#include "validator_exception.h"
#include "soc_tag_configuration.h"

static void dump_node(const YAML::Node &node) {
//  printf("node: %p\n", node);
  switch (node.Type()) {
    case YAML::NodeType::Null: printf("  null\n"); break;
    case YAML::NodeType::Scalar: printf("  scalar\n"); break;
    case YAML::NodeType::Sequence: printf("  sequence\n"); break;
    case YAML::NodeType::Map: printf("  map\n"); break;
    case YAML::NodeType::Undefined: printf("  undefined\n"); break;
      default: printf("  unknown\n"); break;
  }
}

void soc_tag_configuration_t::process_element(std::string element_name, const YAML::Node &n) {
  std::string elt_path;
  soc_element_t elt;
  elt.heterogeneous = false;
//  printf("processing element %s\n", element_name.c_str());
//  dump_node(n);

  if (n["start"]) {
    elt.start = n["start"].as<unsigned>();
  } else {
    throw validator::configuration_exception_t("'start' field not present for element " + element_name);
  }
  
  if (n["end"]) {
    elt.end = n["end"].as<unsigned>();
  } else {
    throw validator::configuration_exception_t("'end' field not present for element " + element_name);
  }
  if (n["heterogeneous"]) {
    elt.heterogeneous = n["heterogeneous"].as<bool>();
  }
  elt.meta_set = factory->get_meta_set(elt_path);
  elements.push_back(elt);
//  printf("done processing element %s\n", element_name.c_str());
}

soc_tag_configuration_t::soc_tag_configuration_t(meta_set_factory_t * factory, std::string file_name)
  : factory(factory) {
  YAML::Node n = YAML::LoadFile(file_name);
  if (n["SOC"]) {
    YAML::Node soc = n["SOC"];
    for (YAML::const_iterator it = soc.begin(); it != soc.end(); ++it) {
      process_element(it->first.as<std::string>(), it->second);
    }
  } else {
    throw validator::configuration_exception_t("Expected a root SOC node");
  }
}

void soc_tag_configuration_t::apply(tag_bus_t *tag_bus) {
}
