#include <yaml-cpp/yaml.h>

#include "validator_exception.h"
#include "soc_tag_configuration.h"

void soc_tag_configuration_t::process_element(std::string element_name, YAML::Node n) {
  std::string elt_path;
  soc_element_t elt;
  elt.heterogeneous = false;
  if (n["name"]) {
    elt_path = n["name"].as<std::string>();
  } else {
    throw validator::configuration_exception_t("'name' field not present for element " + element_name);
  }
  
  if (n["start"]) {
    elt.start = n["start"].as<unsigned>();
  } else {
    throw validator::configuration_exception_t("'start' field not present for element " + element_name);
  }
  
  if (n["end"]) {
    elt.start = n["end"].as<unsigned>();
  } else {
    throw validator::configuration_exception_t("'end' field not present for element " + element_name);
  }
  if (n["heterogeneous"]) {
    elt.heterogeneous = n["heterogenous"].as<bool>();
  }
  elt.meta_set = factory->get_meta_set(elt_path);
  elements.push_back(elt);
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
