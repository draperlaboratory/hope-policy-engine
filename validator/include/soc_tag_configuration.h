#ifndef SOC_TAG_CONFIGURATION_H
#define SOC_TAG_CONFIGURATION_H

#include <string.h>
#include <list>
#include <yaml-cpp/yaml.h>

#include "platform_types.h"
#include "meta_set_factory.h"
#include "tag_converter.h"
#include "tag_utils.h"

namespace policy_engine {
class soc_tag_configuration_t {
  public:
  struct soc_element_t {
    address_t start;
    address_t end;
    bool heterogeneous;
    meta_set_t const *meta_set;
  };

  typedef std::list<soc_element_t>::iterator iterator;

  private:
  std::list<soc_element_t> elements;
  meta_set_factory_t *factory;
  void process_element(std::string element_name, const YAML::Node &n);

  public:

  soc_tag_configuration_t(meta_set_factory_t *factory, std::string file_name);
  void apply(tag_bus_t *tag_bus, tag_converter_t *converter);
  iterator begin() { return elements.begin(); }
  iterator end() { return elements.end(); }
};

} // namespace policy_engine

#endif
