#include <stdio.h>

#include "validator_exception.h"
#include "soc_tag_configuration.h"

int main() {
  try {
  meta_set_cache_t mc;
  meta_set_factory_t msf("/home/eli/src/policy-engine/policy", &mc);
  soc_tag_configuration_t soc(&msf, "soc_cfg.yml");
  for (auto &e: soc) {
    printf("start = 0x%x, end = 0x%x\n", e.start, e.end);
  }
  } catch (validator::exception_t &e) {
    printf("exception: %s\n", e.what().c_str());
  }
}
